# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 93–102).

## How to use this file

- **New coding-agent session / fresh sandbox:** follow `AGENTS.md` (canonical shared policy); use `CLAUDE.md` or `GEMINI.md` only for tool-specific integration. Read this summary and `WORKLOG_INDEX.md`, plus only HANDOFF sections 0-3 and the selected task entry in `TASKS.md`. Do not read entire HANDOFF/TASKS/WORKLOG files by default. Use the index for relevant historical sections and verify old claims against current source and owner decisions.
- **Continuing in the same coding-agent session:** do not reread onboarding files or the entire `WORKLOG.md` without a reason. Use this rolling summary for recent context, then consult `HANDOFF.md` and relevant `TASKS.md` entries. Open only specific historical sections needed to verify an older decision or result.
- A new user prompt, task, or turn does **not** by itself make a new coding-agent session. A genuinely new agent runtime/sandbox with no retained working context does. After context compaction in the same runtime, use this summary rather than restarting the full-history read.
- At the end of each work session, append the session to `WORKLOG.md` first, then refresh this file so it contains exactly the latest ten worklog sessions. Never rewrite or trim `WORKLOG.md`.

## Current project state and working constraints

- Repository: [AhayriSG/Y2KBox](https://github.com/AhayriSG/Y2KBox), `main`.
- This summary can lag behind `main`; verify the current branch HEAD and task status before making changes.
- Y2KBox targets 32-bit Windows XP. Linux product runtime is KVM-only; Windows product runtime is TCG-only. Performance optimization work targets Windows TCG.
- VMware SVGA/SVGA3D via VMVGA is the sole active display/3D path. SDL is a fixed-size virtual monitor: guest resolution controls window client size; host resizing must not change guest geometry; fullscreen toggle is Ctrl+Alt+F.
- Keep the owner-approved guest/device constraints in `HANDOFF.md`; do not infer that a standalone test or green CI proves in-game correctness.
- **Build/CI workflow:** Gemini 4 Neon alone has standing authorization for ordinary local QEMU builds and standard checks; all other agents need explicit owner authorization. Gemini 4 Neon also has standing authorization for relevant, bounded benchmarks/A-B; other agents still need explicit authorization. GitHub Actions remains authoritative; report local results separately. Markdown-only commits run only the lightweight detector; Linux/Windows jobs are skipped. After pushing, do not poll CI.
- Diagnostic captures are independent sessions: every Ctrl+Alt+L start resets counters and writes a new timestamped `.log`; stopping ends that capture interval without restarting the VM.

## Latest ten worklog sessions

## Current investigation / next actions

1. **T13e-F1 / Windows TCG profiling remains active.** The owner supplied a dry-weather capture on 2026-11-23 with `capture_ms=23526`; `cpu.exec` reported 3,415 intervals, 23.515560 s total, 19.554 ms max, 3,164 intervals over 1 ms, 1,064 over 10 ms, and none over 100 ms. This is an interval summary, not workload attribution; inspect the complete capture and compare only equivalent scenes/settings before proposing code changes.
2. **Hitman 2: Silent Assassin D3D8 visual issue remains open.** The owner supplied a sub-second menu capture while investigating static-background rendering. Analyze that capture and current VMVGA/D3D8 path; do not claim a fix or infer visual correctness from CI.
3. **Diagnostic capture coverage remains under investigation (T19).** Separate timestamped sessions and per-capture counter resets have been exercised, but unusually small later log files compared with the first capture need category-by-category comparison before declaring all output coverage complete.
4. **T25 guest OpenGL investigation is active.** Mesa's SVGA Gallium driver translates OpenGL operations into SVGA3D commands; the host dispatcher accepts some legacy commands such as DRAW_PRIMITIVES, but its legacy capability profile is documented as D3D9-oriented. Compare the exact Mesa GL 2.1 command/state/capability needs against current handlers. An XP-side probe is now prepared at `xp/tools/glprobe.c`: it reproduces COD1's id Tech 3 `GLW_ChoosePFD` filters/best-match logic, reports rejection reasons for the 32/24/8 and 32/24/0 attempts, tests strict and software-allowed selection, then checks WGL context creation and GL strings. Next: owner runs it in XP and returns the complete output.
5. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
6. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.

## Key diagnostic interpretation rules

- Performance counters can be nested. Do not add child phase totals to enclosing command/FIFO totals.
- Counters whose implementation uses `qemu_perf_stats_mark()` measure intervals between marks (event cadence), not operation execution time.
- Overlapping lock dimensions (aggregate, vertex/index, full/partial, flags, size buckets) are not additive.
- A green CI run proves build/test coverage only; it does not prove XP guest gameplay or visual correctness.
- Keep the exact same repeatable owner workload for performance comparisons. Distinguish measured facts, owner observations, hypotheses, and verified fixes.


### Session 93 — Move QEMU base and license provenance into the README FAQ
- Removed the upstream QEMU base/version and GPL-2.0 notice from the Fixed target area.
- Added a dedicated FAQ answer identifying upstream QEMU 11.1.90, linking the exact base commit, and noting the unchanged GPL-2.0 license with `COPYING` and `LICENSE` references.
- Documentation-only; no build, benchmark, functional test, or CI polling.

### Session 94 — Reconcile agent guidance and stale handoff instructions
- Re-audited all 16 Markdown files; no malicious prompt injection or unrelated-repository redirection found.
- Clarified that the QEMU codebase skill does not authorize builds/tests and that `AGENTS.md` controls execution permission.
- Scoped CI/Codacy status-hook reporting to Claude; other agents leave post-push review to the owner or a later session and do not poll.
- Replaced obsolete fetch/rebase advice with per-write verification of latest `main` HEAD and exact target blob SHA; clarified Gemini 4 Neon-only standing local build/benchmark authorization.
- Updated the worklog index; historical worklog entries remain append-only. Documentation-only; no builds, tests, benchmarks, guest validation, or CI polling.


### Session 95 — Fix optional GitHub token handling in CI status helper
- Corrected `scripts/session_status.sh` to pass an existing `GITHUB_TOKEN` or `GH_TOKEN` as one properly quoted Authorization header argument to `curl`.
- No change to sandbox GitHub authentication, token setup, permissions, or GitHub Actions; this only improves Claude's optional CI-status API requests.
- Static review only; no token was read/displayed, no authenticated request or CI polling was performed, and no QEMU build/test was run.

### Session 96 — Move diagnostic logging guidance into the README FAQ
- Relocated the Ctrl+Alt+L instructions into a dedicated FAQ entry and retained the details about opt-in capture, separate files, counter resets, and shell redirection.
- Added the expected recording overhead, guidance for comparable performance captures, and a tester-to-developer/LLM-assisted profiling workflow using `Y2KBOX-PERF` output.
- Clarified the distinction between performance profiling and graphics diagnostics; documented `VMVGA_TRACE_DEEP_GB_UPDATE` as an optional compile-time trace disabled by default.
- Documentation-only; no source changes, instrumentation, builds, benchmarks, tests, guest validation, or CI polling.

### Session 97 — Merge preservation rationale into the README project-purpose FAQ
- Combined the former standalone preservation-practicality FAQ with the compatibility-layer answer, connecting the case for a complete XP environment with the practical benefits of emulation.
- Preserved the Wine/wrapper distinction, original-hardware maintenance rationale, portability and repeatability benefits, minimal-workaround vision, Direct3D 8.1 / 9.0c target, and PCSX2 comparison.
- Removed the separate preservation FAQ heading. Documentation-only; no builds, benchmarks, tests, guest validation, or CI polling.

### Session 98 — Move project-purpose FAQ to the bottom of the README
- Moved the merged compatibility-layer and preservation answer to the final FAQ position, after the technical questions.
- Preserved the approved answer wording; only section order changed.
- Documentation-only; no builds, benchmarks, tests, guest validation, or CI polling.

### Session 99 — Add guest OpenGL investigation to the task queue
- Added T25 to investigate a Windows XP guest OpenGL path through VMware SVGA3D, targeting OpenGL 1.1 baseline through OpenGL 2.1 maximum.
- The task begins with WGL pixel-format and ICD diagnostics for Call of Duty 1, audits the guest/host SVGA3D protocol, evaluates an XP-compatible guest ICD, and requires incremental context/rendering tests while preserving D3D9.
- Key distinction: Mesa demonstrates OpenGL 2.1 over VMware SVGA3D, but that does not prove VMware Tools 10.0.12 supplies accelerated OpenGL for Windows XP. No emulator source changes, builds, tests, guest validation, or CI polling.

### Session 100 — Start guest OpenGL investigation and audit the SVGA3D path
- Began T25 (IN PROGRESS), targeting OpenGL 1.1 baseline through OpenGL 2.1 maximum.
- Corrected the protocol model: Mesa translates OpenGL into SVGA3D commands; there is no separate raw OpenGL FIFO. Y2KBox already whitelists some legacy commands used by SVGA3D, but complete GL 2.1 semantics and caps are unverified.
- The latest QEMU diagnostic log has no WGL/PFD output. A guest-side probe is needed to enumerate pixel formats, distinguish generic software from ICD formats, and test context creation.
- Mesa's documented SVGA guest driver stack is Linux-specific; historical Workstation 6 documentation describes XP OpenGL as software-emulated but does not establish VMware Tools 10.0.12 behavior. README Fixed target wording was previewed and awaits owner approval. No source changes, diagnostic utility, builds, tests, guest validation, or CI polling.

### Session 101 — Clarify the guest OpenGL target in the README
- Updated the README Fixed target to include OpenGL 1.1 as the baseline and OpenGL 2.1 as the maximum, explicitly noting that guest OpenGL is not yet implemented.
- T25 remains an investigation: a complete 2.1 path must also preserve compatibility with older 1.x applications, and the Call of Duty 1 WGL pixel-format failure still needs an XP-side probe.
- Documentation-only; no emulator source changes, diagnostic utility, build, benchmark, guest test, or CI polling.

### Session 102 — Build an XP-side COD1 WGL pixel-format probe
- Identified COD1's GLW_ChoosePFD routine as the custom id Tech 3 PFD enumerator, not Windows ChoosePixelFormat. It rejects generic formats by default, requires RGBA/window/OpenGL/double-buffer flags, at least 15 depth bits, and at least 4 stencil bits when stencil is requested.
- Added xp/tools/glprobe.c and its README. It mirrors the game's filtering/best-match logic for 32/24/8 and 32/24/0, explains per-format rejections, compares software-allowed selection, and tests WGL context creation plus GL vendor/renderer/version.
- T25 now points to the probe. Next is an owner-run XP capture; the utility has not been compiled or run. No QEMU build, guest test, benchmark, or CI polling.
