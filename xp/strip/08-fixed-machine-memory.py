#!/usr/bin/env python3
"""T01 part 2a: fixed XP machine memory model. All RAM below 4 GiB, no device
memory / hotplug / CXL / NVDIMM / SP-MEM / HV-balloon. Run from repo root on a
clean tree (not idempotent: git checkout hw/i386 include/hw/i386 first)."""
import re
def rd(p): return open(p).read()
def wr(p, s): open(p, 'w').write(s)
def sub1(s, old, new, p=''):
    assert old in s, (p, old[:70]); return s.replace(old, new, 1)
def drop_function(s, sig):
    i = s.index(sig); j = s.index('{', i); d = 0; k = j
    while True:
        c = s[k]
        if c == '{': d += 1
        elif c == '}':
            d -= 1
            if d == 0: break
        k += 1
    k += 1
    while s[k:k+1] == '\n': k += 1
    return s[:i] + s[k:]
def drop_block(s, header):
    i = s.index(header); j = s.index('{', i); d = 0; k = j
    while True:
        c = s[k]
        if c == '{': d += 1
        elif c == '}':
            d -= 1
            if d == 0: break
        k += 1
    k += 1
    if s[k] == '\n': k += 1
    return s[:s.rfind('\n', 0, i) + 1] + s[k:]

p = 'hw/i386/pc.c'; s = rd(p)
for inc in ('#include "hw/mem/nvdimm.h"\n', '#include "hw/cxl/cxl_host.h"\n'):
    s = sub1(s, inc, '', p)
s = sub1(s, '    cxl_hook_up_pxb_registers(pcms->pcibus, &pcms->cxl_devices_state,\n                              &error_fatal);\n\n    if (pcms->cxl_devices_state.is_enabled) {\n        cxl_fmws_link_targets(&error_fatal);\n    }\n\n', '', p)
for sig in ('static void pc_get_device_memory_range(', 'static uint64_t pc_get_cxl_range_start(', 'static int cxl_get_fmw_end(',
            'static uint64_t pc_get_cxl_range_end(', 'static hwaddr pc_max_used_gpa(',
            'static void pc_memory_pre_plug(', 'static void pc_memory_plug(', 'static void pc_memory_unplug_request(',
            'static void pc_memory_unplug(', 'static void pc_hv_balloon_pre_plug(', 'static void pc_hv_balloon_plug(',
            'static void pc_sp_mem_pre_plug(', 'static void pc_sp_mem_plug('):
    s = drop_function(s, sig)
s = sub1(s, '    if (pcms->sgx_epc.size != 0) {\n        return sgx_epc_above_4g_end(&pcms->sgx_epc);\n    }\n\n    return x86ms->above_4g_mem_start', '    return x86ms->above_4g_mem_start', p)

# minimal pc_memory_init
a = s.index('void pc_memory_init(PCMachineState *pcms,'); b = s.index('/*\n * The 64bit pci hole starts')
new_init = '''void pc_memory_init(PCMachineState *pcms,
                    MemoryRegion *system_memory,
                    MemoryRegion *rom_memory,
                    uint64_t pci_hole64_size)
{
    int linux_boot, i;
    MemoryRegion *option_rom_mr;
    MemoryRegion *ram_below_4g;
    FWCfgState *fw_cfg;
    MachineState *machine = MACHINE(pcms);
    PCMachineClass *pcmc = PC_MACHINE_GET_CLASS(pcms);
    X86MachineState *x86ms = X86_MACHINE(pcms);

    /*
     * XP fork: fixed machine. All guest RAM is below 4 GiB; there is no
     * hotplug, device memory, CXL, NVDIMM or 64-bit PCI hole logic.
     */
    assert(machine->ram_size == x86ms->below_4g_mem_size);
    assert(x86ms->above_4g_mem_size == 0);

    linux_boot = (machine->kernel_filename != NULL);

    ram_below_4g = g_malloc(sizeof(*ram_below_4g));
    memory_region_init_alias(ram_below_4g, NULL, "ram-below-4g", machine->ram,
                             0, x86ms->below_4g_mem_size);
    memory_region_add_subregion(system_memory, 0, ram_below_4g);
    e820_add_entry(0, x86ms->below_4g_mem_size, E820_RAM);

    /* Initialize PC system firmware */
    pc_system_firmware_init(pcms, rom_memory);

    option_rom_mr = g_malloc(sizeof(*option_rom_mr));
    memory_region_init_ram(option_rom_mr, NULL, "pc.rom", PC_ROM_SIZE,
                           &error_fatal);
    if (pcmc->pci_enabled) {
        memory_region_set_readonly(option_rom_mr, true);
    }
    memory_region_add_subregion_overlap(rom_memory, PC_ROM_MIN_VGA,
                                        option_rom_mr, 1);

    fw_cfg = fw_cfg_arch_create(machine,
                                x86ms->boot_cpus, x86ms->apic_id_limit);

    rom_set_fw(fw_cfg);

    if (linux_boot) {
        x86_load_linux(x86ms, fw_cfg, PC_FW_DATA);
    }

    for (i = 0; i < nb_option_roms; i++) {
        rom_add_option(option_rom[i].name, option_rom[i].bootindex);
    }
    x86ms->fw_cfg = fw_cfg;

    /* Init default IOAPIC address space */
    x86ms->ioapic_as = &address_space_memory;

    /* Init ACPI memory hotplug IO base address (still read by ACPI/PM code) */
    pcms->memhp_io_base = ACPI_MEMORY_HOTPLUG_BASE;
}

'''
s = s[:a] + new_init + s[b:]
a = s.index('uint64_t pc_pci_hole64_start(void)\n{'); b = s.index('\n}\n', a) + 3
s = s[:a] + '''uint64_t pc_pci_hole64_start(void)
{
    PCMachineState *pcms = PC_MACHINE(qdev_get_machine());

    return ROUND_UP(pc_above_4g_end(pcms), 1 * GiB);
}
''' + s[b:]

# dispatch callbacks
s = sub1(s, '    if (object_dynamic_cast(OBJECT(dev), TYPE_PC_DIMM)) {\n        pc_memory_pre_plug(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_SP_MEM)) {\n        pc_sp_mem_pre_plug(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {\n        x86_cpu_pre_plug',
            '    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {\n        x86_cpu_pre_plug', p)
s = sub1(s, '    } else if (object_dynamic_cast(OBJECT(dev), TYPE_HV_BALLOON)) {\n        pc_hv_balloon_pre_plug(hotplug_dev, dev, errp);\n    }\n', '    }\n', p)
s = sub1(s, '    if (object_dynamic_cast(OBJECT(dev), TYPE_PC_DIMM)) {\n        pc_memory_plug(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_SP_MEM)) {\n        pc_sp_mem_plug(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {\n        x86_cpu_plug',
            '    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {\n        x86_cpu_plug', p)
s = sub1(s, '    } else if (object_dynamic_cast(OBJECT(dev), TYPE_HV_BALLOON)) {\n        pc_hv_balloon_plug(hotplug_dev, dev, errp);\n    }\n', '    }\n', p)
s = sub1(s, '    if (object_dynamic_cast(OBJECT(dev), TYPE_PC_DIMM)) {\n        pc_memory_unplug_request(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {',
            '    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {', p)
s = sub1(s, '    if (object_dynamic_cast(OBJECT(dev), TYPE_PC_DIMM)) {\n        pc_memory_unplug(hotplug_dev, dev, errp);\n    } else if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {',
            '    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {', p)
s = sub1(s, '    if (object_dynamic_cast(OBJECT(dev), TYPE_PC_DIMM) ||\n        object_dynamic_cast(OBJECT(dev), TYPE_SP_MEM) ||\n        object_dynamic_cast(OBJECT(dev), TYPE_CPU) ||',
            '    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU) ||', p)
s = sub1(s, '        object_dynamic_cast(OBJECT(dev), TYPE_HV_BALLOON) ||\n', '', p)
s = sub1(s, '    if (pcmc->pci_enabled) {\n        cxl_machine_init(obj, &pcms->cxl_devices_state);\n    }\n', '', p)
s = sub1(s, '    mc->nvdimm_supported = true;\n', '', p)
wr(p, s)

p = 'hw/i386/pc_piix.c'; s = rd(p)
s = sub1(s, '#include "hw/mem/nvdimm.h"\n', '', p)
s = sub1(s, '\n    if (machine->nvdimms_state->is_enabled) {\n        nvdimm_init_acpi_state(machine->nvdimms_state, system_io,\n                               x86_nvdimm_acpi_dsmio,\n                               x86ms->fw_cfg, OBJECT(pcms));\n    }\n', '', p)
wr(p, s)
print("pc.c / pc_piix.c done")

# ====================== acpi-build.c ======================
def drop_between(s, a, b, new=''):
    i = s.index(a); j = s.index(b, i) + len(b); return s[:i] + new + s[j:]
p = 'hw/i386/acpi-build.c'; s = rd(p)
for inc in ('#include "hw/cxl/cxl.h"\n', '#include "hw/acpi/vmgenid.h"\n', '#include "hw/acpi/vmclock.h"\n',
            '#include "hw/mem/nvdimm.h"\n', '#include "hw/acpi/cxl.h"\n'):
    s = sub1(s, inc, '', p)
s = sub1(s, 'const struct AcpiGenericAddress x86_nvdimm_acpi_dsmio = {\n    .space_id = AML_AS_SYSTEM_IO,\n    .address = NVDIMM_ACPI_IO_BASE,\n    .bit_width = NVDIMM_ACPI_IO_LEN << 3\n};\n\n', '', p)
s = drop_function(s, 'static void build_acpi0017(')
s = sub1(s, '    uint32_t nr_mem = machine->ram_slots;\n', '', p)
s = sub1(s, '    bool cxl_present = false;\n', '', p)
s = sub1(s, '        if (machine->nvdimms_state->is_enabled) {\n            method = aml_method("_E04", 0, AML_NOTSERIALIZED);\n            aml_append(method, aml_notify(aml_name("\\\\_SB.NVDR"),\n                                          aml_int(0x80)));\n            aml_append(scope, method);\n        }\n', '', p)
s = sub1(s, '    if (pcms->memhp_io_base && nr_mem) {\n        build_memory_hotplug_aml(dsdt, nr_mem, "\\\\_SB.PCI0",\n                                 "\\\\_GPE._E03", AML_SYSTEM_IO,\n                                 pcms->memhp_io_base);\n    }\n\n', '', p)
s = sub1(s, '            if (pci_bus_is_cxl(bus)) {\n                dev = aml_device("CL%.02X", bus_num);\n            } else {\n                dev = aml_device("PC%.02X", bus_num);\n            }\n', '            dev = aml_device("PC%.02X", bus_num);\n', p)
s = drop_between(s, '            if (pci_bus_is_cxl(bus)) {\n                struct Aml *aml_pkg', '                build_cxl_osc_method(dev);\n            } else if (pci_bus_is_express(bus)) {', '            if (pci_bus_is_express(bus)) {')
s = drop_between(s, '            /* Handle the ranges for the PXB expanders */\n            if (pci_bus_is_cxl(bus)) {', '                                 base + memory_region_size(mr) - 1);\n            }\n')
s = sub1(s, '    if (cxl_present) {\n        build_acpi0017(dsdt);\n    }\n\n', '', p)
# SRAT
s = drop_between(s, '/*\n * Emit SRAT memory-affinity entries covering the device_memory region.', ' */\nstatic void build_srat_device_memory(', 'static void build_srat_device_memory(')
s = drop_function(s, 'static void build_srat_device_memory(')
s = sub1(s, '    if (machine->nvdimms_state->is_enabled) {\n        nvdimm_build_srat(table_data);\n    }\n\n', '', p)
s = drop_between(s, '    /*\n     * Entry is required for Windows to enable memory hotplug in OS', '        build_srat_device_memory(table_data, machine);\n    }\n\n')
# acpi_build table list
s = sub1(s, '    Object *vmgenid_dev, *vmclock_dev;\n', '', p)
s = drop_between(s, '    vmgenid_dev = find_vmgenid_dev();\n    if (vmgenid_dev) {\n        acpi_add_table', '                           x86ms->oem_id);\n    }\n\n')
s = drop_between(s, '        if (machine->numa_state->hmat_enabled) {', '                       x86ms->oem_id, x86ms->oem_table_id);\n        }\n')
s = drop_between(s, '    } else if (object_dynamic_cast(OBJECT(iommu), TYPE_VIRTIO_IOMMU_PCI)) {\n        PCIDevice *pdev', '                   x86ms->oem_id, x86ms->oem_table_id);\n    }\n', '    }\n')
s = drop_between(s, '    if (machine->nvdimms_state->is_enabled) {\n        nvdimm_build_acpi(', '                       x86ms->oem_id, x86ms->oem_table_id, &pcms->cxl_devices_state);\n    }\n')
s = sub1(s, '    Object *vmgenid_dev;\n', '', p)
s = drop_between(s, '    vmgenid_dev = find_vmgenid_dev();\n    if (vmgenid_dev) {\n        vmgenid_add_fw_cfg', '                           tables.vmgenid);\n    }\n\n')
wr(p, s)
print("acpi-build.c done")
