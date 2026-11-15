# Y2KBox

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, using **TCG / MTTCG only** (no KVM/WHPX/HVF/Xen).
Long-term goals: fast SSE2 under TCG, VirtualBox-style **VBoxVGA 3D acceleration**
for XP, and a small wxWidgets launcher.

**Contributors / LLMs: read [HANDOFF.md](HANDOFF.md) first**, then pick work from
[TASKS.md](TASKS.md) (claim by pushing) and log what you did in [WORKLOG.md](WORKLOG.md).
Several AIs can work in parallel.

Fixed target: guest RAM 2048 MB (automatic, not configurable), guest CPU SSE2 maximum
(Pentium 4 class), 1 vCPU by default; hosts: Windows and Linux (x86-64-v3 or newer).

Base: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).
License: GPL-2.0, unchanged (see COPYING / LICENSE).

## Development status

_Last updated: 2026-11-14. Legend: done / in progress / planned._

| Area | Status | Notes |
|------|--------|-------|
| Strip: tests, docs, non-x86 targets/boards, user-mode | done | `xp/strip/02-*` |
| i440FX `pc` machine only | done | q35/microvm/isapc/nitro off |
| Full TCG (no KVM/HVF/WHPX/NVMM/MSHV/Nitro) | done | `xp/strip/03-*` |
| Xen removed completely | done | `xp/strip/05-remove-xen.py` |
| x86-64 hosts only, x86-64-v3 baseline enforced | done | other TCG backends + TCI removed |
| Benchmarks + AVX-512 A/B | done | no AVX-512 gain for SSE2 guests; stay on v3 |
| 128-bit guest memory ops (`qemu_ld_vec`/`qemu_st_vec`) | done | loads ~2.7x faster; `xp/bench/vectest.asm` |
| Inline PSHUFD & other shuffles | in progress / next | currently out-of-line helpers |
| Fixed XP machine (2 GiB RAM, 1 vCPU default, no -smp/-cpu/-m flags) | in progress | fixed 2 GiB memory model done, hotplug/NVDIMM/CXL removed; single machine type done; CLI flag removal + dead above-4G code pending (T01) |
| XP guest CPU `xp-p4` (SSE2 max) | in progress | model is default; deleting other models and SSE3+ translators pending (T02) |
| Linux + Windows hosts only | in progress | macOS/BSD code removed; Linux and Windows builds verified by CI (T03 pass 2 pending, T07 done) |
| Thread pinning, general TCG performance (all instruction classes) | planned | T13, T14 |
| Remove IOMMU/SEV/SGX/TDX/VFIO + remaining KVM call sites | planned | heavily entangled; see HANDOFF |
| Device pruning | partly done | XP device set in `default.mak` (220 -> 91 enabled symbols, 439 -> ~247 listed devices); 1015 dead C files, 724 unused headers, non-x86 firmware removed (tree 171 MB -> 41 MB); machine-coupled pieces (DIMM/NVDIMM/CXL/ACPI hotplug) await the fixed-machine rework |
| Boot Windows XP SP3 end to end | **not yet done** | only SeaBIOS boot verified so far |
| VBoxVGA + HGSMI/VBVA + VMMDev/HGCM + 3D host service | planned | VirtualBox 5.2.x (GPLv2) as reference |
| wxWidgets launcher (qcow2 disk, RAM, floppy, ISO) | planned (later) | |

Verified so far: CI builds on Linux and Windows (MSYS2); fresh configure + full build; `-accel help` lists only `tcg`;
SeaBIOS boots under TCG with 2 vCPUs; SSE2 memory-op functional test passes.

## References

QEMU: https://github.com/qemu/QEMU - xemu: https://github.com/xemu-project/xemu -
xbased: https://github.com/crazyriddler/xbased - 86Box VBoxVGA PR:
https://github.com/86Box/86Box/pull/6183 - VirtualBox mirror (use v5.2.x/v6.1.x,
GPLv2): https://github.com/VirtualBox/virtualbox - Bochs GeForce PR:
https://github.com/bochs-emu/Bochs/pull/574 - wine9x: https://github.com/JHRobotics/wine9x -
SoftGPU: https://github.com/JHRobotics/softgpu - DXVK: https://github.com/doitsujin/DXVK -
d3d8to9: https://github.com/crosire/d3d8to9/ - viogpu3d:
https://github.com/virtio-win/kvm-guest-drivers-windows/pull/943 - virglrenderer:
https://github.com/utmapp/virglrenderer - Yttrium: https://github.com/arehnman/yttrium-virtio-gpu.
Details and how each is useful: HANDOFF.md section 9.
