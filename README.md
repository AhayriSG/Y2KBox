<div align="center">

<img width="272" height="281" alt="Y2KBox logo (alpha)" src="https://github.com/user-attachments/assets/af59c3c3-da2c-469f-84e0-1c605ce8d1e8" alt="Y2KBox Logo" width="200" />

# Y2KBox

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0d7875bcbfb40428c030025425f775f)](https://app.codacy.com/gh/AhayriSG/Y2KBox/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![build](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml/badge.svg)](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/AhayriSG/Y2KBox/blob/main/LICENSE)
</div>

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, using **TCG / MTTCG only** (no KVM/WHPX/HVF/Xen).
Long-term goals: fast TCG for all guest instruction classes, **VMware SVGA3D
(qemu-vmvga) 3D acceleration** for XP (VBoxVGA 5.2.x kept as the alternative), and a small wxWidgets launcher.

**Contributors / LLMs: read [HANDOFF.md](HANDOFF.md) first**, then pick work from
[TASKS.md](TASKS.md) (claim by pushing) and log what you did in [WORKLOG.md](WORKLOG.md).
Several AIs can work in parallel.

Fixed target: guest RAM 2048 MB (automatic, not configurable), guest CPU SSE2 maximum
(Pentium 4 class), 1 vCPU by default; hosts: Windows and Linux (x86-64-v3 or newer).

Base: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).
License: GPL-2.0, unchanged (see COPYING / LICENSE).

## Download

Every green CI run uploads Linux and Windows builds as Actions Artifacts (open the latest run on the Actions tab):
`qemu-system-i386[.exe]`, `qemu-img[.exe]`, a `pc-bios/` folder and, on Windows, the required runtime DLLs (incl. SDL2).
Unzip the **whole** archive and keep `pc-bios/` next to the executable; it is found automatically (no `-L` needed).

    qemu-img create -f qcow2 xp.qcow2 10G
    qemu-system-i386 -drive file=xp.qcow2,format=qcow2 -cdrom xp.iso -boot d -display sdl      # or: -display vnc=:0

* **Display:** `-display sdl` (SDL2 is part of the build) or `-display vnc=:0`. A wxWidgets front end is planned (T12).
* **"could not load PC BIOS 'bios-256k.bin'":** the `pc-bios` folder is missing or incomplete (e.g. only the .exe was copied).
  Copy the whole `pc-bios` folder from the artifact next to the executable. As a fallback you can copy the firmware files
  (`bios-256k.bin`, `vgabios-*.bin`, `kvmvapic.bin`, `linuxboot_dma.bin`, `efi-*.rom`, `keymaps/`, ...) from an official
  QEMU installation into Y2KBox's `pc-bios` folder; or point to any firmware folder with `-L <dir>`.
* **Linux:** the binary needs the usual shared libraries (glib2, pixman, libslirp, libfdt, SDL2) from your distribution.

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
| x87 host fast path (add/sub/mul/div/sqrt, exact loads) | done (part 1) | 3.0-4.6x on x87 microbenchmarks, bit-identical to softfloat on 7.2M random ops (T13a) |
| General TCG performance: call/ret, rep string ops, more x87, thread pinning | planned | T13b, T13c, T14 |
| Remove IOMMU/SEV/SGX/TDX/VFIO + remaining KVM call sites | planned | heavily entangled; see HANDOFF |
| Device pruning | partly done | XP device set in `default.mak` (220 -> 91 enabled symbols, 439 -> ~247 listed devices); 1015 dead C files, 724 unused headers, non-x86 firmware removed (tree 171 MB -> 41 MB); machine-coupled pieces (DIMM/NVDIMM/CXL/ACPI hotplug) await the fixed-machine rework |
| Boot Windows XP SP3 end to end | **not yet done** | only SeaBIOS boot verified so far |
| 3D: integrate qemu-vmvga (VMware SVGA II + SVGA3D via DXVK) + TCG backdoor-port fix | planned | T09-T11; VBoxVGA 5.2.x (T16) is the frozen alternative |
| wxWidgets launcher (qcow2 disk, RAM, floppy, ISO) | planned (later) | |

Verified so far: CI builds on Linux and Windows (MSYS2); fresh configure + full build; `-accel help` lists only `tcg`;
SeaBIOS boots under TCG with 2 vCPUs; SSE2 memory-op functional test passes.

## References

QEMU: https://github.com/qemu/QEMU - xemu: https://github.com/xemu-project/xemu -
qemu-vmvga (VMware SVGA3D for QEMU, primary 3D plan): https://github.com/qemus/qemu-vmvga -
xbased: https://github.com/crazyriddler/xbased - 86Box VBoxVGA PR (alternative path):
https://github.com/86Box/86Box/pull/6183 - VirtualBox mirror (use v5.2.x/v6.1.x,
GPLv2): https://github.com/VirtualBox/virtualbox - Bochs GeForce PR:
https://github.com/bochs-emu/Bochs/pull/574 - wine9x: https://github.com/JHRobotics/wine9x -
SoftGPU: https://github.com/JHRobotics/softgpu - DXVK: https://github.com/doitsujin/DXVK -
d3d8to9: https://github.com/crosire/d3d8to9/ - viogpu3d:
https://github.com/virtio-win/kvm-guest-drivers-windows/pull/943 - virglrenderer:
https://github.com/utmapp/virglrenderer - Yttrium: https://github.com/arehnman/yttrium-virtio-gpu - PCBox NV11 (GeForce2 MX) PR, real-GPU route:
https://github.com/PCBox/PCBox/pull/202 .
x87/soft-float references: PCSX2 soft-float interpreter https://github.com/PCSX2/pcsx2/pull/12001 and recompiler
PRs https://github.com/DiscoStarslayer/pcsx2-reliquary/pull/17 , /pull/18 , /pull/29 ; xemu's fpu/:
https://github.com/xemu-project/xemu/tree/master/fpu .
Details and how each is useful: HANDOFF.md section 9.
