# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 117–126).

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

### Session 118 — Define narrow VB-ring profiling handoff
- Defined a focused capture-stop per-surface dynamic D3D9 VB-ring profile and its interval-only counters.
- Documentation-only handoff; no source/build/runtime validation.

### Session 119 — Add capture-interval dynamic VB ring pressure profiling
- Added per-surface profiling-only counters and a surface registry; capture start resets interval counters and stop emits one summary per eligible live dynamic D3D9 VB.
- Static source review only; no build/runtime test. Owner XP capture and CI verification pending.

### Session 120 — Complete VB-ring profile lifecycle accounting
- Included full-buffer and range uploads, distinguished ring wraps from other DISCARD causes, and preserved profiling state across renderer recreation.
- Static review only; no build/runtime test.

### Session 121 — Preserve observed VB-ring capacity on failed appends
- Record successful host-ring upgrades in `capacity_bytes` even if the following NOOVERWRITE operation fails.
- No allocator behavior change; static review only.

### Session 122 — Compact and detailed VMVGA diagnostic capture modes
- Added sampled simple capture and full selected per-event VMVGA output, with interval-scoped flight-recorder reset/final summary and per-command performance counts.
- Static review only; no build, CI, or owner XP verification.

### Session 123 — One global Y2KBox diagnostic-level flag
- Added `-y2kboxdebugmode off|simple|on`; off disables diagnostics, simple selects compact capture, and on selects full selected per-event logging. Default remains off.
- No build, runtime test, or CI run.

### Session 124 — Preserve explicit VMVGA warnings in simple captures
- Routed identified legacy invalid/unknown/rejected cases and SVGA3D unknown-command cases through capture-gated warning records.
- Targeted retention only; repeated-warning aggregation and a universal severity classifier were not implemented. Static review only.

### Session 125 — Correct VMVGA warning placement after owner capture flood
- The owner's one-second capture showed valid FIFO commands and the valid `SVGA_REG_SYNC` write being labelled unknown because warnings ran before dispatch.
- Generic FIFO warning now runs only after the legacy switch and SVGA3D dispatcher decline an unhandled non-3D command; unknown register warning now runs only in the register switch's `default` case.
- Updated T19 task/handoff documentation. Static review only; no build, runtime test, CI, or post-fix XP capture.

### Session 126 — Bound superseded-flip logging and fix BUSY read classification
- Confirmed register index 22 is the valid `SVGA_REG_BUSY`; moved unknown-read warnings into the read switch's `default` case.
- Simple mode now logs the first two D3D9 supersede-flip events and reports capture totals, first/last SID, a four-SID histogram, and overflow-event count for additional SIDs. Full mode keeps every event.
- This changes diagnostics only, not readback/flip behavior. Static source review only; no build/runtime test or CI.

### Session 127 — Record owner verification and close diagnostic logging follow-up
- Owner Windows XP captures verified simple mode (about 48 KB; 734 supersede flips, two individual lines, SID counts 367/367) and full mode (about 1.5 MB; 766 supersede flips and 766 individual lines).
- Both headers confirmed their selected modes, and the false unknown-read warning for valid `SVGA_REG_BUSY` was absent from both captures.
- Session 126's owner-retest follow-up is closed. T19 remains IMPLEMENTED pending Linux/Windows CI and repeated-interval acceptance; no local build or CI was performed.

## Current investigation / next actions

1. **T13e-F1 / Windows TCG profiling remains active.** Keep workload and scene identical when comparing captures; distinguish measured facts from attribution hypotheses.
2. **Hitman 2: Silent Assassin D3D8 visual issue remains open.** Analyze owner captures and current VMVGA/D3D8 path; do not infer visual correctness from CI.
3. **T19 diagnostic capture modes are IMPLEMENTED; owner capture contents are verified, CI/repeated-interval acceptance remains pending.** `-y2kboxdebugmode off|simple|on` selects disabled, compact sampled capture, or full selected per-event VMVGA logging (default: `off`). Ctrl+Alt+L captures the selected mode, with interval-scoped flight-recorder summaries and retained per-command count/timing records. Sessions 125–126 corrected false unknown-FIFO, register-write, and register-read warnings. On 2026-11-24, the owner verified a 48 KB simple capture (734 supersede flips, two sampled event lines, SID counts 367/367) and a 1.5 MB full capture (766 supersede flips, all 766 individual lines); both had the expected mode header and neither showed the false `SVGA_REG_BUSY` unknown-read warning. Session 126's owner-retest follow-up is closed. Linux/Windows CI and repeated-interval acceptance remain outstanding. Repeated genuine-warning aggregation remains a separate possible follow-up, not implemented.
4. **T25 guest OpenGL investigation is PAUSED (low feasibility; awaiting independent reassessment).** Native XP GDI Generic exposes OpenGL 1.1.0 with three extensions; COD1 with software GL creates/makes-current a context but fails its feature check. VMware documents software-emulated OpenGL in XP guests. QindieGL and TitaniumGL remain unproven leads with unresolved compatibility/quality limitations. Do not prototype, modify the guest/driver, build, or test while T25 remains paused.
5. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
6. **T24 fixed-size SDL display is DONE.** The owner confirmed Linux/Windows SDL CI and all XP acceptance checks: guest-driven resolution changes, Ctrl+Alt+F fullscreen toggle, and mouse tracking. The Windows window is decorated and non-resizable with titlebar maximize disabled.
7. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.
