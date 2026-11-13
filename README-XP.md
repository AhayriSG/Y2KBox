# xp-qemu (working title)

A QEMU fork that only targets **32-bit Windows XP guests** on **x86-64-v3 hosts**,
using TCG/MTTCG, with VirtualBox-style VBoxVGA 3D as the long-term display goal.

Base: upstream QEMU `9d7d057` (11.1.90). License: unchanged (GPL-2.0, see COPYING).

## Status

| Done | Next |
|------|------|
| Removed tests/, docs/, all non-x86 targets, boards and user-mode emulators | Prune unused device models and old versioned machine types; remove KVM call sites/headers |
| TCG: vector guest memory ops (qemu_ld_vec/st_vec), inline PSHUFD & friends |
| i386-softmmu builds; only the i440FX `pc` machine remains | IOMMU/SEV/SGX/TDX/VFIO removal (heavily entangled; own pass) |
| Full TCG: KVM/HVF/WHPX/NVMM/MSHV/Nitro, Xen and Hyper-V code removed (`-accel help` -> tcg only) | SSE2 TCG fast paths (gvec) and MTTCG tuning |
| x86-64 hosts only (non-x86 TCG backends + TCI removed); x86-64-v3 enforced at startup | Xen fully removed (stubs, headers, call sites, QAPI/CLI/HMP) |
| SSE2 benchmark + AVX-512 A/B in `xp/bench/` (no AVX-512 gain; vector memory ops are the hotspot) |
| Fresh build graph 1,930 -> 1,716 steps; SeaBIOS boots under TCG (2 vCPUs) |
| | VBoxVGA + HGSMI/VBVA, VMMDev, HGCM, 3D host service |

## Building

    ./configure --target-list=i386-softmmu --disable-tools --disable-guest-agent
    ninja -C build qemu-system-i386

Notes: needs `libfdt-dev` and `libslirp-dev`. `subprojects/keycodemapdb` can be cloned
from https://github.com/qemu/keycodemapdb at the revision in its `.wrap` file if
gitlab.com is unreachable.

## Reproducing the strip

`xp/strip/` holds the scripts used for the mechanical removals.

## VBox reference sources (licensing)

VirtualBox <= 6.1 display/VMMDev sources are GPL version 2 (7.x is GPL-3.0-only and
must not be copied). The Chromium-based 3D host service exists in open source only up
to the 5.x series. Audit every file's header before porting anything.

## Host baseline

x86-64-v3 is the hard minimum (AVX2, BMI1/2, FMA, F16C, LZCNT, MOVBE, POPCNT):
`x86_version` defaults to 3, the binary exits with a message on older CPUs, and the
TCG backend treats those features as compile-time constants. AVX-512 (VL/BW/DQ/VBMI2)
stays runtime-detected: the backend already has gated fast paths for it, so AVX-512
hosts benefit without making v4 mandatory. `-Dx86_version=4` remains available.
