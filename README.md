# Y2KBox

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, using **TCG / MTTCG only** (no KVM/WHPX/HVF/Xen).
Long-term goals: fast SSE2 under TCG, VirtualBox-style **VBoxVGA 3D acceleration**
for XP, and a small wxWidgets launcher.

**Contributors / LLMs: read [HANDOFF.md](HANDOFF.md) first** - aim, decisions,
findings, design notes and exact next steps.

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
| Fixed XP machine (host-thread vCPUs, RAM <= 3.5 GB, below 4 GB only) | planned | no -smp/-cpu/-m flags |
| Remove IOMMU/SEV/SGX/TDX/VFIO + remaining KVM call sites | planned | heavily entangled; see HANDOFF |
| Device pruning (439 models remain) + old machine versions | planned | |
| Boot Windows XP SP3 end to end | **not yet done** | only SeaBIOS boot verified so far |
| VBoxVGA + HGSMI/VBVA + VMMDev/HGCM + 3D host service | planned | VirtualBox 5.2.x (GPLv2) as reference |
| wxWidgets launcher (qcow2 disk, RAM, floppy, ISO) | planned (later) | |

Verified so far: fresh configure + full build; `-accel help` lists only `tcg`;
SeaBIOS boots under TCG with 2 vCPUs; SSE2 memory-op functional test passes.
