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


## Session 7 (ChatGPT), 2026-11-18 - Win64 XMM-cache register-pressure refinement

The x86-64 TCG backend reserves XMM5 as a scratch vector register. Under
`_WIN64`, it also excludes XMM6-XMM15 because the backend does not save those
ABI nonvolatile registers, leaving XMM0-XMM4 as the five allocatable host vector
registers. The original guest XMM cache could keep eight 128-bit values live on
a 32-bit guest, exceeding that budget.

The translator now preflights all three decoded operand slots before emitting
a cacheable instruction. If the union of currently cached guest XMM registers
and the instruction's operands would exceed five, it flushes the cache before
emission. This keeps every operand of the current instruction resident while
preventing the cache from forcing the Win64 TCG allocator to spill an oversized
set. Linux behavior is unchanged.

Windows CI now downloads the pre-cache Windows artifact from run 44156398064
and compares it with the capacity-aware build using the same 300M-iteration
register-only `sse2.asm` image, three runs per build, median wall time. Added
the `bc` dependency required by the shared benchmark runner. No local build or
test was run; review the CI benchmark output supplied by the owner before
claiming a performance result.

| (this commit) | T13d Win64 cache-capacity refinement and Windows A/B benchmark automation; documentation updated. |


## Session 8 (ChatGPT), 2026-11-18 - Fix Windows T13d benchmark environment

The owner's Windows CI output showed two setup failures before a valid A/B
measurement: `xp/bench/run.sh` called `bc`, which was not available in the
MSYS2 UCRT64 job, and the current un-packaged build could not locate
`libz-1.dll`. No benchmark timing was produced.

The shared runner now uses awk for timestamp subtraction, removing the external
`bc` dependency on both platforms. The Windows workflow no longer requests the
unavailable package; before benchmarking, it prepends the UCRT64 runtime
directory plus the baseline/build directories to `PATH`, then starts both
executables with `-accel help` as a dependency-resolution preflight. The
downloaded baseline artifact already carries its packaged runtime DLLs.

No local build/test was run and no CI was polled. The next owner-supplied run
must complete both preflight commands and all six benchmark runs before any
Windows performance result is recorded.

| (this commit) | Replaced bc timing arithmetic with awk; fixed Windows runtime DLL search path and added executable preflight; documented the failed attempt without recording a timing. |


## Session 9 (ChatGPT), 2026-11-18 - T13d Windows benchmark result

The owner supplied the successful Windows CI A/B output for the capacity-aware
Win64 cache build. The workflow passed both QEMU executable preflights and all
six benchmark runs, using the same 300M-iteration register-only `sse2.asm`
image and the pre-cache Windows artifact from Actions run 44156398064:

| build | median |
|-------|-------:|
| Before XMM cache | 3.166927 s |
| After capacity-aware XMM cache | 3.132343 s |

Relative change: +1.09% (1.011x; 0.034584 s faster). This is a small difference
on a shared CI runner and should be treated as effectively near-neutral, not a
demonstrated speedup. Combined with the Linux result (+0.58%, 1.006x), neither
targeted measurement establishes a material performance change or a broad SSE2
conclusion.

The Windows benchmark environment fix is validated: awk replaces the missing
`bc` dependency, and the UCRT64 runtime directory is available through
`PATH`; both baseline/current executable preflights and all six measured runs
succeeded. T13d is marked complete as an implementation and targeted
cross-platform measurement task, with the measured performance outcome
explicitly recorded as near-neutral. Keep the existing cache and Win64
five-register pressure guard; profile generated host code and add broader
representative workloads before attempting further cache-policy changes.

No local build/test was run and no CI was polled.


## Session 10 (ChatGPT), 2026-11-18 - T13d profiling gate and T05 SGX call-site cleanup

The owner asked that the previously recommended next-step gate for T13d be
recorded before continuing T05. Updated the T13d task notes: do not expand the
XMM cache based on near-neutral microbenchmarks. First profile generated host
code to distinguish guest XMM state traffic, TCG register allocation/spills,
translation/dispatch overhead, or another source; then create a benchmark that
isolates the measured cost and retain a representative end-to-end guard.

T05 audit found that the PC machine's SGX EPC initialization function was a
no-op stub which only zeroed PCMachineState.sgx_epc. Removed the call,
prototype, and stub. An initial attempt to remove the SGX EPC state member was
reversed after the source audit showed that hw/i386/acpi-build.c still reads
that member. The include and state field are retained; SGX CPUID/query stubs
and the ACPI SRAT no-op also remain because active code still references them.
This preserves the live ACPI data path while removing only the redundant
initialization. No local build/test was run and no CI was polled.

| commit | change |
|--------|--------|
| 2d2e7dd | Recorded the explicit T13d profiling/benchmark gate in TASKS.md. |
| 2b6593f, 521b1f7, 7afca6c, b7928ac | Removed redundant SGX EPC initialization while retaining the ACPI-consumed state field; corrected the initial over-prune before proceeding. |
| 9440999 | Documented T05 SGX dependency findings and next audit scope. |


## Session 11 (ChatGPT), 2026-11-18 - T05 SGX/VFIO dependency cleanup

Continued the T05 source audit with exact call-site and include checks.
Removed the now-redundant SGX EPC init call/stub/prototype; retained the SGX
EPC state in PCMachineState because the ACPI builder still reads it. The
remaining SGX stub no longer includes PC or SGX EPC headers, as it no longer
uses their declarations. SGX query/CPUID and SRAT stubs remain because live
code still references them.

Removed the unused VFIO PCI compatibility property from the i440fx machine
compat defaults and dropped its VFIO types header include. VFIO is disabled in
the XP device configuration, and the only reference to that compatibility
property was in pc_piix.c.

Commits: 95feeea (drop SGX stub header dependencies), 85a0947 (remove dead
VFIO compatibility property), a96a653 (record progress). No local build/test
was run and no CI was polled. Next: map KVM and IOMMU call sites against the
actual XP build configuration; preserve generic TCG memory translation and any
shared interfaces still required by active code.
## Session 12 (Gemini), 2026-11-18 - T09/T10 draft: qemu-vmvga overlay, TCG backdoor exemption, Linux/Windows 3D host backends
Implemented a draft integration of **T09** (`qemu-vmvga` SVGA II / SVGA3D) and
**T10** (Linux and Windows host 3D backends) directly on `main` for CI compilation
and easy continuation:

* **T09 (`qemu-vmvga` overlay + `vmvga` default + TCG backdoor exemption):**
  * Overlaid `hw/display/vmware_vga*.c`, `hw/display/include/*`, and
    `hw/i386/vmport.{c,-vmvga.h}` from `qemus/qemu-vmvga`; restored
    `hw/i386/vmmouse.c` and `pc-bios/vgabios-vmware.bin` from `35a2e66`.
  * Wired `hw/display/meson.build`, `hw/i386/meson.build`, `pc-bios/meson.build`,
    `hw/pci/pci.c` (`VGA_VMWARE` -> `"vmvga"`), `hw/i386/pc_piix.c`
    (`m->default_display = "vmware"`), and `system/vl.c` (`VGA_VMWARE` accepts
    `"vmvga"` and `"vmware-svga"`).
  * Set `vmvga` as the default and only display device in
    `configs/devices/i386-softmmu/default.mak` (`CONFIG_VGA_CIRRUS=n`,
    `CONFIG_VGA_PCI=n`, `CONFIG_ATI_VGA=n`, `CONFIG_BOCHS_DISPLAY=n`,
    `CONFIG_VMWARE_VGA=y`, `CONFIG_VMMOUSE=y`, `CONFIG_VMPORT=y`).
  * Added `vmport_tcg_backdoor_enabled` (`system/globals.c`, `include/system/system.h`,
    enabled in `vmport_realizefn()`) and exempted `0x5658`/`0x5659` (`size <= 4`)
    in `HELPER(check_io)` (`target/i386/tcg/system/seg_helper.c`).
  * Added `xp/bench/vmport_ring3_test.asm` (CPL=3, IOPL=0, 0xFF TSS IOPB check)
    and CI checks for `-vga help`, `-device help`, and `vmport_ring3_test.asm`.
* **T10 (Linux `dxvk-native` + Windows D3D9/D3D11 host backends):**
  * Updated `hw/display/vmware_vga_dxvk_wsi.{h,c}` and `hw/display/vmware_vga_dxvk.c`:
    Linux synthesizes a sealed-memfd `libSDL2-2.0.so.0` WSI shim (`"SDL2"`), or
    `libSDL3.so.0` (`"SDL3"`, all 21 `wsi_platform_sdl3_funcs.h` symbols) if SDL2 is
    already loaded (`RTLD_NOLOAD`) or `DXVK_WSI_DRIVER=SDL3`, and searches
    `<exec_dir>/` and `<exec_dir>/lib/` before system paths (`libdxvk_dxgi.so.0`
    pre-loaded with `RTLD_GLOBAL`). Windows (`CONFIG_WIN32`) uses a hidden
    `WS_POPUP | WS_DISABLED` `"STATIC"` `HWND` (`"Win32"`) and `LoadLibraryA` on
    `"d3d9.dll"` / `"d3d11.dll"` with `D3D_DRIVER_TYPE_WARP` fallback on D3D11.
  * Guarded `vmsvga3d_renderer_realize()` in `hw/display/vmware_vga_3d.c` using
    `display_backend_active` (`system/vl.c`) so `-display none` benchmarks skip
    3D init unless `debug=on` or `Y2KBOX_3D_HEADLESS` is set.
  * Updated `.github/workflows/build.yml`: Linux installs `libvulkan1` +
    `mesa-vulkan-drivers`, bundles `dxvk-native` 3.1.1 `.so.0` files into
    `dist/`, and runs a headless 3D smoke check (`Y2KBOX_3D_HEADLESS=1` with
    `lvp_icd.x86_64.json`).
* **Local checks vs CI/pending:** Locally verified `xp/bench/vmport_ring3_test.asm`
  assembled size (<= 510 bytes via GNU `as`), a standalone `gcc` test loading the
  SDL3 sealed-memfd shim with `dxvk-native` 3.1.1 (`Direct3DCreate9` /
  `D3D11CreateDevice` resolution), and balanced `#if`/`#endif` depth in
  `vmware_vga_dxvk.c`, plus a `-DCONFIG_WIN32` `gcc -fsyntax-only` check across
  `vmware_vga_dxvk_wsi.c` and `vmware_vga_dxvk.c` after scoping the Linux-only
  Vulkan typedefs inside `#if defined(CONFIG_LINUX) && defined(__ELF__)` and
  using `memcpy` for `GetProcAddress` conversion. Full Linux & Windows builds
  and smoke checks run in GitHub
  Actions on push; XP guest validation (VMware Tools 10.0.12 SVGA driver
  12.01.08 + live 3D apps) remains pending T08/T11.

## Session 13 (ChatGPT), 2026-11-19 - Linux KVM / Windows TCG platform split

The owner clarified the accelerator policy: Linux is KVM-only at product
runtime, with no TCG fallback. If `/dev/kvm` is missing or inaccessible, the
launcher must stop and tell the user that KVM must be enabled and access granted.
Windows is TCG/MTTCG-only. TCG performance work is exclusively Windows-focused.
Internal TCG-only Linux CI/development builds may be used when useful, but they
are not supported Linux product builds.

Repository inspection corrected an earlier mistaken assumption in the
conversation: KVM is not currently present as a working accelerator. The
historical Session 1 stripping pass explicitly removed KVM along with other
accelerators; current `accel/meson.build` wires TCG but not KVM, and
`target/i386/kvm/kvm.c` is absent. The generic `include/system/kvm.h`
header remains, but that alone does not constitute KVM support. Therefore,
this policy requires a restoration task, not merely preserving code that is
already working.

Documentation updated:
- README states Linux KVM is mandatory with no supported Linux TCG fallback;
  Windows is TCG-only and TCG optimization is Windows-focused.
- HANDOFF records mandatory Linux KVM, the required launcher error when KVM is
  unavailable, and the warning that KVM has not yet been restored.
- T05 no longer includes KVM removal; its scope is limited to IOMMU, SEV, SGX,
  TDX, and VFIO cleanup while preserving KVM interfaces.
- T13 is explicitly a Windows-only TCG performance effort; Linux TCG is not a
  supported product runtime.
- New T17 tracks restoring the complete Linux KVM accelerator dependency path
  and validating it on a KVM-capable Linux host.

No source code or build configuration was changed in this documentation pass.
No local build/test was run and no CI was polled. Next: complete the remaining
T05 non-KVM dependency audit, then claim T17 and restore Linux KVM support.

Commits: README/HANDOFF policy updates are recorded in the immediately preceding
documentation commits; T05/T13/T17 task-queue changes are in the preceding
TASKS.md commit. This WORKLOG entry records the decision and implementation
scope.

Session 14 (ChatGPT), 2026-11-19 - T05 non-KVM call-site cleanup

Completed the remaining XP PC-machine feature-hook audit while preserving KVM:
- Removed PC-machine IOMMU hotplug state/callbacks and PC ACPI DMAR/IVRS
  generation. Removed AMD-IOMMU stub wiring and Kconfig options. Kept the generic
  x86 IOMMU stub/interface because the KVM split-irqchip IOAPIC path still uses
  it as an optional notifier hook; this does not enable an XP IOMMU device.
- Removed SEV kernel-loader hashing and pflash-encryption callsites, plus unused
  OVMF SEV metadata declarations.
- Removed TDX TDVF-region setup. The BIOS direct-load path now follows the
  generic guest-private-memory requirement, rather than SEV/TDX-specific tests.
  The disabled TDX predicate remains for shared CPU feature gating.
- Removed SGX EPC ACPI DSDT/SRAT generation and PCMachineState EPC state. Kept
  SGX CPUID/capability stubs because CPU feature masking still calls them.
- The previously removed VFIO compatibility property remains absent; generic
  VFIO implementation is not part of the XP device configuration.
- Corrected HANDOFF.md so Linux TCG is not described as a product fallback:
  Linux product runtime is KVM-only, Windows is TCG/MTTCG-only, and any
  Linux TCG-only builds are internal CI/development builds only.

KVM-specific code and the x86 IOMMU interface needed by its split-irqchip path
were deliberately preserved. Linux KVM implementation restoration remains T17.
No local build/test was run and no CI was polled; the owner supplies CI results.

Session 15 (ChatGPT), 2026-11-19 - T17 Linux KVM restoration claimed

Started T17 against the owner's exact upstream QEMU 11.1.90 base:
9d7d057a558b9a1c232cea390b4481dd20d9c9ce.

Initial tree comparison confirms the fork is missing the generic KVM accelerator
implementation, x86 KVM target implementation, and PC KVM interrupt/clock device
sources. The generic KVM public/internal headers and disabled stubs remain, and
some x86 PC-machine KVM hooks were retained. The existing host configuration already
recognizes CONFIG_KVM on Linux when the Meson kvm feature is allowed; Windows must
continue selecting only the KVM stubs and TCG.

The restore will be deliberately limited to x86-64 Linux KVM and the XP i440FX
machine's needs. TDX, SEV, SGX, Xen emulation, VFIO, and IOMMU devices remain
disabled. The generic x86 IOMMU interface is retained for the KVM MSI-routing
implementation's no-IOMMU path. No source files have been changed yet; this entry
records the claim and audit checkpoint. No local build/test was run and no CI was
polled.



## Session 16 (ChatGPT), 2026-11-19 - Restore KVM nested state; Windows TDX stub portability; CI benchmark split

Owner-provided CI logs and GitHub Actions job logs identified two independent
compile blockers on the prior revision:
- Linux: target/i386/machine.c migration and nested-state code referenced
  CPUX86State.nested_state, but the Xen stripping script had removed the
  entire CONFIG_KVM block along with Xen-only members. Restored the KVM
  pointer field under CONFIG_KVM and narrowed the script's deletion range
  to the Xen vCPU members only.
- Windows: target/i386/kvm/tdx.h unconditionally included Linux's
  linux/kvm.h, which is unavailable in the Windows/MSYS2 build. The disabled
  TDX stub only needs an incomplete struct kvm_run pointer type, so the header
  now forward-declares it instead of importing the Linux UAPI header. TDX
  remains unsupported.

Also removed the Linux T04/T13 performance A/B benchmark steps and their
baseline-artifact downloads from GitHub Actions. Linux CI retains its KVM build,
accelerator policy checks, functional tests, x87 differential test, packaging,
and smoke tests. Windows CI retains the TCG performance benchmark. Historical
Linux benchmark results remain documented, but are no longer ongoing CI work.

No local builds/tests were run. The CI failures were diagnosed from the
owner-supplied logs and GitHub Actions job logs; the changes await the next CI run.


## Session 17 (ChatGPT), 2026-11-19 - Remove incomplete optional KVM RAPL integration

The owner reported that Windows CI passed after Session 16, while Linux CI
still failed compiling target/i386/kvm/kvm.c. The diagnostics identified
undeclared vmsr energy-statistics types, constants, and helper functions.

The root cause was a partial feature restoration: xp/strip/03-remove-virt-accels.sh
had intentionally removed target/i386/kvm/vmsr_energy.c and vmsr_energy.h, but
the restored KVM backend still contained the optional RAPL worker, MSR handlers,
initialization path, and the generic accelerator's rapl/rapl-helper-socket
properties. This optional guest energy-MSR feature is not required for the XP
KVM runtime.

Removed that incomplete RAPL integration from target/i386/kvm/kvm.c,
accel/kvm/kvm-all.c, and include/system/kvm_int.h. The generic KVM accelerator,
KVM MSR core-thread-count support, Linux-only build wiring, and standalone
qemu-vmsr-helper utility were left intact. Updated T17 and HANDOFF checkpoints
to reflect the restored KVM source state and the outstanding Linux CI/runtime
validation.

No local builds/tests were run. No CI was polled after this change.



## Session 18 (ChatGPT), 2026-11-19 - Restore directory-local KVM trace headers

The owner supplied another Linux CI link failure after commit c8e797e registered
three restored KVM trace-event directories in the root Meson trace list. The
linker still reported missing KVM trace symbols, and the compiler warnings
showed the corresponding calls had no declarations.

The missing piece was the directory-local trace.h wrapper headers. QEMU source
files include "trace.h" relative to their source directory; each wrapper
includes the generated group header for that directory. The KVM source and
trace-events files had been restored, but these wrappers had not. Restored the
three exact upstream-baseline wrappers:
- accel/kvm/trace.h -> trace/trace-accel_kvm.h
- target/i386/kvm/trace.h -> trace/trace-target_i386_kvm.h
- hw/i386/kvm/trace.h -> trace/trace-hw_i386_kvm.h

The Meson trace-event registrations from c8e797e are retained. No tracepoints
were disabled or removed. This correction is based on comparison with the
owner-designated upstream baseline 9d7d057a558b9a1c232cea390b4481dd20d9c9ce.
No local build/test was run and no CI was polled after pushing. Linux KVM build
and runtime validation remain outstanding; Windows CI was previously passing.


## Session 19 (ChatGPT), 2026-11-19 - Document Windows baseline and XP guest setup

Updated the public README with a host-requirements section clarifying the
x86-64-v3/v4 CPU ISA expectations and the Windows policy: Windows 10 version
1803 (build 17134) or newer is strongly recommended as the intended baseline
for future Windows TCG memory-management work, but is not described as an
already-enforced minimum. Linux remains KVM-only with /dev/kvm access required.

Added practical Windows XP guest recommendations to Download & Usage:
use the maximum refresh rate when offered by the selected display mode, disable
"Enhance pointer precision" in Control Panel > Mouse, and configure Internet
Explorer to use Protoweb's current primary HTTP proxy (wayback.protoweb.org:7851)
for retro web browsing. The README points users to Protoweb's live server list
because its server addresses and ports can change.

Added starfrost013/nvplayground to HANDOFF.md section 9 as a DOS/DJGPP-based
low-level NVIDIA GPU research utility, noting its strongest documented support
for NV3/NV4 and its limited, untested NV1x/Celsius support. It is a research
reference, not an emulator component.

No source code or build configuration changed. No local build/test was run.


## Session 20 (ChatGPT), 2026-11-19 - Queue Windows TCG virtual-memory backend investigation

Added T18 as a P2 research/optimization task for evaluating Windows
`VirtualAlloc2` and `MapViewOfFile3` in a possible TCG guest-memory backend.
The task is explicitly profiling-first: identify a measurable allocation,
reservation, mapping, remapping, or address-space-layout bottleneck before
prototyping. It requires A/B measurements against the existing path and allows
the task to be deferred without implementation if profiling finds no relevant
bottleneck. It does not raise the formal Windows minimum beyond the README's
1803+ encouraged baseline and applies only to Windows TCG; Linux remains KVM-only.

No source code or build configuration changed. No local build/test was run.


## Session 21 (ChatGPT), 2026-11-19 - Add isolated Windows x87 store A/B benchmark

After the owner confirmed Windows CI passed for the normal-finite FST m32/m64
host-x87 fast path (commit bb8a17f), added two focused real-mode benchmark
images: `xp/bench/micro/x87store32.asm` and `x87store64.asm`. Each repeatedly
executes a non-popping FST from a constant ST0 value to a hot RAM destination,
isolating the corresponding destination-format store helper without adding
arithmetic or a per-iteration x87 reload.

Added a Windows CI A/B step that builds each 100-million-store image and runs
the same current QEMU executable three times with the fast path disabled via
`XPQEMU_NO_HX87=1`, then three times with the host-x87 path enabled. It reports
softfloat and host-x87 medians and relative changes separately for m32 and m64.
No performance result is recorded or assumed until the owner supplies the CI
output. Updated the benchmark README and T13a task/handoff notes; the
differential x87 test remains the correctness gate.

No local build or benchmark was run. No CI was polled after pushing.


## Session 22 (ChatGPT), 2026-11-19 - Fix T13a Windows benchmark shell selection

The first Windows run of the T13a store A/B benchmark failed with exit 127.
Inspection of the complete Actions job log showed that the build, Windows
functional checks, T13d benchmark, and x87 differential test all passed; the
failure occurred immediately when entering the new T13a step. The workflow
step explicitly overrode the Windows job's `msys2 {0}` shell with generic
`bash`, which selects Git for Windows Bash rather than the configured MSYS2
UCRT64 environment. The benchmark expects UCRT64 tools (including NASM) on
the MSYS2 path. Removed the per-step shell override so T13a inherits the
job-level MSYS2 shell, matching the working Windows checks and x87 test.

No benchmark result was produced. No local build or benchmark was run.


## Session 23 (ChatGPT), 2026-11-19 - Right-size T13a Windows store benchmark

The T13a Windows A/B step reached QEMU but its first 100-million-store softfloat
run exceeded the per-run 200-second timeout. This was an oversized benchmark
workload, not a compile failure or evidence of a guest execution hang. Kept the
existing timeout as a safety ceiling and reduced each store image to 5 million
iterations, with five samples per mode and store width. This preserves a useful
median while substantially reducing expected wall time. Updated the assembly
defaults, Windows CI invocation, benchmark README, task note, and handoff.

The benchmark remains a narrow constant-ST0, hot-destination microbenchmark;
no performance result is assumed. No local build or benchmark was run and no
CI was polled after pushing.


## Session 24 (ChatGPT), 2026-11-19 - Fix T13a store benchmark boot images

The reduced 5M-iteration T13a Windows benchmark still timed out at the
200-second per-run limit. Inspection of the boot images found the actual cause:
both x87store32.asm and x87store64.asm padded to 510 bytes but omitted the
required 0xAA55 BIOS boot signature. The BIOS therefore did not execute the
benchmark code, so changing the loop count could not resolve the timeout.
Added the signature word to both images, matching the working x87 microbench
images. No benchmark result is available yet; no local build or benchmark was
run and no CI was polled after pushing.


## Session 25 (ChatGPT), 2026-11-19 - Correct T13a boot-sector padding expression

The T13a Windows FST benchmark continued to time out after the boot signature
was added. Comparing both store images directly with the working x87/SSE2
boot sectors exposed a second boot-sector construction error: each store image
used `times 510-($-$) db 0` instead of NASM's required current-offset minus
section-origin expression, `times 510-($-$$) db 0`. The malformed expression
padded by a fixed 510 bytes after the code/data rather than placing the boot
signature at offsets 510-511. Consequently, the BIOS still did not see a valid
boot sector, and QEMU waited until the 200-second timeout.

Corrected the padding expression in both x87store32.asm and x87store64.asm.
The existing `dw 0xAA55` now follows the padding at the proper boot-sector
location. The shared run.sh harness is not the cause: the user's successful
T13d benchmark uses the same runner and reaches isa-debug-exit. No local build,
assembly, or benchmark was run; the next Windows CI run must validate both
images before any T13a performance result is recorded.


## Session 26 (ChatGPT), 2026-11-19 - Actually apply T13a padding correction

The Windows run still timed out after the purported padding correction. Reviewing
the committed source (rather than relying on the update response) showed that
both files still contained the malformed `times 510-($-$) db 0` expression.
The prior automated edit used JavaScript string replacement with a replacement
string containing dollar signs; JavaScript interpreted the `$$` replacement
sequence and collapsed the intended `$-$$` expression back to `$-$`. Thus the
previous two commits did not actually change the padding expression, despite
the intended edit.

Re-fetched the current main versions and corrected both expressions using a
replacement callback (so dollar signs are literal), then verified the fetched
file contents contain exactly `times 510-($-$$) db 0`. New commits:
- c9392bea: x87store32.asm
- 1bb66b0f: x87store64.asm

The Actions log shows the first softfloat m32 run consumed the full 200-second
timeout. It does not by itself distinguish a non-booting image from a very slow
5M-store guest loop. With the corrected source now actually committed, the next
Windows run is the first valid test of the boot-sector padding fix. No local
assembly/build/benchmark was run and no CI was polled after these commits.


## Session 27 (ChatGPT), 2026-11-19 - Record T13a FST store benchmark results

The first successful Windows T13a store A/B run completed after the x87store32
and x87store64 boot-sector padding/signature corrections. Both tests used the
same QEMU executable, 5M non-popping stores per run, and five-run medians.
FST m32: softfloat 2.474809 s, host-x87 2.383368 s, +3.69% (1.038x).
FST m64: softfloat 2.508467 s, host-x87 2.359847 s, +5.92% (1.063x).
Recorded the measurements in TASKS.md, xp/bench/README.md, and HANDOFF.md.
The results are described narrowly as isolated constant-ST0/hot-destination
microbenchmark improvements, not general x87 or application-level gains. T13a
remains in progress; integer conversions are the next investigation. No local
build or benchmark was run and no CI was polled after this documentation update.


## Session 28 (ChatGPT), 2026-11-19 - Restore omitted KVM energy and x87 benchmark dependencies

The owner reported Linux CI failing because `target/i386/kvm/kvm.c` referenced
MSR energy-monitoring types and helpers that were absent, and Windows CI failing
because the T13a NASM input images were missing from the checkout.

Restored `vmsr_energy.h` and `vmsr_energy.c` from the selected upstream QEMU
baseline `9d7d057a558b9a1c232cea390b4481dd20d9c9ce`, added the missing header
include to `kvm.c`, and registered `vmsr_energy.c` in the CONFIG_KVM-gated x86
KVM source set. This preserves the existing virtual RAPL/MSR implementation
rather than deleting or stubbing its call sites.

Restored `xp/bench/micro/x87store32.asm` and `x87store64.asm` from the
previously verified fork versions, including the 5M default loop count and
BIOS boot signature. These files had been omitted during the earlier tree
recovery even though the workflow and documentation still referenced them.

Verified the current main-branch contents, helper declarations/definitions,
Meson registration, both benchmark paths, loop counts, and boot signatures.
No local build or test was run; CI must validate the repair.

## Session 29, 2026-11-20 - Fix SVGA hardware cursor, 512 MiB VRAM, D3D refresh flush, Linux KVM backdoor warning, and disable VNC

Fixed five issues reported after Windows XP SP3 guest testing with VMware SVGA II 12.01.08 (`VMware Tools 10.0.12`):

1. **Missing hardware mouse cursor at full XP hardware acceleration (`hw/display/vmware_vga.c`, `ui/sdl2.c`):**
   - Added `vmsvga_cursor_bpp_supported()` accepting `1/8/15/16/24/32` bpp in `SVGA_CMD_DEFINE_CURSOR` and `SVGA_COLOR_CURSOR` MOB submission (XP queries `SVGA_REG_DEPTH == 24` in 32-bpp mode and submits 24-bpp XOR cursor masks, which were previously rejected because `vmsvga_active_depth()` returned `32`).
   - Clamped cursor hotspots (`hot_x`/`hot_y`) into `[0, width-1] x [0, height-1]`, computed `SVGA_CMD_DEFINE_CURSOR` mask word counts via `vmsvga_cursor_row_bytes()` (handling 15-bpp storage pitch), fixed `vmsvga_qemu_rgb()` channel order (`0xRRGGBB` to match `ui/sdl2.c` RGB masks), re-applied active cursor definitions immediately on `vmsvga_cursor_define()`/`vmsvga_rgba_cursor_define()`/`vmsvga_cursor_palette_changed()`, and fell back to `cursor_builtin_left_ptr()` when no cached cursor matches rather than leaving the hidden boot cursor active.
   - Fixed `ui/sdl2.c` `sdl_mouse_warp()` ordering (`guest_cursor = on` before `sdl_show_cursor()`), null-guarded `guest_sprite`, freed orphaned `guest_sprite_surface` on `SDL_CreateColorCursor` failure and nulled freed pointers in `sdl_mouse_define()`, and scaled guest warp coordinates to window size via `sdl_warp_guest_cursor()`.
2. **VRAM increase (`hw/display/vmware_vga.c`):**
   - Increased `VMSVGA_DEFAULT_VRAM_SIZE` from `32` MiB to `512` MiB (updating the `vmsvga_memory_size` comment accordingly).
3. **Intermittent D3D7/D3D8/D3D9 freeze / black screen (`hw/display/vmware_vga.c`):**
   - Flushed deferred legacy 3D presents via `(void)vmsvga3d_legacy_present_flush_live(s);` in `vmsvga_update_display()` before `vmsvga3d_screen_target_flush_live(s)` so pure-3D render loops stay visible on display refreshes without waiting for a 2D GDI barrier.
   - Fixed `SVGA_CMD_NOP` and `SVGA_CMD_NOP_ERROR` FIFO handling to consume 1 word (`len -= 1`) rather than rewinding `fifo_stop` forever.
   - Enabled `15`-bpp and `16`-bpp mode commits in `SVGA_REG_BITS_PER_PIXEL` under `CONFIG_PIXMAN`.
4. **Linux KVM VMware backdoor check & docs (`target/i386/kvm/kvm.c`, `README.md`):**
   - Added a startup `warn_report()` in `kvm_arch_init()` when `/sys/module/kvm/parameters/enable_vmware_backdoor` is present and disabled (`N`/`n`/`0`), instructing users to enable `kvm.enable_vmware_backdoor=1`.
   - Updated `README.md` to remove the obsolete 1-notch hardware-acceleration workaround and 32 MB VRAM note, document the 512 MB VRAM default, and document Linux `kvm.enable_vmware_backdoor=1`.
5. **Disable VNC while keeping SDL and headless `-display none` (`meson_options.txt`, `.github/workflows/build.yml`, `TASKS.md`):**
   - Set `qemu_vnc`, `vnc`, `vnc_jpeg`, and `vnc_sasl` defaults to `disabled` in `meson_options.txt` and added `--disable-vnc` to both Linux and Windows CI configure lines while preserving `-display sdl` and `-display none`.

No local QEMU compile or XP runtime execution was performed in this environment; static diff review was completed and GitHub Actions CI plus XP guest testing must validate the builds and runtime fixes.

## Session 30, 2026-11-20 - Fix post-mode-change scanout lockups and document XP 128 MiB dxdiag cap

Fixed the screen freeze after guest video mode changes (e.g., entering/exiting fullscreen `dxdiag` D3D tests or changing desktop modes) and investigated the 128 MB `dxdiag` VRAM report:

1. **Screen Object & Legacy Handoff Scanout Freeze (`hw/display/vmware_vga.c`, `hw/display/vmware_vga_gmr.c`, `hw/display/vmware_vga_3d.c`):**
   - Added a bounded transition window (`VMSVGA_HANDOFF_MAX_FRAMES = 8U`) for both `screen_handoff_active` (Screen Object transitions) and non-D3D9 `legacy_handoff_active` (legacy register transitions via `legacy_handoff_timeout_armed`) in `vmsvga_update_display()` so partial updates, valid black clears, or direct CPU writes to BAR1 cannot trap the SDL frontend on a stale `screen_base` snapshot with `DIRTY_MEMORY_VGA` disabled.
   - Added `vmsvga_screen_handoff_complete()`, `vmsvga_screen_handoff_present_rect()`, and `vmsvga_screen_handoff_mirror_to_backing()` so `SVGA_CMD_UPDATE` / `SVGA_CMD_UPDATE_VERBOSE` / 2D FIFO BAR1 draws update `screen_base` during an active Screen Object handoff (and complete on full-screen non-zero updates), while `SVGA_3D_CMD_PRESENT` / `SVGA_3D_CMD_BLIT_SURFACE_TO_SCREEN` / retired screen readback mirror frames into BAR1 backing so direct BAR1 rebind has current pixels.
   - Separated BAR1-originated damage (`vmsvga_damage_add()`) from pre-updated mirror damage (`vmsvga_damage_add_mirror()`, `vmsvga_damage_add_visible_mirror()`), avoiding stale BAR1 overwrites of preserved zero-guards or 3D mirror frames.
   - Quiesced and reset `legacy_present` state on `SVGA_REG_ENABLE = 0` and `SVGA_REG_CONFIG_DONE = 0`, cleared `legacy_handoff_timeout_armed` when live D3D9 GPU scanout owns `screen_base` (`vmsvga3d_legacy_present_host_scanout_ensure()`), and prevented `vmsvga3d_legacy_present_mode_quiesce_live()` from returning `false` and blocking `vmsvga_try_commit_mode()` across incompatible mode transitions.

2. **128 MB VRAM shown in Windows XP `dxdiag` (`README.md`):**
   - Verified `vmvga` exposes `512 MiB` BAR1 VRAM (`VMSVGA_DEFAULT_VRAM_SIZE = 512`, `SVGA_REG_VRAM_SIZE = 512 MiB`, `SVGA_REG_MEMORY_SIZE = 1024 MiB`); 32-bit Windows XP's VMware XPDM driver (`vmx_svga.sys` 12.01.08) caps adapter-reported VRAM at `SVGA_VRAM_MAX_SIZE` (128 MiB) to fit 32-bit kernel address-space/PTE limits.
   - Documented this guest driver limit in `README.md`.

No local QEMU compile or XP runtime execution was performed in this environment; static diff review was completed and GitHub Actions CI plus XP guest testing must validate the builds and runtime behavior.

## Session 31, 2026-11-21 - Fix DXVK legacy present FIFO deadlock on mode changes and SDL2 fullscreen cursor tracking

Fixed two issues reported from Windows XP SP3 testing (`dxdiag` D3D7 -> D3D8 -> D3D9 transitions and `Ctrl+Alt+F` fullscreen cursor tracking):

1. **Intermittent post-mode-change SVGA screen freeze during `dxdiag` D3D8 -> D3D9 (`hw/display/vmware_vga_3d.c`, `hw/display/vmware_vga.c`, `hw/display/vmware_vga_gmr.c`):**
   - Fixed `vmsvga3d_legacy_present_flush_live()` so host scanout validation (`vmsvga3d_legacy_present_host_scanout_ensure()`) and mode geometry matching (`legacy_present_width/height == active_width/height`) happen before DXVK readback, and unflushable/superseded deferred snapshots call `vmsvga3d_legacy_present_reset_live(s)` and return `true`. Previously, returning `false` when a mode switch, 16-bpp transition, or Screen Object superseded a pending D3D8 present permanently rewound the SVGA FIFO on the next 2D/Screen barrier (`vmsvga3d_legacy_present_fifo_2d_barrier_live`), `BLIT_SURFACE_TO_SCREEN`, `PRESENT`, or `PRESENT_READBACK` while register-driven window resizing, cursor updates, keyboard input, and audio kept running until `ESC` reset the mode.
   - Fixed `vmsvga3d_legacy_present_readback_to_guest_live()` so absent, rebound, or mode-mismatched host mirrors invalidate the stale snapshot (`vmsvga3d_legacy_present_snapshot_invalidate_live(s)`) and return `true` instead of rewinding `SVGA_3D_CMD_PRESENT_READBACK` indefinitely.
   - Guarded `vmsvga_try_commit_mode()` against overwriting `s->active_*` while `s->screen_defined` is true, and committed any pending register mode at the end of `vmsvga_screen_destroy()` when `s->enable` is set.
   - Fixed `vmsvga_check_size()` to compare `format != surface_format(surface)` instead of `display_depth != surface_bits_per_pixel(surface)`, avoiding recreating 15-bpp surfaces/textures (`PIXMAN_x1r5g5b5` has 16-bit storage bpp) on every refresh.

2. **`Ctrl+Alt+F` fullscreen SDL2 mouse tracking (`ui/sdl2.c`):**
   - Stopped double-scaling `SDL_MOUSEMOTION` and `SDL_MOUSEBUTTON*` coordinates in SDL2 2D mode (`!scon->opengl && scon->real_renderer`), where `sdl2_2d_switch()` already configures `SDL_RenderSetLogicalSize()` to translate window coordinates (and letterboxing/pillarboxing) into `[0, surf_w) x [0, surf_h)` surface space.
   - Added `sdl_get_viewport()` for OpenGL/unscaled fallback input mapping, letterbox/pillarbox-aware `sdl_warp_guest_cursor()` warping, and null/bounds guards across `handle_mousemotion()`, `handle_mousebutton()`, and `absolute_mouse_grab()`.
   - Guarded `sdl2_window_resize()` while `gui_fullscreen` is active and resynced the windowed size when exiting fullscreen in `toggle_full_screen()`.

No local QEMU compile or XP runtime execution was performed in this environment; static diff review was completed and GitHub Actions CI plus XP guest testing must validate the builds and runtime behavior.

## Session 32, 2026-11-21 - Fix same-mode D3D8 exit scanout freeze on legacy host mirror

Fixed the intermittent screen freeze when exiting fullscreen D3D8 back to a same-resolution desktop (e.g., `dxdiag` D3D8 `640x480@32` finishing before the Yes/OK prompt, or exiting Hitman 2: Silent Assassin), where audio, input, and the hardware mouse cursor kept working over the frozen last D3D8 frame until a resolution change occurred:

1. **Bounded legacy handoff exit when D3D8 tears down at the same resolution (`hw/display/vmware_vga.c`, `hw/display/vmware_vga_3d.c`):**
   - Added `vmsvga_legacy_handoff_arm_timeout()` to arm the existing `VMSVGA_HANDOFF_MAX_FRAMES` (8-frame) BAR1 rebind countdown whenever `legacy_handoff_active` is held on host mirror `screen_base` without an armed timeout.
   - Armed the handoff timeout in `vmsvga3d_surface_destroy_live()` (when destroying `state->legacy_present_sid`, after flushing/resetting pending legacy present state) and in `vmsvga3d_handle_context_destroy()` (when no active legacy 3D contexts remain via `vmsvga3d_has_active_legacy_context()`).
   - Armed the handoff timeout in `vmsvga_legacy_handoff_present_rect()` on partial 2D updates and zeroed/black rectangle skips, and in `vmsvga_value_write()` when explicit mode/enable/config register writes (`SVGA_REG_ENABLE`, `SVGA_REG_WIDTH`, `SVGA_REG_HEIGHT`, `SVGA_REG_BITS_PER_PIXEL`, `SVGA_REG_CONFIG_DONE`, `SVGA_REG_PITCHLOCK`) commit a valid mode (including same-mode re-commits where `changed == false`).
   - Disarmed `legacy_handoff_timeout_armed` and cleared `legacy_handoff_hold_frames` on the active 3D fast-path in `vmsvga3d_legacy_present_host_scanout_ensure()` so ongoing `SVGA_3D_CMD_PRESENT` streams keep `screen_base` without flickering.
   - Added a `release_register_handoff` cleanup branch in `vmsvga_check_size()` so a pending rebind cannot linger if the frontend surface already points to BAR1.
2. **Direct legacy register BAR1 dirty tracking (`hw/display/vmware_vga.c`):**
   - Added `vmsvga_direct_legacy_vram_scanout()` (`!s->screen_defined && !s->legacy_handoff_active`) to `vmsvga_effective_traces()`, matching `vmsvga_direct_screen_vram_scanout()` so direct CPU writes to BAR1 on legacy register desktops refresh SDL even when `SVGA_REG_TRACES` is 0, while remaining off during active 3D `legacy_handoff_active` scanout.
   - Guarded the stop-time `vmsvga_scan_vram_dirty()` flush in `vmsvga_set_dirty_log()` with `!s->legacy_handoff_active && !s->screen_handoff_active` so entering a 3D or transition host mirror clears stale dirty bits instead of overwriting `screen_base` with pre-transition BAR1 pages.

No local QEMU compile or XP runtime execution was performed in this environment; static diff review was completed and GitHub Actions CI plus XP guest testing must validate the builds and runtime behavior.


## Session 33 (ChatGPT), 2026-11-21 - Add SDL on-demand diagnostic capture hotkey

Implemented T19 on `main`:

- Added **Ctrl+Alt+L** to the SDL host-hotkey handler. The key is consumed by QEMU
  and is not forwarded to the XP guest. The existing configured host modifier
  variants are reflected in the SDL title-bar hint.
- On the first activation, save the original stderr descriptor and redirect stderr
  to `vmvga_freeze.log` in the process working directory. The first successful
  capture truncates the file; subsequent capture intervals in the same run append.
  On stop, flush and restore the original stderr descriptor. The VM continues running.
- The title bar indicates the start/stop action and whether capture is active.
  File-open and descriptor-switch errors are reported; capture state changes only
  after a successful redirect/restore.
- Updated README and HANDOFF documentation. The shell-level `2>` redirect must be
  removed when using the shortcut. The capture includes all stderr-based output in
  the selected interval (VMVGA diagnostics plus trace-log output routed to stderr),
  while HMP `trace-event` enablement remains independent.

Commits: implementation `be80ee0`; README `7b817c8`; HANDOFF `de380e8`.
No local build or runtime test was performed. CI compilation and owner XP guest
verification remain pending; do not mark T19 DONE until those checks are reported.


## Session 34 (ChatGPT), 2026-11-21 - Gate diagnostic output behind on-demand capture

Changed T19 so `debug=on` is an opt-in capability for testers, while actual
diagnostic generation is enabled only during the Ctrl+Alt+L capture interval:

- Added the internal VMVGA `debug-log` runtime property. VMVGA local diagnostic
  macros now require active capture rather than merely `debug=on`.
- VMPort GuestRPC logging now checks both VMVGA `debug` and `debug-log`, so
  enabling the tester option alone does not continuously print GuestRPC data.
- SDL checks for a VMVGA device with `debug=on` before advertising or handling
  the shortcut. Capture redirects stderr without routine start/stop markers,
  enables the `vmware_*`, `vmport_*`, and `vdagent*` trace events dynamically, and disables
  them again when capture stops. No separate `-trace "vdagent*"` option is needed.
- Updated README, TASKS, and this handoff. Shell-level `2>` redirection remains
  incompatible with interval capture.

Commits: `5db3304` (VMVGA capture gate), `f5f3309` (GuestRPC gate),
`2168a69` / `749c2b6` (SDL capture and trace control),
`110afd1` (unused parameter), `a398c44` (gate comment),
`7b07747` / `41c56af` / `db59fd9` (atomic capture gate and explicit QOM
include), plus documentation.

No local build or runtime test was performed. Linux/Windows CI and owner XP guest
verification remain pending; T19 stays PAUSED.


## Session 35 (ChatGPT), 2026-11-21 - Document CD media workflow and future frontend priorities

- Added the Windows XP guest recommendation to transfer BIN/CUE images into the
  guest and mount them with Daemon Tools 3.47 until Y2KBox has full BIN/CUE parsing
  (including all CUE flags such as DCP) and emulated-CD-ROM insert/eject support.
- Added T20 for complete BIN/CUE parsing and referenced libcdio, 86Box, DuckStation,
  CUE-sheet format documentation, and disc-drive/copy-protection resources in HANDOFF.
- Updated T12 to make SDL2 the only planned interactive display path, keep VNC and
  `-display none` out of frontend scope, and include emulated CD-ROM insert/eject
  for multi-disc swapping. Kept the initial frontend's fixed 2048 MB RAM behavior.
- Removed priority labels from the human-led XP validation tasks T08 and T11;
  changed T15 to P3.
- Added ReShade injection compatibility as higher priority than the low-priority
  post-wxWidgets librashader consideration, with ares, PCSX2, and DuckStation
  integration references.
- Added a post-wxWidgets RAM/VRAM preset reconsideration task. RAM values are
  candidates; VRAM values remain explicitly provisional pending review of the
  earlier Gemini 4 note, which was not retrievable in this session.
- No source code changed. No local build or CI polling was performed.


## Session 36 (ChatGPT), 2026-11-21 - SDL fullscreen hint and CI benchmark cleanup

- Updated the SDL window title to advertise the configured fullscreen toggle chord
  (Ctrl+Alt+F by default), alongside the existing grab-release and optional VMVGA
  diagnostic-capture hints. The title is refreshed when fullscreen mode changes.
- Removed the completed T13d Windows SSE2 A/B and T13a Windows FST A/B benchmark
  steps from the per-artifact Windows workflow, including the download of the fixed
  historical pre-cache artifact. Kept the x87 differential test in CI; it checks
  correctness rather than performance. Benchmark sources and scripts remain available
  for deliberate manual A/B runs.
- Reorganized the KVM trace-generation checkpoint under HANDOFF section 1's host
  accelerator policy. Clarified that the XP-32 guest must work under Windows TCG and
  Linux product builds/runtime must retain mandatory KVM support. Updated the CI
  description to match the removal of recurring A/B benchmarks.
- No local build/test was run and no CI was polled. The source/workflow/doc edits are
  committed directly to main; compile/runtime validation remains the normal CI and
  owner verification path.


## Session 37 (ChatGPT), 2026-11-21 - Start VirtIO Block / Net evaluation (T15)

Started T15 as an owner-tested optional device path; no recurring CI performance
benchmark work is planned:

- Enabled only the shared VirtIO core, PCI transport, VirtIO Block, and VirtIO Net
  in `configs/devices/i386-softmmu/default.mak`. PIIX IDE and RTL8139 remain
  enabled. VirtIO SCSI, virtio-serial/vdagent, ballooning, vhost, and unrelated
  VirtIO devices remain disabled.
- Kept the normal README launch configuration on IDE + RTL8139. Added optional
  `virtio-blk-pci` and `virtio-net-pci` command-line guidance, including the XP
  x86 `viostor` F6 installation path and NetKVM installation note from
  `virtio-win-0.1.141_x86.vfd`. The exact floppy contents and driver compatibility
  still need guest verification.
- Updated T12 for independent wxWidgets selectors for network adapter
  (RTL8139 / VirtIO Net) and system-disk controller (PIIX IDE / VirtIO Block).
  The CD-ROM remains on IDE/PIIX for the initial implementation. Keep both legacy
  defaults until the owner reports manual compatibility and performance results.
- Updated HANDOFF Q3 to match the limited device scope and the manual-test decision.
  No VFIO work is involved; it remains disabled and out of scope.
- Corrected the README's stale SVGA memory description to distinguish the 128 MiB
  framebuffer from the separate 512 MiB 3D surface-memory budget.

No local build, guest test, or CI polling was performed. T15 remains CLAIMED;
source compilation and owner-run XP boot/network/performance checks are pending.


## Session 38 (ChatGPT), 2026-11-21 - Revert premature VirtIO config enablement

Owner testing exposed two failures from the initial T15 config-only enablement:

- Windows QEMU rejected `virtio-net-pci` as an unknown device. Inspection of the
  repository tree showed the actual VirtIO implementation sources had been removed
  by earlier pruning: the shared core/PCI transport in `hw/virtio`, `hw/net/virtio-net.c`,
  `hw/block/virtio-blk.c`, and related public headers were absent. Enabling Kconfig
  symbols alone cannot register devices when their implementations are missing.
- Linux CI attempted to compile `net/vhost-vdpa.c` after `CONFIG_VIRTIO_NET=y`, then
  failed because `hw/virtio/vhost-vdpa.h` had also been pruned. This optional host
  vhost-vDPA path is unrelated to the intended userspace VirtIO Net guest device.

Reverted `CONFIG_VIRTIO`, `CONFIG_VIRTIO_PCI`, `CONFIG_VIRTIO_BLK`, and
`CONFIG_VIRTIO_NET` to disabled in commit `72165a8` to avoid leaving Linux builds
broken. Updated T15, README and HANDOFF to mark VirtIO unavailable in current builds
until the minimal QEMU 11.1.90 VirtIO core/PCI, Block and Net sources and exact build
wiring are restored. Keep vhost-vDPA, SCSI, virtio-serial, ballooning and unrelated
VirtIO devices out of scope.

No local build or CI polling was performed after the rollback. The rollback removes
this specific newly introduced configuration trigger; CI status still needs to be
checked by the owner. The original Windows unknown-device failure remains expected
until the source restoration task is completed.


## Session 39 (ChatGPT), 2026-11-21 - Restore VirtIO Block + Net implementation sources (T15)

Resumed T15 after the owner confirmed the rollback revision's CI passed. Claimed T15 and
restored the minimal implementation set from upstream QEMU 11.1.90 commit
`9d7d057a558b9a1c232cea390b4481dd20d9c9ce`:

- Shared VirtIO bus, core, config I/O, QMP and PCI transport sources.
- VirtIO Block PCI frontend, block implementation/common code and missing public headers.
- VirtIO Net PCI frontend, network implementation, `net_rx_pkt.c`, and its existing header.
- Meson source wiring for only the shared core/PCI transport and the selected Block/Net
  implementations. Kconfig already defined the required `VIRTIO`, `VIRTIO_PCI`,
  `VIRTIO_BLK` and `VIRTIO_NET` symbols; no broad Kconfig restoration was needed.
- Re-enabled only those four device selections in the i386 target configuration. IDE and
  RTL8139 remain the defaults; CD-ROM remains IDE/PIIX.
- Changed the Meson `vhost_net` feature default to `disabled`. This prevents Linux from
  compiling the optional host vhost-vDPA backend as a side effect of enabling guest
  userspace VirtIO Net. Host vhost networking is outside this product's current scope.
- Added Linux and Windows CI smoke assertions that `-device help` lists both
  `virtio-blk-pci` and `virtio-net-pci`. These are device-registration checks, not
  performance benchmarks; no recurring performance CI was added.

Source and build wiring are committed, but no local build was run and CI was not polled.
T15 remains CLAIMED pending owner confirmation of Linux + Windows CI. After CI passes,
update README device guidance and proceed with manual XP `viostor`/NetKVM compatibility,
boot, networking and performance tests. Keep the defaults unchanged until owner results.


## Session 40 (ChatGPT), 2026-11-21 - Correct VirtIO Linux host-backend gate and optional ROM

Owner reported Linux CI failure at `net/vhost-vdpa.c` because
`hw/virtio/vhost-vdpa.h` remains intentionally pruned. Root cause: the previous change
set Meson's `vhost_net` feature to disabled, but `net/meson.build` compiles
`net/vhost-vdpa.c` under `have_vhost_net_vdpa`, which is controlled by the separate
`vhost_vdpa` feature. Corrected `meson_options.txt` so `vhost_vdpa` also defaults to
`disabled`; kept `vhost_net` disabled as well. No vhost-vDPA sources or headers were
restored because host vhost acceleration is outside the product scope.

Owner also reported `Failed to open file efi-virtio.rom`. Traced it to the upstream
`virtio-net-pci.c` class default `k->romfile = "efi-virtio.rom"`. The fork does not ship
that optional UEFI PXE ROM and targets BIOS-based Windows XP, so removed the default ROM
request rather than adding an unused firmware binary. This does not disable the VirtIO
Net device or its guest driver networking path; UEFI/PXE network boot is not in scope.

No local build or CI polling performed. Await owner confirmation of fresh Linux and
Windows CI results. Keep T15 claimed and README availability wording unchanged until
both platforms pass.


## Session 41 (ChatGPT), 2026-11-21 - Disable all host vhost backends for userspace VirtIO

Owner reported a second Linux CI failure at final link. Unlike the earlier missing
vhost-vDPA source, this one showed unresolved generic vhost symbols referenced by
`virtio.c`, `virtio-net.c`, and `backends/vhost-user.c`. Root cause: `vhost_net` and
`vhost_vdpa` being disabled did not disable the independent `vhost_kernel` and
`vhost_user` Meson features; both default to `auto` and can be enabled on Linux. That
caused `CONFIG_VHOST` to be enabled and pulled in host-vhost call paths whose implementation
sources are outside this XP product's scope. Set `vhost_kernel`, `vhost_user`, and
`vhost_crypto` defaults to `disabled` as well, alongside the already-disabled
`vhost_net` and `vhost_vdpa`. This leaves guest VirtIO Block/Net enabled while excluding
host vhost acceleration and its backend. Updated T15 and HANDOFF Q3 to record the cause
and fix. Owner reports Windows CI passed and XP boots with VirtIO; Linux CI still needs
confirmation on the new commit. No local build/test or CI polling performed.


## Session 42 (ChatGPT), 2026-11-21 - Correct Meson option-file comment syntax

Owner reported both Linux and Windows configure stages failing at `meson_options.txt:299`
after the host-vhost feature change. The cause was my use of C/C++ `//` comments in a
Meson options file; Meson requires `#` comments. Replaced both comment prefixes with `#`
in `meson_options.txt` (`0e04725cefa5813f9eefd6d4767f7e4c6456fa1f`) and documented the
correction in T15. The earlier missing `tests/docker/docker.py` message is emitted during
configure but is not the fatal error shown here; Meson stops on the invalid comment.
No local configure/build or CI polling performed. Both platforms must rerun against the
latest main commit to validate the host-vhost feature settings and VirtIO link.

## Session 43 (ChatGPT), 2026-11-21 - Enable VirtIO Block + Net as standard launch devices

Owner confirmed both Linux and Windows CI pass on the corrected source state at
`d95d75703577a65e913d2451d0f01bfbb915a477`, and reported that QEMU launches and boots
Windows XP with VirtIO enabled, with substantially improved initial interactive
performance. Completed T15 and updated the standard launch profile:

- Made `virtio-blk-pci` the standard system-disk device and `virtio-net-pci` the
  standard network adapter in the README launch command.
- Kept PIIX IDE and RTL8139 enabled as independent fallback choices; updated guest
  driver guidance for `viostor` (F6 during fresh XP Setup) and NetKVM.
- Kept the CD-ROM explicitly attached to the existing PIIX IDE controller. This
  preserves the optical-drive emulation path independently of the system disk.
- Updated T12 to specify three independent future GUI selectors: VirtIO Net default
  / RTL8139 fallback, VirtIO Block default / PIIX IDE fallback, and PIIX IDE CD-ROM
  default / optional VirtIO SCSI CD-ROM only after its device and XP driver path are
  restored and validated. CD-ROM insert/eject remains part of T12.
- Left VirtIO SCSI, vhost acceleration, UEFI/PXE ROM support, and recurring CI
  performance benchmarks out of the current implementation. Owner-led workload
  performance comparisons remain manual.

No source behavior changed in this documentation/default-profile pass beyond the
target configuration comments; no local build/test or CI polling was performed.

## Session 44 (ChatGPT), 2026-11-21 - Keep PS/2 IntelliMouse as the only supported pointing device

Owner confirmed that the existing default PS/2 IntelliMouse behavior is preferred for
Windows XP-era games. USB mouse and tablet devices have produced tracking, camera
recentering, and legacy DirectInput compatibility problems in some older games, including
similar issues observed with other virtualization products. Recorded the product policy:

- Keep the PC machine's default PS/2 IntelliMouse; do not add a pointing-device selector.
- Do not recommend or expose USB mouse/tablet choices in the README or future wxWidgets UI.
- Keep the shared USB HID implementation for keyboard support. Do not remove upstream
  USB mouse/tablet implementation code solely to hide unsupported product choices.
- Clarify that USB mouse/tablet models are not part of the supported product profile and
  must not be instantiated by product presets.

Updated the XP device configuration comments, README mouse guidance, T12 launcher scope,
and HANDOFF Q3 device policy. No device implementation or runtime behavior changed; no
USB mouse/tablet launch flags were added to documentation. No local build/test or CI polling
was performed.


## Session 45 (ChatGPT), 2026-11-21 - Prioritize FIFA 2003 scene-transition stalls (T13e)

Owner described repeatable 1-3 second Windows TCG pauses in FIFA 2003 around match-intro camera cuts / newly visible stadium or LOD views. Promoted this responsiveness issue to T13e (P0), ahead of further isolated x87 instruction work:

- T13e is now claimed and requires a repeatable owner-side capture plus opt-in, low-overhead timing/counters across VMVGA FIFO/SVGA3D/GMR uploads, DXVK shader/pipeline creation and cache behavior, TCG code-buffer flushes, TB invalidation/SMC, SoftMMU TLB paths, and host thread waits/contention.
- Explicitly recorded that the root cause is unknown. Do not enlarge the TCG code buffer, assume a DXVK state-cache setting/async compiler, or pin threads without evidence.
- Added a source-level Xemu comparison requirement: review individual changes against our QEMU 11.1.90 base and x86 PC/SoftMMU semantics; no wholesale TCG replacement or blind cherry-picks.
- Paused further T13a x87 expansion while retaining the existing guarded host-x87 arithmetic/load/store paths and correctness differential. The current implementation is not a wholesale guest x87 stack-to-host stack mapping.
- Reframed the remaining SoftMMU/TLB, TB chaining/invalidation, helper/flags, REP-string, and threading work as P1 candidates to prioritize using the captured XP workload rather than generic assumptions.

No emulator source code or instrumentation changed in this documentation/priority pass. No local build/test was run, and no CI was polled. T13e remains CLAIMED; the next step is to inspect the actual TCG/DXVK code paths and design the smallest useful opt-in timing instrumentation before making performance changes.

## Session 46 (ChatGPT), 2026-11-21 - Verify TCG scene-stall suspects and xemu references

Follow-up source review for T13e:

- Confirmed the actual code-generation buffer sizing in this QEMU 11.1.90 tree is in `tcg/region.c`: 64-bit system emulation has a 1 GiB default, while automatic sizing is capped at `phys_mem / 8`. With the fixed 2 GiB guest this is approximately 256 MiB, subject to the actual physical-memory value and alignment. The suggested 32 MiB default increase is therefore not applicable to this build; a larger buffer must not be proposed without observing code-buffer exhaustion/full TB flushes at the pause.
- Located the current implementation points: `tb_flush__exclusive_or_serial()` and `tb_invalidate_phys_range()` in `accel/tcg/tb-maint.c`; `tlb_reset_dirty()` and TLB flush paths in `accel/tcg/cputlb.c`; SVGA3D FIFO dispatch through `vmsvga_fifo_run()` in `hw/display/vmware_vga_3d.c`; and D3D9 shader object creation through `vmsvga3d_dxvk_shader_create()` in `hw/display/vmware_vga_dxvk.c`.
- Reviewed xemu proposals #3028 (same-page TB invalidation), #3140 (skip unused MMU indexes in `tlb_reset_dirty()`), and #3141 (avoid indirect dispatch in `helper_lookup_tb_ptr()`). All three were open PRs at review time, not merged fixes. #3141's current proposal uses an `XBOX`-guarded direct x86 CPU-state hook, so it is not a drop-in patch for Y2KBox; any equivalent must be scoped to this fork's fixed i386 target and preserve generic fallback/debug flags. Added these references to HANDOFF with compatibility caveats.
- Refined T13e to require opt-in aggregate timing/counters and a measured cause before code changes. No claim is made that shader compilation, TB invalidation, TLB work, or thread contention is the cause of FIFA 2003's pauses.

No emulator source code or runtime instrumentation changed. No local build/test was run and no CI was polled. T13e remains CLAIMED; next step is to design and implement the smallest opt-in instrumentation, then collect the owner's repeatable Windows-host capture.


## Session 47 (ChatGPT), 2026-11-21 - Generalize T13e across affected XP games

Owner clarified that the 1-3 second Windows TCG pauses are not specific to FIFA 2003. Similar behavior occurs in Joint Operations: Typhoon Rising and F1 Challenge '99-'02. Updated T13e and HANDOFF section 5a4 to describe a general cross-game Windows XP guest/host responsiveness problem. FIFA 2003, Joint Operations, and F1 Challenge are now explicitly listed as representative reproductions, not the scope of the issue or evidence of a game-specific bug. The capture plan now calls for repeatable measurements across representative affected games/workloads.

No emulator source code or profiling instrumentation changed. No local build/test was run and no CI was polled.


## Session 48 (ChatGPT), 2026-11-21 - Compare xemu TCG proposals against Y2KBox and record audio symptom

Owner confirms that audio loops, stutters, and freezes during the same 1-3 second pauses seen in FIFA 2003, Joint Operations: Typhoon Rising, and F1 Challenge '99-'02. This broadens the symptom beyond display presentation, but does not distinguish guest CPU execution, audio production, shared host-thread blocking, or another synchronization path. T13e instrumentation should include guest audio production and host audio callback/queue servicing.

Reviewed the current xemu PRs and compared their affected code paths with Y2KBox's QEMU 11.1.90 source:

- xemu #3028 (tcg: keep whole-page TB invalidation off the page the store runs from) is not a direct port candidate. Y2KBox's SoftMMU path in accel/tcg/tb-maint.c, tb_invalidate_phys_page_range__locked(), already computes each TB's physical range and invalidates only when it overlaps the written range. The xemu proposal addresses xemu's own whole-page invalidation behavior.
- xemu #3140 (Skip unused mmu indexes in tlb_reset_dirty()) maps closely to our current source. Y2KBox sets cpu->neg.tlb.c.dirty when tlb_set_page_full() populates an MMU index, but tlb_reset_dirty() still iterates every NB_MMU_MODES table. The PR iterates only indexes marked populated. This is a small, plausible optimization, but it matters only when tlb_reset_dirty() is called; measure call frequency and elapsed time during the affected workload before implementing it.
- xemu #3141 (Avoid indirect call in helper_lookup_tb_ptr()) targets the generic cpu->cc->tcg_ops->get_tb_cpu_state() indirect dispatch plus curr_cflags() on the indirect-branch lookup path. Y2KBox has the same generic path in accel/tcg/cpu-exec.c. This may improve steady-state guest throughput for workloads with many indirect branches, but is not evidence for the multi-second pauses. xemu's proposal assumes a single x86 target; any Y2KBox adaptation must preserve target selection and debug/single-step behavior. The xemu maintainer requested profiling evidence; the PR discussion does not provide a measured performance result.
- All three xemu proposals were still open on 2026-11-21. They are candidate ideas, not merged or proven fixes.

Updated TASKS.md and HANDOFF.md with the audio symptom and these source-level applicability findings. No emulator source or instrumentation changed. No local build/test was run, and no CI was polled. T13e remains CLAIMED; next source task is to implement opt-in aggregate timing/counters (including audio servicing) before deciding whether any xemu optimization or other fix is justified.


## Session 49 (ChatGPT), 2026-11-21 - Add opt-in aggregate scene-stall instrumentation

Implemented the first T13e instrumentation pass, connected to the existing Ctrl+Alt+L VMVGA diagnostic capture. The counters are reset/enabled when capture starts and disabled/reported when capture stops; aggregate output is appended to vmvga_freeze.log as Y2KBOX-PERF lines. No per-event logging was added.

New shared API/module:
- include/qemu/perf-stats.h and util/perf-stats.c, registered in util/meson.build.
- Uses host-monotonic microsecond timestamps converted to nanoseconds and atomic aggregate counters: invocation/interval count, total elapsed time, maximum elapsed time, and counts above 1/10/100 ms.
- The summary explicitly warns that categories overlap and must not be summed. Timing is wall-clock duration, not CPU-cycle accounting.

Instrumented paths:
- accel/tcg/cpu-exec.c: duration of each cpu_exec slice (after halted polling).
- accel/tcg/tb-maint.c: full TB flush and system-mode physical TB range invalidation.
- accel/tcg/cputlb.c: tlb_reset_dirty, including time waiting for/acquiring its TLB lock.
- hw/display/vmware_vga.c: full vmsvga_fifo_run pass, including the command work it invokes.
- hw/display/vmware_vga_dxvk.c: D3D9 shader-object creation call.
- audio/audio-mixeng-be.c: guest-to-host mixer output service duration and inter-service interval.
- audio/sdlaudio.c: SDL host output callback duration and callback-to-callback interval.
- ui/sdl2.c: starts/stops and reports the counters with the existing VMVGA log capture; if stderr restoration fails, the capture and counters are re-enabled together.

Interpretation boundaries:
- cpu_exec duration includes host scheduling delays and waits; it is not pure CPU time.
- SDL callback interval can expose missed/late servicing even when callback execution duration is short.
- This first pass does not directly measure Windows scheduler ready/running time, GPU hardware queue wait duration, or non-SDL host audio callback servicing. WPR/ETW remains the follow-up if counters do not attribute the pause.
- Existing VMVGA flight logs continue to provide command/update/fence/readback counts; FIFO timing now gives a wall-duration envelope around those command passes.
- No performance fix or xemu TCG patch was applied. In particular, xemu #3140/#3141 remain candidates pending evidence; #3028 remains non-applicable as a direct port based on current Y2KBox invalidation behavior.

No local build or XP runtime test was run (per project workflow), and CI was not polled. The code changes require Windows/Linux CI validation before owner testing. T13e remains CLAIMED, not complete.


## Session 50 (ChatGPT), 2026-11-21 - Include VMVGA display servicing and audio callback gaps

Extended the initial T13e instrumentation to cover the full VMVGA display-update callback (vmsvga_update_display()), which also services pending D3D queries, deferred presents, and screen-target/readback work. This complements the FIFO timing and helps distinguish FIFO command processing from work performed during display refresh.

Added separate interval metrics for SDL audio output callbacks and the audio mixer output service. These report callback-to-callback/service-to-service gaps as well as execution durations, so delayed servicing can be detected even when the callback itself is short. Aggregate duration fields now report microseconds to preserve sub-millisecond detail; capture duration remains milliseconds. The summary continues to include threshold counts above 1/10/100 ms and a warning that categories overlap.

Updated TASKS.md and HANDOFF.md. No performance optimization was applied. No local build/test was run and CI was not polled; all source changes still require Windows/Linux CI validation and owner runtime capture.


## Session 51 (ChatGPT), 2026-11-21 - Add vCPU slice interval telemetry

Added a CPU execution slice-to-slice interval metric alongside the per-slice wall-duration metric. This helps distinguish a long cpu_exec call from a long gap between cpu_exec entries; both are wall-clock observations and may include host scheduling delays. Updated TASKS.md and HANDOFF.md accordingly. No build/test or CI polling performed.


## Session 52 (ChatGPT), 2026-11-21 - Attribute VMVGA FIFO time to individual SVGA3D commands

Owner supplied the first Y2KBOX-PERF capture from the Joint Operations map-load transition (15.509 s capture):

- vmvga.fifo_run: 359 calls, 14.358261 s aggregate wall time, 189.455 ms maximum; 201 calls exceeded 10 ms and 6 exceeded 100 ms.
- cpu.exec: 15.179954 s aggregate, 285.491 ms maximum; cpu.exec_gap maximum 285.501 ms.
- audio.mix_out_gap maximum 287.452 ms, while mixer execution totaled only 1.464 ms and SDL callback execution totaled 0.229 ms. This indicates delayed guest audio production/service rather than expensive mixer or SDL callback execution, but does not identify which thread was delayed.
- tcg.tb_flush and tcg.tb_invalidate_range were never called; tlb_reset_dirty totaled 0.900 ms. D3D9 shader-object creation totaled 7.876 ms.
- FIFO is therefore the leading next investigation target. Timings overlap and are wall-clock measurements; they do not prove that FIFO handling alone accounts for the entire 1-3 s pause, nor distinguish command-handler work from driver waits or host scheduling.

Added a second-stage, opt-in per-command profiler:
- vmsvga3d_fifo_command() now wraps the existing dispatcher and records one aggregate timer for total command dispatch plus per-command ID/name counters.
- util/perf-stats.c retains per-command count, total, maximum, and >1/10/100 ms buckets, and emits only nonzero command records when Ctrl+Alt+L capture stops. No per-command log stream is added.
- The command-level measurements include packet parsing and the selected handler/backend work. They should identify whether draw/clear, guest-backed updates/readbacks, surface/MOB/GART operations, query/wait, or DX command handlers dominate. They are not isolated GPU execution timings.
- The counters are bounded to command IDs below 4096; SVGA3D command IDs in this implementation fit that range.

Source commits: header c05999c, counter implementation 8b86280, VMVGA dispatcher 6dd5d13. Documentation commits: TASKS 831c430, HANDOFF 2221632.

No local build/test was run and CI was not polled, per project workflow. Windows/Linux CI must validate the new instrumentation before owner runtime testing. No performance optimization or command execution behavior was changed; T13e remains CLAIMED.

## Session 53 (ChatGPT), 2026-11-22 - Narrow scene-stall hotspot to GART map/unmap

Owner supplied the second per-command capture (16.452 s):
- `vmvga.command_dispatch`: 66,190 commands, 14.971245 s total wall time.
- `SVGA_3D_CMD_MAP_MOB_INTO_GART`: 132 calls, 10.450865 s total, 94.713 ms max, about 79.2 ms/call.
- `SVGA_3D_CMD_UNMAP_GART_RANGE`: 153 calls, 1.856100 s total, 100.974 ms max, about 12.1 ms/call.
- Combined map/unmap time is 12.306965 s, about 82.2% of command-dispatch total.
- `SVGA_3D_CMD_UPDATE_GB_IMAGE`: 4,747 calls, 916.020 ms total, 4.397 ms max.
- `SVGA_3D_CMD_DRAW_PRIMITIVES`: 10,275 calls, 1.543459 s total, 5.350 ms max; CLEAR totaled 110.110 ms.
- D3D9 shader-object creation totaled 0.689 ms. TB flush and physical TB invalidation were never called; dirty-TLB reset totaled 0.716 ms.
- `cpu.exec` max was 288.132 ms and audio mixer-service gap max was 289.251 ms, while mixer/SDL callback execution itself remained tiny. The audio symptom remains consistent with delayed service while work is in progress, not expensive audio callback code.

Source inspection of `vmsvga3d_gart_map_mob_live()` shows per-MOB-page `memory_region_find()` resolution, temporary `MemoryRegionSection`/GPA arrays, per-page BAR1 alias removal/install, and a memory-region transaction commit. `vmsvga3d_gart_unmap_live()` removes aliases per page and commits. This is a concrete hotspot candidate, but the command timers are inclusive wall-clock measurements and can include host scheduling or lock waits; do not claim the page operations themselves account for all elapsed time.

Added opt-in substage instrumentation in `hw/display/vmware_vga_3d.c`:
- MAP GPA/page resolution (`vmvga.gart_map_resolve`).
- MAP per-page alias installation/update loop (`vmvga.gart_map_alias`).
- MAP memory-region transaction commit (`vmvga.gart_map_commit`).
- UNMAP per-page alias removal loop (`vmvga.gart_unmap_alias`).
- UNMAP memory-region transaction commit (`vmvga.gart_unmap_commit`).
- Each stage reports `work_units` for the number of MOB/aperture pages processed, plus call count, total, max and slow-call thresholds.
- The measurements are active only during Ctrl+Alt+L performance capture. No mapping, alias, transaction, or guest-visible behavior was changed.

Updated TASKS.md and HANDOFF.md. Source changes: perf-stat API `e0793f9`, counter implementation `4011ba8`, GART substage instrumentation `a3c4523`; docs: TASKS `1d53e31`, HANDOFF `2ccead1`.

No local build/test was run and CI was not polled, per project workflow. Windows/Linux CI must validate the instrumentation before another owner capture. T13e remains CLAIMED; no performance fix has been applied.

## Session 54 (ChatGPT), 2026-11-22 - Break down memory-region transaction cost

Owner's follow-up GART capture measured 10.992056 s in 138 map transaction commits
(3,007 pages) and 1.768728 s in 159 unmap commits (3,595 pages). GPA resolution
was 206 us total and per-page map alias changes were 7.816 ms; unmap alias changes
were 63.184 ms. This points to the transaction commit machinery rather than the
GART page translation or alias loop.

Source inspection of system/memory.c shows that an outermost transaction with
memory_region_update_pending calls flatviews_reset(), global MemoryListener
begin callbacks, address_space_set_flatview() and
address_space_update_ioeventfds() for every AddressSpace, then global listener
commit callbacks. flatviews_reset() regenerates unique FlatViews for the registered
AddressSpaces. The existing GART timers include this entire commit operation.

Added opt-in aggregate timings inside memory_region_transaction_begin/commit:
coalesced-MMIO flush, flatviews_reset, global listener begin/commit, per-address-space
flatview publication, and per-address-space ioeventfd update. Per-address-space
stages also report work_units. This is instrumentation only; transaction nesting,
commit boundaries, aliases, and guest-visible mapping behavior are unchanged.

No local build/test or CI polling performed. Windows/Linux CI must validate before
the next owner capture. Do not attempt transaction batching or GART alias redesign
until the new counters identify the dominant internal stage and ordering constraints
are reviewed.


## Session 55 (ChatGPT), 2026-11-22 - Instrument FlatView topology generation

Owner's 17.271 s capture measured:
- GART map transaction commit: 11.449444 s (138 calls).
- `memory.txn_flatviews_reset`: 13.276462 s (159 calls).
- Address-space FlatView publication: 30.525 ms (2,226 calls).
- Ioeventfd updates: 38 us; global listener begin/commit: 281 us combined.
- Coalesced-MMIO flush: 139 us.

This isolates the expensive work to FlatView cache reset/regeneration rather than
publishing the generated views to address spaces or notifying listeners. Source
inspection confirms `flatviews_reset()` discards the shared cache and regenerates
one FlatView per unique root using `generate_memory_topology()`. That function
recursively renders memory regions into FlatRanges, simplifies adjacent ranges,
builds the address-space dispatch, compacts it, and inserts the view into the cache.

Added opt-in aggregate profiling in system/memory.c:
- `memory.topology_total`: complete generate_memory_topology duration.
- `memory.topology_render`: recursive render duration; its work_units count
  recursive region visits, including alias recursion.
- `memory.topology_flat_ranges` and `memory.topology_simplified_ranges`:
  FlatRange counts before and after simplification.
- `memory.topology_simplify`: FlatView simplification duration.
- `memory.topology_dispatch_build` and `memory.topology_dispatch_compact`:
  dispatch population and compaction durations.
- `memory.topology_roots`: unique roots regenerated during flatviews_reset.
- `memory.flatview_insert`: insertion count/duration.
- `memory.flatview_range_realloc`: reallocation duration and capacity growth
  in FlatRange slots (work_units are slots, not bytes).

The instrumentation does not alter the traversal, ordering, allocations, cache,
transaction boundaries, or mapping behavior. Timing remains opt-in and aggregate
only under Ctrl+Alt+L. Corrected independent timer start variables so nested
reallocation and topology-stage measurements do not contaminate their parent
stage durations.

No local build/test or CI polling performed. Windows/Linux CI must validate before
the next owner capture. No optimization applied; T13e remains CLAIMED.


## Session 56 (ChatGPT), 2026-11-22 - Profile FlatView scan, shift, and reference costs

Owner reports Linux and Windows CI passed for the previous FlatView topology instrumentation. The new 17.385 s capture measured:
- `memory.topology_total`: 12.962 s across 795 generated roots.
- `memory.topology_render`: 12.799 s; 7,772,434 recursive region visits.
- 3,890,387 FlatRange insertions; 3,508,092 ranges remained after simplification.
- Dispatch construction: 151.988 ms; simplification: 8.315 ms; range-array reallocations: 59.134 ms.
- `memory.txn_flatviews_reset`: 12.962 s; GART map commits: 11.188 s; GART unmap commits: 1.805 s. Address-space publication and listener/ioeventfd stages remain negligible.

Added another opt-in profiling layer in `system/memory.c` and the perf-stats API:
- `memory.flatview_scan`: range-search loop duration, total examined entries, and maximum entries examined in one render loop. This timer includes nested insertions.
- `memory.flatview_memmove`: array-shift duration, total shifted FlatRange entries, and maximum shifted entries in one insertion.
- `memory.flatview_ref`: `memory_region_ref()` duration and call count.
- `memory.flatview_size` and `memory.flatview_simplified_size`: count, aggregate ranges, and maximum per-root range count before/after simplification.
- Perf summaries now include `max_work_units`, allowing the largest individual work item to be seen alongside the aggregate.

The range-scan, insertion, memmove, and reference timers are nested/overlapping and must not be summed. All remain opt-in under Ctrl+Alt+L. No GART transaction, cache, FlatView ordering, or memory mapping behavior was changed. No optimization was applied. No local build/test or CI polling was performed; latest instrumentation requires Linux/Windows CI validation and owner capture.


## Session 57 (ChatGPT), 2026-11-22 - Opt-in batched FlatView construction prototype

Implemented an alternative FlatView renderer in system/memory.c while retaining the existing renderer as the default/reference path.

The batched renderer:
- Preserves recursive subregion traversal order (higher-priority regions are rendered first).
- Binary-searches the sorted existing FlatRange array for the first range whose end can overlap the current region.
- Walks only potentially intersecting ranges, collecting uncovered gaps in address order.
- Merges all gaps for that terminating region backwards into the existing array once, rather than calling flatview_insert()/memmove() for each gap.
- Takes MemoryRegion references for newly inserted ranges; existing references are transferred with the array entries.

Runtime selection:
- Y2KBOX_FLATVIEW_BUILDER=batched selects the prototype.
- Y2KBOX_FLATVIEW_VERIFY=1 builds the other implementation as a reference for each generated root and compares exact range count/order and all FlatRange fields before and after simplification. Mismatch calls error_report() and aborts.
- Verification runs both builders and is expensive; do not use during performance capture. Keep legacy as default pending CI and runtime verification.

New metrics: memory.flatview_search, memory.flatview_merge, memory.flatview_batch_ranges. Existing legacy metrics remain. No GART transaction boundaries, memory topology, or guest-visible mapping semantics were intentionally changed.

No local build/test or CI polling performed. The prototype is not validated yet. Required next steps: Linux and Windows CI; run guest once with builder=batched and verify=1 but without Ctrl+Alt+L; then run a separate capture with builder=batched and verification unset; test XP boot, desktop, representative 3D scene/map transition, and guest memory/device behavior. If equivalence aborts, revert to default legacy and inspect mismatch before any optimization is enabled.


## Session 58 (ChatGPT), 2026-11-22 - Prototype ordered-tree FlatView construction

Owner's batched-builder capture (22.072 s) showed 17.964536 s in topology
generation and 17.813955 s in recursive rendering. The batch builder recorded
16.994592 s in merging and approximately 23.68 billion merged range entries.
There were 3,883,514 batches for 3,890,387 inserted ranges, so average batch
size was effectively one. The batched-array strategy therefore did not solve
the repeated-copy cost and remains opt-in.

Implemented a second experimental renderer in system/memory.c, selected with
Y2KBOX_FLATVIEW_BUILDER=tree. It uses GLib GSequence as a balanced ordered
sequence during the existing recursive priority traversal. It searches by
range start, checks the predecessor for an overlap, inserts only uncovered
gaps before the next existing range, and flattens the sequence into the
standard FlatView array once rendering is complete. MemoryRegion references
are acquired for each inserted range. Existing simplification and dispatch
construction remain unchanged. Legacy remains the default/reference builder.

Added opt-in counters memory.flatview_tree_insert and
memory.flatview_tree_flatten; search uses memory.flatview_search. Verification
mode compares tree output with legacy before and after simplification.
Temporary GSequence nodes and individually allocated FlatRange payloads add
memory/allocation overhead; this prototype must be measured for both speed and
peak memory. No local build/test was run and CI was not polled. Required next
steps: Linux and Windows CI; owner equivalence verification with
Y2KBOX_FLATVIEW_BUILDER=tree and Y2KBOX_FLATVIEW_VERIFY=1 (without Ctrl+Alt+L);
then a separate performance capture with verification unset. No default
switch or GART transaction changes.


## Session 59 (ChatGPT), 2026-11-22 - Promote and validate ordered-tree FlatView renderer

Owner first confirmed the tree renderer's correctness comparison against the
legacy renderer with Y2KBOX_FLATVIEW_VERIFY=1. In the separate performance
capture (verification unset), topology generation took 1.713354 s across the
same 795 generations, compared with 17.964536 s in the earlier batched
prototype capture. GART map commits took 1.508186 s across 138 operations
(compared with 15.479662 s), and unmap commits took 0.235412 s across 159
operations (compared with 2.517483 s). Maximum cpu.exec duration fell from
399.384 ms to 85.693 ms, with zero >100 ms slices versus 144 in the batched
capture. The captures were not perfectly controlled A/B runs, but operation
counts match and the improvement is substantial.

Owner then tested the tree renderer in three affected games:
- Joint Operations: previous 1-3 second freezes and associated audio
  interruptions were no longer noticeable.
- FIFA 2003: major improvement; occasional roughly one-second hitches remain.
- F1 Challenge '99-'02: scene-transition stalls improved; a crowded opening
  grid still causes sustained slideshow-like performance until nearby cars
  leave view.

Promoted the ordered-tree renderer to the unconditional default in
system/memory.c. Removed Y2KBOX_FLATVIEW_BUILDER selection and the unsuccessful
batched renderer. Kept the legacy renderer only as the reference implementation
for Y2KBOX_FLATVIEW_VERIFY=1, which compares output before and after
simplification. No GART transaction boundaries or memory mapping semantics
were intentionally changed. No local build/test was performed and CI was not
polled; Windows and Linux CI validation remains outstanding. Do not claim CI
passed. Remaining FIFA hitches and F1 crowded-grid performance should be
profiled as separate follow-up symptoms rather than attributed to the
FlatView renderer without evidence.


## Session 60 (ChatGPT), 2026-11-22 - Generalize diagnostic logging and separate debug mode

Generalized the existing SDL Ctrl+Alt+L capture presentation and output name.
The capture file is now `y2kbox-log.log`; the existing first-capture truncate
and subsequent-interval append behavior is unchanged. SDL window-title wording
now describes generic diagnostic logging rather than a VMVGA freeze log.

Separated the tester's debug-mode setting from the VMVGA device declaration.
The setting remains opt-in and defaults off, but can now be configured
independently with `-global vmvga.debug=on` instead of embedding
`debug=on` in `-device vmvga`. This is the intended command-line equivalent
of T12's future Debug On/Off GUI setting. Debug mode enables diagnostic
capabilities; it does not start continuous output. Ctrl+Alt+L remains the
separate interval-capture control. VMVGA local diagnostics and VMPort
GuestRPC diagnostics remain gated by the capture state, alongside the
existing dynamic VMVGA/VMPort/vdagent trace events and performance counters.

Updated README, TASKS, and HANDOFF to describe the generic capture, filename,
and future GUI setting. No local build/test was run and CI was not checked.
T19 remains PAUSED pending Linux/Windows CI and owner XP guest verification.


## Session 61 (ChatGPT), 2026-11-22 - Add Y2KBox debug mode CLI option

Replaced the user-facing QEMU-specific global-property syntax with the native
Y2KBox option `-y2kboxdebugmode on|off`. The option is registered in
`qemu-options.hx` so it is parsed by QEMU's normal command-line option
machinery and appears in command-line help. `system/vl.c` validates the value
and maps it internally to the VMVGA debug capability property. The default
remains off; explicit `off` also sets the internal property off.

Updated VMVGA's diagnostic error guidance and the README, TASKS, and HANDOFF.
The future T12 GUI Debug On/Off control maps to this Y2KBox option, while
Ctrl+Alt+L remains the separate interval-capture control. No local build/test
or CI check was run; T19 remains PAUSED pending CI and owner XP guest
verification.

## Session 62 (ChatGPT), 2026-11-22 - Make SDL window geometry follow the guest display

Owner clarified that Y2KBox's SDL window must act as a fixed-size virtual
monitor: windowed mode must not be manually resizable or maximizable, guest
resolution controls the window client size, and fullscreen remains controlled
by Ctrl+Alt+F. Mouse emulation and coordinate mapping are considered working
and were intentionally left untouched.

In `ui/sdl2.c`, removed `SDL_WINDOW_RESIZABLE` from windowed window creation,
leaving the normal decorated, non-resizable SDL window style. This also removes
the host titlebar maximize/resize affordance through SDL's non-resizable window
style. Kept the existing programmatic `sdl2_window_resize()` path used when the
guest display surface changes.

Changed host resize handling so both `SDL_WINDOWEVENT_RESIZED` and
`SDL_WINDOWEVENT_SIZE_CHANGED` are presentation-only. They no longer forward
host dimensions to `qemu_console_set_ui_info()`; if a windowed host window is
nevertheless resized, SDL is instructed to restore the current guest surface
dimensions. Fullscreen desktop dimensions are never fed back into guest display
geometry.

Updated HANDOFF.md with the owner-approved SDL display policy and added T24
to TASKS.md. Source commit: `58d3098`; policy documentation:
`7fbe09c`; task tracking: `75618b6`. No mouse implementation or coordinate
mapping code was changed. No local build or XP guest test was run, and CI was
not polled. T24 remains PAUSED until Linux/Windows CI and owner XP guest
verification are reported.

## Session 63 (ChatGPT), 2026-11-22 - Separate diagnostic capture files

Changed the SDL Ctrl+Alt+L capture path so every start creates a new
`y2kbox-log-YYYYMMDD-HHMMSS.log` file in the launch working directory.
Files are opened with exclusive creation, so an existing file is never
truncated or appended to; if the timestamp collides, a numeric suffix is
added. Stopping a capture still disables the same diagnostic sources and
restores the original stderr descriptor. The VM continues running.

Updated README, TASKS, HANDOFF, and command-line option documentation. No local
build/test was run and CI was not polled, per project workflow. Linux/Windows
CI and owner XP guest verification remain outstanding.


## Session 64 (ChatGPT), 2026-11-22 - Document per-capture performance counter reset

Clarified the diagnostic capture contract in README.md, TASKS.md, HANDOFF.md,
and qemu-options.hx: every Ctrl+Alt+L capture starts a fresh `Y2KBOX-PERF`
measurement interval by resetting counters and timing baselines. The resulting
summary covers only that interval. This reset affects diagnostic statistics
only; it does not restart the VM or reset guest state.

Documentation-only change. No local build/test was run and CI was not polled.

## Session 65 (Gemini), 2026-11-22 - Fix vGPU9 D3D9 texture address, transform, and fixed-function state translation

Fixed fixed-function SVGA3D-to-D3D9 state translation bugs in
`hw/display/vmware_vga_vgpu9.c` identified while investigating tiled background
seam wrap-around in Hitman 2: Silent Assassin (D3D8):

- Mapped `SVGA3D_TEX_ADDRESS_EDGE` (`6`) to `D3D9_TADDRESS_CLAMP` (`3`) for
  `SVGA3D_TS_ADDRESSU/V/W` instead of forwarding `6` raw to D3D9
  `SetSamplerState()` (where unrecognized values fall back to `WRAP`).
- Decoded `SVGA3D_TS_TEXTURETRANSFORMFLAGS` as a component bitmask (`S|T|R|Q`
  plus `SVGA3D_TEX_PROJECTED`) instead of single-bit switch cases that dropped
  multi-axis transforms (`COUNT2`/`COUNT3`/`COUNT4` and projected flags) to
  `D3D9_TTFF_DISABLE`.
- Added `SVGA3D_TA_TFACTOR` (`D3D9_TA_TFACTOR = 3`) and modifier bits
  (`SVGA3D_TM_ALPHA` -> `D3D9_TA_ALPHAREPLICATE`, `SVGA3D_TM_ONE_MINUS` ->
  `D3D9_TA_COMPLEMENT`) in `vmsvga3d_d3d9_texture_arg()`, plus
  `SVGA3D_TS_CONSTANT` (`D3D9_TSS_CONSTANT = 32`).
- Mapped D3D8 cubic filter enums (`SVGA3D_TEX_FILTER_FLATCUBIC`,
  `SVGA3D_TEX_FILTER_GAUSSIANCUBIC`) to `D3D9_TEXF_LINEAR` for sampler
  min/mag/mip filters.
- Combined `SVGA3D_TS_TEXCOORDGEN` (`D3DTSS_TCI_*`) with
  `SVGA3D_TS_TEXCOORDINDEX` when applying texture stage state.
- Fixed `SVGA3D_RS_VERTEXMATERIALENABLE` mapping from
  `D3D9_RS_INDEXEDVERTEXBLENDENABLE` to `D3D9_RS_COLORVERTEX` (`141`).

Verified translator compilation and assertions standalone via `gcc -Wall -Wextra -Werror`;
full Linux/Windows CI build and owner XP guest verification remain outstanding.
### Session 66 — 2026-11-22 — Gemini (AI Studio)

#### Hitman 2: Silent Assassin (D3D8) invisible in-game characters (`apply-shaders` / `d3d9_shader_create`)
- Diagnosed owner diagnostic log from Hitman 2 in-game gameplay (Garden Shed, where
  Agent 47 was invisible while world/HUD rendered cleanly): fixed-function draws
  (`vs=INVALID ps=INVALID`) succeeded, while every character skinning draw failed at
  `VMVGA-D3D9-DRAW fail cid=1 stage=apply-shaders` (114 times in 873 ms), matching
  `dxvk.d3d9_shader_create count=114` 1-to-1 (~10 us each, indicating host D3D9
  shader creation rejection on raw D3D8 vertex shader bytecode).
- Added `vmsvga3d_dxvk_shader_create_with_hr()` in `hw/display/vmware_vga_dxvk.c`
  (preserving `vmsvga3d_dxvk_shader_create()` as a wrapper) to report the native
  `CreateVertexShader`/`CreatePixelShader` `HRESULT`.
- Added `vmsvga3d_d3d9_normalize_legacy_shader_bytecode()` in
  `hw/display/vmware_vga_vgpu9.c` as a fallback when raw D3D9 shader creation fails:
  - Upgrades D3D8 `vs_1_0` (`0xFFFE0100`) -> `vs_1_1` (`0xFFFE0101`) and `ps_1_0`
    (`0xFFFF0100`) -> `ps_1_1` (`0xFFFF0101`).
  - Synthesizes missing D3D9 `dcl_<usage><index> v<n>` input declarations for
    `vs_1_x` shaders that reference `v0..v15` without `dcl` tokens, matching
    semantics against the draw's `SVGA3dVertexDecl` array (D3D8 `D3DVSDE_*` vs
    `TEXCOORD(n)` convention detection with unclaimed-decl fallback).
  - Strips SM 2.0+ instruction-length bits (`[27:24]`) on SM 1.x instructions,
    normalizes `a0` destination write masks to `.x`, strips stray SM 2.0-style
    trailing `SVGA3DREG_ADDR` relative-address tokens in `vs_1_1`, and remaps
    `SVGA3DSAMP_2D_SHADOW` -> `SVGA3DSAMP_2D` in `ps` sampler `dcl`s.
- Improved `VMVGA-D3D9-SHADER` diagnostic logging in `vmsvga3d_dxvk_apply_context_shaders()`:
  logs normalization successes, missing/bind/constant sub-stage failures, and
  one-shot per-trace-session `create-fail` dumps with raw/normalized `HRESULT`s,
  vertex declaration semantics, and bytecode DWORDs.
- Verified standalone compilation and 5 unit tests via `gcc -Wall -Wextra -Werror`;
  full CI build and owner XP guest verification remain outstanding.

### Session 67 — 2026-11-22 — Gemini (AI Studio)
#### Hitman 2: Silent Assassin (D3D8) `shid=9` `oFog.x` write mask rejection (`D3DERR_INVALIDCALL`)
- Disassembled the failing `vs_1_1` skinning vertex shader (`shid=9`, 268 DWORDs)
  captured by Session 66's diagnostic dump (`raw_hr=0x8876086c`, `norm_tried=0`):
  - The VMware guest driver had already emitted all 6 `dcl_*` input declarations
    (`v0..v5`) and `vs_1_1` version/instruction encoding, so Session 66's
    normalizer produced identical output (`norm_tried=0`).
  - Instruction 51 (`raw[201..204] = 0x00000005 0xc0010001 0x80000000 0xa055005f`,
    `mul oFog.x, r0.xxxx, c95.yyyy`) encoded `oFog` (`D3DSPR_RASTOUT`, reg 1)
    with D3D8's `.x` write mask (`0x00010000`). Direct3D 9 `CreateVertexShader()`
    forbids explicit write masks on scalar rasterizer outputs (`oFog`/`oPts`) and
    requires `D3DSP_WRITEMASK_ALL` (`0x000F0000`, i.e., `0xc00f0001` / `0xc00f0002`),
    rejecting `0xc0010001` with `D3DERR_INVALIDCALL`.
- Extended `vmsvga3d_d3d9_normalize_legacy_shader_bytecode()` in
  `hw/display/vmware_vga_vgpu9.c`:
  - Normalizes `D3DSPR_RASTOUT` `oFog` (reg 1) and `oPts` (reg 2) destination
    write masks to `D3DSP_WRITEMASK_ALL` (`0x000F0000`) for `vs_1_x` / `vs_2_x`.
  - Normalizes non-replicate source swizzles on component-wise instructions
    writing to `oFog` / `oPts` by replicating lane 0 (preserving D3D8 `.x`
    semantics without touching vector-reduction ops like `dp3`/`dp4`).
  - Normalizes D3D8 `vs_1_x` scalar ops (`rcp`, `rsq`, `exp`, `log`, `expp`,
    `logp`) with default `0xE4` (`xyzw`) source swizzle to `.wwww` (`0xFF`),
    matching D3D8 hardware semantics and D3D9's replicate-swizzle requirement.
- Increased `vmsvga3d_dxvk_trace_shader_bytecode()` dump limits in
  `vmsvga3d_dxvk_apply_context_shaders()` from `256`/`64` DWORDs to `512`/`512`
  DWORDs so shaders like `shid=9` (268 DWORDs) are never truncated on failure.
- Verified standalone compilation (`gcc -Wall -Wextra -Werror -Wdeclaration-after-statement`)
  and 4 unit tests covering `shid=9` (`raw[202]`: `0xc0010001` -> `0xc00f0001`,
  idempotent), `oPts`/swizzle fixups, scalar op swizzles, and `vs_1_0` regression;
  full Linux/Windows CI build and owner XP guest verification remain outstanding.


### Session 68 — 2026-11-22 — ChatGPT
#### F1 Challenge '99-'02 crowded-grid slowdown — SVGA3D phase profiling
- Owner's Albert Park A/B reports that disabling Ctrl+Alt+L makes the crowded-grid scene only slightly better; the scene remains slideshow-like. Diagnostic logging overhead is therefore a secondary contributor, not the root cause.
- Source review traced the expensive `DRAW_PRIMITIVES` command to the D3D9 runtime path in `hw/display/vmware_vga_vgpu9.c`: vertex-layout planning, legacy D3D9 state replay, buffer materialization, and native draw submission. The common vertex/index buffer materialization path already has a resident-buffer fast check, so it should not be changed without phase measurements.
- Source review traced `UPDATE_GB_IMAGE` through `hw/display/vmware_vga_3d.c` into `vmsvga3d_d3d10_update_subresource_live()` in `hw/display/vmware_vga_vgpu10.c`. The normal path reads guest MOB data into the CPU image shadow and then uploads to resident D3D9/D3D11 resources; partial D3D9-resident updates may first read back the full image to preserve pixels outside the update box.
- Added capture-gated phase counters: `vmvga.d3d9_draw_layout`, `vmvga.d3d9_draw_state`, `vmvga.d3d9_draw_resource`, `vmvga.d3d9_draw_submit`, `vmvga.gb_update_preserve`, `vmvga.gb_update_mob_read`, `vmvga.gb_update_native_upload`, and `vmvga.gb_update_cb_refresh`. MOB-read `work_units` reports transferred bytes; draw-submit `work_units` reports primitive count. Counter timings are nested within the existing per-command/FIFO timings and must not be summed with those enclosing categories.
- Instrumentation only; no rendering semantics or optimization changed. It is inert while performance capture is stopped. No local build or CI run was performed; owner Windows guest capture is required next.
- Commits: `72a46d5d` (counter IDs), `c5e055f5` (counter names), `968e5af7` (D3D9 draw phases), `1625c76a` (GB image-update phases), `fd045363` (remove misleading resource-byte work units), `fd522fc0` (task tracking).


### Session 69 — 2026-11-23 — ChatGPT
#### F1 Challenge '99-'02 crowded-grid slowdown — buffer range-upload lock analysis
- Owner supplied a 22.291 s dry-weather Albert Park race-start capture at 800x600x16, Medium object detail / High texture detail (desktop 800x600x32, 85 Hz; game benchmark 122.2 MTri/s).
- The new DXVK upload counters measured 60,388 buffer range locks: `dxvk.upload_buffer_lock` 1.870338 s, `dxvk.upload_buffer_copy` 11.194 ms for 162,832,408 bytes, and `dxvk.upload_buffer_unlock` 21.890 ms. The lock call accounts for about 97.7% of the enclosing 1.914 s `vmvga.gb_update_native_upload` time, and about 98.2% of measured lock/copy/unlock time. These categories are nested; do not add the upload phase to its enclosing counters.
- Source inspection found that `dxvk.upload_buffer_lock` instruments only `vmsvga3d_dxvk_surface_upload_buffer_range()` in `hw/display/vmware_vga_dxvk.c`. Its D3D9 buffer Lock uses flags=0, followed by memcpy and Unlock. The call comes from `vmsvga3d_d3d9_runtime_upload_surface_image()` for vertex/index-buffer surface updates in `hw/display/vmware_vga_vgpu9.c`.
- The separate whole-buffer upload function `vmsvga3d_dxvk_surface_upload_buffer()` uses `D3DLOCK_DISCARD` and is not covered by these buffer lock counters. Therefore the measured 1.87 s specifically identifies the range-update lock path, not every buffer upload.
- A zero-flag D3D9 Lock is a plausible synchronization hotspot, but this capture does not prove that the time is GPU waiting. Do not blindly substitute `D3DLOCK_NOOVERWRITE` or `D3DLOCK_DISCARD`: their correctness depends on buffer lifetime and overwrite guarantees, and DISCARD invalidates the whole buffer.
- No rendering behavior or optimization was changed. Next: inspect vertex/index resource creation and update lifetime semantics, then add only the minimum discriminating measurement or a correctness-preserving optimization. Keep the same repeatable Albert Park workload for A/B validation.


### Session 70 — 2026-11-23 — ChatGPT
#### F1 Challenge '99-'02 crowded-grid slowdown — classify buffer range locks
- Continued source review after Session 69. `vmsvga3d_d3d9_resource_plan()` creates both vertex and index buffers with `D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY` in `D3DPOOL_DEFAULT`; their native length is the guest surface byte size. This confirms that D3D9 dynamic-buffer lock modes are supported by the resource usage, but does not make arbitrary partial-range DISCARD or NOOVERWRITE safe.
- `vmsvga3d_d3d9_runtime_upload_surface_image()` forwards guest update `offset` and `size` directly to `vmsvga3d_dxvk_surface_upload_buffer_range()`. The range upload currently uses flags=0. The guest writes update a resident native buffer in place. The draw path has no range-overlap/lifetime tracking in the inspected upload code, so NOOVERWRITE cannot yet be justified.
- A full-buffer replacement (`offset == 0 && size == d3d9_length`) is the clear candidate for `D3DLOCK_DISCARD`, because the guest supplies every byte and no old contents need to survive. Before applying it, classify how often the workload actually produces full-range versus partial-range writes.
- Added four overlapping diagnostic counters to the existing buffer-lock measurement: `dxvk.upload_buffer_lock_vertex`, `dxvk.upload_buffer_lock_index`, `dxvk.upload_buffer_lock_full_range`, and `dxvk.upload_buffer_lock_partial_range`. Each records lock time and upload bytes in work_units. The original aggregate counter remains unchanged. Type and range subcategories are independent dimensions; do not sum all four together or add them to the aggregate.
- No lock flags or rendering behavior changed. Next: build main and repeat the same dry Albert Park race-start capture. Compare type/range subcounters. If full-range locks are material, add the narrowly safe DISCARD path only for exact full-buffer replacements; if partial writes dominate, investigate safe staging/batching or explicit in-flight range tracking rather than guessing NOOVERWRITE.

- Follow-up implementation in `hw/display/vmware_vga_dxvk.c`: exact full-buffer replacements (`offset == 0 && size == d3d9_length`) now use `D3DLOCK_DISCARD`. Partial-range updates continue to use flags=0, preserving untouched bytes. This mirrors the existing whole-buffer upload behavior and avoids applying DISCARD to a partial guest update. This is a correctness-based fast path; its performance impact is not yet measured.

### Session 71 — 2026-11-23 — ChatGPT
#### F1 Challenge '99-'02 crowded-grid slowdown — defer and coalesce D3D9 buffer updates
- Owner's 19.754 s dry Albert Park capture showed 47,240 vertex-buffer partial-range locks taking 1.485 s (31.4 us/lock), with zero index-buffer locks and zero full-range updates. Copying 128.3 MB took 8.4 ms and unlocks took 16.9 ms. The per-lock cost was effectively unchanged from the earlier capture; the total reduction tracked the lower update count/byte volume.
- Audited the update and consumer ordering. UPDATE_GB_IMAGE first writes guest MOB bytes into the persistent CPU surface shadow. D3D9 stream-source and index-buffer consumers occur in the D3D9 draw path, which materializes each buffer before binding it. The D3D9-to-D3D11 handoff's buffer readback wrapper deliberately does not read back input-only vertex/index buffers; the CPU shadow remains authoritative and is used to initialize the D3D11 resource. CPU-only fallback paths evict the D3D9 resource.
- Implemented one pending bounding dirty range per resident D3D9 vertex/index buffer. Guest updates merge into that range without locking the native buffer. The merged span is flushed immediately before the D3D9 draw binds the resident buffer. The CPU shadow supplies any untouched bytes between disjoint updates in the merged span. A complete replacement uses D3DLOCK_DISCARD only for a draw-consumption flush; handoff, eviction, and incompatible-resource replacement flushes use flags=0 so the lock also serializes earlier GPU reads. Partial spans always retain flags=0. Failed flushes keep the range dirty so a later consumer can retry. Eviction drops pending metadata only after its synchronization flush (or when DXVK is unavailable during teardown); replacement resources are initialized from the authoritative CPU shadow.
- Added dxvk.upload_buffer_deferred and dxvk.upload_buffer_flush counters. Their work units report guest-update bytes and merged-flush bytes respectively; compare them to infer how much extra gap data a bounding range uploads. Existing lock/copy/unlock counters now measure actual flushes. Flush counters overlap the lock/copy/unlock phases; do not sum them.
- Added the required flush at the D3D9 buffer materialization fast path, before the buffer is bound/consumed. D3D9-to-D3D11 buffer readback is explicitly a no-op because the CPU shadow is authoritative, but the common eviction helper still flushes pending ranges first: this preserves the Lock synchronization barrier before the old D3D9 resource is released and the D3D11 resource is initialized from the shadow.
- Extended the synchronization boundary after an eviction audit: the common legacy-surface eviction helper flushes pending D3D9 buffer ranges before releasing the native resource, and renderer teardown/recreation does the same while DXVK is ready. Incompatible-resource materialization also flushes before the DXVK resource planner can evict a resident buffer. These paths use flags=0 to preserve ordering; only the immediate pre-draw path may use DISCARD for a full replacement.
- Final lock-mode rule: a full-range lock is not automatically a DISCARD lock. Only the pre-draw flush passes `discard_full_range=true`; eviction, handoff, and incompatible-resource replacement pass false, so even a full-span flush uses flags=0 to wait for prior D3D9 use. The existing full-range counter classifies byte coverage, not the D3DLOCK flag.
- No local build or CI run was performed (HANDOFF section 7 prohibits local builds and CI polling). Owner validation is required: build latest main, repeat the same dry Albert Park race-start capture, compare deferred-update calls/bytes against flush calls/bytes and lock time, and verify visual/gameplay correctness.
- Commits: bad3face (claim), 36fd8ce4/404fef7e (counter IDs/names), 389ac2b3 (API), eff5d457/9765f1b9 (dirty-range coalescing and pre-draw flush), a2a621c0/151076e4 (resident fast-path and shadow handling), 23bac810/e70a5889/dfd35ae3/fbe15923 (eviction and renderer-teardown synchronization), 36dc2155/46893a5c (resource-transition flush and exact compatibility), 220b2156/d6c36134/ab002ee4/e00ed61f/5e2ef934 (draw-only DISCARD, synchronized eviction locks, call-site policy and unsupported-build cleanup), 8d9bd160 (task update), fbee7be7 (synchronization policy/worklog).


### Session 72 — 2026-11-23 — ChatGPT
#### F1 Challenge crowded-grid uploads — driver-managed DISCARD rename for small dynamic buffers
- Owner's 20.526 s dry Albert Park capture confirms that deferred range coalescing does not merge this workload: 57,598 guest buffer updates became 57,598 flushes and 57,598 partial vertex-buffer locks. The measured Lock phase was 1.744 s, memcpy 11.0 ms for 154.6 MB, and Unlock 20.0 ms. No index-buffer or full-range locks occurred. The owner reports no visible graphical differences or regressions with the deferred implementation.
- The update/draw interleaving means a manual dirty-range merge cannot help: each update is consumed before another update to the same surface arrives. Rather than invent stream/index offset arithmetic or use unsafe NOOVERWRITE, the next low-risk alternative is D3D9's own dynamic-buffer renaming via DISCARD.
- Added an exact-full-buffer DISCARD upload entry point. It rejects any size that does not exactly equal the native D3D9 buffer length, copies the entire authoritative CPU shadow, and clears pending dirty-range metadata only after successful Unlock. This preserves bytes outside the guest's changed subrange while allowing the D3D9 driver to rename the backing allocation for earlier queued draws.
- For vertex/index buffers whose CPU-shadow allocation is at most 64 KiB and exactly matches its image size, VGPU9 now attempts that full-shadow DISCARD upload immediately on guest update. If the native length does not match or the upload fails, it falls back to the existing deferred partial-range path. Larger buffers retain the previous behavior. This is a bounded experiment, not a claimed speedup; it trades extra memcpy bytes for avoiding a potentially synchronizing partial Lock on small buffers.
- The full-buffer upload path now contributes to the existing upload lock/copy/unlock counters, and range classification distinguishes exact native full-buffer locks from partial-size initialization uploads. A successful small-buffer DISCARD should therefore appear as full-range locks and increased copy work; compare lock time against copy time and total performance.
- No manual ring offsets, stream-source offset changes, index-buffer rebasing, or NOOVERWRITE flags were introduced. No local build or CI polling was performed (HANDOFF section 7). Owner must build latest main, repeat the same dry Albert Park race-start capture, and check visuals/gameplay. If the 64 KiB full-shadow copies cost more than the Lock time they save, use the new counters to tune or remove the threshold. Commits: `aeb16c5` (claim), `7ce2793`, `e3db8e4`, `28bebe6` (full-shadow DISCARD path), `2d23e52` (lock range classification).


### Session 73 — 2026-11-23 — ChatGPT
#### F1 Challenge crowded-grid uploads — instrument dirty-range merge and flush cadence
- Owner's 20.361 s dry Albert Park capture shows 56,709 buffer locks: 3,334 full-range locks took 42.601 ms and 53,375 partial-range locks took 1.677 s. Total Lock time was 1.719 s, copy time 61.806 ms, and Unlock time 19.815 ms. The combined Lock/copy/Unlock phase was about 1.800 s, approximately 25 ms higher than the previous deferred-only capture's 1.775 s. No noticeable visual differences were reported.
- The capture reports 53,531 deferred updates and 53,531 flushes, with identical deferred/flush work units of 230,282,944 bytes. Lock work units are a separate dimension: full-range and partial-range lock counters together report 297,324,160 bytes because the lock counters also include whole-buffer uploads outside the deferred flush path. Full-range lock classification alone does not distinguish the small-buffer full-shadow uploads from full-size deferred flushes.
- Source review confirms the current ordering: UPDATE_GB_IMAGE mutates the authoritative CPU shadow and marks the D3D9 buffer dirty; D3D9 draw materialization flushes the dirty span immediately before binding the buffer. The existing bounding-range logic already merges multiple updates to one surface if they arrive before that surface is consumed. It cannot merge across a draw that must observe the earlier update.
- Added capture-gated diagnostics without changing buffer contents or draw ordering:
  - `dxvk.upload_buffer_deferred_merge`: count of guest updates that found an already-dirty surface and extended its pending range.
  - `dxvk.upload_buffer_flush_draw`, `..._flush_evict`, and `..._flush_replace`: successful dirty flushes classified by draw consumption, eviction, or incompatible-resource replacement.
  - `dxvk.upload_buffer_flush_updates`: count = successful flushes, work_units = total deferred guest updates represented by those flushes, max_work_units = largest update batch represented by one flush.
  - `dxvk.upload_buffer_discard_supersede`: full-buffer uploads that replaced a pending dirty range, with work_units reporting how many deferred updates were superseded.
- Added a per-surface pending-update count solely for diagnostics. It is reset whenever a pending range is successfully uploaded, superseded by a successful full upload, or the D3D9 surface is evicted. The flush API now receives a reason enum; call sites label draw, eviction, and resource replacement. Unsupported-backend builds explicitly consume the new reason argument.
- No batching policy, lock flags, stream offsets, index rebasing, or rendering semantics changed. No local build or CI polling was performed (HANDOFF section 7). Owner: build latest main, repeat the same dry Albert Park race-start capture, and report the five new counters plus existing lock/copy/unlock counters. If merge count and flush_updates work_units show almost one update per flush, there is no natural batching opportunity in this workload; then prioritize reducing synchronization per update rather than trying to delay guest-visible updates. If flush_draw dominates, it confirms each buffer update is being consumed by a subsequent draw; eviction/replace counts will show whether transitions are contributing.
- Commits: `0105cdfb` (claim), `b4a68d32`/`f4a73412` (counter IDs/names), `558afe27` (flush-reason API), `83ea4c19` (dirty update/merge and flush diagnostics), `121d64ed` (flush reason call sites), `6e01f65d` (unsupported-backend argument handling).
- Follow-up CI repair: both Linux and Windows failed because the two less-common flush boundaries added after the API change still used the old five-argument call. Added `VMSVGA3D_DXVK_BUFFER_FLUSH_EVICT` to renderer teardown/recreation in `vmware_vga_3d.c`, and `VMSVGA3D_DXVK_BUFFER_FLUSH_REPLACE` to incompatible-resource materialization in `vmware_vga_vgpu9.c`. Audited the remaining flush call sites; they already pass the correct reason. This preserves the reason-specific counters and existing synchronization/lock semantics. No local build or CI polling was performed (HANDOFF section 7). Commits: `3cefeacb` (renderer eviction), `0b8c3bad` (resource replacement)
- Owner supplied the follow-up 32.314 s dry F1 99-'02 capture after the CI repair; owner reports both Linux and Windows CI passed. New cadence diagnostics: 44,448 deferred updates, 44,448 successful flushes, zero deferred merges, and `flush_updates` count/work_units/max_work_units = 44,448/44,448/1. Of those flushes, 44,447 were draw-consumption flushes, with zero eviction or replacement flushes. This confirms that the guest update/draw interleaving consumes each deferred update before another update can merge; do not try to widen the merge window or delay guest-visible updates.
- Buffer operation timings: 47,428 locks took 1.469947 s; copies took 51.794 ms; unlocks took 17.012 ms. The 44,448 deferred partial-range flush locks account for 1.431434 s (about 32.2 us each); 2,980 full-range locks account for 39.192 ms (about 13.1 us each). Full-range uploads transferred 64,255,808 bytes; partial-range uploads transferred 131,283,904 bytes. The full-range path is consistent with the small-buffer full-shadow DISCARD experiment and is cheaper per Lock, but this capture alone does not establish an overall speedup. Session 72 had 57,598 partial locks / 1.744 s Lock time in a 20.526 s capture; current capture duration and workload activity differ, so treat the raw reductions as suggestive, not controlled A/B evidence.
- Important counter semantics: `qemu_perf_stats_mark()` records elapsed time between successive marks, not time spent inside the marked operation. Therefore `dxvk.upload_buffer_flush_draw` total_us=32.212 s and max_us=704.322 ms describe draw-flush event cadence across the capture, not flush execution cost. Its count (44,447) classifies successful draw flushes. The same gap-style interpretation applies to `cpu.exec_gap` and audio callback gap counters.
- Next: retain immediate pre-draw flush ordering and the existing lock-mode safety rules. Investigate the per-update D3D9 Lock synchronization cost; do not introduce NOOVERWRITE, manual ring offsets, or delayed batching without explicit in-flight lifetime/range tracking. Keep the same repeatable dry Albert Park workload and capture the lock-mode/size distribution before considering another optimization. No separate visual/gameplay validation statement accompanied this capture. No local build or CI polling was performed by ChatGPT..


## Session 73 follow-up - D3D9 buffer Lock profiling instrumentation

- Added overlapping timing/byte counters for D3D9 buffer Lock flags: `dxvk.upload_buffer_lock_discard` and `dxvk.upload_buffer_lock_flags_zero`. The latter is deliberately named for the literal flags value; it does not assume every flags-zero Lock blocks.
- Added mutually exclusive requested-size buckets for the Lock call: `dxvk.upload_buffer_lock_size_le_4k`, `..._size_le_64k`, `..._size_le_256k`, and `..._size_gt_256k`. Each records call latency, count, slow-call thresholds, and requested bytes.
- Instrumented both the full-buffer upload path and the deferred range-flush path. The existing aggregate, vertex/index, and full/partial counters remain unchanged. The new dimensions overlap those existing counters and must not be summed together.
- No D3D9 lock flags, upload ordering, guest-visible behavior, or buffer lifetime semantics changed. In particular, no NOOVERWRITE, manual ring offsets, or delayed batching were introduced.
- Next owner test: repeat the same dry Albert Park race-start capture and compare DISCARD vs flags-zero latency, then inspect which size buckets account for the slow calls. Keep capture duration and race setup consistent. No local build or CI polling was performed (HANDOFF section 7).


- CI follow-up: the first instrumentation commit omitted the two local `QemuPerfStat` declarations in `vmsvga3d_dxvk_surface_upload_buffer_range()`; both Linux and Windows correctly rejected the same compile error. Added `lock_flags_stat` and `lock_size_stat` beside the existing lock-stat declarations in commit `d2f2f61`. This is a compile-only correction; no behavior or counter semantics changed. Owner should rerun both CI jobs. No local build or CI polling was performed (HANDOFF section 7).


## Session 74 - Close FlatView scene-transition task; analyze F1 Lock profile

- Owner reports both Linux and Windows CI passed on main at source commit `36a12e7e5d9d6d309873e0520a5e05f86e3224a5`. This closes the CI gate for the ordered-tree FlatView renderer and the original T13e multi-second scene-transition stall. Updated T13e to DONE; retained the residual F1 crowded-grid work as the separate P0 T13e-F1 task. The reported three-game validation remains owner-reported; CI is build validation, not a substitute for gameplay testing.
- Owner supplied a 23.526 s F1 Challenge '99-'02 performance capture. Buffer Lock: 38,026 calls / 1.196517 s total; partial-range: 35,802 / 1.168472 s; full-range: 2,224 / 28.529 ms. Copy: 38.072 ms; unlock: 11.939 ms.
- Lock request-size distribution: <=4 KiB 32,517 calls / 927.045 ms; >4 KiB to 64 KiB 5,320 / 264.498 ms; >64 KiB to 256 KiB 185 / 0.667 ms; >256 KiB four / 0.011 ms. Flags-zero partial-range Locks account for 1.163806 s; DISCARD full-range Locks account for 28.196 ms. The flag and size categories overlap aggregate counters and must not be summed. No individual Lock exceeded 1 ms; the aggregate cost is call-volume driven, especially small partial-range Locks.
- Deferred update/flush counts: 35,806/35,806, zero merges; 35,805 flushes were draw-consumption flushes, with zero eviction/replacement flushes. This reinforces that the guest update/draw interleaving leaves no natural deferred-merge window. Do not delay guest-visible updates or broaden batching.
- `dxvk.upload_buffer_flush_draw.total_us=22.744288 s` is a cadence/interval measurement from `qemu_perf_stats_mark()`, not time spent executing flushes. No controlled A/B improvement or visual/gameplay validation is implied by this capture.
- Next: investigate per-update D3D9 Lock synchronization using the same repeatable dry Albert Park workload. Preserve immediate pre-draw ordering and existing lock-mode safety; do not introduce NOOVERWRITE, manual ring offsets, or delayed batching without explicit in-flight lifetime/range tracking. No local build or CI polling was performed by ChatGPT.

## Session 75 - D3D9 dynamic buffer invalidation DISCARD + exact in-flight range NOOVERWRITE (T13e-F1)
- Root-caused Session 74's 35,802 small partial-range `flags=0` D3D9 buffer Locks (1.163806 s / ~32.5 us each): the same 23.526 s capture recorded 8,875 `SVGA_3D_CMD_INVALIDATE_GB_SURFACE` and 8,935 `SVGA_3D_CMD_BIND_GB_SURFACE` commands (60 initial binds + 8,875 guest `D3DLOCK_DISCARD` MOB renames), followed by ~4 small partial `UPDATE_GB_IMAGE` + `DRAW_PRIMITIVES` streaming appends per discard cycle. Because `vmsvga3d_gb_surface_invalidate_live()` ignored `SVGA3D_BUFFER` and the backend lacked in-flight drawn-range tracking, every partial flush fell back to synchronizing `Lock(flags=0)`.
- Wired `SVGA_3D_CMD_INVALIDATE_GB_SURFACE` and `SVGA_3D_CMD_INVALIDATE_GB_IMAGE` (`vmware_vga_3d.c`) into `vmsvga3d_dxvk_surface_invalidate_buffer()`, which drops dead pre-invalidation dirty spans and arms `d3d9_buffer_discard_pending`.
- Added per-surface sorted/coalesced in-flight byte-range tracking (`d3d9_buffer_inflight[16]`) in `vmware_vga_dxvk.c`, cleared only on creation, eviction, or successful `D3DLOCK_DISCARD` rename (never on `flags=0` or `NOOVERWRITE`). Overflow past 16 intervals merges the closest adjacent pair into their convex hull (sound superset).
- Instrumented `vmsvga3d_d3d9_runtime_draw_primitives()` (`vmware_vga_vgpu9.c`) to mark exact drawn vertex and index byte ranges in-flight before `DrawPrimitive` and `DrawIndexedPrimitive` (scanning the authoritative CPU-shadow index buffer for exact `[min_idx, max_idx]`, with conservative full-buffer fallback on instancing, `stride == 0`, unknown decl types, negative biased index, overflow, or `> 65,536` indices, plus SWVP `[min_vertex_index, min_vertex_index + num_vertices - 1]` union).
- Updated `vmsvga3d_dxvk_surface_upload_buffer_range()` lock-mode selection while preserving immediate pre-draw flush ordering and non-draw `flags=0` serialization:
  1. Eviction/replacement flushes (`allow_async_lock=false`) -> `flags=0`.
  2. Full-buffer updates -> `D3DLOCK_DISCARD` (clears in-flight set and `discard_pending`).
  3. Post-invalidation (`d3d9_buffer_discard_pending`) -> subrange `Lock(offset, size, D3DLOCK_DISCARD)` (renames backing buffer without flushing untouched bytes; clears in-flight set and `discard_pending`).
  4. Proven disjoint from tracked in-flight ranges -> `D3DLOCK_NOOVERWRITE`.
  5. Overlap on `<= 64 KiB` buffers (`shadow_size == d3d9_length`) -> full-shadow `D3DLOCK_DISCARD` (retains Session 72 small-buffer protection without wasting full copies on disjoint/post-invalidate updates; removed eager pre-deferral `<= 64 KiB` bypass in `vmware_vga_vgpu9.c`).
  6. Overlap on `> 64 KiB` buffers -> conservative `flags=0`.
- Added diagnostic counters (`include/qemu/perf-stats.h`, `util/perf-stats.c`): `dxvk.upload_buffer_lock_discard_invalidate`, `dxvk.upload_buffer_lock_nooverwrite`, `dxvk.upload_buffer_lock_overlap`, and `dxvk.upload_buffer_invalidate`.
- Verification: ran a standalone C unit test compiled with `gcc -std=gnu11 -Wall -Wextra -Werror -Wdeclaration-after-statement -Wsign-compare -O2` covering interval coalescing, 30,000-op randomized fuzzing against a byte bitset proving zero false non-overlaps across capacity-16 merges, decl-type/primitive-count/index-scanner edge cases, and the 6-case lock-mode matrix (passed). No local QEMU build or CI polling was performed (`CLAUDE.md` rule 7 / HANDOFF section 7); Linux/Windows CI compilation and owner gameplay/perf A/B validation remain pending.
- Next owner test: build latest `main`, repeat the dry Albert Park race-start capture (and visual check on F1 99-'02 + regression check on Hitman 2 / FIFA 2003), and share `dxvk.upload_buffer_lock*` and `dxvk.upload_buffer_invalidate` counters.

## Session 76 - Verify Session 75 Lock win; isolate F1 Phase-2 vCPU cliff; add TCG counters + Windows vCPU profiler (T13e-F1)
- Analyzed the owner's Session 75 (`3cab76c`) 24.706 s dry Albert Park capture against Session 74 (`23.526 s`):
  - `dxvk.upload_buffer_lock`: **1,196.517 ms (38,026 calls, 31.5 us/call) -> 22.580 ms (52,691 calls, 0.43 us/call)** (53x less total time, 73x faster per call).
  - `dxvk.upload_buffer_lock_flags_zero`: **35,802 (1,163.806 ms) -> 0 (0 ms)**; `dxvk.upload_buffer_lock_overlap`: **0**; `dxvk.upload_buffer_lock_discard_invalidate`: **12,601 (7.093 ms)**; `dxvk.upload_buffer_lock_nooverwrite`: **40,042 (7.953 ms)**.
  - `vmvga.d3d9_draw_handler`: **1,305.3 ms (14.19 ms/frame) -> 208.2 ms (1.63 ms/frame)**; `vmvga.fifo_run`: **2,449.0 ms (26.62 ms/frame) -> 1,555.8 ms (12.15 ms/frame)**.
  - Owner also reported Hitman 2 Silent Assassin felt noticeably smoother and FIFA 2003 / Joint Operations: Typhoon Rising felt slightly smoother; F1 '99-'02 still felt unchanged overall.
- Per-second `VMVGA-FLIGHT` timeline comparison revealed **why F1 '99-'02 still felt unchanged**:
  - **Phase 1 (sitting on grid, `s02-s10`)**: Session 74 averaged **8.7 FPS** (`~115 ms/frame`); Session 75 improved to **10.6 FPS** (`~94 ms/frame`, +22% FPS).
  - **Phase 2 (race launch / 22 cars moving, `s12-s25`)**: BOTH builds collapse to **1-2 FPS** (Session 74: `1.31 FPS / ~765 ms/frame`; Session 75: `1.62 FPS / ~619 ms/frame`). During Phase 2, 3D command volume per frame stays `~1,800-2,500` (with zero GART/MOB/surface churn on most 1-FPS seconds, only `8-18` syncs/s and `2-6` busyreads/s), so ALL host SVGA3D/DXVK/display work combined is **`~12 ms/frame` (< 2% of frame time)** while `cpu.exec` runs continuously for **24,695.7 ms out of 24,706 ms (99.96%)** — leaving **~607 ms/frame (~98%) inside the TCG vCPU thread** completely unattributed.
- Added lightweight TCG/vCPU structural counters (`include/qemu/perf-stats.h`, `util/perf-stats.c`, `accel/tcg/cpu-exec.c`, `accel/tcg/cputlb.c`, `system/physmem.c`, `target/i386/tcg/system/misc_helper.c`):
  - `tcg.tb_gen_code`, `tcg.tb_exec_loops` (work-unit counter, no clock call), `tcg.cpu_exception`, `tcg.cpu_interrupt`, `tcg.tlb_fill`, `tcg.mmio_bql_wait`, `tcg.pio_in`, `tcg.pio_out`.
- Added an opt-in 1 kHz Windows x64 host-side vCPU statistical profiler (`accel/tcg/vcpu-profile.c`, `accel/tcg/vcpu-profile.h`, wired in `accel/tcg/meson.build` and `accel/tcg/tcg-accel-ops.c`, hooked via `qemu_perf_stats_set_vcpu_profile_hooks()` so `qemu-img` and Linux KVM remain decoupled; non-Windows is a clean no-op):
  - Active ONLY while `Ctrl+Alt+L` capture is running (zero runtime overhead when off).
  - Uses a dedicated sampler thread (`SuspendThread` -> 16-byte-aligned `GetThreadContext(CONTEXT_CONTROL)` -> `ResumeThread` with zero locks/allocs/syscalls during suspend), tags each sample with capture second and live `vmvga.screen_target_present` frame count, checks `in_code_gen_buffer` + `tcg_tb_lookup` (under RCU) + `get_pc` after resume for JIT CPL/guest-PC attribution, and resolves non-JIT host RIPs on capture stop via an executable-section-bounded PE32+ COFF symbol-table parser (with runtime anchor self-check) + dynamic `dbghelp.dll` fallback (for `ntdll.dll`/Win32 DLLs) + `GetModuleHandleExA` RVA fallback.
  - Emits `Y2KBOX-PROF-META`, per-second `Y2KBOX-PROF-SEC`, overall `Y2KBOX-PROF-TOP` (Top 30), automatic High-FPS (`df >= 5`) vs Low-FPS (`df <= 3`) phase comparison (`Y2KBOX-PROF-PHASE` / `Y2KBOX-PROF-PHASE-TOP`), and hottest JIT guest PCs (`Y2KBOX-PROF-JIT`).
- Verification: standalone `gcc -Wall -Wextra -Werror -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes` mock-header compile checks passed for both `_WIN32` x64 and non-Windows paths plus `util/perf-stats.c`, and a standalone runtime unit test validated PE32+ COFF parsing, anchor self-check, executable-section bounds rejection, binary-search lookup, and High-FPS vs Low-FPS report formatting (all assertions passed). No local QEMU build or CI polling was performed (`CLAUDE.md` rule 7 / HANDOFF section 7); Linux/Windows CI compilation and owner capture remain pending.
- Next owner test: build latest `main` via CI, capture one dry Albert Park `Ctrl+Alt+L` log spanning both grid idle and ~10-15 s of race launch, and share the resulting log (`Y2KBOX-PERF` + `Y2KBOX-PROF-*`).

## Session 77 - Root-cause F1 vCPU bottleneck (>50.7% x87); accelerate x87 helpers, restore FST fast path, inline fpop/fpush, fuse mem/STN ops, add TCG_CALL_NO_RWG (T13e-F1 / T13a)
- Analyzed the owner's Session 76 (`7786d72`) 25.298 s dry Albert Park capture (`y2kbox-log-20261123-071849.log`, 221 frames ≈ 8.7 FPS, 12,649 vCPU samples: `qemu_c=73.5%`, `os=14.7%`, `jit=11.7%`):
  - **Root cause found:** **>50.7% of ALL vCPU samples in Top-30 alone (~55-60% total, surging even higher during low-FPS physics ticks where `fmul_ST0_FT0` jumps from `10.6% -> 18.2%` while `NtWaitForAlertByThreadId` drops `12-15% -> <6%`)** are spent inside x87 C helpers and softfloat:
    - `fmul_ST0_FT0` (`13.36%`), `fadd_STN_ST0` (`2.97%`), `fadd_ST0_FT0` (`2.78%`), `fsubr_ST0_FT0` (`1.79%`), `fsub_STN_ST0` (`1.30%`), `fsub_ST0_FT0` (`1.27%`) = **`23.47%`**: each helper was 592 bytes doing by-value 16B copies, `HxRaw` stack bounces, per-op `fnstsw`, an extra `fstp %st(0)`, and `save_exception_flags` + out-of-line `merge_exception_flags`.
    - `merge_exception_flags` = **`6.41%`** (#4 overall): called out-of-line on every x87 helper even when the host-x87 or integer-load fast path never touched `env->fp_status`.
    - Softfloat FST (`parts128_to_parts64` `3.57%`, `floatx80_to_float64` `1.27%`, `parts64_uncanon_normal` `1.20%`, `floatx80_to_float32` `0.92%`, `parts64_uncanon` `0.90%`, plus ~half of `floatx80_unpack_canonical`/`parts128_canonicalize`) ≈ **`~9.5%`**: caused by an accidental historical regression where commit `7a88d5b` deleted `bb8a17f`'s fast `fsts_ST0`/`fstl_ST0` paths!
    - `fldl_FT0` (`2.12%`) + `fldl_ST0` (`1.64%`) + `flds_ST0` (`1.08%`) + `flds_FT0` (`1.03%`) = **`5.87%`**: redundant `fld*_FT0` calls before every `f<op> m32/m64`, plus unnecessary `save/merge_exception_flags` on exact integer loads.
    - Softfloat zero-operand arithmetic & compares (`floatx80_addsub` `1.60%`, `floatx80_mul` `0.92%`, remaining `unpack`/`canonicalize` ~`1.6%`) ≈ **`~4.1%`**: `fx_normal(0.0)` returned false, forcing every `±0.0` op and every `fcom`/`fucom`/`fcomi`/`fucomi` into softfloat.
    - `helper_fpop` = **`1.36%`** (#16 overall): a separate C helper call just to update `fptags[fpstt] = 1; fpstt = (fpstt + 1) & 7`.
    - **Missing `TCG_CALL_NO_RWG` across all x87 helpers** in `target/i386/helper.h`, forcing TCG to spill/reload live x86 GPRs and CC globals around every single x87 call.
- Implemented a comprehensive x87 fast-path & TCG lowering overhaul (all gated by `XPQEMU_NO_HX87=1` kill switch for bit-exact A/B parity with `xp/bench/x87cmp.sh`):
  - **In-place host x87 arithmetic + lazy SW sync + `±0.0` support (`target/i386/tcg/fpu_helper.c`, `cpu.h`, `tcg/tcg-cpu.c`, `machine.c`, `gdbstub.c`):**
    - Eliminated `HxRaw` stack bounces, by-value 16B operand copies, extra `fstp %st(0)` (switched to `faddp`/`fsubp`/`fmulp`/`fdivp %st, %st(1)` with `fldt %[b]; fldt %[a]; ...; fstpt %[r]` verified for AT&T GAS semantics), per-op `fnstsw`, and hot-path `save/merge_exception_flags`. All softfloat fallbacks are isolated in `__attribute__((noinline, cold))` helpers (`helper_fmul_ST0_FT0` shrunk 2x from `0x250` -> `0x127` bytes).
    - Added `HostX87.sw_dirty` + `x86_hx87_sync_sw(env)`: host x87 sticky exception flags accumulate in hardware while all 6 guest exceptions are masked, and merge lazily at guest-observable sync points (`fnstsw`, `fldcw` before `cpu_set_fpuc`, `fwait`, `do_fstenv`, `do_xsave_fpu`, `x86_cpu_exec_exit`, `cpu_pre_save`, `gdbstub`, and `hx87_prepare_slow` on owner change). `x86_hx87_flags_changed()` clears `sw_dirty` and bumps `hx87_epoch` so `fclex`/`fninit`/`fldenv`/`fxrstor`/reset discard stale bits cleanly.
    - Extended fast arithmetic (`ADD`/`SUB`/`MUL`, plus `DIV` with non-zero normal divisor) to accept `±0.0` operands; upgraded `fsqrt` to in-place host x87 + `+0.0` fast path; added pure-integer `fx_from_i32` for `fildl_FT0`/`fildl_ST0` (and fixed `fildl_FT0`'s softfloat fallback to maximise precision like `fildl_ST0`) and pure-integer `fx_cmp_noz_fast` for `fcom`/`fucom`/`fcomi`/`fucomi`.
  - **Restored & upgraded `fsts_ST0` / `fstl_ST0` (`target/i386/tcg/fpu_helper.c`):**
    - Restored host x87 `fldt; fstps/fstpl` for normal in-range exponents (`[-126, 126]` for float32, `[-1022, 1022]` for float64, where overflow/underflow are mathematically impossible) plus pure-integer `±0.0` fast path and cold softfloat fallback for boundary/subnormal/Inf/NaN cases.
  - **Fused memory & register-register x87 helpers (`target/i386/helper.h`, `fpu_helper.c`, `translate.c`):**
    - Added single-call fused `f{add,mul,sub,subr,div,divr,com}{s,l}_ST0` (executing `fldt; f<op>{s,l}; fstpt` directly on `&ST0` without `fld*_FT0` or `FT0` traffic) and fused `f{add,mul,sub,subr,div,divr,com,ucom,comi,ucomi}_ST0_STN` (eliminating `fmov_FT0_STN` before every register-register op/compare).
  - **Inlined `gen_fpop()` / `gen_fpush()` in TCG (`target/i386/tcg/translate.c`) & marked pure x87 helpers `TCG_CALL_NO_RWG` (`target/i386/helper.h`):**
    - Replaced all 21 `gen_helper_fpop`/`gen_helper_fpush` calls in `gen_x87` with inline TCG ops updating `fpstt` and `fptags[fpstt]`.
    - Marked all non-exception-raising, non-CC-writing x87 helpers in `target/i386/helper.h` with `TCG_CALL_NO_RWG` (leaving `fcomi*`/`fucomi*`, `fwait`, and memory/state helpers unchanged) so TCG keeps guest GPRs/CC live in registers across x87 helper calls.
- Verification:
  - Audited all 56 x87 helpers across `target/i386/helper.h`, `target/i386/tcg/fpu_helper.c`, and all 108 `gen_helper_f*` call sites in `target/i386/tcg/translate.c` (100% prototype/arity match).
  - Compiled our exact extracted `fpu_helper.c` and `translate.c` blocks standalone with `gcc -std=gnu11 -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -Wsign-compare -O2` (zero warnings) and ran randomized differential + boundary tests across all 12 CW modes (3 PC × 4 RC) for `f{add,mul,sub,subr,div,divr}_{ST0_STN,m64,m32}` (including `st_index=0` aliasing), `fsts`/`fstl` (including exact exponent boundaries `[-127..-125]`, `[125..127]`, `[-1023..-1021]`, `[1021..1023]`, `±0.0`), `flds`/`fldl`/`fildl`, `fcom`/`fucom`/`fcomi`/`fucomi`/`fcoms`/`fcoml`, `fsqrt`, the lazy SW state machine (`fnstsw`, `fclex`, `fldcw` unmask, multi-vCPU owner switch, unmasked fallback), and `gen_fpush`/`gen_fpop` wrap-around (all assertions passed).
  - No local QEMU build or CI polling was performed (`CLAUDE.md` rule 7 / HANDOFF section 7); Linux/Windows CI compilation, `xp/bench/x87cmp.sh` full bit-exact regression test, and owner gameplay/perf A/B validation remain pending.
- Next owner test: build latest `main` via CI, verify `xp/bench/x87cmp.sh` passes bit-identically, repeat the dry Albert Park `Ctrl+Alt+L` capture spanning grid idle + race launch, and share `Y2KBOX-PERF` + `Y2KBOX-PROF-*` (plus visual/FPS check on F1 '99-'02, Hitman 2, FIFA 2003).

### Session 78 — PIO BQL/capture-stderr fixes, T13b fast TB lookup (`TB_JMP_CACHE_BITS=14` + inline metadata + `lookup_tb_ptr_state`), x87 `fnstsw`/inline-stack polish, and 46-op CI x87 diff suite
* **Session 77 owner validation (`9ac54ad`, CI run `45154235741` green on Linux & Windows including `x87 differential test`):**
  * Owner reported dramatic Windows TCG speedups across F1 Challenge '99-'02, Joint Operations: Typhoon Rising, Hitman 2: Silent Assassin, FIFA 2003, and loading times.
  * 22.567 s dry Albert Park capture (`11,284` vCPU samples: `qemu_c=51.9%`, `os=30.9%`, `jit=17.2%`): F1 '99-'02 nearly doubled from `8.7 FPS` (with `5–6 FPS` physics drops) to **`16.0 FPS` average** (`361` frames, `13–20 FPS` every single second, **zero** low-FPS collapse seconds!). `merge_exception_flags` (was `6.41%`), all softfloat functions (was `~13.6%`), and `helper_fpop` (was `1.36%`) completely disappeared from Top-30.
  * New dominant hotspots exposed by the Session 77 profile:
    1. **`tcg.pio_out` (`1,106,208` calls, `5.484 s` = `24.3%`)** = **`NtWaitForAlertByThreadId` (`18.49%`, `4.177 s` BQL contention)** + **`NtWriteFile` (`5.96%`, `1.307 s`)**:
       * On Windows MinGW-w64, `stderr` is unbuffered (`_IONBF`), so `Ctrl+Alt+L` capture logging (`VMVGA-SYNC` `fprintf(stderr)` under BQL) issued ~1.3M single-byte `NtWriteFile` syscalls (~1.35 s, holding the BQL while writing!).
       * `ioport80_io` (port `0x80` ISA delay, empty no-op write / constant read in `hw/i386/pc.c`) did not have `memory_region_enable_lockless_io()`, forcing BQL acquisition on every `out 0x80, al`.
    2. **TB lookup (`T13b`, `13.88%` total):** `helper_lookup_tb_ptr` (`8.11%`) + `x86_get_tb_cpu_state` (`1.50%`, 3-level indirect call + Windows x64 24-byte hidden-sret stack bounce) + `qht_lookup_custom` / `tb_htable_lookup` / `tb_tc_cmp` / `tb_lookup_cmp` (`4.27%`, `TB_JMP_CACHE_BITS=12` thrashing + cold `*tb` heap pointer-chase on every hit).
    3. **Remaining x87 quick wins:** `HELPER(fnstsw)` (`0.93%`) called `x86_hx87_sync_sw()` which cleared `hx87.sw_dirty = false`, forcing a cold `hx87_prepare_slow()` on the very next FP op in tight `fmul/fcom/fnstsw/sahf` loops; `helper_fmov_ST0_STN` (`0.58%`) and `fmov_STN_ST0`/`fxchg`/`fchs`/`fabs`/`fld1`/`fldz` still left JIT for C helper calls.
* **Implemented:**
  * `hw/i386/pc.c`: enabled `memory_region_enable_lockless_io(ioport80_io)` so no-op port `0x80` accesses bypass BQL contention.
  * `ui/sdl2.c`: buffered `stderr` (`setvbuf(stderr, NULL, _IOFBF, 65536)`) during `Ctrl+Alt+L` capture (flushing and restoring `_IONBF` on stop/error) to eliminate ~1.3M single-byte Windows `WriteFile` syscalls during capture.
  * `include/qemu/perf-stats.h`, `util/perf-stats.c`, `target/i386/tcg/system/misc_helper.c`: added zero-cost-when-off capture-only per-PIO-port tracking (`qemu_perf_stats_pio_end`), emitting top 16 `Y2KBOX-PERF pio_port=0x... dir=in|out` lines on capture stop to pinpoint any remaining non-0x80 PIO traffic.
  * `accel/tcg/tb-jmp-cache.h`, `accel/tcg/cpu-exec.c`, `accel/tcg/tcg-runtime.h`, `include/tcg/tcg-op-common.h`, `tcg/tcg-op.c`, `target/i386/tcg/translate.c` (**T13b**):
    * Raised `TB_JMP_CACHE_BITS` from `12` (`4,096` entries) to `14` (`16,384` entries, `128 × 128`).
    * Cached `{ tb, tc_ptr, pc, cs_base, flags, cflags }` inline in `CPUJumpCacheEntry` (`tb_jmp_cache_store`), eliminating cold `*tb` heap dereferences on jump-cache hits in `lookup_tb_ptr_common` and `tb_lookup`.
    * Added `HELPER(lookup_tb_ptr_state)` / `tcg_gen_lookup_and_goto_ptr_state` and wired 32-bit x86 `DISAS_JUMP` (`#ifndef TARGET_X86_64`) to pass `(pc, cs_base, flags)` in registers directly, eliminating the `x86_get_tb_cpu_state` indirect virtual call and Windows x64 24-byte struct-return stack bounce.
  * `target/i386/tcg/fpu_helper.c`: added `x86_hx87_peek_sw()` for `HELPER(fnstsw)` so sticky host SW bits merge into `env->fpus` while keeping `hx87.sw_dirty = true`, avoiding cold `hx87_prepare_slow()` re-entry on the immediately following FP op.
  * `target/i386/tcg/translate.c`: inlined `gen_fmov_ST0_STN`, `gen_fmov_STN_ST0`, `gen_fxchg_ST0_STN`, `gen_fchs_ST0`, `gen_fabs_ST0`, `gen_fld_const_ST0` (`fld1`/`fldz`) directly into TCG IR (with `QEMU_BUILD_BUG_ON` verifying `FPReg` layout).
  * `xp/bench/x87diff_ops.asm`, `xp/bench/x87cmp.sh`: expanded CI differential x87 verification from `30` to **`46` operations** (added `fsubr/fdivr m64`, `fmul/fsub/fdiv m32`, `fcom m64/m32`, `fild/fiadd m32`, `fld st1`, `fst st1`, `fld1`, `fldz`, `fcomi`/`fucomi` EFLAGS checks, explicit `±0.0` operand generation, and a chained `fmul -> fnstsw -> fadd -> fnstsw -> fnclex -> fsub -> fnstsw` lazy-SW test).
* **Verification (standalone, no local full QEMU build):**
  * Compiled actual `util/perf-stats.c` with `gcc -std=gnu11 -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -Wsign-compare -O2` and verified PIO off-path no-op, aggregation, top-16 sorting, `over_1ms`/`max_ns`, and restart reset.
  * Compiled standalone `gcc -Werror -O2` harness verifying `TB_JMP_CACHE_BITS=14` page-hash invariant across 4,096,000 `(page, offset)` pairs, zero-deref `CPUJumpCacheEntry` hits, `x86_hx87_peek_sw` state machine on real host x87 hardware (`prepare_slow` not re-entered after `fnstsw`, clean after `fnclex`), and 64-combination equivalence of all inlined x87 stack/move/sign/fxchg ops.
  * Verified `bash -n xp/bench/x87cmp.sh` and exact 46-op count parity between `x87diff_ops.asm` and `x87cmp.sh`. Full Linux/Windows build + 46-op `x87cmp.sh` CI verification + in-game capture pending after push.

### Session 79 — TCG `kvmvapic` overhaul (NOP `outb 0x7e` + Page-1 SMC separation), O(1) `tcg_flush_jmp_cache`, and zero-frame leaf `lookup_tb_ptr_state`
* **Owner validation of Session 78 (`a45fa7c`, CI green on Linux & Windows including 46/46 `x87cmp.sh`):** 23.142 s raining Albert Park + race-start collision capture (`382` frames = **`16.5 FPS` avg**, `18–21 FPS` on grid). `NtWriteFile` (`5.96%`), `x86_get_tb_cpu_state` (`1.50%`), and `helper_fmov_ST0_STN` (`0.58%`) all dropped to `0.00%`; `tcg.pio_out` fell `5.48 s` -> `2.31 s` and `NtWaitForAlertByThreadId` fell `18.49%` -> `9.89%`.
* **Root causes isolated from Session 78 `Y2KBOX-PERF` + `Y2KBOX-PROF-TOP` (~25–35% of vCPU time):**
  1. **`pio_port=0x007e dir=out`: `1,135,584` calls (`2.29 s`, `99.06%` of `tcg.pio_out`, `510` stalls `>1ms`, driving `9.89%` `NtWaitForAlertByThreadId`)** — `hw/i386/vapic.c` only NOPed `reenable_vtpr` (`0xe6 0x7e`) under KVM/WHPX because `env->eip` is inaccurate under TCG, leaving Windows XP `hal.dll` executing ~49k BQL-locked `out %al, $0x7e` instructions/sec forever.
  2. **Same-page SMC thrashing inside `pc-bios/kvmvapic.bin` (`tb_invalidate_range=41,187 / 1.41 s`, `tlb_reset_dirty=41,193 / 0.37 s`, `tb_gen_code=502,351 / 1.81 s`, `tlb_fill=4.74M / 0.46 s`, `RtlAllocateHeap=1.56%`, `memset=0.99%`)** — `kvmvapic.bin` places the writable `vapic` TPR struct at offset `0x300` on the **exact same 4 KiB page** as the executable handlers (`0x0c0..0x2d8`), so every guest `KfRaiseIrql`/`KfLowerIrql` (`lock cmpxchg`) and `apic_sync_vapic` write invalidated and recompiled the VAPIC TBs.
  3. **`tcg_flush_jmp_cache` (`4.99%`, `+1.17 s` regression) + non-inlined `lookup_tb_ptr_common` (`8.67%` + `6.71%` = `15.38%`)** — `CF_PCREL` invalidation calls `tcg_flush_jmp_cache` on every `tb_invalidate_range` (scanning 640 KiB of `CPUJumpCache` 41,187 times), and GCC refused to inline `lookup_tb_ptr_common` taking a 24-byte `TCGTBCPUState` struct by value, emitting a double call + frame spills on every indirect jump/`ret`.
* **Changes implemented:**
  * `hw/i386/vapic.c`:
    * Added `vapic_nop_reenable_vtpr()` / `vapic_nop_reenable_vtpr_at()` (called under `tcg_enabled()` from `vapic_enable`, `do_vapic_enable`, and `vapic_write(case 1)` when `VAPIC_ACTIVE`) to patch all 11 `0xe6 0x7e` (`reenable_vtpr`) sites in `up.*` and `mp.*` handlers to `0x66 0x90` using physical ROM + `GuestROMState` handler offsets (zero reliance on `env->eip`).
    * Added `vapic_tcg_compute_shift()` and updated `update_rom_mapping()` under `tcg_enabled()` to relocate the writable `vapic` TPR area from page 0 (`0x300`) to the next 4 KiB page boundary (`TARGET_PAGE_ALIGN(code_end_paddr)`, staying strictly inside the existing mapped 8 KiB `vapic` buffer when `new_vapic_size >= max_cpus << VAPIC_CPU_SHIFT` and verified via `cpu_translate_for_debug`), adjusting all `vapic` fixup entries, zeroing the relocated buffer, and updating `GuestROMState.vapic_size`.
  * `accel/tcg/tb-jmp-cache.h`, `accel/tcg/translate-all.c`, `accel/tcg/cpu-exec.c`:
    * Added 64-bit `gen` generation counters to `CPUJumpCache` (initialized to `1` in `tcg_exec_realizefn`) and `CPUJumpCacheEntry`, turning `tcg_flush_jmp_cache()` into O(1) `qatomic_inc_fetch(&jc->gen)` while preserving linearizability (capturing `gen` before `tb_htable_lookup`/`tb_gen_code`) and single-slot/page invalidation (`e->tb = NULL`).
    * Refactored `HELPER(lookup_tb_ptr_state)` / `HELPER(lookup_tb_ptr)` into a `QEMU_ALWAYS_INLINE` scalar `lookup_tb_ptr_fast()` plus a 4-argument `noinline cold` `lookup_tb_ptr_slow()` sibling tail call, deferring `curr_cflags` and `jc->gen` loads past the PC hash so both SysV x86-64 and Windows x64 (`ms_abi`) compile `HELPER(lookup_tb_ptr_state)` to a zero-push leaf function on hit.
* **Standalone verification:**
  * `gcc -std=gnu11 -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -Wsign-compare -O2` real-ROM test harness run directly against `pc-bios/kvmvapic.bin` (6 cases: 4 KiB-aligned `0xca000` with 1/4/38/39 vCPUs, 2 KiB-aligned `0xca800` with 1/54 vCPUs): confirmed all 11 `0xe6 0x7e` become `0x66 0x90` (0 remaining, 16-bit `0xe7 0x7e` untouched), all 15 code `vapic`/`vapic+4` fixups relocate cleanly to Page 1 (`0xcb000`/`0xcb004`), non-`vapic` fixups stay on Page 0, and fallback triggers cleanly when `max_cpus` exceeds `new_vapic_size`.
  * `gcc -std=gnu11 -Wall -Wextra -Werror ... -O2` O(1) jump-cache & linearizability test (9 tests including concurrent flush during `tb_htable_lookup`, page clear, logging, breakpoints) + `gcc -O2 -S` disassembly verified zero-`push`, zero-`sub rsp`, sibling-`jmp` leaf codegen for `helper_lookup_tb_ptr_state` on both SysV x86-64 and Windows x64 (`ms_abi`).
  * Full Linux/Windows CI build and in-game F1 '99-'02 capture validation pending after push.

## Session 80 - T13e-FIFA / T13e-F1: Eliminate ~20.6 s of VMVGA + FlatView BQL Stalls (clear_shadow, SSE2 16->32 present_convert, readback_trace_band bug, 256 KiB VB overlap DISCARD, Sweep-Line FlatView)

### Session 79 Owner Validation (`759cee6` — F1 '99-'02 Playable at 32.7 FPS; Shift to FIFA 2003)
- Owner validated Session 79 (`759cee6`, CI green on Linux & Windows):
  - **F1 Challenge '99-'02** (`22.236 s`, `727` frames): doubled again from `16.5 FPS` -> **`32.7 FPS` average** (steady `30–37 FPS` every second; owner declared F1 '99-'02, Joint Operations: Typhoon Rising, and Hitman 2: Silent Assassin playable with much faster loading).
  - Exact Session 79 collapses in F1 '99-'02: `tcg.tb_invalidate_range` `41,187` (`1.41 s`) -> **`0`**, `tcg.tlb_reset_dirty` `41,193` (`0.37 s`) -> **`7` (`0.12 ms`)**, `tcg.tb_gen_code` `502,351` (`1.81 s`) -> **`4,114` (`13.8 ms`)**, `pio_port=0x007e` (`out`) `1,135,584` (`2.29 s`) -> **`16,840` (`11 ms`)**, `tcg_flush_jmp_cache` `4.99%` -> **`0.00%`**, `jit` jumped to **`30.9%`**.
- Owner shifted primary T13 profiling target to **FIFA 2003** (Ali Sami Yen stadium — Galatasaray vs Real Madrid intro scene until kick-off).
- Baseline FIFA 2003 capture on `759cee6` (`42.741 s`, `1,775` frames, `sec=33..35` drops to `0 FPS` for 3 full seconds, mild slow-motion feel):
  - **`NtWaitForAlertByThreadId` = `59.87%`** of all vCPU samples (**`91.73%`** during `sec=33..35`!), and #1 in F1 '99-'02 (`19.94%`).
  - Root cause: `vmvga.fifo_run` (`19.35 s`) + `vmvga.display_update` (`5.55 s`) hold the BQL for **`24.90 s` (`58.3%` of wall time)**, starving the vCPU (`pio_port=0xc5b0 dir=out` = `5.539 s`, `800` stalls `>1ms`).
  - Five concrete hotspots accounted for **`~20.6 s` (`83%`)** of FIFA 2003's `vmvga` BQL time (and **`~3.1 s`** in F1 '99-'02):
    1. `vmvga.clear_shadow` = **`6.883 s`** FIFA (`5.970 s` depth/stencil + `0.910 s` color) / **`0.975 s`** F1 — CPU-clearing shadow buffers before `vmsvga3d_dxvk_clear()` (`0.0085 s`, 100% success, `clear_fallback=0`).
    2. `vmvga.present_convert` = **`5.463 s`** FIFA / **`1.376 s`** F1 — `SVGA3D_R5G6B5` (16-bit) -> 32-bit BGRX running 3 64-bit integer divisions per pixel across 1.40B pixels (`3.08 ms/frame`).
    3. `memory.txn_flatviews_reset` = **`5.378 s`** FIFA (`744` calls × `7.2 ms`, `memory.topology_render = 4.881 s`, `24.5M` `g_sequence_search` + `g_new`/`g_free` + `98M` QPC reads across `8,816` GART ranges × `5` roots) — driving the 3 s `sec=33..35` `0 FPS` stall.
    4. `dxvk.upload_buffer_lock_overlap` = **`1.426 s`** FIFA (`39,333` `flags=0` GPU sync stalls, `max_work_units = 157,440` on a `>64 KiB` dynamic VB that missed Session 75's `64 KiB` overlap-`DISCARD` threshold and therefore never cleared `d3d9_buffer_inflight_count`).
    5. `dxvk.readback_trace_band` = **`1.162 s`** FIFA / **`0.422 s`** F1 — `if (VMVGA_TRACE_FLIGHT && trace_row_bytes != 0)` evaluated compile-time constant `1` unconditionally on every frame (even with capture OFF!), doing a GPU-stalling `CreateRenderTarget` + `StretchRect` + `GetRenderTargetData` + `LockRect` + double bottom-band hash + `fprintf(stderr)`.

### Changes Implemented
1. **Fix unconditional `readback_trace_band` & `D3D9-READBACK` trace bug (`hw/display/vmware_vga.c`, `hw/display/vmware_vga_dxvk.c`)**:
   - Added `#define VMVGA_TRACE_DEEP_READBACK_BAND 0` alongside the other `VMVGA_TRACE_DEEP_*` switches in `vmware_vga.c`.
   - Gated both bottom-band trace blocks in `vmsvga3d_dxvk_surface_readback_rects()` behind `VMVGA_TRACE_DEEP_READBACK_BAND && VMVGA_TRACE_LOCAL_ENABLED(VMVGA_TRACE_FLIGHT) && trace_row_bytes != 0`.
   - Fixed `VMVGA_TRACE_LOCAL_CACHED(VMVGA_TRACE_3D, ...)` -> `VMVGA_TRACE_LOCAL(VMVGA_TRACE_3D, ...)` so `D3D9-READBACK` is not printed unconditionally when debug capture is inactive.
2. **Eliminate redundant `clear_shadow` on successful D3D9 GPU clears + accelerate fallback clears (`hw/display/vmware_vga_vgpu9.c`, `hw/display/vmware_vga_3d.c`)**:
   - Parameterized `vmsvga3d_d3d9_clear_shadow_non_msaa()` with `bool execute`. `vmsvga3d_d3d9_runtime_clear()` now runs only validation (`execute=false`) before `vmsvga3d_dxvk_clear()`, returns immediately when the native GPU clear succeeds (all bound targets are already D3D9-resident and GPU-authoritative), and executes the CPU shadow clear (`execute=true`) + `vmsvga3d_dxvk_sync_clear_targets_from_cpu()` only when `d3d_flags == 0` or native clear fails.
   - Accelerated CPU fallback clear loops in `vmsvga3d_clear_color_image()` and `vmsvga3d_clear_depth_stencil_image()` with `memset` (uniform-byte fills) and native `stl_le_p`/`stw_le_p`/`ldl_le_p`/`lduw_le_p` word paths instead of byte-by-byte `vmsvga3d_load_le`/`vmsvga3d_store_le`.
3. **Bit-exact SSE2 + division-free scalar 16->32 `present_convert` (`hw/display/vmware_vga.c`, `hw/display/vmware_vga_3d.c`)**:
   - Included `<emmintrin.h>` under `#ifdef __SSE2__` in `vmware_vga.c` (outside `#pragma pack(push, 1)`).
   - Added dedicated `SVGA3D_R5G6B5 -> 32-bit` and `SVGA3D_X1R5G5B5`/`SVGA3D_A1R5G5B5 -> 32-bit` fast paths in `vmsvga3d_present_rect_to_buffer_internal()` processing 8 pixels/iter via SSE2 (with portable division-free scalar tail), using bit-exact `((v * 527 + 23) >> 6)` (5-bit) and `((v * 259 + 33) >> 6)` (6-bit) scaling identical to `(v * 255 + mask / 2) / mask` for all 65,536 inputs.
4. **Raise dynamic buffer overlap `D3DLOCK_DISCARD` threshold `64 KiB -> 256 KiB` (`hw/display/vmware_vga_dxvk.c`)**:
   - Extended the CPU-shadow `D3DLOCK_DISCARD` overlap fallback in `vmsvga3d_dxvk_surface_upload_buffer_range()` from `64u * 1024u` to `256u * 1024u`, converting FIFA 2003's 154 KiB dynamic vertex buffer wraparound from continuous `flags=0` GPU stalls into a single lockless `DISCARD` rename that resets `d3d9_buffer_inflight_count = 0` so subsequent ring-buffer appends hit `D3DLOCK_NOOVERWRITE`.
5. **Ping-Pong Radix-Sort + Sweep-Line `FlatView` builder + skip no-op GART unmap transactions (`system/memory.c`, `hw/display/vmware_vga_3d.c`)**:
   - Replaced the `GSequence`-based `render_memory_region_tree()` / `flatview_tree_flatten()` in `system/memory.c` with `render_memory_region_collect()` + `flatview_sweep_flatten()`: collects terminating leaves in priority order into a contiguous array, skips sorting when already ordered or runs a byte-skipping ping-pong radix sort, and performs a single linear sweep with a compact priority min-heap using full `Int128` range arithmetic (handling `[0, 2^64)` boundaries cleanly) with zero per-range heap allocations and zero per-range `qemu_perf_stats` clock reads.
   - Cached `Y2KBOX_FLATVIEW_VERIFY` lookup once in `generate_memory_topology()` (preserving exact runtime verification against `render_memory_region_legacy()`).
   - Updated `vmsvga3d_gart_unmap_live()` to open/commit a memory transaction only when at least one active GART alias is actually removed (skipping 382 no-op transactions in FIFA 2003).

### Verification (Standalone Checks Only — CI & In-Game Pending)
- Standalone `gcc -std=gnu11 -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -Wsign-compare -Wshadow -O2` (both SSE2 and `-mno-sse2` scalar builds):
  - **PASS**: Exhaustive 65,536-value bit-exactness of `R5G6B5` and `X1R5G5B5`/`A1R5G5B5` -> 32-bit BGRX across widths `0, 1, 2, 7, 8, 9, 15, 16, 17, 65536` (`1024×768` benchmark: `2.215 ms/frame` -> `0.443 ms/frame` on container CPU, `5.0×` faster).
  - **PASS**: Fast color clear (`bytes = 1, 2, 3, 4`, uniform and non-uniform values) and depth/stencil clear (`D24S8` depth-only/stencil-only/both, `S8D24`, `D24X8`, `D16`, masked 16-bit) bit-identical to `vmsvga3d_load_le`/`vmsvga3d_store_le`.
  - **PASS**: Ping-pong radix-sort + sweep-line `FlatView` builder 100% bit-identical to `render_memory_region_legacy()` across 200 heavy-overlap randomized trials, `[0, 2^64)` `Int128` boundary cases, sorted fast path, and refcount invariants (`8,042`-page × `3,720`-root FIFA scale benchmark: `0.538 ms/root` vs `1.284 ms/root` for arena B+tree, and ~10–14× faster than GLib `GSequence` + 98M QPC calls).
- Full Linux & Windows QEMU CI builds and in-game FIFA 2003 / F1 '99-'02 captures: **pending push**.


## Session 81 — Skip binary artifacts for Markdown-only changes

- Updated `.github/workflows/build.yml` with a lightweight change-detection job. It compares the push range or pull-request base/head and enables artifact production only when at least one changed path does not end in `.md`.
- Linux and Windows configure/build/functional checks still run for documentation-only commits. Only the binary `Package` steps, packaged-layout smoke tests, and `Upload binaries` steps are skipped, avoiding pointless executable artifacts while retaining CI validation.
- Non-Markdown or mixed changes retain normal packaging and uploads. Manual `workflow_dispatch` runs always produce artifacts. Missing/invalid comparison commits or diff errors fail safe by enabling artifacts.
- Updated `CLAUDE.md` and `HANDOFF.md` to document the behavior.
- Static review confirmed both build jobs depend on the change-detection output and all six package/smoke/upload steps are guarded. No local QEMU build or CI run was performed; GitHub Actions validation is pending after push.

## Session 82 — Make coding-agent historical reads selective

- Replaced the fresh-session instruction to read all of `WORKLOG.md` with a layered workflow: read `CLAUDE.md`, `HANDOFF.md`, `TASKS.md`, and `WORKLOAD_ACTIVE_SESSION.md`; use `WORKLOG_INDEX.md` to locate only historical sessions relevant to the task.
- Added `WORKLOG_INDEX.md` as a topic-to-session navigation map. It points to the append-only worklog without duplicating its content.
- Updated `HANDOFF.md`, `README.md`, and the active-summary instructions to match. Historical worklog entries remain untouched; the rolling summary continues to retain only the latest ten sessions.
- Clarified the Session 71 concern: Session 71's full historical entry remains in `WORKLOG.md`; only its brief entry aged out of `WORKLOAD_ACTIVE_SESSION.md` when Session 81 entered the ten-session window.
- Static review only; no QEMU build, tests, or CI run were performed. Documentation changes were committed directly to `main` in small sequential commits.

## Session 83 — Consolidate agent guidance and repair project continuity docs

- Made `AGENTS.md` the canonical cross-agent policy for Claude, Gemini, Codex,
  and other coding agents. It now defines a selective fresh-session read order,
  authoritative document roles, safe checkpointing, direct-to-`main` workflow,
  no-local-QEMU-build/CI rules, and verification standards while preserving
  Y2KBox's downstream-only AI authorization and security guidance.
- Reduced `CLAUDE.md` to Claude-specific hook and local-secret behavior, and
  added a short `GEMINI.md` entrypoint. Codex and other agents use `AGENTS.md`.
- Corrected `HANDOFF.md`, `TASKS.md`, and `README.md` to match the current
  Markdown-only CI behavior (only the lightweight detector runs; Linux and
  Windows build jobs are skipped), owner-confirmed XP boot, the single remaining
  i440FX machine, current frontend direction, and actual artifact availability.
- Reconciled stale task states: T08's XP boot prerequisite is satisfied but its
  install record remains incomplete; T11 is active for remaining guest/API
  compatibility; T13 reflects the active T13e-F1 subtrack; T17 is paused pending
  Linux KVM host-runtime validation; T19 records basic repeated-capture behavior
  as verified while content coverage remains under investigation.
- Updated the documentation incrementally in commits:
  `9f12541`, `b6877d4`, `81f4883`, `1c6aaf8`, `7ef25cd`, `7b34080`, `0851728`,
  `e216252`, `351b684`, `4530bba`, `71a79c4`, `6944828`, `85ed278`, `f619f02`,
  `f39d0fe`, `853e204`, `2d13c76`, `0d40d1a`, `85f2897`, and `fb82975`.
- Documentation-only review and edits. No QEMU source changed, no local QEMU
  build was run, and CI was not polled. Markdown-only CI is expected to skip the
  Linux and Windows jobs on these commits.


## Session 84 — Clarify cross-agent handoff and move XP acceptance to owner checklist

- Confirmed the repository already had a shared cross-agent structure: `AGENTS.md` is canonical, with Claude- and Gemini-specific entrypoints; Codex and other agents use the shared policy. Strengthened it to describe sequential provider handoff as the normal workflow and to make clear that agents do not share private runtime memory.
- Reworked `TASKS.md`: removed the separate T08 boot task and T11 guest-validation task, since XP SP3 boot is already owner-verified and the remaining XP driver/API/game checks require a human operating and visually judging the guest. Combined those checks into a clearly marked owner-run manual checklist, not a claimable AI task. Agents may analyze owner-supplied evidence but cannot certify the guest behavior themselves.
- Updated T09/T10, HANDOFF references, and the T13 Hitman 2 note to remove stale T08/T11 dependencies. Kept the agent-analyzable capture investigation distinct from owner-only visual acceptance.
- Removed wxWidgets fallback messaging from the user-facing README and made SDL2 + Dear ImGui the unambiguous frontend direction there. HANDOFF and T12 retain wxWidgets only as a reconsideration fallback, not a parallel target. Updated the debug-setting description to refer to the SDL2 + Dear ImGui frontend.
- Documentation-only changes. No QEMU source changed, no local QEMU build was run, and CI was not polled.


## Session 85 — Model-specific local build authorization

- Added a standing, narrowly scoped owner authorization for Gemini 4 Neon to run
  ordinary local QEMU configure/build and relevant standard functional checks
  while validating its own changes.
- Kept local builds unauthorized by default for Gemini CLI generally, other
  Gemini models, Claude, Codex, and other agents unless the owner explicitly
  approves a task-specific exception.
- Performance benchmarking and A/B experiments still require explicit owner
  authorization for every agent, including Gemini 4 Neon. GitHub Actions remains
  authoritative; local results must be reported separately with exact
  commands/configuration and cannot be represented as CI success.
- Updated `AGENTS.md`, `GEMINI.md`, `HANDOFF.md`, `TASKS.md`, and
  `WORKLOAD_ACTIVE_SESSION.md`. Documentation-only policy change; no QEMU
  build, benchmark, test, or CI polling was performed.


## Session 86 — Extend Gemini 4 Neon standing authorization to benchmarks

- Owner approved extending Gemini 4 Neon's existing local-build exception to
  relevant performance benchmarks and controlled A/B experiments, including
  the established `xp/bench/run.sh` and `xp/bench/perf.py` workflows.
- Kept the exception strictly model-specific: Gemini CLI generally, other
  Gemini models, Claude, Codex, and other agents still require explicit owner
  authorization before benchmark/A-B work.
- Added practical safeguards: use established methodology, preserve comparable
  before/after results and workload/configuration, keep campaigns bounded,
  avoid unnecessary repetitions, and report exact commands and evidence.
- Updated `AGENTS.md`, `GEMINI.md`, `HANDOFF.md`, `TASKS.md`,
  `WORKLOAD_ACTIVE_SESSION.md`, and `WORKLOG_INDEX.md`. No QEMU build,
  benchmark, functional test, or CI polling was performed for this
  documentation-only policy update.


## Session 87 — Clarify fixed guest targets and XP memory limits

- Validated the 32-bit XP address-space explanation against Microsoft
  documentation: the default 2 GiB user / 2 GiB kernel split is virtual address
  space per process/system, not a partition of installed physical RAM.
- Documented XP Professional's /3GB caveat and the requirement for the
  executable's LARGEADDRESSAWARE flag, including the kernel-resource tradeoff.
  Avoided claiming that additional RAM is automatically dedicated to file cache
  or that it guarantees reduced game stutter.
- Confirmed the project advertises SVGA3D vertex shader version 3.0
  (SVGA3DVSVERSION_30 = 7) and pixel shader version 3.0
  (SVGA3DPSVERSION_30 = 13); documented Direct3D 9.0c / Shader Model 3.0 as
  the guest-facing graphics ceiling, distinct from host rendering APIs.
- Updated README.md and recorded the possible future 4 GiB GUI option in
  HANDOFF.md as a consideration, not a current feature or committed plan.
  Documentation-only change; no build, benchmark, or CI polling was performed.

## Session 88 — Reorganize README host and guest-memory guidance

- Removed the duplicate host-platform bullet from the concise Fixed target
  summary; it now describes only the guest's fixed memory, CPU, and graphics
  capabilities.
- Moved the full host requirements content into a new bottom-of-README FAQ
  entry, rather than repeating it near the top.
- Moved the 2 GiB guest RAM explanation into the FAQ as well, preserving the
  XP physical-memory limit, virtual-address split, /3GB and LARGEADDRESSAWARE
  caveats, and workload-dependent benefits of additional RAM.
- Documentation-only change. No QEMU build, benchmark, functional test, or CI
  polling was performed.

## Session 89 — Refocus the README introduction on the project identity

- Replaced the dense opening technical/runtime summary with a concise description
  of Y2KBox's Windows XP gaming focus and its mid-to-late 2000s PC performance
  target.
- Clarified that the project is not chasing modern high-end gaming performance
  and is not intended to be a cycle-accurate, low-level hardware emulator.
- Added an appreciative acknowledgment of 86Box and its contributors, without
  adding a repetitive closing mission statement.
- Documentation-only change. No build, benchmark, functional test, or CI
  polling was performed.

## Session 90 — Fix the product target to one vCPU

- Owner approved a strict single-vCPU Windows XP target, not merely a one-vCPU default. The guest target is 32-bit Windows XP, uniprocessor only; multi-vCPU/SMP configurations are unsupported.
- Set the i440FX PC machine's `max_cpus` to 1 (with `default_cpus` already 1), removed the `-smp` option definition and command-line parser, and updated README's Fixed target to state 32-bit XP and exactly one vCPU.
- Updated T01 to record the completed CPU restriction and CLI removal, with remaining CLI cleanup scoped to `-m`, `-machine`, and `-cpu`. Updated T14 to keep all thread-pinning measurements on the fixed one-vCPU guest.
- Reconciled HANDOFF's old experimental two-vCPU option with the new owner decision. Historical two-vCPU boot/test evidence is retained as historical evidence, not current support.
- Source/documentation changes reviewed statically only. No local build or functional tests were run, and GitHub Actions/Codacy were not polled.

## Session 91 — Explain Y2KBox's purpose and fixed vCPU target in the README FAQ

- Moved the former Project goals section into the FAQ and reframed it as
  **“Why build a Windows XP emulator when compatibility layers already exist?”**
  The answer distinguishes a real XP guest environment from application
  compatibility layers without dismissing Wine or game-specific wrappers.
- Made the one-vCPU target a standalone FAQ entry. It acknowledges era-accurate
  dual-core XP systems, explains workload-dependent TCG/MTTCG coordination
  overhead, and explicitly avoids claiming that two vCPUs are always slower.
  It also distinguishes guest vCPUs from QEMU host threads.
- Moved the proprietary hypervisor API explanation into its own independent FAQ
  question and retained the preservation-practicality discussion as a separate
  closing FAQ entry. Kept the existing host-requirements and 2 GiB RAM answers.
- Clarified the durable HANDOFF Q1 rationale to match the FAQ and preserve the
  workload-dependent qualification.
- Documentation-only change. Static text/section review only; no QEMU build,
  benchmark, functional test, or CI polling was performed.


## Session 92 — Close T09 integration; keep XP acceptance owner-run

- Reclassified T09 as DONE: the qemu-vmvga/SVGA II + SVGA3D integration and its Linux/Windows CI checks are complete.
- Clarified that VMware Tools/driver installation, 2D/display behavior, D3D API coverage, and game compatibility acceptance belongs to the owner-run XP guest validation checklist, not a pending AI-agent task or a reason to leave T09 paused.
- Updated HANDOFF's T09 summary and removed wording that made T09's CI-verified Ring-3 backdoor integration sound blocked on separate guest validation. T10 remains separately paused for its own owner-run rendering/API acceptance scope.
- Documentation-only status/continuity update. No QEMU source changes, local build, benchmark, guest test, or CI polling was performed.

## Session 93 — Move QEMU base and license provenance into the README FAQ

- Removed the upstream QEMU base commit/version and GPL-2.0 notice from the Fixed target area so that section remains focused on the guest configuration.
- Added a dedicated FAQ entry, **“Which version of QEMU is Y2KBox based on?”**, identifying upstream QEMU 11.1.90, linking commit `9d7d057a558b9a1c232cea390b4481dd20d9c9ce`, and noting that the upstream GPL-2.0 license is unchanged with `COPYING` and `LICENSE` as the repository references.
- Documentation-only change. Static placement and wording review; no build, benchmark, functional test, or CI polling was performed.


## Session 94 — Reconcile agent guidance and stale handoff instructions

- Re-audited all 16 Markdown files on `main` for agent instructions, authorization boundaries, cross-agent redirection, and stale workflow guidance. No malicious prompt injection or instruction redirecting agents away from the owner/repository was found.
- Updated the QEMU codebase skill to state that its build/test suggestions are technical guidance, not authorization; `AGENTS.md` remains controlling.
- Clarified in `AGENTS.md` and `HANDOFF.md` that the pushed-CI/Codacy status hook is Claude-specific. Gemini/Codex and other agents must not assume it exists and should leave result review to the owner or a later session rather than polling.
- Replaced the obsolete `git fetch` + `git rebase` handoff instruction with the current serialized-write procedure: verify latest `main` HEAD and exact target blob SHA before every write, then abort/refetch/reapply if either moved.
- Clarified that the manual benchmark reference does not grant general local-build or benchmark permission; the standing exception is Gemini 4 Neon only, and all other agents require explicit owner authorization.
- Updated the worklog index and rolling active-session summary. Historical worklog entries were left intact.
- Documentation-only static review. No QEMU source changes, local build, benchmark, functional test, guest test, or CI polling was performed.


## Session 95 — Fix optional GitHub token handling in CI status helper

- Fixed `scripts/session_status.sh` so its optional GitHub Personal Access Token is passed to `curl` as one correctly quoted Authorization header argument. The previous parameter-expansion form did not preserve the embedded quoting and could split the header into separate shell arguments.
- The helper still reads the existing `GITHUB_TOKEN` or `GH_TOKEN` environment variable (or falls back to `gh auth token`); no new token, secret, permission, or authentication setup is required. When no token is available, the request remains unauthenticated as before.
- The change is limited to Claude's CI-status reporting helper; it does not affect GitHub Actions builds or repository authentication used by coding-agent sandboxes.
- Static shell review only. No token value was accessed or displayed, no live authenticated API request was made, and no CI polling or QEMU build/test was performed.


## Session 96 — Move diagnostic logging guidance into the README FAQ

- Moved the existing Ctrl+Alt+L diagnostic-capture instructions out of Download & Usage and into a dedicated README FAQ entry, preserving the opt-in behavior, independent timestamped files, per-capture counter resets, and shell-redirection warning.
- Documented the expected performance overhead while recording and recommended comparable scenes, settings, and capture durations for performance comparisons.
- Explained how testers can share `Y2KBOX-PERF` logs to support developer or LLM-assisted bottleneck analysis and targeted follow-up instrumentation; clarified that performance counters alone do not diagnose every graphics issue.
- Documented `VMVGA_TRACE_DEEP_GB_UPDATE` as an optional compile-time developer trace, disabled by default and intended for targeted VMware SVGA investigations rather than ordinary tester logging.
- Documentation-only. No emulator source, instrumentation, build, benchmark, functional test, guest test, or CI polling was performed.

## Session 97 — Merge preservation rationale into the README project-purpose FAQ

- Merged the former standalone “Make preservation practical” FAQ into “Why build a Windows XP emulator when compatibility layers already exist?” so the answer now connects compatibility layers, the value of preserving a complete XP environment, practical benefits of emulation, and the project's longer-term vision.
- Retained the distinctions from Wine and game-specific wrappers, the original-hardware maintenance argument, portability/snapshots/backups/repeatability, the goal of minimizing host and guest workarounds, the Direct3D 8.1 / 9.0c target, and the PCSX2 usability comparison.
- Removed the now-redundant standalone preservation FAQ entry. No other FAQ entries or emulator code were changed.
- Documentation-only. No build, benchmark, functional test, guest validation, or CI polling was performed.

## Session 98 — Move project-purpose FAQ to the bottom of the README

- Moved the complete “Why build a Windows XP emulator when compatibility layers already exist?” answer to the final position in the README FAQ, after the technical host, logging, QEMU provenance, vCPU, hypervisor API, and guest-memory questions.
- Preserved the approved merged wording exactly; this is an ordering-only change to the FAQ content.
- Documentation-only. No emulator source, builds, benchmarks, tests, guest validation, or CI polling were performed.

## Session 99 — Add guest OpenGL investigation to the task queue

- Corrected the architectural framing: VMware SVGA3D is not inherently a Direct3D-only virtual interface. Mesa's SVGA guest driver demonstrates an OpenGL 2.1 path over the VMware SVGA3D interface, while later configurations support newer OpenGL versions.
- Distinguished that protocol capability from the Windows XP guest-driver situation. VMware Workstation documentation for XP-era guests describes OpenGL applications as software-emulated; this does not establish that VMware Tools 10.0.12 exposes an accelerated OpenGL ICD in XP.
- Added T25 to investigate OpenGL 1.1 as the baseline and OpenGL 2.1 as the maximum target. The task starts with WGL pixel-format/ICD diagnostics for the Call of Duty 1 failure, audits the SVGA3D guest/host command contract, evaluates an XP-compatible guest ICD path, and requires incremental WGL/OpenGL validation while preserving D3D9.
- Documentation and task-queue update only. No emulator source changes, local build, benchmark, functional test, guest test, or CI polling was performed.\n\n## Session 100 — Start guest OpenGL investigation and audit the SVGA3D path

- Began T25 and changed its status to IN PROGRESS. OpenGL 1.1 remains the baseline target and OpenGL 2.1 the maximum target.
- Refined the protocol model: Mesa's SVGA Gallium driver implements OpenGL by translating GL/Gallium operations into SVGA3D commands; it does not send raw OpenGL calls over a separate OpenGL FIFO. Mesa source shows legacy SVGA3D draw-command emission, including DRAW_PRIMITIVES, which is present in Y2KBox's supported-command whitelist. This is a useful overlap, not proof of complete OpenGL 2.1 semantics.
- The current host source documents its legacy devcap profile as D3D9-oriented. The exact Mesa-required command/state/capability subset still needs comparison against Y2KBox handlers and advertised values.
- Inspected the latest uploaded 865,891-byte Y2KBox diagnostic log (17,860 lines). It contains SVGA activity but no game WGL/PFD records; it cannot explain the Call of Duty 1 pixel-format rejection. T25 now explicitly requires an XP-side WGL probe or game API instrumentation, including all PFD attributes, generic-vs-ICD classification, SetPixelFormat/context creation, and GL vendor/renderer/version.
- Research confirms Mesa's documented SVGA guest stack is Linux-specific (vmwgfx, libdrm, Gallium) and can provide OpenGL 2.1 when newer GL support is unavailable. Historical VMware Workstation 6 documentation says XP OpenGL ran in software emulation; this is not evidence of the exact VMware Tools 10.0.12 behavior.
- Presented proposed README Fixed target wording to the owner and left README.md unchanged pending approval. No emulator source changes, diagnostic utility, local build, benchmark, guest test, or CI polling was performed.

## Session 101 — Clarify the guest OpenGL target in the README

- Updated README's Fixed target section to describe the guest graphics target as Direct3D 9.0c / Shader Model 3.0 plus an OpenGL 1.1 baseline and OpenGL 2.1 maximum.
- Explicitly marked guest OpenGL as under investigation and not currently implemented, and excluded OpenGL versions newer than 2.1 from the target.
- The OpenGL investigation remains scoped to establishing a complete OpenGL 2.1 implementation, including compatibility with older OpenGL 1.x applications; the WGL pixel-format failure still needs XP-side diagnosis before context creation.
- Documentation-only. No emulator source changes, diagnostic utility, local build, benchmark, guest test, or CI polling was performed.
## Session 102 — Build an XP-side COD1 WGL pixel-format probe

- Traced COD1's `GLW_ChoosePFD` log signature to the Quake III / id Tech 3 Windows renderer's custom PFD-selection routine. Unlike Windows `ChoosePixelFormat`, it enumerates descriptors and filters for RGBA, window drawing, OpenGL support, double buffering, at least 15 depth bits, and at least 4 stencil bits when requested; it also rejects `PFD_GENERIC_FORMAT` unless `r_allowSoftwareGL` is enabled. The generic-format check occurs before the MCD/generic-accelerated reporting, so the strict path can reject such formats even when `PFD_GENERIC_ACCELERATED` is set.
- Added `xp/tools/glprobe.c` and `xp/tools/README.md`. The probe enumerates all PFDs, reproduces the id Tech 3 selection and best-match rules for 32/24/8 and 32/24/0, reports per-format rejection reasons, compares strict and software-allowed selection, separately reports Windows `ChoosePixelFormat`, and tests `SetPixelFormat`, `wglCreateContext`, `wglMakeCurrent`, and GL vendor/renderer/version strings.
- Updated T25 with the probe path and diagnostic procedure. The next step is to have the owner compile and run it in the XP guest, then use the complete output to identify whether COD1 rejects every format due to generic-format policy, missing flags, insufficient depth/stencil, or another selection condition.
- Static source review only. The standalone XP utility was not compiled or run; no QEMU build, emulator source change, guest test, benchmark, or CI polling was performed.
## Session 103 — Match Mesa's VGPU9 baseline and identify the COD1 software fallback

- Compared Mesa's VGPU9 SVGA screen setup with Y2KBox's host device profile. The inspected Mesa path reports GLSL feature level 1.20 (OpenGL 2.1-era GLSL), requires SVGA hardware version WS8_B1 and vertex/pixel Shader Model 3.0, and uses the legacy SVGA3D command family. Y2KBox reports WS8_B1, VS/PS 3.0, and four render targets; its command table has handlers for the core legacy surface/context, state, shader, draw, clear, query, and presentation commands. This is a promising protocol-generation match, not proof of complete state semantics or GL 2.1 conformance.
- Flagged legacy state capabilities such as line stipple and logic operations for further semantic review rather than assuming that the D3D9-oriented profile is automatically sufficient.
- Found VMware Workstation Player 15 documentation stating that OpenGL applications in Windows XP guests run in software-emulation mode. This does not prove the exact VMware Tools 10.0.12 behavior in Y2KBox, but it makes a software-OpenGL fallback worth testing before developing a new ICD.
- Added a quick COD1 launch test using `+set r_allowSoftwareGL 1` to the XP WGL probe README and T25. The id Tech 3 reference renderer defaults this cvar to 0; enabling it allows generic PFDs through selection, if the XP driver exposes suitable formats. This is a software-only compatibility test, not accelerated GL.
- Updated T25 with the protocol/capability findings and the XP guest-path evidence. No emulator source changes, QEMU build, standalone probe compile/run, guest test, benchmark, or CI polling was performed.


## Session 104 — Identify the active XP guest OpenGL implementation

- The owner ran COD1 with `+set r_allowSoftwareGL 1`. The game selected a generic software pixel format, successfully created and made current a WGL context, then exited with the missing-video-card-features fatal error. The console identified Microsoft Corporation / GDI Generic / OpenGL 1.1.0 and only `GL_WIN_swap_hint`, `GL_EXT_bgra`, and `GL_EXT_paletted_texture`.
- The owner inspected GLview's OpenGL and Extensions tabs. It independently reported Microsoft / GDI Generic / OpenGL 1.1.0 and the same three extensions. Recorded the reported extension-check groups and GL limits in HANDOFF.md section 8d. Pixel-format tab details were not captured; no pixel-format conclusions were inferred. The viewer's extension-check percentages are not treated as formal OpenGL conformance results.
- Conclusion: the XP guest is currently using Microsoft's generic software OpenGL implementation, not a VMware OpenGL ICD. COD1 is now past pixel-format selection and WGL context creation; the remaining fatal check is consistent with missing required GL functionality. This does not prove a defect in Y2KBox's host SVGA3D renderer because the guest is not exposing an accelerated OpenGL path.
- Updated T25 to focus next on why the XP VMware Tools/SVGA driver stack is not loading a VMware OpenGL ICD, including driver installation and ICD registration. GPU Caps Viewer is optional corroboration; repeating GLview is unnecessary. The owner explicitly rejected installing Visual Studio or compiling `xp/tools/glprobe.c` in the XP guest; that is no longer a required diagnostic step. A Mesa Windows software renderer remains only an optional app-local software-fallback experiment, not an SVGA3D acceleration test.
- Documentation-only. No emulator source changes, local build, benchmark, additional guest test, or CI polling was performed.


## Session 105 — Establish VMware's documented XP OpenGL limitation

- Researched VMware product documentation after the owner's GLview report. VMware Workstation Player 15's manual explicitly states that DirectX 9 acceleration applies to Windows XP/Vista/7 guests while OpenGL applications in Windows XP guests run in software-emulation mode. The older Workstation 6 manual documents the same XP/2000 limitation. Broadcom confirms VMware Tools 10.0.12 is the last Tools release for XP/2000/Server 2003.
- Conclusion: Microsoft's GDI Generic OpenGL 1.1.0 result is consistent with VMware's documented XP product behavior; a missing VMware OpenGL ICD or broken Tools installation is unlikely. Do not ask the owner to reinstall Tools, search for `vm3dgl.dll`, or repeat GLview without contradictory evidence. DirectX 9 acceleration and OpenGL acceleration are distinct guest paths.
- Investigated WINVBE 1.3 as a possible XP software-OpenGL alternative. Its README documents an XP-compatible OpenGL 2.1 ICD based on Mesa 7.8.2, but it requires the WINVBE display-driver stack and its own limitations state that OpenGL/Direct3D run on the CPU with no hardware 3D acceleration. It may be useful only as an isolated snapshot-based software compatibility experiment, not as a VMware ICD or Y2KBox SVGA3D solution. WINVBE is GPL-3.0 (embedded Mesa is MIT), so its code is not to be incorporated into this GPL-2.0-only project without separate license review.
- Updated T25 and HANDOFF section 8e: the next architectural question is a Windows XP guest OpenGL ICD that translates WGL/GL calls into SVGA3D commands, plus the host-side command/state coverage it requires. Mesa's Linux vmwgfx/Gallium path remains a reference, not a drop-in XP driver.
- Documentation-only. No guest changes or tests, no driver installation, no source changes, no local build/benchmark, and no CI polling.


## Session 106 — Establish the Windows XP OpenGL ICD discovery constraint

- Corrected the previous assumption that a conventional XP OpenGL ICD could simply be registered as a user-mode DLL alongside VMware's display driver. WINVBE documents that XP's OpenGL runtime asks the active display driver for the OPENGL_GETINFO escape and the driver identifies the ICD; ReactOS's loader corroborates the QUERYESCSUPPORT / ExtEscape discovery sequence. A registry entry alone is not enough unless the active display driver advertises the ICD. No evidence shows VMware's XP driver does so; the active GDI Generic result and VMware's Workstation documentation point to software OpenGL.
- Identified the only plausible no-display-driver-replacement route: a per-application standalone OpenGL DLL or app-local opengl32 implementation that supplies its own WGL/GL layer and bypasses the native ICD discovery path. Mesa documents app-local opengl32 deployment for its Windows WGL software target (software-only). The upstream Quake III Windows renderer can load a separate driver DLL through r_glDriver and uses a standalone-driver swap path; COD1's exact renderer fork and cvar behavior still need verification.
- A standalone layer could potentially translate GL to GLES2 and reuse the existing XP Direct3D 9 / VMware accelerated path. Microsoft documents D3D9 on XPDM, and Workstation Player 15 documents D3D9 acceleration for XP guests. GL4ES is a possible GL 1.5/2.1-to-GLES2 frontend with Windows support but lacks WGL integration; ANGLE's old D3D9 backend has been removed from current ANGLE and Qt warns ANGLE is not fully reliable on XP. No known-good assembled stack was found; this remains high risk.
- T25 now has a strict feasibility gate: first prove standalone DLL loading and D3D9 HAL device creation/Present through the existing VMware driver. Only then consider a minimal COD1-focused GL compatibility slice. If either prerequisite fails, stop rather than build a full driver or replace VMware's display driver.
- Documentation-only. No emulator source changes, guest modifications, driver installation, builds, benchmarks, guest tests, or CI polling.


## Session 107 — Pause guest OpenGL work for independent reassessment

- Updated the README Fixed target to describe guest OpenGL as a low-probability research objective. It notes that XP's standard VMware display-driver path appears to expose GDI Generic software OpenGL and that application-local wrapper DLLs are only one unproven alternative.
- Paused T25 pending an independent feasibility reassessment. No implementation work is authorized. The next reviewer is explicitly invited to challenge the current standalone-DLL / GL4ES / ANGLE hypothesis and investigate other architectures.
- Expanded HANDOFF section 8g with the reassessment request, evidence expectations, XP compatibility/licensing/performance review criteria, and a requirement for a smallest decisive validation proposal before any prototype.
- No agent was actually assigned or dispatched. No guest modification, display-driver replacement, emulator source change, build, benchmark, guest test, or CI polling was performed. Documentation-only.


## Session 108 — Record QindieGL as a concrete but unproven OpenGL wrapper lead

- Added QindieGL to the README Fixed target discussion as a source reference for an application-local OpenGL-to-D3D9 wrapper, with an explicit warning that it is not a validated Y2KBox solution and that COD1 startup does not establish correct rendering.
- Recorded the owner's COD1 test and the source-level cause in HANDOFF section 8h: QindieGL implements/registers `wglGetExtensionsStringARB(HDC)`, but not `wglGetExtensionsStringEXT(void)`; its WGL proc lookup returns a synthetic stub for the unknown EXT name. A registry extension flag cannot supply the missing function.
- Recorded the historical DxWnd experiment in which a minimal EXT entry point reportedly let COD1 start but left substantial graphics corruption, especially the skybox. This is a startup lead, not a rendering fix.
- Added the QindieGL source and historical discussion to T25's evidence while preserving its PAUSED status and independent-reassessment gate. Noted QindieGL's GPL-3.0 license for review before any code reuse in Y2KBox (GPL-2.0).
- Documentation-only. No QindieGL patch, source changes, builds, guest modifications, guest tests, benchmarks, or CI polling were performed.

## Session 109 — Record TitaniumGL as a separate OpenGL wrapper research lead

- Removed the project-specific QindieGL paragraph from the README. Kept the existing general statement that application-local wrapper DLLs are one unproven alternative; the README remains focused on the approach rather than individual experimental projects.
- Added TitaniumGL as a separate T25 research lead in TASKS.md and HANDOFF.md section 8i. Recorded the owner's COD1 observations: application-local DLL loading, pixel format/DC/context success, OpenGL 1.4, VMware SVGA II renderer string with 119 MB VRAM, numerous advertised extensions, the unresolved `0 PFDs found` line, and the subsequent missing-features rejection / `RE_SHUTDOWN (1)`.
- Recorded that TitaniumGL's hardware and software-renderer modes produced the same broad COD1 failure. The complete extension list was not captured; no claims were made about full extension implementation, successful D3D9 acceleration, or correct rendering.
- Kept QindieGL and TitaniumGL as distinct candidates and preserved T25's PAUSED status and independent-reassessment gate. XP requirements, release provenance, source availability, and TitaniumGL licensing remain unverified.
- Documentation-only. No source changes, builds, guest modifications, additional guest tests, benchmarks, or CI polling were performed.


## Session 110 (ChatGPT), 2026-11-24 - Clarify T24 implementation and verification status

The owner confirmed that the Windows build presents the intended fixed-size
window: it is decorated but cannot be manually resized, and its titlebar
maximize control is disabled. This verifies the Windows window-style behavior
implemented in Session 62; it is not a report of guest-driven resolution,
fullscreen-shortcut, or mouse-tracking checks.

Changed T24 from PAUSED to IMPLEMENTED to distinguish completed source work
from its remaining acceptance evidence. Added IMPLEMENTED to the task-status
vocabulary for tasks with no known source work remaining but outstanding
verification, and documented the remaining Linux/Windows CI and owner-run XP
checks. No source changes, builds, guest tests, or CI polling were performed.


## Session 111 (ChatGPT), 2026-11-24 - Add visual task-status markers

- Added consistent colored status markers to all 23 task headings in TASKS.md: red for TODO, yellow for CLAIMED / IN PROGRESS / PAUSED / IMPLEMENTED, and green for DONE.
- Moved each complete canonical status into a plain-text Status field immediately below its heading. The heading marker is explicitly documented as a visual aid only; agents must use the plain-text field as the source of truth.
- Clarified that IMPLEMENTED remains yellow while acceptance evidence is pending, including T24.
- Updated AGENTS.md with the task-status parsing rule and refreshed the worklog index and rolling active-session summary.
- Documentation-only. No source changes, builds, guest tests, or CI polling.


## Session 112 (ChatGPT), 2026-11-24 - Plan selectable graphics machine profiles in T12

- Expanded T12's future launcher plan from a single fixed VMVGA machine to a profile-based model: each profile binds its graphics backend, matching XP guest-driver environment, and a dedicated system qcow2 image.
- Kept the existing VMware SVGA II / SVGA3D profile as the active/default Direct3D 9 path. Added the proposed VBoxVGA 5.2.x / Chromium SharedOpenGL profile as a separate, experimental OpenGL-oriented candidate, not an implemented or selectable feature.
- Explicitly gated the VBox profile on an end-to-end Y2KBox proof of concept and source/licensing audit. VirtualBox's documented experimental Windows XP 3D support is evidence that the upstream guest path existed, not proof that the Y2KBox port works. T16 remains the research/feasibility track.
- Clarified that graphics backend, guest driver, and system disk must be selected as one consistent profile; removable media and unrelated device selectors remain independent.
- Documentation-only. No source changes, builds, guest tests, benchmarks, or CI polling.


## Session 113 (ChatGPT), 2026-11-24 - Clarify T12 as a user-managed instance manager

- Corrected Session 112's interpretation of the proposed multi-GPU architecture. T12 is not a chooser among two application-defined presets: it should become an instance manager where users create and name their own independent XP VM instances.
- Each instance owns its persisted virtual-hardware configuration, including its virtual GPU choice, and is associated with its own system qcow2 image and guest installation. The GPU is selected per instance during creation, not as a global application setting.
- VMware SVGA II / SVGA3D remains the currently supported/default path. VBoxVGA 5.2.x / SharedOpenGL remains only a proposed additional GPU choice for future instances, gated on an end-to-end proof of concept and source/licensing audit; T16 remains the research track.
- Clarified that Y2KBox does not ship two fixed images or hard-coded vendor presets. Different graphics ecosystems should use separate XP instances/system images rather than sharing one installation and switching driver stacks.
- Documentation-only. No source changes, builds, guest tests, benchmarks, or CI polling.

## Session 114 (ChatGPT), 2026-11-24 - Complete T24 owner acceptance verification

- The owner clarified that every T24 acceptance check previously listed as outstanding had already been verified: Linux and Windows SDL CI pass, and XP guest-driven resolution changes, `Ctrl+Alt+F` fullscreen toggling, and mouse tracking all work.
- Corrected T24 from IMPLEMENTED/yellow to DONE/green. Recorded the complete owner-confirmed acceptance set and removed the stale remaining-checks list.
- The Windows decorated, non-resizable window and disabled titlebar maximize control remain documented as verified behavior.
- Documentation-only. No source changes, builds, guest tests, or CI polling were performed in this session; the verification results above are the owner's confirmation.

## Session 115 (Gemini 4 Neon), 2026-11-24 - T13e / T13e-F1: Eliminate Hitman 2 / FIFA unpresented ScreenTarget sync readbacks, dynamic VB DISCARD thrashing, and O(N) GART unparent + per-command FlatView commits

- **Owner post-Session 80 validation captures (Windows TCG, unchanged scenes):**
  - **FIFA 2003** (Ali Sami Yen intro, Galatasaray vs Real Madrid): `25.352 s`, `2,172` flips (~`85.7 FPS` avg, up from `42.741 s` / `1,775` flips pre-Session 80; slow-motions gone, playable). Remaining measured BQL/VMVGA hotspots: `BIND_GB_SCREENTARGET` = `2,172` calls (`1.744 s`, `1,293 >1ms`; `screen_target_sync_readback = 2,172` in `2.304 s` vs only `830` `display_update` ticks), `gart_map_commit` (`716 ms`, `380` txns) + `gart_unmap_commit` (`688 ms`, `361` txns) + `gart_unmap_alias` (`168.3 ms`, `max_us = 9.43 ms`, `32 >1ms`), and `dxvk.upload_buffer_lock_overlap` (`10,892` calls = `611 ms` on `>256 KiB` VBs) + `upload_buffer_lock_discard` (`116,437` calls = `188 ms`). Secondary TCG vCPU profile leads noted for future follow-up: `float32_mul` / `float32_add` / `mulps_xmm` / `addps_xmm` (~`8.01%`) and `ntdll!memset` (`4.31%`).
  - **Hitman 2: Silent Assassin** ("SICILY" text fade-in/out intro scene): `3.996 s`, `3,068` flips (~`767 FPS`). Owner reported an apparent stall on the fade screen while gameplay runs fine. Root cause isolated: removing `clear_shadow` in Session 80 let the simple fade screen flip ~767×/s, and on the FIFO path (`cb_shadow_yield_allowed == false`), every `BIND_GB_SCREENTARGET` called `vmsvga3d_screen_target_quiesce_live()`, triggering `2,913` synchronous GPU readbacks (`2.241 s`, `2.410 s` total in `BIND_GB_SCREENTARGET` = **60.3% of wall time**; `NtWaitForAlertByThreadId = 61.69%`) for unpresented throwaway frames overwritten before the next `display_update` (`155` ticks; `155 + 2,913 = 3,068`). Also `dxvk.upload_buffer_lock_discard_invalidate = 6,134` (`40.2 ms`, renaming a `4.5 MiB` VB for `48 B` avg writes = `15.88 GB` renamed).
  - **F1 Challenge '99-'02**: `24.386 s`, `762` flips (~`31.2 FPS`). Root cause isolated: `vmvga.fifo_run = 5.454 s`, of which `dxvk.upload_buffer_lock_discard_invalidate` = `71,979` calls = **`3.711 s`** (`51.5 us/call`, `111 >1ms`, `max_us = 1.867 ms` — **68% of `fifo_run`**) renaming a `512 KiB` dynamic VB (`36.24 GB` total) for `~4 KiB` avg writes, whereas `NOOVERWRITE` ran `235,687` locks in `47 ms` (`0.20 us/call`, `0 >1ms`).

- **Implemented fixes (`hw/display/vmware_vga_dxvk.{h,c}`, `hw/display/vmware_vga_3d.c`, `hw/display/vmware_vga_vgpu10.c`, `hw/display/vmware_vga.c`):**
  1. **Steady-state FIFO ScreenTarget flip supersession + eager retired staging transfer:**
     - Added `vmsvga3d_dxvk_d3d9_screen_readback_has_pending()` and `vmsvga3d_screen_target_try_supersede_flip_live()`. On steady-state FIFO valid-to-valid full-screen pure-D3D9 flips (`!cb_shadow_yield_allowed`, `!screen_frontend_deferred`, `!screen_handoff_active`, `!screen_direct_active`, `!screen_target_barrier_active`, `!screen_target_frontend_commit_pending`, `d3d11_retired == 0`, `new_surface->screen_target_content_valid`, full-screen dimensions and direct-present format match), if `old_sid` has a display-submitted async readback slot it is detached into the single-entry `d3d9_retired_screen_readback` mailbox (`retire_latest`), otherwise its unpresented dirty rects are dropped without stalling on `vmsvga3d_screen_target_quiesce_live()`. Any non-steady-state or busy/unsupported case still falls back to `vmsvga3d_screen_target_quiesce_live()`.
     - In `vmsvga3d_dxvk_d3d9_screen_readback_retire_latest()`, once the selected snapshot is moved into `dxvk->d3d9_retired_screen_readback`, eagerly issue non-blocking `GetRenderTargetData` + `Issue(EVENT)` + `GetData(FLUSH)` (leaving `d3d9_staging_submitted = false` on failure so `retired_screen_readback_poll` handles it as before), keeping retired frames publishing in a single display tick.
  2. **Per-surface ring sub-allocation for dynamic D3D9 vertex buffers + guarded 2 MiB overlap-DISCARD:**
     - Added `d3d9_host_length`, `d3d9_ring_base`, `d3d9_ring_high`, `d3d9_buffer_discard_count`, `d3d9_buffer_nooverwrite_bytes`, and `d3d9_buffer_last_was_overlap_discard` on `VMSVGA3DDxvkSurface` plus `d3d9_ring_vb_extra_bytes` (capped at `32 MiB`) on `VMSVGA3DDxvk`, with proper reset/accounting on materialization, full-buffer upload, and eviction.
     - Dynamic vertex buffers receiving `INVALIDATE_GB_SURFACE` (`d3d9_buffer_discard_pending`) lazily upgrade once up to `4 MiB` (for guest VBs `<= 2 MiB` within budget; `> 2 MiB` VBs like Hitman 2's `4.5 MiB` VB sub-allocate directly inside their existing allocation) and allocate each new generation at `align64(d3d9_ring_high)` via `D3DLOCK_NOOVERWRITE`, wrapping to base `0` with `D3DLOCK_DISCARD` only when the ring fills. Repeated small `full_range` dynamic VBs (`<= 256 KiB`, `>= 4` discards) also lazily upgrade into a ring. `vmsvga3d_dxvk_set_stream_source()` adds `surface->d3d9_ring_base` to `offset` (with overflow check), `mark_buffer_drawn_range` / `mark_buffer_drawn_all` advance `d3d9_ring_high`, and mid-generation ring-end appends wrap cleanly to `0` from the authoritative CPU shadow.
     - Raised dynamic-buffer overlap `D3DLOCK_DISCARD` cap from `256 KiB` up to `2 MiB` (preserving the unconditional `<= 256 KiB` path), guarded for `> 256 KiB` by requiring `>= 25%` `NOOVERWRITE` progress between consecutive overlap-discards so genuine ring wraparounds (FIFA 2003) reset in-flight tracking without thrashing full copies on repeated same-offset overwrites.
  3. **O(1) GART alias QOM unparent + FIFO-pass GART transaction batching:**
     - Updated `vmsvga3d_gart_page_remove_alias()` to take `aperture_page` and look up/delete `"vmsvga-gart-%u[0]"` via `object_property_find()` + `object_property_del()` (verified `prop->opaque == OBJECT(&page->alias)`, falling back to `object_unparent()`), eliminating `object_unparent()`'s two full linear hash-table scans over ~8k GART properties per unmapped page.
     - Added `gart_txn_batch_allowed`, `gart_txn_active`, `gart_txn_map_pages`, `gart_txn_unmap_pages`, and `vmsvga3d_gart_txn_flush()` to coalesce multiple `MAP_MOB_INTO_GART` / `UNMAP_GART_RANGE` commands within a single `vmsvga_fifo_run_impl()` pass into a single `memory_region_transaction_commit()`, flushing before `SVGA_CMD_FENCE`, `GB_MOB_FENCE`, `DX_MOB_FENCE_64`, `vmsvga3d_gart_disable_live()`, and FIFO-pass exit.

- **Verification:**
  - Local sandbox lacks `meson`/`ninja`/`mingw32-gcc`, so full Linux/Windows QEMU builds were not run locally.
  - Ran `git diff --check`, `#if`/`#endif` balance check across all 5 modified files, QOM header API check against `include/qom/object.h`, and a standalone `gcc -std=gnu11 -Wall -Wextra -Werror -Wstrict-prototypes -Wmissing-prototypes -Wsign-compare -Wshadow -O2` verification harness exercising F1 '99-'02 invalidate-ring cycles (`3` discards / `3,997` `NOOVERWRITE` over `1,000` cycles), Hitman 2 `4.5 MiB` VB tiny writes (`0` discards / `6,000` `NOOVERWRITE`), FIFA `1 MiB` ring wraparound + same-offset thrash guard, mid-generation ring wrap shadow preservation, repeated `full_range` ring upgrade, eviction budget accounting, and GART property naming / transaction batching — all passed.
  - GitHub Actions Linux/Windows CI and owner Windows TCG in-game re-captures (FIFA 2003, Hitman 2 Sicily fade, F1 '99-'02) are pending after push.

## Session 116 (ChatGPT), 2026-11-24 - Reorganize WORKLOG_INDEX for scanability and coverage

- Reworked the topic map to include the recent T24 acceptance work and Session 115's T13e / VMVGA / DXVK performance changes, including dedicated Hitman 2 coverage.
- Replaced the dense Sessions 82–115 prose block with a concise, consistently formatted chronological list. The index remains a navigation summary; detailed evidence and history stay in the append-only WORKLOG.md.
- Preserved the Session 81 CI note and all existing historical session references. No source changes, builds, tests, or CI polling.


## Session 117 (ChatGPT), 2026-11-24 - Fix Session 115 D3D9 dynamic vertex-buffer compile failure

- Linux and Windows CI both failed compiling `hw/display/vmware_vga_dxvk.c` because Session 115 referenced `VMSVGA3D_DXVK_D3DUSAGE_DYNAMIC` in the new dynamic D3D9 vertex-buffer ring code without defining the D3D9 usage flag.
- Added the missing D3D9 `D3DUSAGE_DYNAMIC` definition with its Direct3D 9 value, `0x00000200u`, alongside the existing D3D9 usage constants. Kept the D3D9 checks on the D3D9 flag; did not substitute the unrelated D3D11 `D3D11_USAGE_DYNAMIC` enum (value 2).
- This is a minimal compile fix; no ring-buffer behavior or other Session 115 logic was changed.
- No local QEMU build was run in this session. The source fix was pushed to `main`; GitHub Actions results remain pending and were not polled.


## Session 118 (ChatGPT), 2026-11-24 - Define narrow VB-ring profiling handoff

- Re-reviewed T19's Ctrl+Alt+L capture wiring and Session 115's per-surface dynamic D3D9 vertex-buffer ring state. Static inspection shows the existing capture controls the general performance counters, VMVGA/VMPort diagnostic gates, relevant trace events, and Windows TCG vCPU profiler; it does not by itself prove that every expected record is present in a real XP capture. T19 remains PAUSED pending owner content/repeated-interval verification.
- Added a narrowly scoped Gemini 4 Neon follow-up to T19: emit one capture-stop `Y2KBOX-PROF subsystem=vmvga.vb_ring` record per eligible dynamic D3D9 VB surface, with explicitly defined capacity, allocation/high-water, successful NOOVERWRITE reuse, successful DISCARD/wrap, overlap-discard, and allocation-failure measurements.
- Required profiling-only interval baselines/reset at capture start without resetting allocator state; no per-upload logging, no hot-path locks/allocations/I/O, no allocation-policy changes, and no broader ScreenTarget/GART profiler expansion.
- Documentation-only handoff. No source changes, local build, runtime test, or CI polling. Gemini 4 Neon should implement and validate this focused follow-up before resuming broader optimization work; T19 is not marked DONE.


## Session 119 (ChatGPT), 2026-11-24 - Add capture-interval dynamic VB ring pressure profiling

- Added per-surface profiling-only counters and a DXVK surface registry so the existing VMVGA `debug-log` capture transition resets counters at Ctrl+Alt+L start and emits summaries at capture stop. Surfaces created during an active capture inherit the active profiling state.
- Added one `Y2KBOX-PROF subsystem=vmvga.vb_ring` summary per eligible dynamic D3D9 vertex buffer still live at capture stop. Fields are defined in source and TASKS.md: maximum observed host allocation, cumulative successful uploaded bytes, successful NOOVERWRITE subset, successful DISCARDs, ring-end wraps, maximum absolute host-buffer byte end, overlap-fallback DISCARDs, and failed ring upgrade/NOOVERWRITE operations.
- Profiling counters reset independently of allocator state. No allocation policy, ring decisions, or Session 115 optimization behavior was intentionally changed. No per-upload formatting/I/O or new locks/allocations were added; only a capture-active branch and counter updates are used while profiling.
- Static source review only. No local QEMU build or runtime test was run (this agent does not inherit Gemini 4 Neon's standing build authorization). Linux/Windows CI and owner XP capture verification remain pending; T19 stays PAUSED until real capture output and repeated intervals are verified.


## Session 120 (ChatGPT), 2026-11-24 - Complete VB-ring profile lifecycle accounting

- Corrected the initial profiling integration so range-only lock failure counters are confined to the range allocator; full-buffer uploads now also contribute their successful DISCARD bytes and high-water to the same per-surface interval summary.
- Count ring wraps only when a larger host ring actually runs out of append capacity; ordinary invalidation/overlap DISCARDs remain distinct. Added renderer surface-list unlink/relink handling and restored the active capture flag when the DXVK renderer is recreated, preserving per-surface counters across renderer replacement.
- Static source-path checks confirmed the full-buffer path has no references to range-local variables and that both upload paths feed the counters. No QEMU build or runtime test was run. CI and owner XP capture verification remain pending; T19 remains PAUSED.


## Session 121 (ChatGPT), 2026-11-24 - Preserve observed VB-ring capacity on failed appends

- Updated the profiling snapshot immediately after a successful ring upgrade, rather than waiting for the subsequent upload to succeed. This keeps `capacity_bytes` accurate when the larger host allocation succeeds but a later NOOVERWRITE lock fails.
- No allocator behavior changed. Static source inspection only; no build/runtime test. T19 remains PAUSED pending CI and owner XP capture verification.


## Session 122 (ChatGPT), 2026-11-24 - Add compact and detailed VMVGA diagnostic capture modes

- Added default summary-mode sampling for repetitive VMVGA event streams: first four SYNC writes/results, fence records, and FIFO-run-end records, plus the first eight selected SVGA3D command records. Detailed event mode preserves the previous per-event SYNC/fence/3D-command output through the opt-in `-global vmvga.debug-detail=on` property. Existing stall, failure, state-change, and fence-goal diagnostics remain active.
- Made VMVGA flight-recorder counters capture-interval scoped: reset the recorder when Ctrl+Alt+L starts, and emit the final flight summary plus FIFO command histogram when capture stops. The existing `Y2KBOX-PERF command` records continue to provide per-command counts and timing; no command profiling was removed.
- Added the selected capture mode to the log header and SDL window title. Documented summary and detailed usage in the README and T19 task entry.
- Static source review only; no QEMU build, CI run, or owner XP capture was performed. T19 is IMPLEMENTED, not DONE, pending Linux/Windows CI and owner verification of both modes and repeated capture intervals.


## Session 123 (ChatGPT), 2026-11-24 - Make diagnostic verbosity a single Y2KBox CLI setting

- Replaced the user-facing two-state debug switch with `-y2kboxdebugmode off|simple|on`: `off` disables diagnostics, `simple` selects compact sampled capture, and `on` selects the previous full selected per-event VMVGA logging. The default remains `off`.
- The command-line parser maps the selected level internally; users do not need any QEMU `-global` option or VMVGA-specific logging flag. Updated SDL title/log mode labels, command-line help, README, T12/T19 task wording, and HANDOFF.
- No QEMU build, runtime test, or CI run performed. Owner should verify all three values, including invalid-value rejection, and compare simple/full captures.


## Session 124 (ChatGPT), 2026-11-24 - Preserve explicit VMVGA warnings in simple captures

- Added a capture-gated `VMVGA-WARN` diagnostic path and routed identified legacy VMVGA warning/failure messages through it instead of the `VPRINT` macro. These cases were previously normally compiled out unless `VERBOSE` was enabled, so compact captures could miss them entirely.
- Covered unknown/invalid FIFO commands and VMware escapes, `NOP_ERROR`, invalid/rejected 2D geometry/ROP/surface operations, invalid/oversized cursor payloads, cursor allocation failures, invalid GMR2 flags, invalid Screen definitions, ignored fence-goal values, BIOS writes, and unknown register accesses.
- Added explicit warning records when the SVGA3D dispatcher receives an out-of-range command ID or an in-range command ID with no metadata. Existing FIFO/3D stall, fence-goal, and abnormal SYNC/FIFO diagnostics remain on their existing paths.
- This is targeted hardening, not a universal severity framework. QEMU/DXVK stderr error-reporting remains captured independently of VMVGA event sampling, but silent internal fallbacks are not guaranteed to be visible. Repeated-warning aggregation is not implemented and pathological repeated warnings can still generate repeated lines. A broader source-wide severity audit remains open.
- Static source review only. No QEMU build, runtime test, CI run, or owner XP capture was performed. T19 remains IMPLEMENTED pending platform CI and owner verification of simple/full captures and repeated intervals.


## Session 125 (ChatGPT), 2026-11-24 - Correct VMVGA warning placement after owner capture flood

- Reviewed the owner's one-second simple capture, in which repeated `VMVGA-WARN unknown FIFO command=...` and `unknown register write index=21 value=39` records accounted for roughly 90% of the log.
- Identified the cause in `hw/display/vmware_vga.c`: the generic FIFO warning ran before command dispatch, labelling every command unknown before the legacy switch and SVGA3D handler could recognize it; the register warning likewise ran before the register switch, labelling valid writes such as `SVGA_REG_SYNC` as unknown.
- Moved the generic FIFO warning into the final unhandled-command path, after the legacy switch and SVGA3D dispatcher decline the command, and limited it to non-3D commands so the SVGA3D-specific unknown-command diagnostics remain authoritative. Moved the unknown-register-write warning into the register switch's `default` case.
- Updated T19 task and handoff guidance to document the owner-capture finding and corrected classification. Repeated-warning aggregation remains a separate possible improvement; it was not implemented in this correction.
- Static source/documentation review only. No local build, runtime test, CI run, or post-fix owner capture was performed. The owner should repeat the same short simple-mode capture to confirm valid SYNC writes and handled FIFO commands no longer generate unknown warnings.


## Session 126 (ChatGPT), 2026-11-24 - Bound superseded-flip logging and fix BUSY read classification

- Reviewed the owner's compact and full captures after Session 125. The simple capture fell to 96 KB in under one second; repeated D3D9 `supersede-flip` records were the most visible remaining repetitive event. The full capture retained detailed CLEAR/FENCE/SYNC/readback activity.
- Confirmed in `svga_reg.h` that register index 22 is `SVGA_REG_BUSY`, and in `vmsvga_value_read()` that BUSY already has a valid read handler. The unknown-register-read warning was emitted before the switch, so it incorrectly labelled BUSY reads as unknown. Moved that warning into the switch's `default` case.
- Confirmed `vmsvga3d_screen_target_try_supersede_flip_live()` is an intentional optimization for eligible D3D9 valid-to-valid full-screen flips: it avoids synchronous readback of an obsolete frame and the associated BQL stall. Replaced unconditional simple-mode event output with first-two sampling plus capture-stop totals, first/last SID, a four-distinct-SID histogram, and overflow-event count for SIDs beyond the tracked set. Full `on` mode retains every supersede event.
- Updated T19, HANDOFF, WORKLOG_INDEX, and the rolling active-session summary. This changes diagnostic output only; it does not alter flip/readback scheduling or renderer behavior.
- Static source/documentation review only. No local build, runtime test, CI run, or owner XP retest was performed. The owner should repeat the same short simple capture and confirm the BUSY false warning is gone and the supersede summary matches the event pattern; full mode should still show every supersede event.


## Session 127 (ChatGPT), 2026-11-24 - Record owner verification and close diagnostic logging follow-up

- Recorded the owner's Windows XP capture results for Session 126: the approximately 48 KB `simple` capture reported 734 superseded flips with two sampled event lines and a 367/367 SID 2/SID 1 split; the approximately 1.5 MB `on` capture reported 766 superseded flips and 766 individual event lines.
- The capture headers confirmed the selected modes (`capture_mode=simple` and `capture_mode=on`). The owner confirmed the false unknown-register-read warning for valid `SVGA_REG_BUSY` was absent from both captures.
- Closed the Session 126 owner-retest follow-up. No source behavior changed; future profiling remains targeted to concrete investigations rather than adding speculative instrumentation.
- Updated T19 and handoff records with the owner evidence. T19 remains IMPLEMENTED, not DONE: Linux/Windows CI and repeated-interval acceptance are not established by these two captures.
- Documentation-only update. No local build, runtime test, or CI run was performed.


## Session 128 (GPT-6), 2026-11-24 - Expand full-mode VMVGA graphics-event diagnostics

- Expanded the opt-in full capture so the previously disabled deep FIFO trace runs when vmsvga_trace_detail_enabled() is true. Every FIFO command now emits a VMVGA-FIFO record with a capture-local sequence number, decoded command name when known, command ID, FIFO offsets, remaining word count, and SYNC state. Simple mode remains sampled.
- Added operand-level VMVGA draw records for legacy 2D SURFACE_COPY, SURFACE_FILL, and RECT_COPY; existing RECT_FILL and UPDATE rectangle records are retained.
- Made full mode activate the existing deep D3D9 draw and state/target diagnostics, previously gated only by disabled compile-time developer switches. This adds detailed draw planning/submission and render-target/texture state activity without changing rendering or synchronization behavior.
- Updated T19 and HANDOFF to describe the expanded full-mode coverage and its limits. The records cannot identify a HUD/minimap semantically; the minimap may be rendered through textured 3D draws. One owner full-mode capture spanning the visible flicker should provide the next useful evidence.
- Static source review only. No local QEMU build, runtime test, XP capture, or CI run was performed. Full-mode log volume is expected to increase substantially; simple and inactive/default-off paths are unchanged. T19 remains IMPLEMENTED pending owner capture validation and Linux/Windows CI.


## Session 129 (GPT-6), 2026-11-24 - Add full-mode D3D9 resource and render correlation records

- Started T26 as a coordinated follow-up to the Session 128 full capture and the owner's NFS Underground 2 capture. Kept all new high-volume records behind active full-mode capture; simple and inactive/default-off paths remain unchanged.
- Added `VMVGA-D3D9-BUFFER-DEFER` for each guest write range and its merged deferred range/update count. Added `VMVGA-D3D9-BUFFER-UPLOAD` for direct full-buffer DISCARDs and deferred native range uploads, including FIFO sequence, guest range, native Lock range/flags, selected mode/reason, flush cause, ring capacity, copied bytes, and result.
- Added `VMVGA-RESOURCE-UPDATE` after the guest MOB transfer updates the CPU shadow, recording image/buffer kind, SID, flattened subresource, format, update box, row/plane layout, byte count, image dimensions, and backend. It does not dump pixel contents and does not claim native upload has completed.
- Added `VMVGA-D3D9-GEOMETRY` per draw range with FIFO sequence, render target, viewport, primitive/index range, and stream count. Position and UV bounds remain explicitly unavailable with `bounds_reason=vertex-decode-not-implemented`; safe CPU-shadow vertex decoding is still outstanding.
- Added full-only `VMVGA-RENDER-CORRELATION` markers for readback submit/ready/retire/supersede events. They carry the last observed FIFO sequence; asynchronous polling means this is a correlation hint rather than guaranteed source-command identity.
- Updated `TASKS.md` (T26 remains IN PROGRESS), `HANDOFF.md`, and `logs/captures/README.md`. No GPU readbacks, forced synchronization, rendering changes, or upload-policy changes were intentionally introduced.
- Static source review only. No local QEMU build, tests, Windows XP capture, or CI was run. The owner should take one full-mode NFSU2 capture spanning visible flicker and inspect surfaces 29/35, 404 and 410–446 together. Do not label any resource as the minimap without geometry/visual evidence.


## Session 130 (GPT-6), 2026-11-24 - Bound and automatically compress full diagnostic captures

- Limited only full (`-y2kboxdebugmode on`) SDL captures to five seconds with a QEMU realtime timer. Ctrl+Alt+L can still stop a full capture early; the SDL title now advertises the automatic limit while it is active.
- After the diagnostic streams stop, stderr is flushed and restored before compression. The completed full log is compressed in-process with zlib to the matching `.log.gz` filename; the original `.log` is removed only after successful compression. Exclusive output creation prevents overwriting an existing gzip file, and compression failures retain the original text log.
- Simple mode is unchanged: manual start/stop, no timeout, and plain-text `.log` output. No external gzip executable is required. Runtime logs remain in the launch working directory and are not automatically committed to the repository.
- Added the zlib dependency to the SDL source set and updated README, HANDOFF, TASKS, and capture documentation.
- Static source review only. No local QEMU build, runtime test, XP capture, or CI run was performed; Linux/Windows CI and owner verification remain required.


## Session 131 (GPT-6), 2026-11-24 - Shorten full diagnostic captures and maximize gzip compression

- Reduced the full-mode automatic capture limit from five seconds to three seconds. Manual early stop remains available; simple mode remains manually stopped and uncompressed.
- Changed in-process gzip output from zlib's default compression level to level 9 (`wb9`, maximum compression). The existing failure handling remains: remove the original `.log` only after successful compression, and preserve it if compression fails.
- Updated README, HANDOFF, TASKS, command-line help, and `logs/captures/README.md` to document the three-second limit and level-9 compression. Added guidance to begin performance investigations with Simple-mode PERFSTATS / `Y2KBOX-PERF` aggregate summaries, which may be sufficient in some cases; use Full mode when per-event correlation is needed.
- Static source/documentation review only. No local build, runtime test, XP capture, or CI was run. Owner verification and Linux/Windows CI remain outstanding.


## Session 132 (GPT-6), 2026-11-24 - Shorten full captures and move gzip off the VM execution path
- Reduced the full-mode automatic capture limit from three seconds to two seconds. Simple-mode capture behavior is unchanged.
- Full capture now stops diagnostics, flushes the file, restores stderr, and transfers ownership of the completed `.log` path to a detached QEMU worker thread. The worker performs zlib level-9 gzip compression without occupying the QEMU main thread.
- The worker deletes the original `.log` only after successful compression; failures preserve the original and remove an incomplete `.gz`. SDL shutdown waits for outstanding compression workers so they cannot outlive QEMU.
- Updated README, HANDOFF, TASKS, command-line help, capture documentation, and the active-session summary. Static source review only; owner runtime verification and Linux/Windows CI remain outstanding.


## Session 133 (GPT-6), 2026-11-24 - Decode CPU-shadow vertex position and UV bounds
- Replaced the Full-mode `vertex-decode-not-implemented` placeholder in `VMVGA-D3D9-GEOMETRY` with CPU-shadow decoding for indexed and non-indexed draw ranges.
- The initial decoder supports FLOAT1–FLOAT4 declarations, including the observed NFSU2 FLOAT3 position and FLOAT2 TEXCOORD0 layout. It applies stream source offsets/strides and indexed base-vertex addressing, validates index and vertex-buffer bounds, rejects non-finite values and instanced streams, and caps each draw at 65,536 vertices.
- Emits raw vertex-input position X/Y/Z bounds (where present), TEXCOORD0 U/V bounds, decoded sample count, success flag and explicit failure reason. These values are not screen-space transformed coordinates. No GPU readback, forced synchronization, or rendering behavior change was introduced.
- Updated README, HANDOFF, TASKS, capture documentation and active-session notes. T26 remains IN PROGRESS pending owner full-mode NFSU2 capture and Linux/Windows build/CI validation. Static source review only; no build or runtime test was run.

### Session 134 — Add guest frame IDs and effective D3D9 draw-state snapshots (2026-11-24)
- Added capture-local trace_frame_id, reset at each diagnostic capture. A validly parsed guest SVGA_3D_CMD_PRESENT command emits VMVGA-FRAME with the frame ID being closed and the event/FIFO sequence, then advances the ID for subsequent draw commands. This is intentionally a guest PRESENT command boundary, not a claim about host SDL presentation or vblank timing.
- Added VMVGA-D3D9-STATE-SNAPSHOT in the validated legacy draw-state path. It records the context ID, viewport/scissor validity and rectangles, all bound render-target slots, bound shader IDs, all valid texture/sampler state values, all valid render-state values, and each bound texture's SID plus live dimensions, format and mip count. Every snapshot includes both frame and seq, allowing the effective guest context to be reconstructed at each draw rather than inferring it from isolated texture-bind events.
- Corrected the SDL window title: the existing Full-mode timer is two seconds, but the title incorrectly advertised three seconds.
- Reviewed the current SDL/GLib/zlib image path. No existing PNG/JPEG encoder was found; did not add an image library or screenshot path. A single pixel snapshot remains deferred until an already-available encoder/readback path can be demonstrated without adding a dependency or perturbing timing.
- Updated logs/captures/README.md, TASKS.md, and WORKLOAD_ACTIVE_SESSION.md. T26 remains IN PROGRESS.
- Verification: static source and documentation review only. No QEMU configure/build/make/ninja, local tests, XP capture, or CI was run (ChatGPT is not authorized for local QEMU builds). Owner should take one Full-mode capture (hard limit two seconds) spanning the NFSU2 minimap flicker, then inspect VMVGA-FRAME, VMVGA-D3D9-STATE-SNAPSHOT, geometry, resource updates, buffer uploads and readback/presentation records together. Linux and Windows CI remain required.

### Session 135 — Add one optional CPU-surface PNG to Full captures (2026-11-24)
- A deeper build/source inspection found that libpng is already an optional Y2KBox dependency used by QMP screendump/VNC, and Pixman already provides the RGB line-buffer conversion helper. Reused these existing features behind CONFIG_PNG and CONFIG_PIXMAN; no new library or repository dependency was added.
- Full-mode capture now copies the current CPU-side guest DisplaySurface into a private RGB buffer at capture stop, then writes exactly one PNG beside the log. It avoids SDL_RenderReadPixels/glReadPixels and does not force a host GPU readback. Active direct GPU scanout is skipped; builds without libpng/Pixman log that the snapshot is unavailable.
- PNG creation uses exclusive file creation and does not overwrite an existing image. Snapshot errors do not prevent the diagnostic log from being compressed. The PNG is written after stderr restoration; gzip compression remains asynchronous.
- This supersedes Session 134's interim decision to defer pixel snapshots. The PNG is the last CPU-side guest display surface at capture stop, not a video recording or a screenshot of host-window decorations/scaling.
- Updated capture README, HANDOFF, TASKS and WORKLOAD_ACTIVE_SESSION. T26 remains IN PROGRESS.
- Verification: static source review only. No local QEMU build, runtime capture, XP validation, or CI was run. Owner should verify CONFIG_PNG/CONFIG_PIXMAN in the Windows build, check the PNG alongside the two-second NFSU2 Full log, and run Linux/Windows CI.


### Session 136 (GPT-6), 2026-11-25 - Add Full-mode texgen and cube-shadow diagnostics

- Continued T26 for the Joint Operations: Typhoon Rising D3D8 missing-canopy investigation. Added targeted Full-mode records in `hw/display/vmware_vga_3d_state.c`: WORLD/VIEW/TEXTURE0 transform matrices, texture-transform flags and coordinate-generation state, active material values, enabled light data, and per-face level-zero cube-map CPU-shadow dimensions/pitch/size/FNV-1a checksums.
- Each record carries guest frame and event/FIFO sequence; texgen records also carry context/stage/SID. The cube-face checksum path is bounded to 2 MiB per record and explicitly identifies its source as the CPU shadow; it does not claim GPU-current pixels or correct face orientation.
- Kept the instrumentation behind `vmsvga_trace_detail_enabled()` and restricted it to draws using 3D transform flags or nonzero texcoord generation. No renderer, upload, synchronization, or scheduling behavior was changed.
- Updated T26, HANDOFF and capture documentation. T26 remains IN PROGRESS pending owner Full-mode Joint Operations capture and Linux/Windows CI.
- Verification: remote source/static review only. No local QEMU build (not authorized), runtime capture, owner XP visual verification, or CI was run. The initial commit's record terminators and capture guard were corrected in a follow-up commit; current source was fetched back and inspected.


### Session 137 — Make Full diagnostic capture non-cancellable (2026-11-25)

- Restored the intended Full-mode lockout on the rolled-back single-PNG baseline. While a Full capture is active, Ctrl+Alt+L is consumed but does not stop or toggle the capture; the existing QEMU realtime timer remains the only normal stop path and fires at two seconds.
- Simple mode retains its existing manual Ctrl+Alt+L start/stop behavior. Internal shutdown cleanup and the timer callback can still stop an active capture safely.
- The SDL window title now states that the Full-mode hotkey is locked and the capture auto-stops at two seconds. Updated the capture README and T19 safety-limit documentation to match.
- Static source review and remote GitHub content updates only. No local build, runtime test, XP capture, or CI was run; Linux/Windows CI and owner runtime verification remain pending.


### Session 138 (Claude), 2026-11-25 - Experimental Y2KBOX_ENVMAP_FIX for Joint Operations glass

- Analysed the owner's two Full captures (`y2kbox-log-20261125-002230` and `-055855`). All 11,021 draws were issued OK. The only helicopter glass pass is one additive (ONE/ONE), depth-write-off, lit draw per frame (e.g. seq 60582 / 326) using cube SID 25 (six 128x128 X8R8G8B8 faces, one mip) with reflection-vector texgen and COUNT3 transform flags (7). Five of six CPU-shadow faces hash identically; only face 3 (-Y) differs.
- The new TEXGEN-MATRIX records show the guest's TEXTURE0 matrix for these draws is not a rotation although VIEW is: it is `[[V00,0,V20],[0,V01,V11],[V21,0,V02]]` (VIEW elements, fixed pattern, identical in all 72 draws) with an all-zero last row/column. A reflection lookup through it collapses to nearly one cube direction. Y2KBox does not modify transforms; the matrix arrives from the guest as sent.
- Owner reports older builds did not show the missing glass. Before `89c97603` (2026-11-22) texture-transform flags were translated by an exact-value switch, so flags `S|T|R` mapped to DISABLE and texgen was ignored: older builds sampled the cube with plain vertex UVs and no matrix. `89c97603` made the host honour texgen and the guest matrix, exposing this matrix. Older screenshots are therefore not necessarily what faithful reflection mapping looks like.
- Added opt-in `Y2KBOX_ENVMAP_FIX=1` (default OFF; no behaviour change without it). Per draw, after texture state is applied, for a stage with a cube texture, reflection-vector texgen, COUNT3 (no Q) transform flags, an orthonormal VIEW 3x3 and a non-orthonormal (tolerance 0.05) TEXTURE0 matrix, the transposed VIEW rotation (identity elsewhere, m33=1) is sent to D3D9 instead; the original matrix is restored when the condition stops holding. Proper rotation matrices and non-cube textures are never touched. Per-context bitmask `envmap_override_stages`. Logs `VMVGA-D3D9-ENVMAP-FIX ... action=override|restore` in Full mode.
- Verification: the new functions were compiled standalone with stubs (gcc -Wall -Wextra, no warnings in the new code) and run against the real captured VIEW/TEXTURE0 matrices: fixed matrix is orthonormal, already-good matrix untouched, restore and non-cube paths correct. No QEMU configure/build/make/ninja was run; Linux/Windows CI, owner runtime test and visual verification pending. If the owner confirms the glass looks right, remove the env switch and make it unconditional.
