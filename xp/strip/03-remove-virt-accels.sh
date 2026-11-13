#!/bin/sh
# Go full TCG: remove KVM/HVF/WHPX/NVMM/MSHV/Nitro accelerators, Xen and
# Hyper-V device code. Headers and stub files that shared code still includes
# are kept for now (kvm_i386.h, tdx.h, hyperv-proto.h, xen_*.h, *-stub.c).
set -eu

# 1. Never auto-enable these.
for o in kvm mshv whpx hvf nitro nvmm xen; do
  sed -i "/^option('$o'/s/value: 'auto'/value: 'disabled'/" meson_options.txt
done
sed -i "/^option('hv_balloon'/s/value *: *'auto'/value : 'disabled'/" meson_options.txt

# 2. Accelerator implementations.
for d in hvf whpx kvm xen mshv nitro; do
  git rm -rq accel/$d
  sed -i "/^  subdir('$d')\$/d" accel/meson.build
done
git rm -rq target/i386/hvf target/i386/whpx target/i386/nvmm target/i386/mshv target/i386/emulate
sed -i "/^subdir('whpx')\$/d;/^subdir('nvmm')\$/d;/^subdir('hvf')\$/d;/^subdir('emulate')\$/d;/^subdir('mshv')\$/d" target/i386/meson.build

# target/i386/kvm: keep only headers still included elsewhere + hyperv stub.
( cd target/i386/kvm && git rm -q kvm.c kvm-cpu.c vmsr_energy.c vmsr_energy.h \
    xen-emu.c xen-emu.h tdx.c tdx-quote-generator.c tdx-quote-generator.h \
    tdx-stub.c hyperv.c trace-events )
cat > target/i386/kvm/meson.build <<'EOM'
# XP fork: KVM, TDX and Xen emulation were removed. Only the Hyper-V stub
# remains; the headers in this directory are still included by shared code.
i386_system_ss.add(files('hyperv-stub.c'))
EOM

# hw/i386: KVM in-kernel irqchip/clock, MSHV, Xen.
git rm -rq hw/i386/mshv hw/i386/xen
( cd hw/i386/kvm && git rm -q apic.c clock.c i8254.c i8259.c ioapic.c \
    xen_evtchn.c xen_gnttab.c xen_overlay.c xen_primary_console.c \
    xen_xenstore.c xenstore_impl.c xenstore_impl.h trace-events )
cat > hw/i386/kvm/meson.build <<'EOM'
# XP fork: in-kernel KVM devices and Xen emulation were removed. The stubs
# keep the callers in pc.c / pci / msi linking; headers remain for them.
stub_ss.add(files('xen-stubs.c'))
EOM
sed -i "/^subdir('mshv')\$/d;/^subdir('xen')\$/d;/^i386_ss.add_all(xenpv_ss)\$/d" hw/i386/meson.build

# Xen PV machine and Xen backends/frontends.
git rm -rq hw/xenpv
sed -i "/^subdir('xenpv')/d" hw/meson.build
for f in $(git ls-files hw/xen | grep '\.c$' | grep -v '^hw/xen/xen_stubs\.c$'); do git rm -q $f; done
git rm -q hw/xen/trace-events
printf '%s\n' "# XP fork: Xen support removed; only the stubs remain." "stub_ss.add(files('xen_stubs.c'))" > hw/xen/meson.build
git rm -q hw/block/xen-block.c hw/block/dataplane/xen-block.c hw/char/xen_console.c \
    hw/display/xenfb.c hw/net/xen_nic.c hw/usb/xen-usb.c hw/9pfs/xen-9p-backend.c \
    hw/pci-host/xen_igd_pt.c
sed -i "/xen/Id" hw/block/meson.build hw/block/dataplane/meson.build hw/char/meson.build \
    hw/display/meson.build hw/net/meson.build hw/usb/meson.build hw/9pfs/meson.build hw/pci-host/meson.build

# Hyper-V devices (they depend on KVM anyway). Keep only the balloon stub.
for f in $(git ls-files hw/hyperv | grep -E '\.(c|h)$' | grep -v 'hv-balloon-stub\.c$'); do git rm -q $f; done
git rm -q hw/hyperv/trace-events
printf '%s\n' "# XP fork: Hyper-V device models removed; only the QMP stub remains." "system_ss.add(files('hv-balloon-stub.c'))" > hw/hyperv/meson.build

# trace-events directory list.
for t in accel/hvf accel/kvm accel/mshv accel/nitro hw/hyperv hw/i386/xen hw/i386/kvm hw/xen target/i386/kvm target/i386/whpx; do
  sed -i "\#^ *'$t',\$#d" meson.build
done
git rm -q hw/i386/kvm/trace.h hw/xen/trace.h target/i386/kvm/trace.h

# Fixups for headers that shared code still included.
python3 - <<'PY'
import re
p='target/i386/cpu.c'; s=open(p).read()
s=s.replace('#include "whpx/whpx-i386.h"\n','')
while True:
    m=re.search(r'    \} else if \(whpx_enabled\(\)\) \{\n.*?(?=    \} else)', s, re.S)
    if not m: break
    s=s[:m.start()]+s[m.end():]
open(p,'w').write(s)
p='target/i386/kvm/tdx.h'; s=open(p).read()
s=s.replace('#include "tdx-quote-generator.h"\n\n','')
open(p,'w').write(s)
PY

python3 - <<'PY'
import re
p='target/i386/cpu.c'; s=open(p).read()
s=s.replace('#include "hvf/hvf-i386.h"\n','')
for t in ('hvf','mshv'):
    while True:
        m=re.search(r'    \} else if \(%s_enabled\(\)\) \{\n.*?(?=    \} else)'%t, s, re.S)
        if not m: break
        s=s[:m.start()]+s[m.end():]
open(p,'w').write(s)
PY

# TDX is gone: replace tdx.h with a minimal stub header.
cat > target/i386/kvm/tdx.h <<'EOM'
/*
 * XP fork: Intel TDX support was removed. This header only keeps the
 * symbols that shared x86 code still names; they are all dead code.
 */
#ifndef QEMU_I386_TDX_H
#define QEMU_I386_TDX_H

#define is_tdx_vm() 0

int tdx_parse_tdvf(void *flash_ptr, int size);
void tdx_set_tdvf_region(MemoryRegion *tdvf_mr);

#endif /* QEMU_I386_TDX_H */
EOM
