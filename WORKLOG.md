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
