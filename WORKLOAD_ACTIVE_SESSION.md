# Y2KBox active session context

> Rolling working summary derived from `WORKLOG.md`. This is a navigation and continuity aid, not a replacement for the append-only historical record. `TASKS.md` remains authoritative for task status; `HANDOFF.md` remains authoritative for project decisions and operating instructions. The ten sessions below are the latest ten entries currently recorded in `WORKLOG.md` (Sessions 121–130).

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
- Diagnostic captures are independent sessions: every Ctrl+Alt+L start resets counters and writes a new timestamped capture in the launch working directory; simple mode stays as a manually stopped `.log`, while full mode auto-stops at two seconds (or can be stopped early) and is compressed to `.log.gz` after stderr is restored.

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

### Session 128 — Expand full-mode VMVGA graphics-event diagnostics
- Full capture now emits every FIFO command with a capture-local sequence number and command metadata; adds operands for legacy 2D surface copy/fill and rectangle copy.
- Full mode also enables deep D3D9 draw planning/submission and render-target/texture state traces previously disabled by compile-time developer switches. Simple mode and inactive/default-off behavior remain unchanged.
- Static source review only; no local build, XP capture, runtime test, or CI. Next owner step: one full-mode NFSU2 capture spanning visible minimap flicker. T19 remains IMPLEMENTED pending XP validation and Linux/Windows CI.

### Session 129 — Add full-mode D3D9 resource and render correlation records
- T26 is IN PROGRESS. Full mode now logs each deferred guest buffer range and merged range (`VMVGA-D3D9-BUFFER-DEFER`), plus direct full-buffer DISCARD and deferred native range Lock decisions (`VMVGA-D3D9-BUFFER-UPLOAD`), with FIFO sequence, ranges, flags/mode/reason, flush cause and copied bytes.
- Guest-backed image/buffer updates emit subresource, box, row/plane layout, byte count and backend after MOB data reaches the CPU shadow (`VMVGA-RESOURCE-UPDATE`). Each D3D9 draw range emits FIFO-sequenced target/viewport/primitive/index/stream metadata (`VMVGA-D3D9-GEOMETRY`). Full-only `VMVGA-RENDER-CORRELATION` records connect readback events to the last observed FIFO sequence.
- Position/UV bounds are explicitly unavailable pending safe CPU-shadow vertex decoding. No GPU readback or forced synchronization was added. Static review only; no build, runtime test, XP capture, or CI.

### Session 130 — Bound and gzip full diagnostic captures
- Full-mode captures automatically stop after two seconds, with Ctrl+Alt+L still available for an earlier stop. The SDL title advertises the time limit while full capture is active.
- After stderr is restored, full logs are compressed asynchronously in a background worker to `.log.gz`; the original `.log` is deleted only after successful compression. Failures preserve the original. Simple captures remain manually stopped and uncompressed.
- Static source review only; no local build, runtime test, owner capture, or CI.

## Current investigation / next actions

1. **T13e-F1 / Windows TCG profiling remains active.** Keep workload and scene identical when comparing captures; distinguish measured facts from attribution hypotheses.
2. **Hitman 2: Silent Assassin D3D8 visual issue remains open.** Analyze owner captures and current VMVGA/D3D8 path; do not infer visual correctness from CI.
3. **T19 diagnostic capture modes are IMPLEMENTED; owner capture contents are verified, CI/repeated-interval acceptance remains pending.** `-y2kboxdebugmode off|simple|on` selects disabled, compact sampled capture, or full selected per-event VMVGA logging (default: `off`). Ctrl+Alt+L captures the selected mode, with interval-scoped flight-recorder summaries and retained per-command count/timing records. Sessions 125–126 corrected false unknown-FIFO, register-write, and register-read warnings. On 2026-11-24, the owner verified a 48 KB simple capture (734 supersede flips, two sampled event lines, SID counts 367/367) and a 1.5 MB full capture (766 supersede flips, all 766 individual lines); both had the expected mode header and neither showed the false `SVGA_REG_BUSY` unknown-read warning. Session 126's owner-retest follow-up is closed. Linux/Windows CI and repeated-interval acceptance remain outstanding. Repeated genuine-warning aggregation remains a separate possible follow-up, not implemented. **T26 full-mode resource diagnostics are IN PROGRESS:** deferred/native D3D9 buffer updates, guest-backed resource updates, compact draw-range metadata and readback/FIFO correlation are emitted only in full mode. Sessions 133–134 add CPU-shadow raw vertex-input position and TEXCOORD0 UV bounds for FLOAT1–FLOAT4 indexed and non-indexed draws, plus guest-PRESENT frame IDs and per-draw effective D3D9 state snapshots with bound texture metadata; one owner full-mode NFSU2 capture and Linux/Windows build/CI validation remain outstanding. Full-mode capture duration is now bounded to two seconds and auto-compressed asynchronously at zlib level 9; owner verification and Linux/Windows CI for Sessions 130–133 remain outstanding. Simple-mode PERFSTATS may be sufficient for some performance investigations before requesting full event detail.
4. **T25 guest OpenGL investigation is PAUSED (low feasibility; awaiting independent reassessment).** Native XP GDI Generic exposes OpenGL 1.1.0 with three extensions; COD1 with software GL creates/makes-current a context but fails its feature check. VMware documents software-emulated OpenGL in XP guests. QindieGL and TitaniumGL remain unproven leads with unresolved compatibility/quality limitations. Do not prototype, modify the guest/driver, build, or test while T25 remains paused.
5. Keep the F1 crowded-grid, FIFA residual hitches, and Hitman 2 D3D8 rendering issue as distinct investigations unless evidence connects them. Do not reopen the resolved general FlatView scene-transition stall without new evidence.
6. **T24 fixed-size SDL display is DONE.** The owner confirmed Linux/Windows SDL CI and all XP acceptance checks: guest-driven resolution changes, Ctrl+Alt+F fullscreen toggle, and mouse tracking. The Windows window is decorated and non-resizable with titlebar maximize disabled.
7. Update the selected task entry and append the next worklog session when new implementation, measurements, or verified conclusions are produced.


### Session 131 — Shorten full diagnostic captures and maximize gzip compression
- Reduced the full-mode automatic capture limit from five seconds to two seconds; manual early stop and the simple-mode behavior are unchanged.
- Moved gzip compression to a detached background worker while retaining zlib level 9 (`wb9`, maximum compression). The worker only receives the completed log after stderr is restored; it removes the source only after successful compression. The worker avoids writing status messages to redirected stderr, so it cannot contaminate a newer capture. It deletes the source `.log` only after successful compression and preserves it on failure.
- Updated README, HANDOFF, TASKS, command-line help, and capture documentation to describe the two-second limit, maximum compression, and the guidance to start performance investigations with Simple-mode PERFSTATS/`Y2KBOX-PERF` summaries, which may be sufficient before enabling Full mode.
- Static source/documentation review only. No local build, runtime test, XP capture, or CI was run; owner verification and Linux/Windows CI remain outstanding.


### Session 133 — Decode CPU-shadow vertex positions and UVs
- Full-mode `VMVGA-D3D9-GEOMETRY` now decodes raw vertex-input position and TEXCOORD0 UV bounds from the CPU shadow for indexed and non-indexed draws using FLOAT1–FLOAT4 declaration elements. The decoder validates little-endian indices and float components, base-vertex addressing, stream offsets/strides, buffer bounds and finite values; it rejects instanced streams and caps work at 65,536 vertex references per draw.
- Records report separate position/UV reference counts, `bounds_ok`, and an explicit `bounds_reason`. No GPU readback, forced synchronization, or rendering behavior change was added.
- Static source review only; no QEMU build, runtime test, owner XP capture, or CI. Next owner step: take one Full-mode NFSU2 capture spanning the minimap flicker and inspect the newly decoded bounds alongside surface updates and render/readback correlation. T26 remains IN PROGRESS pending useful capture evidence and Linux/Windows build/CI.

### Session 134 — Add guest frame IDs and effective D3D9 draw-state snapshots
- Added capture-local `VMVGA-FRAME` boundaries for guest `SVGA_3D_CMD_PRESENT` commands. Each boundary records the frame being closed and advances the ID for subsequent draws; it is explicitly not a host SDL/vblank frame identifier.
- Added `VMVGA-D3D9-STATE-SNAPSHOT` once per validated draw, recording context, render targets, viewport/scissor, shader bindings, all valid texture/sampler and render states, and bound texture SIDs with live dimensions, format and mip count. Each snapshot carries both `frame` and the existing event/FIFO correlation `seq`.
- Corrected the SDL title to advertise the already-enforced two-second Full-mode limit.
- No PNG/JPEG snapshot was added: no existing encoder was found in the current SDL/GLib/zlib path, and no new dependency was introduced.
- Static source/documentation review only. No local QEMU build, runtime test, owner XP capture, or CI was run. Next: inspect one two-second NFSU2 Full capture and validate the frame/state/resource correlations; Linux/Windows CI remains required. T26 stays IN PROGRESS.
