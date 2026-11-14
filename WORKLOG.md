# Y2KBox work log (newest entries at the bottom; append, never rewrite history)

Format: `date | who | commit | what / result / evidence`.
Every session MUST append here before stopping (see HANDOFF.md section 10).

## Session 1 (Claude), 2026-11-13 .. 2026-11-14
Base: upstream QEMU 9d7d057 (11.1.90). Sandbox: 1 core, 3 GB RAM (builds ~10 min at -j2).

| commit | what |
|--------|------|
| 74e26e7 | remove `tests/` |
| ca31674 | remove `docs/` (build_docs=false) |
| 2d22bf2 | remove all non-x86 targets/boards, linux-user/bsd-user/common-user; configure patched; Kconfig stubs for dangling symbols (`xp/strip/02-*`) |
| e9c3a24 | i440FX `pc` only (default.mak disables q35/microvm/isapc/nitro/SEV/SGX/TDX/IOMMU/OVMF) |
| c3bc2f6 | full TCG: remove KVM/HVF/WHPX/NVMM/MSHV/Nitro accelerators, Xen/Hyper-V implementations (`xp/strip/03-*`) |
| 0dd67e4 | x86-64 hosts only; non-x86 TCG backends + TCI removed; `x86_version=3` default; startup v3 check; v3 features constant in `tcg/x86_64`; fixed CPUID leaf typo (`xp/strip/04-*`) |
| 5091ce3 | Xen removed completely incl. QAPI/HMP/CLI (`xp/strip/05-remove-xen.py`) |
| ffc195e | SSE2 benchmark (`xp/bench/`), `XPQEMU_NO_AVX512=1` switch. Result: AVX-512 gives no gain for SSE2 guests -> stay on v3 |
| abae63c | **`qemu_ld_vec`/`qemu_st_vec` TCG ops**: 128-bit guest loads 1.56s->0.58s, mixed mem loop 6.20s->2.82s; `xp/bench/vectest.asm` functional test |
| 0ab9db8 | XP device set in default.mak (220->91 symbols); 1015 dead C files, 724 headers, non-x86 firmware, contrib removed (tree 88MB->41MB); `xp/strip/06-*`; qemu-img verified |

Notable lessons: removing option ROMs broke the default NIC (caught by the boot test);
`git grep` on a blobless clone downloads everything (timeouts); meson option defaults
only apply to a FRESH configure; `ninja -t compdb` lists non-default targets (use
`ninja -t commands <target>` for the real compile set).

### Open at end of session 1
T01 (fixed machine) and T03 (Linux+Windows hosts only) were started; see TASKS.md for
their exact state. Anything not committed on a `task/*` branch was lost - check branches.

| (this push) | T03 pass 1: macOS/BSD/Solaris/WASM code removed (`xp/strip/07-*`); Linux fresh configure+full build+SeaBIOS+vectest green. Windows build NOT verified. |
| (this push) | T02 part 1: `xp-p4` CPU model (family 15, SSE2 max) is the default; verified with `xp/bench/cpuid.asm` (EDX=078BFBFD ECX=80000000 -> SSE/SSE2 yes, SSE3/SSSE3/SSE4/AVX no). T01 part 1: default RAM 2 GiB (e820 = one 0..0x80000000 RAM entry), default 1 vCPU, max_cpus 2. Owner answered Q1-Q4 (recorded in HANDOFF 4): 1 vCPU default, 5.2.x VBoxVGA baseline, RTL8139 default (virtio = opt-in eval T15), RAM fixed 2048 MB. Perf scope clarified: ALL instruction classes (T13), plus threading/pinning (T14). |
| (this push) | T01 part 2a: fixed-machine memory model. `pc.c` -442 lines (minimal `pc_memory_init`, hotplug handlers, CXL), `acpi-build.c` NVDIMM/hotplug/CXL/HMAT/VIOT/VMGenID/VMClock removed, features switched off in default.mak. Lesson: removing `mc->nvdimm_supported` made `machine->nvdimms_state` NULL -> segfault in `build_dsdt` (found with gdb backtrace); a missing `memhp_io_base` was NOT the cause. Checks: e820 2 GiB, vectest OK, -smp 2, 244 devices. |
| (this push) | T01 part 2b: single machine type. pc_piix.c 553 -> ~325 lines; 88 compat tables removed. `-machine help` shows `pc` (alias of pc-i440fx-11.2) and `none`. e820 2 GiB, vectest OK. |

| (this push) | T04 session started: claimed T04 on main (`a264499096d9f39dfa89884ac193a9f2005e9677`) and created `task/T04-inline-xmm-shuffles`. Inspected HANDOFF/TASKS, current TCG SSE helper declarations and emit paths, x86_64 target vector op inventory, and benchmark harness. No source changes or baseline tests: local `git clone` failed because this shell could not resolve `github.com`; GitHub connector file access does not provide an executable checkout. T04 paused; resume after a working local checkout/build environment is available. |

| 2026-11-14 | Resumed T04 after owner clarified local PC is unavailable. Added branch-scoped `.github/workflows/t04-ci.yml` to provide an Ubuntu build runner and baseline/regression checks; run #43638713937 queued on commit `4cd38fbe137e7a946f034776a1c35b4e2a5f320f`. This avoids relying on local shell network access; baseline results pending. |
