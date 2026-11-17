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

| commit | what |
|--------|------|
| (this push) | T03 pass 1: macOS/BSD/Solaris/WASM code removed (`xp/strip/07-*`); Linux fresh configure+full build+SeaBIOS+vectest green. Windows build NOT verified. |
| (this push) | T02 part 1: `xp-p4` CPU model (family 15, SSE2 max) is the default; verified with `xp/bench/cpuid.asm` (EDX=078BFBFD ECX=80000000 -> SSE/SSE2 yes, SSE3/SSSE3/SSE4/AVX no). T01 part 1: default RAM 2 GiB (e820 = one 0..0x80000000 RAM entry), default 1 vCPU, max_cpus 2. Owner answered Q1-Q4 (recorded in HANDOFF 4): 1 vCPU default, 5.2.x VBoxVGA baseline, RTL8139 default (virtio = opt-in eval T15), RAM fixed 2048 MB. Perf scope clarified: ALL instruction classes (T13), plus threading/pinning (T14). |
| (this push) | T01 part 2a: fixed-machine memory model. `pc.c` -442 lines (minimal `pc_memory_init`, hotplug handlers, CXL), `acpi-build.c` NVDIMM/hotplug/CXL/HMAT/VIOT/VMGenID/VMClock removed, features switched off in default.mak. Lesson: removing `mc->nvdimm_supported` made `machine->nvdimms_state` NULL -> segfault in `build_dsdt` (found with gdb backtrace); a missing `memhp_io_base` was NOT the cause. Checks: e820 2 GiB, vectest OK, -smp 2, 244 devices. |
| (this push) | T01 part 2b: single machine type. pc_piix.c 553 -> ~325 lines; 88 compat tables removed. `-machine help` shows `pc` (alias of pc-i440fx-11.2) and `none`. e820 2 GiB, vectest OK. |
| (this push) | Coordination note: another agent claimed T04, hit a build-environment blocker, then "reverted" - that revert swapped the contents of TASKS.md and WORKLOG.md. Restored both from d12dba6. T04 is unclaimed (TODO). Tip for parallel agents: never restore whole files by copy; use `git revert <commit>` and check `head -1 TASKS.md WORKLOG.md` before pushing. T02 part 2 claimed by Claude (session 2). |
| (this push) | T02 part 2a: only `xp-p4` CPU model remains (47 -> 1), 17 unused tables removed by compiler-warning loop (`xp/strip/10b-*`: pass FILE, BUILD_DIR, OBJECT; note gcc prints typographic quotes). cpu.c 11085 -> 6205 lines. Checks: CPUID EAX=0F29 EDX=078BFBFD ECX=80000000 unchanged, vectest OK, -smp 2, e820 2 GiB. |
| (this push) | T07 done: CI workflow (Linux + Windows/MSYS2 UCRT64) added; another agent fixed the Windows job (UCRT64 packages, error annotations, `pc-bios/qemu-nsis.ico`). Run 43695682586 on 92dbf22: linux 2m36s success, windows 11m13s success incl. vectest OK. Windows host build is now VERIFIED. API tip: `GET /repos/AhayriSG/Y2KBox/actions/runs/<id>/jobs` shows steps; failure text via `/check-runs/<job_id>/annotations` (raw log download needs a redirect and returned empty from the sandbox). |
| (this push) | T13 baseline: real-mode microbenchmark suite `xp/bench/micro` + `perf.py`. ALU/imul/div/branch/memory/push-pop near-native; hotspots: x87 ~24 ns/insn (softfloat), call+ret ~12 ns/pair, rep movsd/stosd ~2 GB/s. Pitfalls: loop counter on the code page = SMC retranslation (50x slow); small N dominated by ~0.09 s startup + sandbox noise (+-0.3 s): use >=1 s runs, min-of-3. Plan: T13a host-x87 helpers, T13b ret shadow stack, T13c rep-string fast path. Also: tool shell is dash - no `{a,b}` brace expansion. |
| (this push) | 3D plan changed on owner proposal: PRIMARY = qemu-vmvga (VMware SVGA II + SVGA3D, DXVK; repo read and evaluated: 44-file overlay, GPL-2.0-or-later, QEMU 11.1.1-based, XP driver 12.01.08 / VMware Tools 10.0.12). VBoxVGA 5.2.x demoted to frozen alternative T16. Verified in our code that TCG has the same ring-3 backdoor-port #GP as KVM (`gen_check_io` -> `helper_check_io`); fix is a tiny exemption for ports 0x5658/0x5659 (part of T09), no host module setting needed. Added references: PCSX2 soft-float PRs, xemu fpu (QEMU softfloat copy, nothing Xbox-specific found), qemu-vmvga. Tasks T09-T11 rewritten. NOTE: `hw/i386/vmport.c` was deleted by the dead-source pass (VMPORT off); overlay brings its own. |
| (this push) | **T13a part 1: x87 host fast path.** Measured first: host x87 add = 2.6 ns but `fnclex` = ~60 ns on this CPU, so flags are NOT cleared per op: host sticky flags are merged (union) into guest SW, with a lazy epoch-based `fnclex` only after the guest clears/loads flags (fnclex/fninit/fldenv/frstor/fxrstor/reset/vmstate load). Fast path only for normal finite operands, all-masked guest CW, valid precision; everything else = softfloat. Differential test `xp/bench/x87diff.asm` (random 80-bit operands incl. denormal/inf/NaN/unnormal/zero x 12 precision/rounding modes x 30 ops): 7.2M ops, checksum F5042A13 identical fast vs `XPQEMU_NO_HX87=1`. Benchmarks: x87add 65.3->21.7 ns/iter (3.0x), x87div 153.9->33.5 (4.6x). **Bug found in stock QEMU softfloat:** `fld m32/m64` rounded the loaded value to the control-word precision (24/53-bit PC) and set inexact; real x87 loads are exact. Matters for XP (Direct3D uses 24-bit PC). Fixed in the fallback (`fx_exact_conv*`), so softfloat reference checksums 09345915/73044A54 are obsolete. Wrong-reference lesson: compare the SAME binary with the switch off, not an old build. Also added: Codacy badge (owner enabled Codacy), reference PCBox PR #202. |
| (this push) | Token-optimised workflow (owner request): `scripts/session_status.sh` + `.claude/settings.json` hooks (UserPromptSubmit, SessionStart/compact) report HEAD's CI run and Codacy issues once, then stay silent; CI now uploads Linux and Windows binary artifacts (+ pc-bios, MSYS2 DLLs) and runs the x87 differential test (`xp/bench/x87cmp.sh BUILD_DIR`, now portable); `CLAUDE.md` created; `.gitignore` covers `.claude/settings.local.json` and `.env*`; docs rule 7 rewritten (no local builds, no polling after push). Notes: `app.codacy.com` is unreachable from the Claude sandbox (HTTP 000) so the Codacy part is untested from here and fails soft; `.claude/rules/agents-md.md` imports upstream's AGENTS.md (AI-content policy for UPSTREAM contributions) - clarified in CLAUDE.md. Secrets were pasted in chat by the owner: rotate them. |
| (this push) | Owner tested the Windows artifact: `-display sdl` rejected (CI built without SDL2 -> configure silently disabled it) and `bios-256k.bin` not found. Causes: (1) CI never installed SDL2 -> added libsdl2-dev / mingw SDL2 and `--enable-sdl` (configure now fails loudly if SDL is missing); (2) QEMU's default firmware search is only the configured datadir relocated to the exe dir, which on Windows means firmware directly BESIDE the exe, but our zip had a `pc-bios/` subfolder -> added `qemu_get_exec_dir()` and default search of `<exe dir>/pc-bios` and `<exe dir>` in `util/datadir.c`. CI now smoke-tests the packaged layout from `dist/` with no `-L` (incl. `-display help` must list sdl). README Download section documents layout, SDL/VNC, the pc-bios fallback. Could not inspect the artifact from the sandbox (artifact download redirect is blocked). |
| (this push) | Fixed CI `Package` regression in `.github/workflows/build.yml` (Linux + Windows): `build/pc-bios` only held `keymaps/` and `optionrom/`, missing all `.bin`/`.rom` firmware blobs in `pc-bios/`, which caused `Smoke test the packaged layout (no -L)` to fail with `could not load PC BIOS 'bios-256k.bin'`. Now packages `pc-bios/*.bin`, `pc-bios/*.rom`, and `pc-bios/keymaps` cleanly. T01 part 2c-1: removed dead above-4G / `max_ram_below_4g` / `pci_hole64` / `gigabyte_align` / `AMD_HT_*` / `pc_init1` memory-split machinery across `pc_piix.c`, `pc.c`, `x86.c`, `acpi-build.c`, `i440fx.c`, `pc.h`, `x86.h`. |
| 352c0ef | T04 follow-up (2026-11-17): added legacy XMM inline lowering for PUNPCKLBW/HBW/LWD/HWD/LDQ/HDQ/LQDQ/HQDQ, PACKSSWB/PACKUSWB/PACKSSDW and SHUFPS using existing x86 TCG vector opcodes. MMX and VEX/YMM helper paths remain unchanged. Added multi-sector boot test and Linux/Windows CI checks. CI validation and performance measurements pending. |
| 746dae5 | T04 CI fix and validation (2026-11-17): removed duplicate `BINARY_INT_SSE` emitter invocations for `PUNPCKLQDQ`/`PUNPCKHQDQ`, leaving the newer vector-backed emitters active. Linux + Windows run 44156398064 passed full builds, Checks (including `shuffletest.asm` and `packunpacktest.asm`, both printing `OK`), x87 differential tests, packaging, and packaged-layout smoke tests. Performance benchmark remains outstanding. |

## Session 2 (ChatGPT), 2026-11-17 - T04 benchmark automation

| commit | what |
|--------|------|
| 069b1f0 | Added Linux CI A/B benchmark for the isolated PSHUFD loop against an empty-loop control, using 300M iterations and minimum-of-three timings for each image. |
| c2a2428 | Download the pre-T04 Linux build artifact from run 44148654097 so baseline and current QEMU can be measured on the same runner. |
| e79d5b3 | Added `bc`, required by `xp/bench/run.sh`, to Linux CI dependencies. |
| a8cf18c | Hardened `xp/bench/run.sh`: only the expected `isa-debug-exit` status is accepted; timeout, unexpected exit status, or stderr aborts the measurement. |
| 0766854 | Updated T04 task notes to describe the automated measurement path. |

The workflow reports CPU model and runner details, measures baseline and current
PSHUFD images plus matching empty-loop controls, and prints net times and relative
change. **No performance result has been assumed or recorded yet**; the benchmark
must complete successfully and its output must be reviewed before T04 is marked
done. Per HANDOFF section 7, do not poll Actions after the push.

| (this commit) | T04 benchmark measurement correction (2026-11-17): successful CI runs showed that the 300M-iteration PSHUFD and empty-loop measurements were rounded to 0.01 s before subtraction, yielding 0.00 s or negative net times. These are invalid measurements, not evidence of a speedup or regression. Increased the loop to 1B iterations, raised the sample count from 3 to 5, retained six decimal places through subtraction, and made CI fail explicitly if either net time is non-positive. The valid A/B result is still pending. |


## Session 3 (ChatGPT), 2026-11-18 - Correct T04 benchmark methodology

The first 1B-iteration CI attempt still produced negative values after subtracting
the empty-loop runtime (PSHUFD: 3.609034 s baseline / 3.199697 s current;
empty control: 4.291559 s / 4.462503 s). This demonstrates that the empty loop
is not a stable additive overhead for this TCG benchmark: its translation-block
shape and dispatch behavior differ from the PSHUFD workload. The subtraction
method is invalid; these numbers do not establish a speedup or regression.

| commit | change |
|--------|--------|
| 733fdf9 | Replaced the invalid empty-loop subtraction with a direct comparison of identical 300M-iteration PSHUFD images on baseline/current binaries. |
| c1876f4, 105fd86, 75ce168 | Changed `xp/bench/run.sh` to report the median of five successful runs; corrected the output formatting. |
| c84d38c | Updated the benchmark README with the valid raw-runtime comparison method. |
| b5af358 | Updated T04 task notes; result remains pending until the corrected CI measurement is reported. |
| 5e4f480 | Updated HANDOFF benchmark instructions to match the corrected method. |

The revised CI benchmark performs five runs per binary at 300M iterations and
compares raw median runtimes. It no longer builds or runs the empty-loop image,
and should take materially less time than the 1B-iteration, four-series benchmark.
No performance result is recorded yet. Per HANDOFF section 7, do not poll CI
after pushing.


## Session 4 (ChatGPT), 2026-11-18 - T04 Linux benchmark result

The owner supplied the successful Linux CI benchmark output (AMD EPYC 9V74,
Ubuntu 24.04 Azure runner; same 300M-iteration PSHUFD image; five runs per
binary; median timing):

| build | median |
|-------|-------:|
| Pre-T04 baseline (`c5ef98e`) | 3.083675 s |
| T04 | 3.166926 s |

Relative change: -2.70% (0.974x; T04 was 0.083251 s slower in this run).
This isolated microbenchmark does not show a speedup. The small difference from
one shared CI runner is not conclusive evidence of a general regression.
Recorded the result in `xp/bench/README.md`, `TASKS.md`, and `HANDOFF.md`.
Linux benchmark result is accepted; Windows CI is only needed for the existing
cross-platform build/functional validation, not to complete this Linux timing.
T04's inline lowering and functional coverage are complete; broader SSE2
performance is outside this isolated PSHUFD measurement.

## Session 5 (ChatGPT), 2026-11-18 - T13d legacy XMM register caching

| commit | change |
|--------|--------|
| c272958 | Implemented per-TB caching for eligible legacy XMM register-only operations using TCG EBB vector temporaries. Added lazy loads, dirty tracking, and flush/free barriers before non-cacheable instructions, exception generation, and TB exits. MMX, VEX/YMM, memory operands, and unsupported vector operations retain their existing paths. Added a MOVD cache-barrier case to `shuffletest.asm` and Linux CI A/B automation. |
| 7c81602 | Added T13d to `TASKS.md` as paused pending CI review and benchmark output. |
| 45bd37c | Updated `HANDOFF.md` with the cache design and pending validation state. |
| 060c995 | Documented the T13d A/B benchmark in `xp/bench/README.md`. |

The Linux workflow compares the same 100M-iteration `sse2.asm` workload on
the pre-cache artifact from run 44156398064 and the current build, using the
median of three runs. No local build or test was run, and no CI result was
polled. The benchmark result is not yet known; no speedup or regression is
claimed. Next: review the owner's CI feedback and Linux benchmark output,
fix any failures, then interpret the measured result.

## Session 6 (ChatGPT), 2026-11-18 - T13d Linux benchmark result and README cleanup

The owner supplied the successful Linux CI A/B output for the same
100M-iteration `sse2.asm` workload, comparing the pre-cache artifact from
run 44156398064 with the XMM-cache build (median of three runs):

| build | median |
|-------|-------:|
| Before XMM cache | 0.877822 s |
| After XMM cache | 0.872720 s |

Relative change: +0.58% (1.006x; 0.005102 s faster). This is a marginal
difference on a shared CI runner and should be treated as effectively
near-neutral, not as a meaningful demonstrated speedup or a broad performance
conclusion. Windows build/functional confirmation remains separate.

| commit | change |
|--------|--------|
| cfb4411 | Recorded the T13d Linux result and updated its task status to paused pending Windows CI confirmation. |
| ff09ac7 | Removed the redundant Development status section from the public README, as requested by the owner. |
| 445300a | Recorded the T13d timing and interpretation in `xp/bench/README.md`. |
| e54dca9 | Updated HANDOFF.md with the benchmark result and current validation state. |

No local build/test was run and no CI was polled. Next: review Windows CI
functional/build feedback when supplied. Given the near-neutral result, do not
claim that XMM caching materially improves performance; if pursuing further
speedups, profile the generated host code and target the measured register
state traffic rather than expanding the cache blindly.
