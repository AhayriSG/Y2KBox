# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 115–124).

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


### Session 113 — Clarify T12 as a user-managed instance manager
- T12 now describes a launcher/instance manager where users create and name independent XP VM instances and select virtual hardware, including the virtual GPU, per instance.
- Each instance has independently persisted hardware configuration and its own system disk/guest installation. No hard-coded vendor presets or two bundled XP images are implied.
- VMVGA/SVGA3D remains the supported path. VBoxVGA 5.2.x/SharedOpenGL is only a future research candidate, gated on an end-to-end proof of concept and licensing/source audit; it is not selectable today.
- Documentation-only; no source changes, builds, guest tests, benchmarks, or CI polling.

### Session 114 — Complete T24 owner acceptance verification
- The owner confirmed Linux and Windows SDL CI pass, and XP guest-driven resolution changes, Ctrl+Alt+F fullscreen toggling, and mouse tracking all work.
- T24 is now DONE/green. TASKS.md records the complete acceptance set and no outstanding checks.
- Documentation-only; no source changes, builds, guest tests, or CI polling. The verification results are the owner's confirmation.

### Session 115 — T13e / T13e-F1: ScreenTarget flip supersession, dynamic VB ring sub-allocation, and O(1) GART unparent + FIFO txn batching
- Recorded owner post-Session 80 Windows TCG validation captures: FIFA 2003 improved to ~85.7 FPS (`25.352 s` / `2,172` flips, playable, slow-motions gone), Hitman 2 Sicily fade showed `2,913` unpresented sync ScreenTarget readbacks (`2.410 s` = 60.3% wall time) at ~767 FPS, and F1 '99-'02 spent `3.711 s` (68% of `fifo_run`) in `71,979` `INVALIDATE` `D3DLOCK_DISCARD` VB renames.
- Implemented three targeted VMVGA/DXVK/GART fixes on `main`: (1) steady-state FIFO full-screen D3D9 flip supersession (`vmsvga3d_screen_target_try_supersede_flip_live`) + eager staging copy in `retire_latest`; (2) per-surface ring sub-allocation for dynamic D3D9 vertex buffers (`4 MiB` lazy upgrade, `32 MiB` global extra budget, `SetStreamSource` `d3d9_ring_base` offset) + guarded `<= 2 MiB` overlap-`DISCARD`; (3) O(1) `"vmsvga-gart-%u[0]"` QOM property deletion + FIFO-pass GART `memory_region_transaction` batching (flushed before fences, disable, and FIFO exit).
- Verified locally with `git diff --check`, preprocessor/header checks, and standalone `gcc -Werror` logic tests (no local `meson`/`mingw32-gcc`). Linux/Windows CI and owner Windows TCG in-game re-captures are pending.

### Session 116 — Reorganize WORKLOG_INDEX for scanability and coverage
- Updated the topic map to include recent T24 acceptance work and Session 115's T13e / VMVGA / DXVK performance changes, including dedicated Hitman 2 coverage.
- Replaced the dense Sessions 82–115 prose block with a concise chronological list; detailed evidence remains in WORKLOG.md.
- Documentation-only; no source changes, builds, tests, or CI polling.


### Session 117 — Fix Session 115 D3D9 dynamic vertex-buffer compile failure
- Linux and Windows CI both caught an undefined `VMSVGA3D_DXVK_D3DUSAGE_DYNAMIC` used by the new dynamic D3D9 vertex-buffer ring code.
- Added the missing Direct3D 9 usage flag definition (`0x00000200u`) beside the other D3D9 usage constants. The D3D9 checks remain distinct from the unrelated D3D11 `D3D11_USAGE_DYNAMIC` enum.
- Minimal compile-only correction; no Session 115 ring behavior changed. No local QEMU build was run; GitHub Actions validation is pending and was not polled.

## Current investigation / next actions

1. **T13e-F1 / Windows TCG profiling remains active.** The owner supplied a dry-weather capture on 2026-11-23 with `capture_ms=23526`; `cpu.exec` reported 3,415 intervals, 23.515560 s total, 19.554 ms max, 3,164 intervals over 1 ms, 1,064 over 10 ms, and none over 100 ms. This is an interval summary, not workload attribution; inspect the complete capture and compare only equivalent scenes/settings before proposing code changes.
2. **Hitman 2: Silent Assassin D3D8 visual issue remains open.** The owner supplied a sub-second menu capture while investigating static-background rendering. Analyze that capture and current VMVGA/D3D8 path; do not claim a fix or infer visual correctness from CI.
3. **T19 diagnostic capture modes are IMPLEMENTED; owner acceptance remains pending.** `-y2kboxdebugmode off|simple|on` selects disabled, compact sampled capture, or full selected per-event VMVGA logging (default: `off`). Ctrl+Alt+L captures the selected mode, with interval-scoped flight-recorder summaries and retained per-command count/timing records. Verify all three levels, log sizes/content, and repeated intervals in XP; the existing VB-ring pressure summaries remain part of capture stop output.
4. **T25 guest OpenGL investigation is PAUSED (low feasibility; awaiting independent reassessment).** Native XP GDI Generic exposes OpenGL 1.1.0 with three extensions; COD1 with software GL creates/makes-current a context but fails its feature check (HANDOFF 8d / Session 104). VMware documents software-emulated OpenGL in XP guests (8e / 105), and conventional XP ICD discovery requires the active display driver's OPENGL_GETINFO escape (8f / 106). QindieGL is a concrete but unproven app-local GL-to-D3D9 lead: its source lacks `wglGetExtensionsStringEXT`, and a historical minimal-entry-point experiment reportedly reached COD1 startup with severe graphics corruption (8h / 108). TitaniumGL is a separate lead: the owner observed COD1 loading its DLL, creating/making-current an OpenGL 1.4 context, and seeing numerous extensions plus renderer `VMware SVGA II/119MB VRAM`; COD1 still rejects the card, with the same broad result in hardware and software modes. The `0 PFDs found` line and failed capability check are unresolved (8i / 109). Do not prototype, modify the guest/driver, build, or test while T25 remains paused. See HANDOFF 8g–8i / Sessions 107–109.
5. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
6. **T24 fixed-size SDL display is DONE.** The owner confirms Linux and Windows SDL CI pass and all XP acceptance checks are verified: guest-driven resolution changes, `Ctrl+Alt+F` fullscreen toggling, and mouse tracking. The Windows window is decorated, non-resizable, and has its titlebar maximize control disabled. No further T24 work is pending absent a new reproducible defect.
7. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.


### Session 118 — Define narrow VB-ring profiling handoff
- T19 remains PAUSED: static wiring review confirms the Ctrl+Alt+L capture controls existing performance counters, VMVGA/VMPort diagnostic gates, relevant trace events, and the Windows TCG vCPU profiler, but actual capture-content completeness still requires owner XP verification.
- Added a focused Gemini 4 Neon follow-up for one capture-stop `Y2KBOX-PROF subsystem=vmvga.vb_ring` record per eligible dynamic D3D9 VB surface, with interval-only counters for capacity, allocation/high-water, NOOVERWRITE reuse, DISCARD/wrap, overlap discard, and allocation failures.
- No source changes or tests. Preserve Session 115 allocator behavior; avoid per-upload logging and broader profiling expansion.

### Session 119 — Implement capture-interval VB-ring pressure profiling
- Added per-surface capture-interval counters and a DXVK surface registry; Ctrl+Alt+L start resets profiling-only state and stop emits one summary per eligible live dynamic D3D9 VB surface. New surfaces inherit active capture state.
- Defined capacity, cumulative uploaded bytes, NOOVERWRITE reused bytes, successful DISCARDs, ring-end wraps, absolute host-buffer high-water, overlap-fallback DISCARDs, and failed ring upgrade/NOOVERWRITE operations.
- No allocator policy changes, per-upload logging, hot-path allocations, or locks. Source changes are statically reviewed only; no build or runtime test. T19 remains PAUSED pending CI and owner XP capture/repeated-interval verification.

### Session 120 — Complete VB-ring profile lifecycle accounting
- Included successful full-buffer DISCARD uploads as well as range uploads; narrowed wrap counts to actual capacity exhaustion and kept overlap/invalidation DISCARDs distinct.
- Made the per-surface registry follow renderer teardown/recreation so active capture state and interval counters survive renderer replacement.
- Static source-path checks only; no QEMU build/runtime test. T19 remains PAUSED pending CI and owner XP capture/repeated-interval verification.

### Session 121 — Preserve observed VB-ring capacity on failed appends
- Record a successful host-ring upgrade in `capacity_bytes` immediately, even if the following NOOVERWRITE lock fails.
- No allocator behavior change. Static inspection only; no build/runtime test. T19 remains PAUSED pending CI and owner XP capture verification.

## Key diagnostic interpretation rules

- Performance counters can be nested. Do not add child phase totals to enclosing command/FIFO totals.
- Counters whose implementation uses `qemu_perf_stats_mark()` measure intervals between marks (event cadence), not operation execution time.
- Overlapping lock dimensions (aggregate, vertex/index, full/partial, flags, size buckets) are not additive.
- A green CI run proves build/test coverage only; it does not prove XP guest gameplay or visual correctness.
- Keep the exact same repeatable owner workload for performance comparisons. Distinguish measured facts, owner observations, hypotheses, and verified fixes.


### Session 122 — Compact and detailed VMVGA diagnostic capture modes
- Default Ctrl+Alt+L summary mode samples repetitive SYNC/fence/FIFO-run-end/3D-command events instead of printing every occurrence; existing stalls, failures, state changes, and fence-goal events remain visible.
- Added a full selected per-event VMVGA output mode alongside compact sampled capture. Both modes retain per-command performance counts/timing.
- Reset VMVGA flight-recorder counters at capture start and emit the interval's final flight summary/FIFO histogram at stop. SDL title and capture header identify the mode.
- Static review only; no build, CI, or owner XP verification. T19 is IMPLEMENTED pending CI and owner validation.


### Session 123 — One global Y2KBox diagnostic-level flag
- `-y2kboxdebugmode off|simple|on`: off disables diagnostics, simple uses compact sampled capture, on restores full selected per-event logging. Default remains off.
- Removed the user-facing VMVGA `-global` instruction. The parser maps the Y2KBox-wide setting internally; SDL title and capture header identify simple/full mode.
- Static review only; no build, runtime test, or CI run. Owner verification of all three values and both capture outputs remains pending.

### Session 124 — Preserve explicit VMVGA warnings in simple captures
- Routed identified invalid/unknown/rejected legacy FIFO and register cases, cursor payload/allocation failures, and invalid GMR2/Screen inputs through capture-gated `VMVGA-WARN` records; added equivalent warnings for unknown SVGA3D command IDs/metadata.
- Existing stall, fence-goal, and abnormal SYNC/FIFO records remain independently visible. Routine repetitive events stay sampled.
- Targeted coverage only: no universal severity classifier or repeated-warning aggregation yet. Static review only; no build/runtime/CI/XP verification.
