/*
 * QEMU PC System Emulator
 *
 * Copyright (c) 2003-2004 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include CONFIG_DEVICES

#include "qemu/units.h"
#include "hw/char/parallel-isa.h"
#include "hw/i386/x86.h"
#include "hw/i386/pc.h"
#include "hw/i386/apic.h"
#include "hw/pci-host/i440fx.h"
#include "hw/southbridge/piix.h"
#include "hw/display/ramfb.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_ids.h"
#include "hw/usb/usb.h"
#include "net/net.h"
#include "hw/ide/pci.h"
#include "hw/core/irq.h"
#include "system/kvm.h"
#include "hw/i386/kvm/clock.h"
#include "hw/core/sysbus.h"
#include "hw/i2c/smbus_eeprom.h"
#include "system/memory.h"
#include "hw/acpi/acpi.h"
#include "hw/vfio/types.h"
#include "qapi/error.h"
#include "qapi/qapi-type-infos-machine.h"
#include "qemu/error-report.h"
#include "migration/global_state.h"
#include "migration/misc.h"
#include "system/runstate.h"
#include "system/numa.h"
#include "hw/hyperv/vmbus-bridge.h"
#include "hw/uefi/var-service-api.h"
#include "hw/i386/acpi-build.h"
#include "target/i386/cpu.h"

static GlobalProperty pc_piix_compat_defaults[] = {
    { TYPE_RAMFB_DEVICE, "use-legacy-x86-rom", "true" },
    { TYPE_VFIO_PCI_NOHOTPLUG, "use-legacy-x86-rom", "true" },
};
static const size_t pc_piix_compat_defaults_len =
    G_N_ELEMENTS(pc_piix_compat_defaults);

static const char *pc_south_bridge_type[] = {
    [PC_SOUTH_BRIDGE_OPTION_PIIX3] = TYPE_PIIX3_DEVICE,
    [PC_SOUTH_BRIDGE_OPTION_PIIX4] = TYPE_PIIX4_PCI_DEVICE,
};

/*
 * Return the global irq number corresponding to a given device irq
 * pin. We could also use the bus number to have a more precise mapping.
 */
static int pc_pci_slot_get_pirq(PCIDevice *pci_dev, int pci_intx)
{
    int slot_addend;
    slot_addend = PCI_SLOT(pci_dev->devfn) - 1;
    return (pci_intx + slot_addend) & 3;
}

/* PC hardware initialisation */
static void pc_init1(MachineState *machine, const char *pci_type)
{
    PCMachineState *pcms = PC_MACHINE(machine);
    PCMachineClass *pcmc = PC_MACHINE_GET_CLASS(pcms);
    X86MachineState *x86ms = X86_MACHINE(machine);
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *system_io = get_system_io();
    Object *phb;
    ISABus *isa_bus;
    Object *piix4_pm = NULL;
    qemu_irq smi_irq;
    GSIState *gsi_state;
    MemoryRegion *ram_memory;
    MemoryRegion *pci_memory = NULL;
    PCIDevice *pci_dev;
    DeviceState *dev;
    size_t i;

    assert(pcmc->pci_enabled);

    /* XP fork: all guest RAM lives below 4 GiB (2 GiB default, <= 3.5 GiB). */
    ram_memory = machine->ram;
    if (machine->ram_size > 0xe0000000ULL) {
        error_report("XP fork: guest RAM (%" PRIu64 " bytes) exceeds 3.5 GiB",
                     (uint64_t)machine->ram_size);
        exit(1);
    }
    x86ms->below_4g_mem_size = machine->ram_size;

    pc_machine_init_sgx_epc(pcms);
    x86_cpus_init(x86ms, pcmc->default_cpu_version);

    if (kvm_enabled()) {
        kvmclock_create(pcmc->kvmclock_create_always);
    }

    pci_memory = g_new(MemoryRegion, 1);
    memory_region_init(pci_memory, NULL, "pci", UINT64_MAX);

    phb = OBJECT(qdev_new(TYPE_I440FX_PCI_HOST_BRIDGE));
    object_property_add_child(OBJECT(machine), "i440fx", phb);
    object_property_set_link(phb, PCI_HOST_PROP_RAM_MEM,
                             OBJECT(ram_memory), &error_fatal);
    object_property_set_link(phb, PCI_HOST_PROP_PCI_MEM,
                             OBJECT(pci_memory), &error_fatal);
    object_property_set_link(phb, PCI_HOST_PROP_SYSTEM_MEM,
                             OBJECT(system_memory), &error_fatal);
    object_property_set_link(phb, PCI_HOST_PROP_IO_MEM,
                             OBJECT(system_io), &error_fatal);
    object_property_set_uint(phb, PCI_HOST_BELOW_4G_MEM_SIZE,
                             x86ms->below_4g_mem_size, &error_fatal);
    object_property_set_str(phb, I440FX_HOST_PROP_PCI_TYPE, pci_type,
                            &error_fatal);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(phb), &error_fatal);

    pcms->pcibus = PCI_BUS(qdev_get_child_bus(DEVICE(phb), "pci.0"));
    pci_bus_map_irqs(pcms->pcibus,
                     pc_pci_slot_get_pirq);

    /* allocate ram and load rom/bios */
    pc_memory_init(pcms, system_memory, pci_memory);

    gsi_state = pc_gsi_create(&x86ms->gsi, true);

    pci_dev = pci_new_multifunction(-1,
                                    pc_south_bridge_type[pcms->south_bridge]);
    object_property_set_bool(OBJECT(pci_dev), "has-usb",
                             machine_usb(machine), &error_abort);
    object_property_set_bool(OBJECT(pci_dev), "has-acpi",
                             x86_machine_is_acpi_enabled(x86ms),
                             &error_abort);
    object_property_set_bool(OBJECT(pci_dev), "has-pic", false,
                             &error_abort);
    object_property_set_bool(OBJECT(pci_dev), "has-pit", false,
                             &error_abort);
    qdev_prop_set_uint32(DEVICE(pci_dev), "smb_io_base", 0xb100);
    object_property_set_bool(OBJECT(pci_dev), "smm-enabled",
                             x86_machine_is_smm_enabled(x86ms),
                             &error_abort);
    dev = DEVICE(pci_dev);
    for (i = 0; i < ISA_NUM_IRQS; i++) {
        qdev_connect_gpio_out_named(dev, "isa-irqs", i, x86ms->gsi[i]);
    }
    pci_realize_and_unref(pci_dev, pcms->pcibus, &error_fatal);


    isa_bus = ISA_BUS(qdev_get_child_bus(DEVICE(pci_dev), "isa.0"));
    x86ms->rtc = ISA_DEVICE(object_resolve_path_component(OBJECT(pci_dev),
                                                          "rtc"));
    piix4_pm = object_resolve_path_component(OBJECT(pci_dev), "pm");
    dev = DEVICE(object_resolve_path_component(OBJECT(pci_dev), "ide"));
    pci_ide_create_devs(PCI_DEVICE(dev));
    pcms->idebus[0] = qdev_get_child_bus(dev, "ide.0");
    pcms->idebus[1] = qdev_get_child_bus(dev, "ide.1");


    if (x86ms->pic == ON_OFF_AUTO_ON || x86ms->pic == ON_OFF_AUTO_AUTO) {
        pc_i8259_create(isa_bus, gsi_state->i8259_irq);
    }

    ioapic_init_gsi(gsi_state, phb);

    if (tcg_enabled()) {
        x86_register_ferr_irq(x86ms->gsi[13]);
    }

    pc_vga_init(isa_bus, pcms->pcibus);

    /* init basic PC hardware */
    pc_basic_device_init(pcms, isa_bus, x86ms->gsi, x86ms->rtc,
                         !MACHINE_CLASS(pcmc)->no_floppy, 0x4);

    pc_nic_init(pcmc, isa_bus, pcms->pcibus);

    if (piix4_pm) {
        smi_irq = qemu_allocate_irq(pc_acpi_smi_interrupt, first_cpu, 0);

        qdev_connect_gpio_out_named(DEVICE(piix4_pm), "smi-irq", 0, smi_irq);
        pcms->smbus = I2C_BUS(qdev_get_child_bus(DEVICE(piix4_pm), "i2c"));
        /* TODO: Populate SPD eeprom data.  */
        smbus_eeprom_init(pcms->smbus, 8, NULL, 0);

        object_property_set_link(OBJECT(machine), PC_MACHINE_ACPI_DEVICE_PROP,
                                 piix4_pm, &error_abort);
    }
}

static int pc_get_south_bridge(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->south_bridge;
}

static void pc_set_south_bridge(Object *obj, int value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->south_bridge = value;
}

static void pc_i440fx_init(MachineState *machine)
{
    pc_init1(machine, TYPE_I440FX_PCI_DEVICE);
}

#define DEFINE_I440FX_MACHINE(major, minor) \
    DEFINE_PC_VER_MACHINE(pc_i440fx, "pc-i440fx", pc_i440fx_init, false, NULL, major, minor);

#define DEFINE_I440FX_MACHINE_AS_LATEST(major, minor) \
    DEFINE_PC_VER_MACHINE(pc_i440fx, "pc-i440fx", pc_i440fx_init, true, "pc", major, minor);

static void pc_i440fx_machine_options(MachineClass *m)
{
    PCMachineClass *pcmc = PC_MACHINE_CLASS(m);
    ObjectClass *oc = OBJECT_CLASS(m);
    pcmc->default_south_bridge = PC_SOUTH_BRIDGE_OPTION_PIIX3;
    pcmc->pci_root_uid = 0;
    pcmc->default_cpu_version = 1;

    m->family = "pc_piix";
    m->desc = "Standard PC (i440FX + PIIX, 1996)";
    m->default_machine_opts = "firmware=bios-256k.bin";
    m->default_display = "std";
    m->default_nic = "e1000";
    m->no_floppy = !module_object_class_by_name(TYPE_ISA_FDC);
    m->no_parallel = !module_object_class_by_name(TYPE_ISA_PARALLEL);
    machine_class_allow_dynamic_sysbus_dev(m, TYPE_RAMFB_DEVICE);
    machine_class_allow_dynamic_sysbus_dev(m, TYPE_VMBUS_BRIDGE);
    machine_class_allow_dynamic_sysbus_dev(m, TYPE_UEFI_VARS_X64);

    object_class_property_add_enum(oc, "x-south-bridge",
                                   "PCSouthBridgeOption",
                                   &PCSouthBridgeOption_type_info,
                                   pc_get_south_bridge, pc_set_south_bridge);
    object_class_property_set_description(oc, "x-south-bridge",
                                     "Use a different south bridge than PIIX3");
    compat_props_add(m->compat_props,
                     pc_piix_compat_defaults, pc_piix_compat_defaults_len);
}

static void pc_i440fx_machine_11_2_options(MachineClass *m)
{
    pc_i440fx_machine_options(m);
}

DEFINE_I440FX_MACHINE_AS_LATEST(11, 2);
