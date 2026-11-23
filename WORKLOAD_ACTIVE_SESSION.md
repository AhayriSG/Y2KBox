# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 80–89).

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

### Session 80 — FIFA/VMVGA and FlatView BQL stalls
- FIFA 2003 profiling found BQL starvation: `vmvga.fifo_run` + `vmvga.display_update` held the BQL for 24.90 s (58.3% of a 42.741 s capture). Five hotspots accounted for about 20.6 s: redundant CPU shadow clears before successful GPU clears, 16->32-bit present conversion divisions, FlatView topology rebuild overhead, dynamic-buffer overlap locks on a >64 KiB vertex buffer, and an accidentally unconditional readback trace path.
- Implemented: skip CPU shadow clear when native GPU clear succeeds (retain fallback); bit-exact SSE2 and division-free scalar 16->32 conversion; gate deep readback tracing correctly; raise overlapping-buffer full-shadow DISCARD threshold from 64 KiB to 256 KiB; replace the GLib GSequence FlatView builder with ping-pong radix sorting plus a sweep-line/min-heap renderer; skip no-op GART unmap transactions.
- Standalone checks passed, including exhaustive 65,536-value pixel conversion, clear-path equivalence, and randomized FlatView equivalence against the legacy renderer.
- At the time Session 80 was appended, full Linux/Windows CI and in-game FIFA/F1 validation were marked pending. The owner subsequently reported both Linux and Windows CI passing. A separate 23.526 s dry-weather capture was also shared: `cpu.exec` count 3,415, total 23.515560 s, max 19.554 ms, 3,164 intervals >1 ms, 1,064 >10 ms, and none >100 ms. The capture's full workload attribution is not reproduced here; consult its diagnostic log before drawing conclusions.

### Session 81 — Skip binary artifacts for Markdown-only changes
- Added a fail-safe change detector. Initially, Markdown-only commits still ran Linux/Windows builds and functional checks, but skipped packaging, packaged smoke tests, and artifact uploads.
- Any non-Markdown change and manual `workflow_dispatch` run retained normal artifact production; missing comparison commits or diff errors conservatively enabled artifacts.
- The later job-level skip (commit `8ab36ee`, recorded in Session 83) changed the current behavior so Markdown-only commits now run only the detector.

### Session 82 — Make coding-agent historical reads selective
- Replaced full-history onboarding with a layered read: current handoff/task state, the rolling summary, and a topic index; open only relevant worklog sessions.
- Added `WORKLOG_INDEX.md` as a topic-to-session navigation map. The full archive remains append-only and is never a default context dump.
- The full Session 71 record remains in `WORKLOG.md`; it only aged out of the rolling summary when Session 81 entered the ten-session window.


### Session 83 — Consolidate agent guidance and repair project continuity docs
- Made `AGENTS.md` the canonical shared policy for Claude, Gemini, Codex, and other agents; reduced `CLAUDE.md` to Claude hooks/secrets and added a short `GEMINI.md` entrypoint.
- Tightened onboarding to read HANDOFF sections 0-3, the task overview plus only the selected task, the active summary, and the worklog index. Historical details remain available on demand.
- Aligned documentation with the job-level Markdown-only CI skip introduced in `8ab36ee`, corrected XP boot status, single-machine state, frontend direction, artifact guidance, and several task statuses. Condensed the T13 task entry from about 20 KB to 4 KB by moving detailed historical evidence back to the indexed worklog. No QEMU source changed and no local QEMU build was run.

### Session 84 — Clarify cross-agent handoff and human validation
- Confirmed Claude, Gemini, Codex, and other agents use the same durable repo state through `AGENTS.md`, task status, handoff, active summary, and worklog. Sequential agent/provider switching is the normal owner workflow; agents do not share private runtime memory.
- Removed T08 and T11 as claimable tasks. XP SP3 boot is owner-verified; remaining driver, D3D API, game, visual, and performance acceptance is combined into a manual owner-run checklist in `TASKS.md`. Agents can analyze supplied evidence but cannot certify guest behavior.
- Updated T09/T10 and HANDOFF references, clarified the separate Hitman 2 capture investigation, and removed wxWidgets from README frontend messaging. SDL2 + Dear ImGui is the clear primary frontend direction.
- No QEMU source changes, local build, or CI polling.

### Session 85 — Model-specific local build authorization
- Added standing owner authorization for Gemini 4 Neon to run ordinary local QEMU configure/build and relevant standard functional checks while validating its own changes.
- Other Gemini models, Claude, Codex, and other agents still need explicit authorization before local QEMU builds. Benchmarking/A-B authorization was still restricted at the time of this session.
- GitHub Actions remains authoritative; local results must report exact commands/configuration and remain distinct from CI status. Updated shared agent guidance, Gemini entrypoint, handoff, task workflow, and worklog/index. No build, benchmark, or CI polling was performed.


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


### Session 86 — Extend Gemini 4 Neon standing authorization to benchmarks
- Owner approved Gemini 4 Neon for relevant, bounded performance benchmarks and controlled A/B experiments, including the established `xp/bench/run.sh` and `xp/bench/perf.py` workflows.
- The exception remains specific to Gemini 4 Neon; other Gemini models, Claude, Codex, and other agents still need explicit authorization.
- Updated the shared policy, Gemini entrypoint, handoff, task workflow, and continuity records. No build, benchmark, functional test, or CI polling was performed.

### Session 87 — Clarify fixed guest targets and XP memory limits
- README now explains that the current 2048 MiB setting is a Y2KBox product default, not XP's physical RAM ceiling; a future 4 GiB GUI option is only a possibility.
- Clarified the 32-bit XP 2 GiB/2 GiB virtual-address split, /3GB plus LARGEADDRESSAWARE requirements and kernel tradeoff, and why extra RAM may help the OS without guaranteeing game access or reduced stutter.
- Documented the guest-facing Direct3D 9.0c / Shader Model 3.0 ceiling, confirmed against the project's advertised SVGA3D caps. Updated HANDOFF's durable RAM note. No build, benchmark, or CI polling was performed.

### Session 88 — Reorganize README host and guest-memory guidance
- Kept the Fixed target summary guest-only by removing its duplicate host-platform bullet.
- Moved host requirements and the full 2 GiB RAM explanation into separate entries under a new FAQ at the bottom of README.
- Preserved the XP physical-memory, virtual-address, /3GB, LARGEADDRESSAWARE, and workload caveats. Documentation-only; no build, benchmark, functional test, or CI polling.

### Session 89 — Refocus the README introduction on the project identity
- Replaced the dense opening technical/runtime summary with a concise description of Y2KBox's Windows XP gaming focus and its mid-to-late 2000s PC performance target.
- Clarified that the project is not chasing modern high-end gaming performance and is not intended to be a cycle-accurate, low-level hardware emulator.
- Added an appreciative acknowledgment of 86Box and its contributors, without a repetitive closing mission statement.
- Documentation-only; no build, benchmark, functional test, or CI polling.
