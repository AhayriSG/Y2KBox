#!/usr/bin/env python3
"""Remove Xen from the shared code. Run from the repo root.
Each edit asserts that it matched so a changed tree fails loudly."""
import re, sys, subprocess

def rd(p): return open(p).read()
def wr(p, s): open(p, 'w').write(s)

def sub1(s, old, new, p=''):
    assert old in s, "no match in %s: %r" % (p, old[:70])
    return s.replace(old, new, 1)

def drop_block(s, header, p=''):
    """Delete an 'if (...) {' block starting at the exact header text,
    including its balanced closing brace and trailing newline."""
    i = s.index(header)
    j = s.index('{', i)
    depth, k = 0, j
    while True:
        c = s[k]
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0: break
        k += 1
    k += 1
    if s[k] == '\n': k += 1
    # also swallow one blank line that followed, if the block was followed by one
    line_start = s.rfind('\n', 0, i) + 1
    return s[:line_start] + s[k:]

def drop_lines(s, text, p=''):
    assert text in s, "no match in %s: %r" % (p, text[:70])
    return s.replace(text, '', 1)

# ---------------------------------------------------------------- physmem.c
p = 'system/physmem.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_lines(s, '#include "system/xen-mapcache.h"\n', p)
s = drop_block(s, '    if (xen_map_cache_enabled() &&\n        memory_access_is_direct(mr, is_write, attrs)) {', p)
s = sub1(s, 'if (!mask && !xen_enabled()) {', 'if (!mask) {', p)
s = drop_block(s, '    if (xen_enabled()) {\n        xen_hvm_modified_memory(start, length);', p)
s = drop_block(s, '        if (xen_enabled()) {\n            xen_hvm_modified_memory(start, pages << TARGET_PAGE_BITS);', p)
# RAM allocation: keep only the non-Xen branch
a = s.index('        if (xen_enabled()) {\n            xen_ram_alloc(')
b = s.index('        } else {\n            new_block->host = qemu_anon_ram_alloc(', a)
c = s.index('\n        }\n    }\n', b)       # end of the else body
body = s[b + len('        } else {\n'):c + 1]
body = '\n'.join(l[4:] if l.startswith('    ') else l for l in body.split('\n'))
s = s[:a] + body + s[c + len('\n        }\n'):]
s = drop_block(s, '    if (xen_enabled()) {\n        error_setg(errp, "-mem-path not supported with Xen");', p)
s = drop_lines(s, '    } else if (xen_map_cache_enabled()) {\n        xen_invalidate_map_cache_entry(block->host);\n', p)
s = drop_lines(s, '            } else if (xen_enabled()) {\n                abort();\n', p)
s = drop_block(s, '    if (xen_map_cache_enabled() && block->host == NULL) {', p)
s = drop_block(s, '    if (xen_map_cache_enabled()) {\n        ram_addr_t ram_addr;', p)
s = drop_block(s, '        if (xen_map_cache_enabled()) {\n            xen_invalidate_map_cache_entry(buffer);', p)
s = drop_block(s, '    if (xen_map_cache_enabled()) {\n        xen_invalidate_map_cache_entry(cache->ptr);', p)
wr(p, s)

# ---------------------------------------------------------------- globals.c
p = 'system/globals.c'; s = rd(p)
s = drop_lines(s, '#include "hw/xen/xen.h"\n', p)
for l in ('uint32_t xen_domid;\n', 'enum xen_mode xen_mode = XEN_DISABLED;\n', 'bool xen_domid_restrict;\n',
          'bool xen_is_stubdomain;\n', 'struct evtchn_backend_ops *xen_evtchn_ops;\n',
          'struct gnttab_backend_ops *xen_gnttab_ops;\n', 'struct foreignmem_backend_ops *xen_foreignmem_ops;\n',
          'struct xenstore_backend_ops *xen_xenstore_ops;\n'):
    s = drop_lines(s, l, p)
wr(p, s)
print("physmem.c, globals.c done")


def unwrap_else(s, head, p=''):
    """head = exact text of 'if (...) { ... } else {\n'. Replace the whole
    if/else by the else-body, dedented one level."""
    i = s.index(head)
    body_start = i + len(head)
    depth, k = 1, body_start
    while depth:
        c = s[k]
        if c == '{': depth += 1
        elif c == '}': depth -= 1
        k += 1
    body = s[body_start:k - 1]            # text before the closing brace of else
    body = '\n'.join(l[4:] if l.startswith('    ') else l for l in body.split('\n'))
    k2 = k + 1 if s[k] == '\n' else k
    line_start = s.rfind('\n', 0, i) + 1
    return s[:line_start] + body.lstrip('\n').rstrip(' ') + s[k2:]

def drop_between(s, start, end, p=''):
    i = s.index(start); j = s.index(end, i) + len(end)
    return s[:i] + s[j:]

# ---------------------------------------------------------------- PCI core
p = 'hw/pci/pci.c'; s = rd(p)
s = drop_lines(s, '#include "hw/xen/xen.h"\n', p)
s = drop_lines(s, '#include "hw/i386/kvm/xen_evtchn.h"\n', p)
s = drop_between(s, '    /*\n     * Xen uses the high bits', '        return;\n    }\n', p)
s = drop_block(s, '    if (xen_mode == XEN_EMULATE) {\n        xen_evtchn_remove_pci_device(pci_dev);', p)
wr(p, s)

p = 'hw/pci/msix.c'; s = rd(p)
for l in ('#include "hw/xen/xen.h"\n', '#include "system/xen.h"\n', '#include "hw/i386/kvm/xen_evtchn.h"\n'):
    s = drop_lines(s, l, p)
s = drop_between(s, '    /* MSIs on Xen can be remapped into pirqs.', '        return false;\n    }\n', p)
s = drop_block(s, '    if (xen_mode == XEN_EMULATE) {\n        MSIMessage msg = msix_prepare_message(dev, vector);', p)
wr(p, s)

p = 'hw/pci/msi.c'; s = rd(p)
for l in ('#include "hw/xen/xen.h"\n', '#include "system/xen.h"\n', '#include "hw/i386/kvm/xen_evtchn.h"\n'):
    s = drop_lines(s, l, p)
s = drop_block(s, '    if (xen_enabled() && xen_is_pirq_msi(data)) {', p)
s = drop_block(s, '    if (xen_mode == XEN_EMULATE) {\n        for (vector = 0; vector < msi_nr_vectors(flags); vector++) {', p)
wr(p, s)

p = 'hw/tpm/tpm_crb.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = sub1(s, '    if (xen_enabled()) {\n        tpm_crb_reset(dev);\n    } else {\n        qemu_register_reset(tpm_crb_reset, dev);\n    }\n',
            '    qemu_register_reset(tpm_crb_reset, dev);\n', p)
wr(p, s)

# ---------------------------------------------------------------- x86 machine code
p = 'hw/i386/x86-common.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_lines(s, '#ifdef CONFIG_XEN_EMU\n#include "hw/xen/xen.h"\n#include "hw/i386/kvm/xen_evtchn.h"\n#endif\n', p)
s = drop_between(s, '#ifdef CONFIG_XEN_EMU\n    /*\n     * Xen delivers the GSI', '    }\n#endif\n', p)
wr(p, s)

p = 'hw/i386/pc.c'; s = rd(p)
for l in ('#include "system/xen.h"\n', '#include "hw/xen/xen.h"\n', '#include "hw/i386/kvm/xen_overlay.h"\n',
          '#include "hw/i386/kvm/xen_evtchn.h"\n', '#include "hw/i386/kvm/xen_gnttab.h"\n',
          '#include "hw/i386/kvm/xen_xenstore.h"\n',
          '#ifdef CONFIG_XEN_EMU\n#include "hw/xen/xen-legacy-backend.h"\n#include "hw/xen/xen-bus.h"\n#endif\n'):
    s = drop_lines(s, l, p)
a = s.index('void xen_load_linux(PCMachineState *pcms)\n{')
b = s.index('\n}\n', a) + 3
s = s[:a] + s[b:].lstrip('\n')
s = drop_between(s, '#ifdef CONFIG_XEN_EMU\n    if (xen_mode == XEN_EMULATE) {\n        xen_overlay_create();', '        xen_bus_init();\n    }\n#endif\n\n', p)
s = sub1(s, '    if (!xen_enabled() &&\n        (x86ms->pit == ON_OFF_AUTO_AUTO || x86ms->pit == ON_OFF_AUTO_ON)) {',
            '    if (x86ms->pit == ON_OFF_AUTO_AUTO || x86ms->pit == ON_OFF_AUTO_ON) {', p)
s = sub1(s, 'pcms->vmport = (xen_enabled() || !pcms->i8042_enabled)', 'pcms->vmport = !pcms->i8042_enabled', p)
s = sub1(s, '    } else if (xen_enabled()) {\n        i8259 = xen_i8259_init();\n', '', p)
wr(p, s)
p = 'include/hw/i386/pc.h'; s = rd(p)
s = drop_lines(s, 'void xen_load_linux(PCMachineState *pcms);\n', p)
wr(p, s)

p = 'hw/i386/pc_piix.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_lines(s, '#ifdef CONFIG_XEN\n#include <xen/hvm/hvm_info_table.h>\n#include "hw/xen/xen_pt.h"\n#include "hw/xen/xen_igd.h"\n#endif\n', p)
s = drop_lines(s, '#include "hw/xen/xen-x86.h"\n#include "hw/xen/xen.h"\n', p)
s = drop_lines(s, '#define XEN_IOAPIC_NUM_PIRQS 128ULL\n\n', p)
a = s.index('static void piix_intx_routing_notifier_xen(PCIDevice *dev)'); b = s.index('\n}\n', a) + 3
s = s[:a] + s[b:].lstrip('\n')
s = drop_lines(s, '     *  - Note that Xen has its own ram setup code in xen_ram_init(),\n     *    called via xen_hvm_init_pc().\n     *\n', p)
s = unwrap_else(s, '    if (xen_enabled()) {\n        xen_hvm_init_pc(pcms, &ram_memory);\n    } else {\n', p)
s = sub1(s, '                     xen_enabled() ? xen_pci_slot_get_pirq\n                                   : pc_pci_slot_get_pirq);', '                     pc_pci_slot_get_pirq);', p)
s = unwrap_else(s, '    if (!xen_enabled()) {\n        pc_memory_init(pcms, system_memory, pci_memory, hole64_size);\n    } else {\n', p) if False else s
a = s.index('    if (!xen_enabled()) {\n        pc_memory_init(pcms, system_memory, pci_memory, hole64_size);\n    } else {')
b = s.index('    gsi_state = pc_gsi_create', a)
s = s[:a] + '    pc_memory_init(pcms, system_memory, pci_memory, hole64_size);\n\n' + s[b:]
s = drop_block(s, '    if (xen_enabled()) {\n        pci_device_set_intx_routing_notifier(', p)
s = drop_between(s, '#ifdef CONFIG_XEN\nstatic void pc_xen_hvm_init(MachineState *machine)', '    pci_create_simple(pcms->pcibus, -1, "xen-platform");\n}\n#endif\n\n', p)
s = drop_between(s, '#ifdef CONFIG_XEN\nstatic void xenfv_machine_4_2_options', 'xenfv_machine_4_2_options);\n#endif\n', p)
wr(p, s)
print("pci, tpm, x86 machine code done")

# ============================================================ batch 3
def drop_function(s, sig, p=''):
    i = s.index(sig)
    j = s.index('{', i)
    depth, k = 0, j
    while True:
        c = s[k]
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0: break
        k += 1
    k += 1
    while s[k:k+1] == '\n': k += 1
    return s[:i] + s[k:]

def drop_qapi(s, name, p=''):
    start = s.index('##\n# @%s:' % name)
    end_doc = s.index('\n##\n', start + 3) + 4
    d0 = s.index('{', end_doc)
    depth, k = 0, d0
    while True:
        c = s[k]
        if c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0: break
        k += 1
    k += 1
    while s[k:k+1] == '\n': k += 1
    return s[:start] + s[k:]

# ---- target/i386
p = 'target/i386/cpu.h'; s = rd(p)
s = drop_lines(s, '#define XEN_NR_VIRQS 24\n\n', p)
s = drop_between(s, '#if defined(CONFIG_KVM)\n    struct kvm_nested_state *nested_state;', '    QemuMutex xen_timers_lock;\n#endif\n', p)
s = drop_lines(s, '    bool xen_vapic;\n', p)
wr(p, s)
p = 'target/i386/cpu.c'; s = rd(p)
s = drop_lines(s, '    DEFINE_PROP_BOOL("xen-vapic", X86CPU, xen_vapic, false),\n', p)
wr(p, s)
p = 'target/i386/machine.c'; s = rd(p)
s = drop_lines(s, '#include "hw/xen/xen.h"\n', p)
s = drop_lines(s, '#include "system/kvm_xen.h"\n', p)
s = drop_between(s, 'static bool xen_vcpu_needed(void *opaque)', '        VMSTATE_END_OF_LIST()\n    }\n};\n', p)
s = drop_lines(s, '        &vmstate_xen_vcpu,\n', p)
wr(p, s)
p = 'target/i386/cpu-apic.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
a = s.index('APICCommonClass *apic_get_class(Error **errp)\n{'); b = s.index('\n}\n', a) + 3
s = s[:a] + 'APICCommonClass *apic_get_class(Error **errp)\n{\n    return APIC_COMMON_CLASS(object_class_by_name("apic"));\n}\n' + s[b:]
wr(p, s)

# ---- misc devices / machine core
p = 'hw/acpi/piix4.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_block(s, '    if (xen_enabled()) {\n        s->acpi_pci_hotplug.use_acpi_hotplug_bridge = false;', p)
wr(p, s)
p = 'hw/core/machine.c'; s = rd(p); s = drop_lines(s, '#include "system/xen.h"\n', p); wr(p, s)
p = 'hw/display/vga.c'; s = rd(p)
s = drop_lines(s, '#include "hw/xen/xen.h"\n', p); s = drop_lines(s, '    xen_register_framebuffer(&s->vram);\n', p); wr(p, s)

# ---- vl.c, system.h, module.h, hmp.h
p = 'system/vl.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p); s = drop_lines(s, '#include "hw/xen/xen.h"\n', p)
s = drop_lines(s, '    { .driver = "xen-console",          .flag = &default_serial    },\n', p)
s = drop_lines(s, '#ifdef CONFIG_XEN_BACKEND\n    [VGA_XENFB] = {\n        .opt_name = "xenfb",\n        .name = "Xen paravirtualized framebuffer",\n    },\n#endif\n', p)
s = drop_between(s, '    value = qdict_get_try_str(qdict, "igd-passthru");', '        qdict_del(qdict, "igd-passthru");\n    }\n\n', p)
s = drop_between(s, '            case QEMU_OPTION_xen_domid:', '                xen_domid_restrict = true;\n                break;\n', p)
wr(p, s)
p = 'include/system/system.h'; s = rd(p); s = sub1(s, 'VGA_VMWARE, VGA_XENFB, VGA_QXL', 'VGA_VMWARE, VGA_QXL', p); wr(p, s)
p = 'include/qemu/module.h'; s = rd(p)
s = drop_lines(s, '    MODULE_INIT_XEN_BACKEND,\n', p)
s = drop_lines(s, '#define xen_backend_init(function) module_init(function, \\\n                                               MODULE_INIT_XEN_BACKEND)\n', p)
wr(p, s)
p = 'include/monitor/hmp.h'; s = rd(p)
s = drop_lines(s, 'void hmp_xen_event_inject(MonitorHMP *hmp, const QDict *qdict);\nvoid hmp_xen_event_list(MonitorHMP *hmp, const QDict *qdict);\n', p)
wr(p, s)

# ---- CLI + HMP docs
p = 'hmp-commands.hx'; s = rd(p)
s = drop_between(s, '    {\n        .name       = "xen-event-inject",', 'List event channels in the guest\nERST\n', p)
wr(p, s)
p = 'qemu-options.hx'; s = rd(p)
s = drop_between(s, 'DEF("xen-domid", HAS_ARG', '    specified domain id (XEN only).\nERST\n\n', p)
s = s.replace('supported accelerators are kvm, xen, hvf, nitro, nvmm, whpx, mshv or tcg (default: tcg)', 'the only accelerator is tcg (default: tcg)')
s = s.replace('kvm, xen, hvf, nitro, nvmm, whpx, mshv or tcg', 'tcg')
s = s.replace('-vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]', '-vga [std|cirrus|vmware|qxl|tcx|cg3|virtio|none]')
s = s.replace('to select the value based on accel and i8042. For accel=xen or\n        i8042=off the default is off otherwise the default is on.',
              'to select the value based on i8042. For i8042=off the default\n        is off otherwise the default is on.')
for key in ('igd-passthru', 'kernel-irqchip', 'kvm-shadow-mem', 'dirty-ring-size', 'eager-split-size', 'notify-vmexit'):
    s = re.sub(r'(?m)^    "                %s=.*\\n"\n' % key, '', s)
s = sub1(s, '    "                thread=single|multi (enable multi-threaded TCG)\\n"\n    "                device=path (KVM device path, default /dev/kvm)\\n", QEMU_ARCH_ALL)',
            '    "                thread=single|multi (enable multi-threaded TCG)\\n", QEMU_ARCH_ALL)', p)
a = s.index('``-accel name[,prop=value[,...]]``'); b = s.index('ERST', a)
parts = re.split(r'(?m)^(?=    ``)', s[a:b])
gone = ('igd-passthru', 'kernel-irqchip', 'kvm-shadow-mem', 'dirty-ring-size', 'eager-split-size', 'notify-vmexit', 'device=', 'hyperv')
parts = [x for x in parts if not any(x.startswith('    ``' + g) for g in gone)]
s = s[:a] + ''.join(parts).rstrip('\n') + '\n' + s[b:]
wr(p, s)

# ---- QAPI
p = 'qapi/misc-i386.json'; s = rd(p)
for n in ('EvtchnPortType', 'EvtchnInfo', 'xen-event-list', 'xen-event-inject'): s = drop_qapi(s, n, p)
wr(p, s)
p = 'qapi/migration.json'; s = rd(p)
for n in ('xen-save-devices-state', 'xen-set-global-dirty-log', 'xen-load-devices-state', 'xen-set-replication',
          'ReplicationStatus', 'query-xen-replication-status', 'xen-colo-do-checkpoint'): s = drop_qapi(s, n, p)
wr(p, s)
p = 'qapi/accelerator.json'; s = rd(p)
s = re.sub(r"(?m)^# @(hvf|kvm|mshv|nvmm|whpx|xen):.*\n#\n", '', s)
s = sub1(s, "'data': ['hvf', 'kvm', 'mshv', 'nvmm', 'qtest', 'tcg', 'whpx', 'xen']", "'data': ['qtest', 'tcg']", p)
wr(p, s)

# ---- migration + block
p = 'migration/savevm.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_between(s, '    /* Validate if it is a device\'s state */\n    if (xen_enabled()', '        return -EINVAL;\n    }\n\n', p)
s = drop_function(s, 'void qmp_xen_save_devices_state(', p)
s = drop_function(s, 'void qmp_xen_load_devices_state(', p)
wr(p, s)
p = 'migration/options.c'; s = rd(p)
s = drop_lines(s, '#include "system/xen.h"\n', p)
s = drop_block(s, '    if (xen_enabled()) {\n        error_setg(errp, "Snapshots are not compatible with Xen");', p)
wr(p, s)
p = 'migration/colo.c'; s = rd(p)
for sig in ('void qmp_xen_set_replication(', 'ReplicationStatus *qmp_query_xen_replication_status(', 'void qmp_xen_colo_do_checkpoint('):
    s = drop_function(s, sig, p)
wr(p, s)
p = 'blockdev.c'; s = rd(p)
s = drop_lines(s, '    [IF_XEN] = "xen",\n', p)
s = sub1(s, 'Ignore IF_VIRTIO or IF_XEN,', 'Ignore IF_VIRTIO,', p)
s = sub1(s, 'dinfo->type == IF_XEN || ', '', p)
a = s.index('    } else if (type == IF_XEN) {'); b = s.index('    }\n', s.index('&error_abort);\n', s.index('"drive", qdict_get_str(bs_opts, "id")', a))) + 6
s = s[:a] + '    }\n' + s[b:]
s = drop_lines(s, '    case IF_XEN:\n', p)
wr(p, s)
p = 'include/system/blockdev.h'; s = rd(p); s = re.sub(r'\bIF_VIRTIO, IF_XEN,', 'IF_VIRTIO,', s); wr(p, s)

# ---- build system
p = 'meson.build'; s = rd(p)
s = drop_between(s, "if cpu == 'x86_64'\n  xen_targets = ['i386-softmmu', 'x86_64-softmmu']", "accelerator_targets += { 'CONFIG_XEN': xen_targets }\n\n", p)
s = drop_between(s, "xen = not_found\nif get_option('xen')", "  .allowed()\n\n", p)
s = drop_lines(s, "config_host_data.set('CONFIG_XEN_BACKEND', xen.found())\n", p)
s = drop_between(s, "if xen.found()\n  # protect from xen.version()", "endif\n", p)
s = drop_between(s, "  summary_info += {'Xen support':       xen.found()}", "  summary_info += {'Xen emulation':     config_all_devices.has_key('CONFIG_XEN_EMU')}\n", p)
wr(p, s)
p = 'meson_options.txt'; s = rd(p)
s = drop_lines(s, "option('xen', type: 'feature', value: 'disabled',\n       description: 'Xen backend support')\noption('xen_pci_passthrough', type: 'feature', value: 'auto',\n       description: 'Xen PCI passthrough support')\n", p)
wr(p, s)
subprocess.run(['sed', '-i', '/xen/d', 'scripts/meson-buildoptions.sh'])
p = 'stubs/meson.build'; s = rd(p)
s = re.sub(r"  stub_ss\.add\(files\('(xen-hw-stub|qmp-i386-xen)\.c'\)\)\n", '', s); wr(p, s)
p = 'accel/stubs/meson.build'; s = rd(p); s = drop_lines(s, "  'xen-stub.c',\n", p); wr(p, s)
p = 'hw/meson.build'; s = rd(p); s = drop_lines(s, "subdir('xen')\n", p); wr(p, s)
p = 'hw/Kconfig'; s = rd(p); s = drop_lines(s, 'source xen/Kconfig\n', p); wr(p, s)
p = 'accel/Kconfig'; s = rd(p)
s = drop_between(s, '\nconfig XEN\n    bool', '    select XEN_BUS\n', p); wr(p, s)
p = 'hw/i386/Kconfig'; s = rd(p)
s = drop_between(s, 'config XEN_EMU\n    bool', '    depends on KVM && I386\n', p); wr(p, s)
p = 'hw/pci-host/Kconfig'; s = rd(p)
s = drop_between(s, 'config XEN_IGD_PASSTHROUGH\n    bool', '    depends on XEN && PCI_I440FX\n\n', p); wr(p, s)
p = 'configs/devices/i386-softmmu/default.mak'; s = rd(p); s = drop_lines(s, 'CONFIG_XEN_EMU=n\n', p); wr(p, s)
p = 'hw/i386/meson.build'; s = rd(p); s = drop_lines(s, "subdir('kvm')\n", p); wr(p, s)

# ---- delete the leftovers
dead = ['hw/xen', 'include/hw/xen', 'include/system/xen.h', 'include/system/xen-mapcache.h',
        'include/system/kvm_xen.h', 'stubs/xen-hw-stub.c', 'stubs/qmp-i386-xen.c', 'accel/stubs/xen-stub.c',
        'hw/block/dataplane/xen-block.h', 'hw/block/xen_blkif.h', 'hw/9pfs/xen-9pfs.h',
        'target/i386/kvm/xen-compat.h', 'scripts/xen-detect.c']
for d in dead: subprocess.run(['git', 'rm', '-rq', d], check=True)
for f in subprocess.check_output(['git', 'ls-files', 'hw/i386/kvm']).decode().split():
    if f != 'hw/i386/kvm/clock.h': subprocess.run(['git', 'rm', '-q', f], check=True)
print("batch 3 done")

# stale trace events of the deleted Xen devices
subprocess.run("sed -i '/xen/Id' hw/block/trace-events hw/net/trace-events hw/char/trace-events hw/9pfs/trace-events hw/display/trace-events", shell=True)
