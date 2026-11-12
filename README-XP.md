# xp-qemu (working title)

A QEMU fork that only targets **32-bit Windows XP guests** on **x86-64-v3 hosts**,
using TCG/MTTCG, with VirtualBox-style VBoxVGA 3D as the long-term display goal.

Base: upstream QEMU `9d7d057` (11.1.90). License: unchanged (GPL-2.0, see COPYING).

## Status

| Done | Next |
|------|------|
| Removed tests/, docs/, all non-x86 targets, boards and user-mode emulators | Prune unused device models and old versioned machine types |
| i386-softmmu builds; only the i440FX `pc` machine remains | Drop non-x86 TCG host backends / TCI; assume x86-64-v3 |
| Build graph 1,930 -> 1,430 steps | SSE2 TCG fast paths (gvec) and MTTCG tuning |
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
