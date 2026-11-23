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
#include "qemu/units.h"
#include "exec/target_page.h"
#include "hw/i386/pc.h"
#include "hw/char/serial-isa.h"
#include "hw/char/parallel.h"
#include "hw/hyperv/hv-balloon.h"
#include "hw/i386/fw_cfg.h"
#include "hw/i386/vmport.h"
#include "system/cpus.h"
#include "hw/ide/ide-bus.h"
#include "hw/timer/hpet.h"
#include "hw/core/loader.h"
#include "hw/rtc/mc146818rtc.h"
#include "hw/intc/i8259.h"
#include "hw/timer/i8254.h"
#include "hw/input/i8042.h"
#include "hw/audio/pcspk.h"
#include "system/system.h"
#include "system/reset.h"
#include "kvm/kvm_i386.h"
#include "qapi/qapi-type-infos-common.h"
#include "qobject/qlist.h"
#include "qemu/error-report.h"
#include "hw/acpi/acpi.h"
#include "hw/acpi/pc-hotplug.h"
#include "acpi-build.h"
#include "hw/usb/usb.h"
#include "hw/net/ne2000-isa.h"
#include "hw/virtio/virtio-md-pci.h"
#include "hw/mem/memory-device.h"
#include "hw/mem/sp-mem.h"
#include "e820_memory_layout.h"
#include "trace.h"
#include CONFIG_DEVICES


/*
 * @PC_FW_DATA:
 * Size of the chunk of memory at the top of RAM for the BIOS ACPI tables
 * and other BIOS datastructures.
 *
 * BIOS ACPI tables: 128K. Other BIOS datastructures: less than 4K
 * reported to be used at the moment, 32K should be enough for a while.
 */
#define PC_FW_DATA (0x20000 + 0x8000)

GSIState *pc_gsi_create(qemu_irq **irqs, bool pci_enabled)
{
    GSIState *s;

    s = g_new0(GSIState, 1);
    if (kvm_ioapic_in_kernel()) {
        kvm_pc_setup_irq_routing(pci_enabled);
    }
    *irqs = qemu_allocate_irqs(gsi_handler, s, IOAPIC_NUM_PINS);

    return s;
}

static void ioport80_write(void *opaque, hwaddr addr, uint64_t data,
                           unsigned size)
{
}

static uint64_t ioport80_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0xffffffffffffffffULL;
}

/* MS-DOS compatibility mode FPU exception support */
static void ioportF0_write(void *opaque, hwaddr addr, uint64_t data,
                           unsigned size)
{
    if (tcg_enabled()) {
        cpu_set_ignne();
    }
}

static uint64_t ioportF0_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0xffffffffffffffffULL;
}

/* PC cmos mappings */

#define REG_EQUIPMENT_BYTE          0x14

static void cmos_init_hd(MC146818RtcState *s, int type_ofs, int info_ofs,
                         int16_t cylinders, int8_t heads, int8_t sectors)
{
    mc146818rtc_set_cmos_data(s, type_ofs, 47);
    mc146818rtc_set_cmos_data(s, info_ofs, cylinders);
    mc146818rtc_set_cmos_data(s, info_ofs + 1, cylinders >> 8);
    mc146818rtc_set_cmos_data(s, info_ofs + 2, heads);
    mc146818rtc_set_cmos_data(s, info_ofs + 3, 0xff);
    mc146818rtc_set_cmos_data(s, info_ofs + 4, 0xff);
    mc146818rtc_set_cmos_data(s, info_ofs + 5, 0xc0 | ((heads > 8) << 3));
    mc146818rtc_set_cmos_data(s, info_ofs + 6, cylinders);
    mc146818rtc_set_cmos_data(s, info_ofs + 7, cylinders >> 8);
    mc146818rtc_set_cmos_data(s, info_ofs + 8, sectors);
}

/* convert boot_device letter to something recognizable by the bios */
static int boot_device2nibble(char boot_device)
{
    switch(boot_device) {
    case 'a':
    case 'b':
        return 0x01; /* floppy boot */
    case 'c':
        return 0x02; /* hard drive boot */
    case 'd':
        return 0x03; /* CD-ROM boot */
    case 'n':
        return 0x04; /* Network boot */
    }
    return 0;
}

static void set_boot_dev(PCMachineState *pcms, MC146818RtcState *s,
                         const char *boot_device, Error **errp)
{
#define PC_MAX_BOOT_DEVICES 3
    int nbds, bds[3] = { 0, };
    int i;

    nbds = strlen(boot_device);
    if (nbds > PC_MAX_BOOT_DEVICES) {
        error_setg(errp, "Too many boot devices for PC");
        return;
    }
    for (i = 0; i < nbds; i++) {
        bds[i] = boot_device2nibble(boot_device[i]);
        if (bds[i] == 0) {
            error_setg(errp, "Invalid boot device for PC: '%c'",
                       boot_device[i]);
            return;
        }
    }
    mc146818rtc_set_cmos_data(s, 0x3d, (bds[1] << 4) | bds[0]);
    mc146818rtc_set_cmos_data(s, 0x38, (bds[2] << 4) | !pcms->fd_bootchk);
}

static void pc_boot_set(void *opaque, const char *boot_device, Error **errp)
{
    PCMachineState *pcms = opaque;
    X86MachineState *x86ms = X86_MACHINE(pcms);

    set_boot_dev(pcms, MC146818_RTC(x86ms->rtc), boot_device, errp);
}

static void pc_cmos_init_floppy(MC146818RtcState *rtc_state, ISADevice *floppy)
{
    int val, nb;
    FloppyDriveType fd_type[2] = { FLOPPY_DRIVE_TYPE_NONE,
                                   FLOPPY_DRIVE_TYPE_NONE };

#ifdef CONFIG_FDC_ISA
    /* floppy type */
    if (floppy) {
        for (int i = 0; i < 2; i++) {
            fd_type[i] = isa_fdc_get_drive_type(floppy, i);
        }
    }
#endif

    val = (cmos_get_fd_drive_type(fd_type[0]) << 4) |
        cmos_get_fd_drive_type(fd_type[1]);
    mc146818rtc_set_cmos_data(rtc_state, 0x10, val);

    val = mc146818rtc_get_cmos_data(rtc_state, REG_EQUIPMENT_BYTE);
    nb = 0;
    if (fd_type[0] != FLOPPY_DRIVE_TYPE_NONE) {
        nb++;
    }
    if (fd_type[1] != FLOPPY_DRIVE_TYPE_NONE) {
        nb++;
    }
    switch (nb) {
    case 0:
        break;
    case 1:
        val |= 0x01; /* 1 drive, ready for boot */
        break;
    case 2:
        val |= 0x41; /* 2 drives, ready for boot */
        break;
    }
    mc146818rtc_set_cmos_data(rtc_state, REG_EQUIPMENT_BYTE, val);
}

typedef struct check_fdc_state {
    ISADevice *floppy;
    bool multiple;
} CheckFdcState;

static int check_fdc(Object *obj, void *opaque)
{
    CheckFdcState *state = opaque;
    Object *fdc;
    uint32_t iobase;
    Error *local_err = NULL;

    fdc = object_dynamic_cast(obj, TYPE_ISA_FDC);
    if (!fdc) {
        return 0;
    }

    iobase = object_property_get_uint(obj, "iobase", &local_err);
    if (local_err || iobase != 0x3f0) {
        error_free(local_err);
        return 0;
    }

    if (state->floppy) {
        state->multiple = true;
    } else {
        state->floppy = ISA_DEVICE(obj);
    }
    return 0;
}

static const char * const fdc_container_path[] = {
    "unattached", "peripheral", "peripheral-anon"
};

/*
 * Locate the FDC at IO address 0x3f0, in order to configure the CMOS registers
 * and ACPI objects.
 */
static ISADevice *pc_find_fdc0(void)
{
    int i;
    Object *container;
    CheckFdcState state = { 0 };

    for (i = 0; i < ARRAY_SIZE(fdc_container_path); i++) {
        container = machine_get_container(fdc_container_path[i]);
        object_child_foreach(container, check_fdc, &state);
    }

    if (state.multiple) {
        warn_report("multiple floppy disk controllers with "
                    "iobase=0x3f0 have been found");
        error_printf("the one being picked for CMOS setup might not reflect "
                     "your intent");
    }

    return state.floppy;
}

static void pc_cmos_init_late(PCMachineState *pcms)
{
    X86MachineState *x86ms = X86_MACHINE(pcms);
    MC146818RtcState *s = MC146818_RTC(x86ms->rtc);
    int16_t cylinders;
    int8_t heads, sectors;
    int val;
    int i, trans;

    val = 0;
    if (pcms->idebus[0] &&
        ide_get_geometry(pcms->idebus[0], 0,
                         &cylinders, &heads, &sectors) >= 0) {
        cmos_init_hd(s, 0x19, 0x1b, cylinders, heads, sectors);
        val |= 0xf0;
    }
    if (pcms->idebus[0] &&
        ide_get_geometry(pcms->idebus[0], 1,
                         &cylinders, &heads, &sectors) >= 0) {
        cmos_init_hd(s, 0x1a, 0x24, cylinders, heads, sectors);
        val |= 0x0f;
    }
    mc146818rtc_set_cmos_data(s, 0x12, val);

    val = 0;
    for (i = 0; i < 4; i++) {
        /* NOTE: ide_get_geometry() returns the physical
           geometry.  It is always such that: 1 <= sects <= 63, 1
           <= heads <= 16, 1 <= cylinders <= 16383. The BIOS
           geometry can be different if a translation is done. */
        BusState *idebus = pcms->idebus[i / 2];
        if (idebus &&
            ide_get_geometry(idebus, i % 2,
                             &cylinders, &heads, &sectors) >= 0) {
            trans = ide_get_bios_chs_trans(idebus, i % 2) - 1;
            assert((trans & ~3) == 0);
            val |= trans << (i * 2);
        }
    }
    mc146818rtc_set_cmos_data(s, 0x39, val);

    pc_cmos_init_floppy(s, pc_find_fdc0());

    /* various important CMOS locations needed by PC/Bochs bios */

    /* memory size */
    /* base memory (first MiB) */
    val = MIN(x86ms->below_4g_mem_size / KiB, 640);
    mc146818rtc_set_cmos_data(s, 0x15, val);
    mc146818rtc_set_cmos_data(s, 0x16, val >> 8);
    /* extended memory (next 64MiB) */
    if (x86ms->below_4g_mem_size > 1 * MiB) {
        val = (x86ms->below_4g_mem_size - 1 * MiB) / KiB;
    } else {
        val = 0;
    }
    if (val > 65535)
        val = 65535;
    mc146818rtc_set_cmos_data(s, 0x17, val);
    mc146818rtc_set_cmos_data(s, 0x18, val >> 8);
    mc146818rtc_set_cmos_data(s, 0x30, val);
    mc146818rtc_set_cmos_data(s, 0x31, val >> 8);
    /* memory between 16MiB and 4GiB */
    if (x86ms->below_4g_mem_size > 16 * MiB) {
        val = (x86ms->below_4g_mem_size - 16 * MiB) / (64 * KiB);
    } else {
        val = 0;
    }
    if (val > 65535)
        val = 65535;
    mc146818rtc_set_cmos_data(s, 0x34, val);
    mc146818rtc_set_cmos_data(s, 0x35, val >> 8);
    /* memory above 4GiB (always 0 in XP fork) */
    mc146818rtc_set_cmos_data(s, 0x5b, 0);
    mc146818rtc_set_cmos_data(s, 0x5c, 0);
    mc146818rtc_set_cmos_data(s, 0x5d, 0);

    val = 0;
    val |= 0x02; /* FPU is there */
    val |= 0x04; /* PS/2 mouse installed */
    mc146818rtc_set_cmos_data(s, REG_EQUIPMENT_BYTE, val);
}

static void handle_a20_line_change(void *opaque, int irq, int level)
{
    X86CPU *cpu = opaque;

    /* XXX: send to all CPUs ? */
    /* XXX: add logic to handle multiple A20 line sources */
    x86_cpu_set_a20(cpu, level);
}

#define NE2000_NB_MAX 6

static const int ne2000_io[NE2000_NB_MAX] = { 0x300, 0x320, 0x340, 0x360,
                                              0x280, 0x380 };
static const int ne2000_irq[NE2000_NB_MAX] = { 9, 10, 11, 3, 4, 5 };

static gboolean pc_init_ne2k_isa(ISABus *bus, NICInfo *nd, Error **errp)
{
    static int nb_ne2k = 0;

    if (nb_ne2k == NE2000_NB_MAX) {
        error_setg(errp,
                   "maximum number of ISA NE2000 devices exceeded");
        return false;
    }
    isa_ne2000_init(bus, ne2000_io[nb_ne2k],
                    ne2000_irq[nb_ne2k], nd);
    nb_ne2k++;
    return true;
}

void pc_acpi_smi_interrupt(void *opaque, int irq, int level)
{
    X86CPU *cpu = opaque;

    if (level) {
        cpu_interrupt(CPU(cpu), CPU_INTERRUPT_SMI);
    }
}

static
void pc_machine_done(Notifier *notifier, void *data)
{
    PCMachineState *pcms = container_of(notifier,
                                        PCMachineState, machine_done);
    X86MachineState *x86ms = X86_MACHINE(pcms);

    /* set the number of CPUs */
    x86_rtc_set_cpus_count(x86ms->rtc, x86ms->boot_cpus);

    pci_bus_add_fw_cfg_extra_pci_roots(x86ms->fw_cfg, pcms->pcibus,
                                       &error_abort);

    acpi_setup();
    if (x86ms->fw_cfg) {
        fw_cfg_build_smbios(pcms, x86ms->fw_cfg, pcms->smbios_entry_point_type);
        fw_cfg_add_e820(x86ms->fw_cfg);
        fw_cfg_build_feature_control(MACHINE(pcms), x86ms->fw_cfg);
        /* update FW_CFG_NB_CPUS to account for -device added CPUs */
        fw_cfg_modify_i16(x86ms->fw_cfg, FW_CFG_NB_CPUS, x86ms->boot_cpus);
    }

    pc_cmos_init_late(pcms);
}

/* setup pci memory address space mapping into system address space */
void pc_pci_as_mapping_init(MemoryRegion *system_memory,
                            MemoryRegion *pci_address_space)
{
    /* Set to lower priority than RAM */
    memory_region_add_subregion_overlap(system_memory, 0x0,
                                        pci_address_space, -1);
}

#define PC_ROM_MIN_VGA     0xc0000
#define PC_ROM_MIN_OPTION  0xc8000
#define PC_ROM_MAX         0xe0000
#define PC_ROM_ALIGN       0x800
#define PC_ROM_SIZE        (PC_ROM_MAX - PC_ROM_MIN_VGA)

void pc_memory_init(PCMachineState *pcms,
                    MemoryRegion *system_memory,
                    MemoryRegion *rom_memory)
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

DeviceState *pc_vga_init(ISABus *isa_bus, PCIBus *pci_bus)
{
    DeviceState *dev = NULL;

    if (pci_bus) {
        PCIDevice *pcidev = pci_vga_init(pci_bus);
        dev = pcidev ? &pcidev->qdev : NULL;
    } else if (isa_bus) {
        ISADevice *isadev = isa_vga_init(isa_bus);
        dev = isadev ? DEVICE(isadev) : NULL;
    }

    return dev;
}

static const MemoryRegionOps ioport80_io_ops = {
    .write = ioport80_write,
    .read = ioport80_read,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 1,
    },
};

static const MemoryRegionOps ioportF0_io_ops = {
    .write = ioportF0_write,
    .read = ioportF0_read,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 1,
    },
};

static void pc_superio_init(ISABus *isa_bus, bool create_fdctrl,
                            bool create_i8042, bool no_vmport, Error **errp)
{
    int i;
    DriveInfo *fd[MAX_FD];
    qemu_irq *a20_line;
    ISADevice *i8042, *port92, *vmmouse;

    serial_hds_isa_init(isa_bus, 0, MAX_ISA_SERIAL_PORTS);
    parallel_hds_isa_init(isa_bus, MAX_PARALLEL_PORTS);

    for (i = 0; i < MAX_FD; i++) {
        fd[i] = drive_get(IF_FLOPPY, 0, i);
        create_fdctrl |= !!fd[i];
    }
    if (create_fdctrl) {
#ifdef CONFIG_FDC_ISA
        ISADevice *fdc = isa_new(TYPE_ISA_FDC);
        if (fdc) {
            isa_realize_and_unref(fdc, isa_bus, &error_fatal);
            isa_fdc_init_drives(fdc, fd);
        }
#endif
    }

    if (!create_i8042) {
        if (!no_vmport) {
            error_setg(errp,
                       "vmport requires the i8042 controller to be enabled");
        }
        return;
    }

    i8042 = isa_create_simple(isa_bus, TYPE_I8042);
    if (!no_vmport) {
        isa_create_simple(isa_bus, TYPE_VMPORT);
        vmmouse = isa_try_new("vmmouse");
    } else {
        vmmouse = NULL;
    }
    if (vmmouse) {
        object_property_set_link(OBJECT(vmmouse), TYPE_I8042, OBJECT(i8042),
                                 &error_abort);
        isa_realize_and_unref(vmmouse, isa_bus, &error_fatal);
    }
    port92 = isa_create_simple(isa_bus, TYPE_PORT92);

    a20_line = qemu_allocate_irqs(handle_a20_line_change, first_cpu, 2);
    qdev_connect_gpio_out_named(DEVICE(i8042),
                                I8042_A20_LINE, 0, a20_line[0]);
    qdev_connect_gpio_out_named(DEVICE(port92),
                                PORT92_A20_LINE, 0, a20_line[1]);
    g_free(a20_line);
}

void pc_basic_device_init(struct PCMachineState *pcms,
                          ISABus *isa_bus, qemu_irq *gsi,
                          ISADevice *rtc_state,
                          bool create_fdctrl,
                          uint32_t hpet_irqs)
{
    int i;
    DeviceState *hpet = NULL;
    int pit_isa_irq = 0;
    qemu_irq pit_alt_irq = NULL;
    ISADevice *pit = NULL;
    MemoryRegion *ioport80_io = g_new(MemoryRegion, 1);
    MemoryRegion *ioportF0_io = g_new(MemoryRegion, 1);
    X86MachineState *x86ms = X86_MACHINE(pcms);

    memory_region_init_io(ioport80_io, OBJECT(pcms), &ioport80_io_ops, NULL,
                          "ioport80", 1);
    memory_region_enable_lockless_io(ioport80_io);
    memory_region_add_subregion(isa_bus->address_space_io, 0x80, ioport80_io);

    memory_region_init_io(ioportF0_io, OBJECT(pcms), &ioportF0_io_ops, NULL,
                          "ioportF0", 1);
    memory_region_add_subregion(isa_bus->address_space_io, 0xf0, ioportF0_io);

    /*
     * Check if an HPET shall be created.
     */
    if (pcms->hpet_enabled) {
        qemu_irq rtc_irq;

        hpet = qdev_try_new(TYPE_HPET);
        if (!hpet) {
            error_report("couldn't create HPET device");
            exit(1);
        }
        /*
         * For pc-piix-*, hpet's intcap is always IRQ2. For pc-q35-*,
         * use IRQ16~23, IRQ8 and IRQ2.  If the user has already set
         * the property, use whatever mask they specified.
         */
        uint8_t compat = object_property_get_uint(OBJECT(hpet),
                HPET_INTCAP, NULL);
        if (!compat) {
            qdev_prop_set_uint32(hpet, HPET_INTCAP, hpet_irqs);
        }
        sysbus_realize_and_unref(SYS_BUS_DEVICE(hpet), &error_fatal);
        sysbus_mmio_map(SYS_BUS_DEVICE(hpet), 0, HPET_BASE);

        for (i = 0; i < IOAPIC_NUM_PINS; i++) {
            sysbus_connect_irq(SYS_BUS_DEVICE(hpet), i, gsi[i]);
        }
        pit_isa_irq = -1;
        pit_alt_irq = qdev_get_gpio_in(hpet, HPET_LEGACY_PIT_INT);
        rtc_irq = qdev_get_gpio_in(hpet, HPET_LEGACY_RTC_INT);

        /* overwrite connection created by south bridge */
        qdev_connect_gpio_out(DEVICE(rtc_state), 0, rtc_irq);
    }

    object_property_set_alias(OBJECT(pcms), "rtc-time", OBJECT(rtc_state));

    qemu_register_boot_set(pc_boot_set, pcms);
    set_boot_dev(pcms, MC146818_RTC(rtc_state),
                 MACHINE(pcms)->boot_config.order, &error_fatal);

    if (x86ms->pit == ON_OFF_AUTO_AUTO || x86ms->pit == ON_OFF_AUTO_ON) {
        if (kvm_pit_in_kernel()) {
            pit = kvm_pit_init(isa_bus, 0x40);
        } else {
            pit = i8254_pit_init(isa_bus, 0x40, pit_isa_irq, pit_alt_irq);
        }
        if (hpet) {
            /* connect PIT to output control line of the HPET */
            qdev_connect_gpio_out(hpet, 0, qdev_get_gpio_in(DEVICE(pit), 0));
        }
        object_property_set_link(OBJECT(pcms->pcspk), "pit",
                                 OBJECT(pit), &error_fatal);
        isa_realize_and_unref(pcms->pcspk, isa_bus, &error_fatal);
    }

    if (pcms->vmport == ON_OFF_AUTO_AUTO) {
        pcms->vmport = !pcms->i8042_enabled
            ? ON_OFF_AUTO_OFF : ON_OFF_AUTO_ON;
    }

    /* Super I/O */
    pc_superio_init(isa_bus, create_fdctrl, pcms->i8042_enabled,
                    pcms->vmport != ON_OFF_AUTO_ON, &error_fatal);

    pcms->machine_done.notify = pc_machine_done;
    qemu_add_machine_init_done_notifier(&pcms->machine_done);
}

void pc_nic_init(PCMachineClass *pcmc, ISABus *isa_bus, PCIBus *pci_bus)
{
    MachineClass *mc = MACHINE_CLASS(pcmc);
    bool default_is_ne2k = g_str_equal(mc->default_nic, TYPE_ISA_NE2000);
    NICInfo *nd;

    while ((nd = qemu_find_nic_info(TYPE_ISA_NE2000, default_is_ne2k, NULL))) {
        pc_init_ne2k_isa(isa_bus, nd, &error_fatal);
    }

    /* Anything remaining should be a PCI NIC */
    if (pci_bus) {
        pci_init_nic_devices(pci_bus, mc->default_nic);
    }
}

void pc_i8259_create(ISABus *isa_bus, qemu_irq *i8259_irqs)
{
    qemu_irq *i8259;

    if (kvm_pic_in_kernel()) {
        i8259 = kvm_i8259_init(isa_bus);
    } else {
        i8259 = i8259_init(isa_bus, x86_allocate_cpu_irq());
    }

    for (size_t i = 0; i < ISA_NUM_IRQS; i++) {
        i8259_irqs[i] = i8259[i];
    }

    g_free(i8259);
}

static void pc_machine_device_pre_plug_cb(const HotplugHandler *hotplug_dev,
                                          DeviceState *dev, Error **errp)
{
    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {
        x86_cpu_pre_plug(hotplug_dev, dev, errp);
    } else if (object_dynamic_cast(OBJECT(dev), TYPE_VIRTIO_MD_PCI)) {
        virtio_md_pci_pre_plug(VIRTIO_MD_PCI(dev), MACHINE(hotplug_dev), errp);
    }
}

static void pc_machine_device_plug_cb(const HotplugHandler *hotplug_dev,
                                      DeviceState *dev, Error **errp)
{
    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {
        x86_cpu_plug(hotplug_dev, dev, errp);
    } else if (object_dynamic_cast(OBJECT(dev), TYPE_VIRTIO_MD_PCI)) {
        virtio_md_pci_plug(VIRTIO_MD_PCI(dev), MACHINE(hotplug_dev), errp);
    }
}

static void pc_machine_device_unplug_request_cb(const HotplugHandler *hotplug_dev,
                                                DeviceState *dev, Error **errp)
{
    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {
        x86_cpu_unplug_request_cb(hotplug_dev, dev, errp);
    } else if (object_dynamic_cast(OBJECT(dev), TYPE_VIRTIO_MD_PCI)) {
        virtio_md_pci_unplug_request(VIRTIO_MD_PCI(dev), MACHINE(hotplug_dev),
                                     errp);
    } else {
        error_setg(errp, "acpi: device unplug request for not supported device"
                   " type: %s", object_get_typename(OBJECT(dev)));
    }
}

static void pc_machine_device_unplug_cb(const HotplugHandler *hotplug_dev,
                                        DeviceState *dev, Error **errp)
{
    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU)) {
        x86_cpu_unplug_cb(hotplug_dev, dev, errp);
    } else if (object_dynamic_cast(OBJECT(dev), TYPE_VIRTIO_MD_PCI)) {
        virtio_md_pci_unplug(VIRTIO_MD_PCI(dev), MACHINE(hotplug_dev), errp);
    } else {
        error_setg(errp, "acpi: device unplug for not supported device"
                   " type: %s", object_get_typename(OBJECT(dev)));
    }
}

static const HotplugHandler *pc_get_hotplug_handler(MachineState *machine,
                                                    DeviceState *dev)
{
    if (object_dynamic_cast(OBJECT(dev), TYPE_CPU) ||
        object_dynamic_cast(OBJECT(dev), TYPE_VIRTIO_MD_PCI)) {
        return HOTPLUG_HANDLER(machine);
    }

    return NULL;
}

static int pc_machine_get_vmport(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    return pcms->vmport;
}

static void pc_machine_set_vmport(Object *obj, int value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    pcms->vmport = value;
}

static bool pc_machine_get_fd_bootchk(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->fd_bootchk;
}

static void pc_machine_set_fd_bootchk(Object *obj, bool value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->fd_bootchk = value;
}

static bool pc_machine_get_smbus(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->smbus_enabled;
}

static void pc_machine_set_smbus(Object *obj, bool value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->smbus_enabled = value;
}

static bool pc_machine_get_sata(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->sata_enabled;
}

static void pc_machine_set_sata(Object *obj, bool value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->sata_enabled = value;
}

static bool pc_machine_get_hpet(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->hpet_enabled;
}

static void pc_machine_set_hpet(Object *obj, bool value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->hpet_enabled = value;
}

static bool pc_machine_get_i8042(Object *obj, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    return pcms->i8042_enabled;
}

static void pc_machine_set_i8042(Object *obj, bool value, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    pcms->i8042_enabled = value;
}

static void pc_machine_get_smbios_ep(Object *obj, Visitor *v, const char *name,
                                     void *opaque, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    SmbiosEntryPointType smbios_entry_point_type = pcms->smbios_entry_point_type;

    visit_type_SmbiosEntryPointType(v, name, &smbios_entry_point_type, errp);
}

static void pc_machine_set_smbios_ep(Object *obj, Visitor *v, const char *name,
                                     void *opaque, Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    visit_type_SmbiosEntryPointType(v, name, &pcms->smbios_entry_point_type, errp);
}

static void pc_machine_get_max_fw_size(Object *obj, Visitor *v,
                                       const char *name, void *opaque,
                                       Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    uint64_t value = pcms->max_fw_size;

    visit_type_size(v, name, &value, errp);
}

static void pc_machine_set_max_fw_size(Object *obj, Visitor *v,
                                       const char *name, void *opaque,
                                       Error **errp)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    uint64_t value;

    if (!visit_type_size(v, name, &value, errp)) {
        return;
    }

    /*
     * We don't have a theoretically justifiable exact lower bound on the base
     * address of any flash mapping. In practice, the IO-APIC MMIO range is
     * [0xFEE00000..0xFEE01000] -- see IO_APIC_DEFAULT_ADDRESS --, leaving free
     * only 18MiB-4KiB below 4GiB. For now, restrict the cumulative mapping to
     * 16MiB in size.
     */
    if (value > 16 * MiB) {
        error_setg(errp,
                   "User specified max allowed firmware size %" PRIu64 " is "
                   "greater than 16MiB. If combined firmware size exceeds "
                   "16MiB the system may not boot, or experience intermittent"
                   "stability issues.",
                   value);
        return;
    }

    pcms->max_fw_size = value;
}


static void pc_machine_initfn(Object *obj)
{
    PCMachineState *pcms = PC_MACHINE(obj);
    PCMachineClass *pcmc = PC_MACHINE_GET_CLASS(pcms);

#ifdef CONFIG_VMPORT
    pcms->vmport = ON_OFF_AUTO_AUTO;
#else
    pcms->vmport = ON_OFF_AUTO_OFF;
#endif /* CONFIG_VMPORT */
    pcms->smbios_entry_point_type = pcmc->default_smbios_ep_type;
    pcms->south_bridge = pcmc->default_south_bridge;

    /* acpi build is enabled by default if machine supports it */
    pcms->acpi_build_enabled = pcmc->has_acpi_build;
    pcms->smbus_enabled = true;
    pcms->sata_enabled = true;
    pcms->i8042_enabled = true;
    pcms->max_fw_size = 8 * MiB;
#if defined(CONFIG_HPET)
    pcms->hpet_enabled = true;
#endif
    pcms->fd_bootchk = true;

    pc_system_flash_create(pcms);
    pcms->pcspk = isa_new(TYPE_PC_SPEAKER);
    object_property_set_alias(OBJECT(pcms), "pcspk-audiodev",
                              OBJECT(pcms->pcspk));
}

static void pc_machine_finalize(Object *obj)
{
    PCMachineState *pcms = PC_MACHINE(obj);

    if (pcms->pcspk && !qdev_is_realized(DEVICE(pcms->pcspk))) {
        object_unref(OBJECT(pcms->pcspk));
    }
}

static void pc_machine_reset(MachineState *machine, ResetType type)
{
    CPUState *cs;
    X86CPU *cpu;

    qemu_devices_reset(type);

    /* Reset APIC after devices have been reset to cancel
     * any changes that qemu_devices_reset() might have done.
     */
    CPU_FOREACH(cs) {
        cpu = X86_CPU(cs);

        x86_cpu_after_reset(cpu);
    }
}

static void pc_machine_wakeup(MachineState *machine)
{
    cpu_synchronize_all_states();
    pc_machine_reset(machine, RESET_TYPE_WAKEUP);
    cpu_synchronize_all_post_reset();
}

static void pc_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    X86MachineClass *x86mc = X86_MACHINE_CLASS(oc);
    PCMachineClass *pcmc = PC_MACHINE_CLASS(oc);
    HotplugHandlerClass *hc = HOTPLUG_HANDLER_CLASS(oc);

    pcmc->pci_enabled = true;
    pcmc->has_acpi_build = true;
    pcmc->smbios_defaults = true;
    pcmc->isa_bios_alias = true;
    pcmc->kvmclock_create_always = true;
    x86mc->apic_xrupt_override = true;
    assert(!mc->get_hotplug_handler);
    mc->get_hotplug_handler = pc_get_hotplug_handler;
    mc->auto_enable_numa_with_memhp = true;
    mc->auto_enable_numa_with_memdev = true;
    mc->has_hotpluggable_cpus = true;
    mc->default_boot_order = "cad";
    mc->block_default_type = IF_IDE;
    mc->reset = pc_machine_reset;
    mc->wakeup = pc_machine_wakeup;
    hc->pre_plug = pc_machine_device_pre_plug_cb;
    hc->plug = pc_machine_device_plug_cb;
    hc->unplug_request = pc_machine_device_unplug_request_cb;
    hc->unplug = pc_machine_device_unplug_cb;
    mc->default_cpu_type = TARGET_DEFAULT_CPU_TYPE;
    mc->smp_props.dies_supported = true;
    mc->smp_props.modules_supported = true;
    mc->smp_props.cache_supported[CACHE_LEVEL_AND_TYPE_L1D] = true;
    mc->smp_props.cache_supported[CACHE_LEVEL_AND_TYPE_L1I] = true;
    mc->smp_props.cache_supported[CACHE_LEVEL_AND_TYPE_L2] = true;
    mc->smp_props.cache_supported[CACHE_LEVEL_AND_TYPE_L3] = true;
    mc->default_ram_id = "pc.ram";
    /* XP fork: fixed guest memory (2 GiB, entirely below 4 GiB) and one vCPU. */
    mc->default_ram_size = 2 * GiB;
    mc->default_cpus = 1;
    mc->max_cpus = 1;
    pcmc->default_smbios_ep_type = SMBIOS_ENTRY_POINT_TYPE_AUTO;

    object_class_property_add_enum(oc, PC_MACHINE_VMPORT, "OnOffAuto",
                                   &OnOffAuto_type_info,
                                   pc_machine_get_vmport,
                                   pc_machine_set_vmport);
    object_class_property_set_description(oc, PC_MACHINE_VMPORT,
        "Enable vmport (pc & q35)");

    object_class_property_add_bool(oc, PC_MACHINE_SMBUS,
        pc_machine_get_smbus, pc_machine_set_smbus);
    object_class_property_set_description(oc, PC_MACHINE_SMBUS,
        "Enable/disable system management bus");

    object_class_property_add_bool(oc, PC_MACHINE_SATA,
        pc_machine_get_sata, pc_machine_set_sata);
    object_class_property_set_description(oc, PC_MACHINE_SATA,
        "Enable/disable Serial ATA bus");

    object_class_property_add_bool(oc, "hpet",
        pc_machine_get_hpet, pc_machine_set_hpet);
    object_class_property_set_description(oc, "hpet",
        "Enable/disable high precision event timer emulation");

    object_class_property_add_bool(oc, PC_MACHINE_I8042,
        pc_machine_get_i8042, pc_machine_set_i8042);
    object_class_property_set_description(oc, PC_MACHINE_I8042,
        "Enable/disable Intel 8042 PS/2 controller emulation");

    object_class_property_add(oc, PC_MACHINE_MAX_FW_SIZE, "size",
                              &size_type_info,
                              pc_machine_get_max_fw_size,
                              pc_machine_set_max_fw_size,
                              NULL, NULL);
    object_class_property_set_description(oc, PC_MACHINE_MAX_FW_SIZE,
        "Maximum combined firmware size");

    object_class_property_add(oc, PC_MACHINE_SMBIOS_EP, "str",
                              &str_type_info,
                              pc_machine_get_smbios_ep,
                              pc_machine_set_smbios_ep,
                              NULL, NULL);
    object_class_property_set_description(oc, PC_MACHINE_SMBIOS_EP,
        "SMBIOS Entry Point type [32, 64]");

    object_class_property_add_bool(oc, "fd-bootchk",
        pc_machine_get_fd_bootchk,
        pc_machine_set_fd_bootchk);
    object_class_property_set_description(oc, "fd-bootchk",
        "Enable/disable boot signature checking for floppy disks in BIOS");

#if defined(CONFIG_IGVM)
    object_class_property_add_link(oc, "igvm-cfg",
                                   TYPE_IGVM_CFG,
                                   offsetof(X86MachineState, igvm),
                                   object_property_allow_set_link,
                                   OBJ_PROP_LINK_STRONG);
    object_class_property_set_description(oc, "igvm-cfg",
                                          "Set IGVM configuration");
#endif

    object_class_property_add_alias(oc, "pcspk-audiodev",
                                    offsetof(PCMachineState, alias_pcspk),
                                    TYPE_PC_SPEAKER,
                                    "audiodev");
    object_class_property_add_alias(oc, "rtc-time",
                                    offsetof(PCMachineState, alias_rtc_time),
                                    TYPE_MC146818_RTC,
                                    "date");
    object_class_property_add_link(oc, PC_MACHINE_ACPI_DEVICE_PROP,
                                   TYPE_HOTPLUG_HANDLER,
                                   offsetof(X86MachineState, acpi_dev),
                                   object_property_allow_set_link,
                                   OBJ_PROP_LINK_STRONG);
    object_class_property_add_alias(oc, "pflash0",
                                    offsetof(PCMachineState, alias_pflash0),
                                    TYPE_PFLASH_CFI01,
                                    "drive");
    object_class_property_add_alias(oc, "pflash1",
                                    offsetof(PCMachineState, alias_pflash1),
                                    TYPE_PFLASH_CFI01,
                                    "drive");
}

static const TypeInfo pc_machine_info = {
    .name = TYPE_PC_MACHINE,
    .parent = TYPE_X86_MACHINE,
    .abstract = true,
    .instance_size = sizeof(PCMachineState),
    .instance_init = pc_machine_initfn,
    .instance_finalize = pc_machine_finalize,
    .class_size = sizeof(PCMachineClass),
    .class_init = pc_machine_class_init,
    .interfaces = (const InterfaceInfo[]) {
         { TYPE_HOTPLUG_HANDLER },
         { }
    },
};

static void pc_machine_register_types(void)
{
    type_register_static(&pc_machine_info);
}

type_init(pc_machine_register_types)
