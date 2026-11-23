# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 87–96).

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
4. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
5. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.

## Key diagnostic interpretation rules

- Performance counters can be nested. Do not add child phase totals to enclosing command/FIFO totals.
- Counters whose implementation uses `qemu_perf_stats_mark()` measure intervals between marks (event cadence), not operation execution time.
- Overlapping lock dimensions (aggregate, vertex/index, full/partial, flags, size buckets) are not additive.
- A green CI run proves build/test coverage only; it does not prove XP guest gameplay or visual correctness.
- Keep the exact same repeatable owner workload for performance comparisons. Distinguish measured facts, owner observations, hypotheses, and verified fixes.


### Session 88 — Reorganize README host and guest-memory guidance
- Kept the Fixed target summary guest-only by removing its duplicate host-platform bullet.
- Moved host requirements and the full 2 GiB RAM explanation into separate entries under a new FAQ at the bottom of README.
- Preserved the XP physical-memory, virtual-address, /3GB, LARGEADDRESSAWARE, and workload caveats. Documentation-only; no build, benchmark, functional test, or CI polling.

### Session 89 — Refocus the README introduction on the project identity
- Replaced the dense opening technical/runtime summary with a concise description of Y2KBox's Windows XP gaming focus and its mid-to-late 2000s PC performance target.
- Clarified that the project is not chasing modern high-end gaming performance and is not intended to be a cycle-accurate, low-level hardware emulator.
- Added an appreciative acknowledgment of 86Box and its contributors, without a repetitive closing mission statement.
- Documentation-only; no build, benchmark, functional test, or CI polling.

### Session 90 — Fix the product target to one vCPU
- Owner approved a strict 32-bit Windows XP uniprocessor target: exactly one vCPU, with no multi-vCPU/SMP mode.
- Set `max_cpus` to 1 and removed QEMU's `-smp` CLI option/parser. README now states the 32-bit guest OS and exact one-vCPU limit.
- Updated T01, T14, and HANDOFF so future implementation and thread-pinning work preserve the fixed CPU count. Old two-vCPU boot evidence is explicitly historical.
- Static review only; no local build, functional test, or CI polling.

### Session 91 — Explain Y2KBox's purpose and fixed vCPU target in the README FAQ
- Moved the former Project goals section into the FAQ and explained why a real
  Windows XP environment serves a different purpose from compatibility layers.
- Added a standalone one-vCPU FAQ answer covering era-accurate dual-core XP,
  workload-dependent TCG/MTTCG coordination overhead, and the distinction
  between guest vCPUs and host threads.
- Separated the proprietary hypervisor API rationale into its own FAQ question
  and retained the preservation-practicality discussion as a closing entry.
- Clarified HANDOFF's durable vCPU rationale. Documentation-only; no QEMU build,
  benchmark, functional test, or CI polling.

### Session 92 — Close T09 integration; keep XP acceptance owner-run
- T09's qemu-vmvga integration and Linux/Windows CI checks are complete; its task status is DONE.
- VMware Tools/driver, 2D/display, D3D API, and game compatibility checks remain in the separate owner-run XP guest validation checklist, not as unfinished agent work.
- HANDOFF wording now distinguishes completed T09 integration from ongoing human acceptance. T10 remains separately paused for its own validation scope.
- Documentation-only update; no source changes, local build, benchmark, guest test, or CI polling.

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
