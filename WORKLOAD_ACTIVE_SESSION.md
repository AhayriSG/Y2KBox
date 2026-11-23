# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 73–82).

## How to use this file

- **New coding-agent session / fresh sandbox:** follow `CLAUDE.md` and `HANDOFF.md`; read this summary and `WORKLOG_INDEX.md`. Do not read the full `WORKLOG.md` by default. Use the index to open only historical sessions relevant to the task, and verify important old claims against current source and task/handoff state.
- **Continuing in the same coding-agent session:** do not reread onboarding files or the entire `WORKLOG.md` without a reason. Use this rolling summary for recent context, then consult `HANDOFF.md` and relevant `TASKS.md` entries. Open only specific historical sections needed to verify an older decision or result.
- A new user prompt, task, or turn does **not** by itself make a new coding-agent session. A genuinely new agent runtime/sandbox with no retained working context does. After context compaction in the same runtime, use this summary rather than restarting the full-history read.
- At the end of each work session, append the session to `WORKLOG.md` first, then refresh this file so it contains exactly the latest ten worklog sessions. Never rewrite or trim `WORKLOG.md`.

## Current project state and working constraints

- Repository: [AhayriSG/Y2KBox](https://github.com/AhayriSG/Y2KBox), `main`.
- Latest repository HEAD observed while preparing this summary: `b1bdf5bf0967203661915f946f8a52610b9d59dd` — Session 80 implementation commit, dated 2026-11-23.
- Y2KBox targets 32-bit Windows XP. Linux product runtime is KVM-only; Windows product runtime is TCG-only. Performance optimization work targets Windows TCG.
- VMware SVGA/SVGA3D via VMVGA is the sole active display/3D path. SDL is a fixed-size virtual monitor: guest resolution controls window client size; host resizing must not change guest geometry; fullscreen toggle is Ctrl+Alt+F.
- Keep the owner-approved guest/device constraints in `HANDOFF.md`; do not infer that a standalone test or green CI proves in-game correctness.
- **Build/CI workflow:** follow `CLAUDE.md` and HANDOFF section 7. Do not build locally unless explicitly doing approved benchmarking/A-B work. After pushing, finish the turn; do not poll CI. The status hook reports CI/Codacy results.
- Diagnostic captures are independent sessions: every Ctrl+Alt+L start resets counters and writes a new timestamped `.log`; stopping ends that capture interval without restarting the VM.

## Latest ten worklog sessions

### Session 73 (+ follow-up) — Lock profiling dimensions
- Added counters separating DISCARD from flags-zero locks and requested lock-size buckets (<=4 KiB, <=64 KiB, <=256 KiB, >256 KiB), while retaining aggregate and vertex/index/full/partial dimensions.
- Fixed missing local counter declarations after Linux and Windows CI caught the compile error.
- Owner's later 32.314 s dry F1 capture showed 44,448 deferred updates and 44,448 flushes, zero merges; 44,447 flushes were draw-consumption flushes. Lock time remained dominated by partial-range flags-zero calls. Counter dimensions overlap; do not sum them.
- `qemu_perf_stats_mark()` reports elapsed time between marks, not time spent inside the operation. Treat flush-draw `total_us` as cadence, not execution cost.

### Session 74 — Close FlatView scene-transition work; classify F1 locks
- Owner reported Linux and Windows CI passing at `36a12e7e5d9d6d309873e0520a5e05f86e3224a5`; the ordered-tree FlatView renderer and original multi-second scene-transition stall were marked DONE. Residual crowded-grid work remained separate as T13e-F1.
- A 23.526 s F1 capture showed 38,026 buffer locks / 1.197 s, overwhelmingly small partial flags-zero locks. Deferred updates again had zero merges.
- The profile did not prove a controlled performance gain or GPU-wait cause. Preserve immediate pre-draw ordering and safe lock semantics.

### Session 75 — In-flight range tracking and safe DISCARD/NOOVERWRITE selection
- Root cause of repeated partial Lock synchronization: guest invalidation commands signaled dynamic-buffer DISCARD/rename cycles, but the backend did not track drawn byte ranges in flight.
- Wired guest buffer invalidation into the DXVK backend and added per-surface sorted/coalesced in-flight byte ranges. Draw paths mark exact vertex/index ranges, with conservative full-buffer fallback for uncertain cases.
- Lock policy: flags=0 for eviction/replacement; DISCARD for full replacement or post-invalidation rename; NOOVERWRITE only when the written span is proven disjoint from in-flight ranges; overlapping small buffers (<=64 KiB) may use full-shadow DISCARD; larger overlaps remain flags=0.
- Standalone randomized interval and lock-mode tests passed. Linux/Windows CI and owner runtime A/B were pending at the time of the entry.

### Session 76 — Validate buffer-lock improvement; add vCPU profiler
- Owner's Session 75 capture reduced measured buffer Lock time from 1,196.5 ms to 22.58 ms (about 53x less total time; per-call latency about 73x lower). NOOVERWRITE and invalidation DISCARD handled the measured updates; owner also reported smoother Hitman 2 and slight improvement in FIFA 2003 / Joint Operations. F1 still felt unchanged overall.
- Phase profiling isolated a separate F1 launch cliff: during race launch the vCPU was busy almost continuously while host SVGA3D/display work accounted for under 2% of frame time; most time was unattributed inside TCG.
- Added structural TCG counters and an opt-in 1 kHz Windows x64 vCPU sampler, active only during Ctrl+Alt+L capture. It reports per-second FPS phases, host symbols, and JIT guest PCs.
- Standalone compile/runtime checks passed; full Linux/Windows CI and owner capture were pending.

### Session 77 — x87 TCG bottleneck overhaul
- vCPU profiling found x87 helpers and softfloat consumed more than half of samples, with additional cost from exception-flag merging, redundant loads, a lost FST fast path, zero handling, helper_fpop, and missing TCG_CALL_NO_RWG annotations.
- Implemented in-place host-x87 arithmetic with lazy status synchronization and softfloat fallback, restored FST m32/m64 fast paths, optimized exact integer loads and zero operands, inlined stack/move/sign operations, and added TCG call annotations. Changes retained an `XPQEMU_NO_HX87=1` kill switch for bit-exact A/B comparison.
- Owner validation after Session 77 reported F1 Challenge rising from 8.7 FPS to 16.0 FPS average in the dry Albert Park capture, with no low-FPS collapse seconds. The x87 changes were validated by Linux/Windows CI and the differential test.
- New hotspots shifted toward PIO/BQL contention and TB lookup; do not assume the x87 work alone explains later workload behavior.

### Session 78 — PIO/BQL, TB lookup, and x87 cleanup
- Removed BQL acquisition for no-op ISA port 0x80 and buffered stderr during Ctrl+Alt+L capture, eliminating a large number of single-byte Windows WriteFile calls. Added capture-only PIO-port counters.
- Optimized TCG TB lookup: larger jump cache, inline cache metadata, direct guest-state arguments, and a leaf fast path. Further x87 status and stack/move helpers were optimized.
- Expanded CI x87 differential coverage from 30 to 46 operations. Standalone checks passed; full CI and owner capture were pending at the time of the entry.
- Owner validation of this work later showed NtWriteFile and several x87/TB lookup hotspots disappearing; PIO out and BQL wait time fell materially.

### Session 79 — KVMVAPIC, same-page SMC, and O(1) jump-cache flush
- Root causes included XP HAL repeatedly issuing TCG-ineffective `out 0x7e`, writable VAPIC data sharing an executable ROM page and triggering SMC/TB churn, and linear jump-cache flushes on invalidation.
- Patched the TCG VAPIC ROM handlers to NOP the relevant 0x7e writes, relocated writable VAPIC data to a separate page, and changed jump-cache flush to a generation increment (O(1)). Refactored the TB lookup hit path into a zero-frame leaf fast path.
- Standalone ROM, invalidation/linearizability, and SysV/Windows x64 code-generation checks passed.
- Owner validation at commit `759cee6` reported F1 Challenge at 32.7 FPS average (steady 30–37 FPS) and CI green on Linux and Windows. Owner considered F1, Joint Operations, and Hitman 2 playable; the primary profiling target shifted to FIFA 2003.

### Session 80 — FIFA/VMVGA and FlatView BQL stalls
- FIFA 2003 profiling found BQL starvation: `vmvga.fifo_run` + `vmvga.display_update` held the BQL for 24.90 s (58.3% of a 42.741 s capture). Five hotspots accounted for about 20.6 s: redundant CPU shadow clears before successful GPU clears, 16->32-bit present conversion divisions, FlatView topology rebuild overhead, dynamic-buffer overlap locks on a >64 KiB vertex buffer, and an accidentally unconditional readback trace path.
- Implemented: skip CPU shadow clear when native GPU clear succeeds (retain fallback); bit-exact SSE2 and division-free scalar 16->32 conversion; gate deep readback tracing correctly; raise overlapping-buffer full-shadow DISCARD threshold from 64 KiB to 256 KiB; replace the GLib GSequence FlatView builder with ping-pong radix sorting plus a sweep-line/min-heap renderer; skip no-op GART unmap transactions.
- Standalone checks passed, including exhaustive 65,536-value pixel conversion, clear-path equivalence, and randomized FlatView equivalence against the legacy renderer.
- At the time Session 80 was appended, full Linux/Windows CI and in-game FIFA/F1 validation were marked pending. The owner subsequently reported both Linux and Windows CI passing. A separate 23.526 s dry-weather capture was also shared: `cpu.exec` count 3,415, total 23.515560 s, max 19.554 ms, 3,164 intervals >1 ms, 1,064 >10 ms, and none >100 ms. The capture's full workload attribution is not reproduced here; consult its diagnostic log before drawing conclusions.

### Session 81 — Skip binary artifacts for Markdown-only changes
- Added a workflow change-detection job for pushes and pull requests. If every changed path ends in `.md`, Linux and Windows builds and functional checks still run, but package creation, packaged-layout smoke tests, and binary artifact uploads are skipped.
- Mixed/code changes retain normal artifact production; manual `workflow_dispatch` always produces artifacts. Missing comparison commits or diff errors conservatively enable artifact production.
- Updated `CLAUDE.md` and `HANDOFF.md`. Static review confirmed both build jobs consume the detection output and all six artifact-only steps are guarded. CI validation is pending; no local QEMU build or CI polling was performed.

### Session 82 — Make coding-agent historical reads selective
- Changed fresh-session onboarding to read the current handoff/task/recent-summary documents and `WORKLOG_INDEX.md`, not the entire append-only `WORKLOG.md` by default.
- Added a topic-to-session index so agents can retrieve only historical evidence relevant to the active task. Updated `CLAUDE.md`, `HANDOFF.md`, and `README.md` to use the same policy.
- The full Session 71 record remains in `WORKLOG.md`; it only aged out of the ten-entry rolling summary when Session 81 was added.
- Documentation-only changes; no QEMU build, tests, or CI run performed.

## Current investigation / next actions

1. Treat Session 80's implementation as present at the observed `main` HEAD, but verify the current branch and status hook before making further changes.
2. Obtain the complete owner capture and compare the relevant VMVGA/FlatView/DXVK counters against the Session 80 baseline. Do not infer a performance win from standalone microbenchmarks or CI alone.
3. Validate FIFA 2003 and F1 Challenge '99-'02 visually and with comparable captures. Keep workload, scene, weather, resolution, and capture duration consistent for A/B comparisons.
4. If a regression appears, isolate the responsible Session 80 change with targeted evidence; avoid broad reverts.
5. Update `TASKS.md`, append the next `WORKLOG.md` session, then roll this file forward to the latest ten worklog sessions.

## Key diagnostic interpretation rules

- Performance counters can be nested. Do not add child phase totals to enclosing command/FIFO totals.
- Counters whose implementation uses `qemu_perf_stats_mark()` measure intervals between marks (event cadence), not operation execution time.
- Overlapping lock dimensions (aggregate, vertex/index, full/partial, flags, size buckets) are not additive.
- A green CI run proves build/test coverage only; it does not prove XP guest gameplay or visual correctness.
- Keep the exact same repeatable owner workload for performance comparisons. Distinguish measured facts, owner observations, hypotheses, and verified fixes.
