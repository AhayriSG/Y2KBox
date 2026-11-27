# WORKLOG index

This is a navigation aid for the append-only `WORKLOG.md`, not a second source
of truth. Use it to find historical evidence only when it is relevant to the
current task. Session numbers refer to headings in `WORKLOG.md`; some sessions
have follow-up subsections under the same number.

## How to use

1. Establish the current state from `AGENTS.md`, HANDOFF sections 0-3, the task
   overview plus selected task entry in `TASKS.md`, and `WORKLOAD_ACTIVE_SESSION.md`.
2. Find the relevant topic below and open only the corresponding session section(s)
   in `WORKLOG.md`.
3. Treat old entries as historical evidence. Verify current behavior against
   current source, tests, task status, and handoff decisions before relying on it.
4. If a topic is not listed, search `WORKLOG.md` for a distinctive term or inspect
   its session headings. Add a topic mapping when a new area of work becomes
   substantial; do not copy session summaries into this index.
5. Never remove or rewrite historical entries in `WORKLOG.md`. The separate
   `WORKLOAD_ACTIVE_SESSION.md` remains a rolling summary of the latest ten
   sessions; entries aging out of that summary remain in the worklog.

## Topic map

| Topic | Relevant worklog sessions | Use these when investigating |
|---|---:|---|
| Repository bootstrap, pruning, fixed XP platform and build configuration | 1, 10–11, 13, 16–20, 28–29 | Original project decisions, removed QEMU subsystems, Linux KVM / Windows TCG split, build dependencies and platform-specific regressions |
| TCG performance, x87, XMM/SSE and benchmark methodology | 2–9, 19–28, 68, 73–80, 115, 141–143, 145–148 | T04/T13 benchmark setup, x87 fast paths, XMM caching, TCG helper costs, differential testing and measured performance changes |
| VMware SVGA / SVGA3D, DXVK, D3D and guest OpenGL | 12, 29–32, 65–67, 80, 99–109, 115 | VMVGA integration, display/scanout issues, D3D9/D3D8 translation, shader/state compatibility, rendering regressions, guest OpenGL research, and Session 115's ScreenTarget/DXVK changes |
| SDL windowing, fullscreen, input and diagnostic capture | 33–36, 60–64, 110, 114, 122–130 | Capture hotkey/file lifecycle, debug-mode controls, guest-driven window sizing, fullscreen behavior, mouse tracking and T24 acceptance verification |
| Launcher architecture and user-managed VM instances | 112–113 | T12's planned SDL2/Dear ImGui instance manager, independently persisted per-instance virtual hardware and GPU choice, and the conditional VBoxVGA 5.2.x / SharedOpenGL research path |
| VirtIO Block / Net and supported guest devices | 37–44 | VirtIO source restoration, Linux host-backend build gates, device defaults and PS/2 pointing-device policy |
| Scene-stall profiling, VMVGA FIFO, GART and FlatView | 45–59, 68–80, 115 | T13e investigation history, instrumentation semantics, GART and memory transactions, FlatView algorithms, BQL contention, ScreenTarget readback stalls and performance hypotheses |
| D3D9 dynamic-buffer uploads, DISCARD / NOOVERWRITE and lock profiling | 68–76, 80, 115, 129 | Buffer invalidation, in-flight range tracking, lock-mode selection, deferred uploads, ring sub-allocation, merge/flush cadence and measured lock improvements |
| Full-mode resource/update/draw correlation diagnostics | 128–130, 136 | FIFO sequence correlation, guest-backed image updates, D3D9 buffer upload decisions, draw-range metadata, and readback event linkage for visual investigations |
| F1 Challenge performance investigation | 45–47, 68–79, 115 | Crowded-grid slowdown, vCPU profiling, x87/TCG hotspots, owner validation and Session 115's dynamic vertex-buffer DISCARD investigation |
| FIFA 2003 performance investigation | 45, 48–59, 80, 115 | Scene-transition stalls, VMVGA/DXVK timing, memory topology costs, Session 80 optimizations and Session 115 validation/fixes |
| Hitman 2: Silent Assassin performance and rendering | 115 | Sicily fade-in/out ScreenTarget readback stalls and dynamic vertex-buffer upload behavior |
| CI workflow, documentation and session-continuity automation | 35–36, 81–98, 111, 114–115 | Markdown-only CI job skipping, shared agent instructions, selective context loading, session-continuity automation, task-status markers, T24 verification record and performance-change verification notes |

## Session 81 note

Session 81 initially skipped packaging, packaged smoke tests, and artifact uploads
for Markdown-only commits while still running Linux/Windows builds and functional
checks. The later job-level condition in commit `8ab36ee` skips both platform jobs
entirely for Markdown-only commits; only the lightweight detector runs. Any
non-Markdown change and manual `workflow_dispatch` run the full platform jobs.
See `.github/workflows/build.yml`.

## Sessions 82–115

The entries below are a chronological navigation summary, not a replacement for
the detailed records. Open the matching heading in `WORKLOG.md` for full context.

- **Session 82 — Selective historical reads and topic index:** Introduced the
  worklog navigation model and selective historical lookup.
- **Session 83 — Shared agent instructions:** Centralized common operating policy
  in `AGENTS.md`.
- **Session 84 — Multi-agent handoff and XP acceptance:** Clarified sequential
  agent handoff and moved XP guest acceptance to an owner-run checklist.
- **Session 85 — Local build authorization:** Added the owner-approved Gemini 4
  Neon-only exception for ordinary local QEMU builds.
- **Session 86 — Benchmark authorization:** Extended that model-specific
  authorization to relevant, bounded benchmark/A-B work.
- **Session 87 — Fixed guest target constraints:** Clarified guest RAM rationale,
  XP x86 virtual/physical memory limits, and Direct3D 9.0c / Shader Model 3.0 ceiling.
- **Session 88 — README FAQ organization:** Moved host requirements and RAM
  rationale into the FAQ while keeping the Fixed target summary guest-focused.
- **Session 89 — README project introduction:** Emphasized the XP gaming target,
  mid-to-late-2000s performance goal, and distinction from cycle-accurate emulation.
- **Session 90 — One-vCPU target:** Fixed the product target at one vCPU, removed
  QEMU's `-smp` option, and aligned T01, T14, README and HANDOFF.
- **Session 91 — Project-purpose FAQ:** Distinguished Y2KBox from compatibility
  layers, explained the one-vCPU/TCG trade-off, and separated the proprietary
  hypervisor API rationale.
- **Session 92 — T09 closure:** Closed integration/CI work while retaining XP
  driver, display, D3D API and game acceptance in the manual checklist; T10 stayed paused.
- **Session 93 — QEMU provenance:** Moved the upstream QEMU 11.1.90 base commit
  and unchanged GPL-2.0 notice into a dedicated README FAQ entry.
- **Session 94 — Documentation and policy audit:** Reconciled agent skill
  authorization, CI hook wording, serialized writes and model-specific permissions.
- **Session 95 — CI status script:** Fixed optional GitHub PAT Authorization-header
  handling in `scripts/session_status.sh`.
- **Session 96 — Diagnostic logging FAQ:** Documented opt-in logging, recording
  overhead, tester profiling workflow and optional VMVGA deep trace.
- **Session 97 — Preservation rationale merge:** Merged the preservation-practicality
  rationale into the compatibility-layer FAQ.
- **Session 98 — README FAQ ordering:** Moved the merged project-purpose answer
  to the final FAQ position.
- **Session 99 — Guest OpenGL investigation:** Added T25 for investigating XP
  guest OpenGL over VMware SVGA3D, with OpenGL 1.1 baseline and 2.1 maximum.
- **Session 100 — OpenGL architecture research:** Clarified Mesa's SVGA3D command
  path, existing legacy-command overlap and limits of the available QEMU log.
- **Session 101 — OpenGL target wording:** Clarified the README's OpenGL 1.1
  baseline / 2.1 maximum target; guest OpenGL remained unimplemented.
- **Session 102 — COD1 WGL probe:** Traced `GLW_ChoosePFD` behavior and added
  an XP-side probe matching the game's pixel-format filters and best-match rules.
- **Session 103 — VGPU9 requirements:** Compared Mesa's VGPU9 baseline with the
  advertised profile and added a COD1 software-OpenGL test path.
- **Session 104 — XP OpenGL baseline:** Recorded GDI Generic OpenGL 1.1.0 and
  COD1's successful context creation followed by its feature-check failure.
- **Session 105 — VMware XP OpenGL limitation:** Established that VMware documents
  software-emulated OpenGL in XP guests; WINVBE was noted as a software-only experiment.
- **Session 106 — XP ICD discovery:** Documented the active display driver's
  `OPENGL_GETINFO` requirement and speculative application-local wrapper alternatives.
- **Session 107 — Pause T25:** Paused guest OpenGL work for independent reassessment
  and characterized the objective as low probability.
- **Session 108 — QindieGL lead:** Recorded its missing `wglGetExtensionsStringEXT`
  implementation, COD1 startup failure, historical corruption and GPL-3.0 concern.
- **Session 109 — TitaniumGL lead:** Recorded COD1's OpenGL 1.4 context and
  extension observations, unresolved `0 PFDs found` output and continued rejection.
- **Session 110 — T24 implementation status:** Recorded the verified Windows
  non-resizable window behavior; at that point, CI and remaining XP acceptance
  checks had not yet been confirmed.
- **Session 111 — Visual task-status markers:** Added red/yellow/green heading
  markers while keeping the plain-text Status field authoritative; updated
  `AGENTS.md` with the parsing rule.
- **Session 112 — Initial T12 graphics-profile proposal:** Proposed separate
  VMVGA/SVGA3D and experimental VBoxVGA/SharedOpenGL graphics profiles.
- **Session 113 — T12 instance-manager correction:** Reframed T12 as user-created,
  independently configured XP instances rather than application-defined presets.
- **Session 114 — T24 acceptance completion:** The owner confirmed Linux/Windows
  SDL CI and all XP checks (guest-driven resolution, Ctrl+Alt+F fullscreen and
  mouse tracking); T24 was changed to DONE/green.
- **Session 115 — T13e performance work:** Recorded owner Windows TCG captures
  for FIFA 2003, Hitman 2 and F1, then implemented ScreenTarget flip supersession,
  dynamic D3D9 vertex-buffer ring sub-allocation and O(1) GART alias removal /
  FIFO-pass transaction batching. Standalone local checks passed; Linux/Windows
  CI and owner in-game re-captures remained pending at the time of the entry.

See Sessions 82–115 in `WORKLOG.md` for the detailed chronology and evidence.


## Session 122

- **Session 122 — Compact and detailed VMVGA diagnostic capture:** default
  summary-mode event sampling, opt-in full selected per-event VMVGA tracing,
  capture-interval flight-recorder reset/final summary, and per-command
  performance count/timing retention. T19 remains IMPLEMENTED pending CI and
  owner XP verification.


## Session 123

- **Session 123 — One global Y2KBox diagnostic-level flag:** changed the
  command-line interface to `-y2kboxdebugmode off|simple|on`, with off as
  default, simple for compact sampled captures, and on for full selected
  per-event VMVGA logging. Removed user-facing VMVGA global-property syntax.


## Session 124

- **Session 124 — Preserve explicit VMVGA warnings in simple captures:** route
  identified invalid/unknown/rejected FIFO, register, cursor, GMR2 and Screen
  conditions through capture-gated warning records; add SVGA3D unknown-command
  warnings. Targeted static review only; no build/runtime/CI/XP verification.


## Session 125

- **Session 125 — Correct VMVGA warning placement after owner capture flood:** move
  generic FIFO warnings after command dispatch rejects an unhandled non-3D
  command, and unknown-register warnings into the register switch's `default`
  case. The owner capture showed valid SYNC writes and handled FIFO commands
  being mislabeled as unknown. Static review only; no build/runtime/CI/XP
  verification.


## Session 126

- **Session 126 — Bound superseded-flip logging and fix BUSY read classification:**
  move unknown-register-read warnings after dispatch; keep every D3D9 supersede-flip
  event in full mode while simple mode samples two and emits interval/per-SID counts plus an overflow-event count.
  Static source review only; no build/runtime/CI/XP verification.


## Session 128
- **Session 128 — Expand full-mode VMVGA graphics-event diagnostics:** enable
  per-command FIFO records with sequence numbers, add legacy 2D operation
  operands, and activate deep D3D9 draw/state traces in full capture only.

## Session 127

- **Session 127 — Owner verification of diagnostic capture modes:** recorded the Windows XP simple/full capture sizes, sampled-versus-full supersede event counts, mode headers, and absence of the false `SVGA_REG_BUSY` unknown-read warning. Session 126's owner-retest follow-up is closed; T19 remains IMPLEMENTED pending Linux/Windows CI and repeated-interval acceptance.


## Session 129
- **Session 129 — Full-mode D3D9 resource and render correlation:** added guest buffer defer/native upload records, guest-backed image/buffer update metadata, per-range D3D9 draw summaries and full-only readback/FIFO correlation. Position/UV bounds remain explicitly unavailable pending safe CPU-shadow decoding. T26 is IN PROGRESS; static review only, no build/runtime/XP/CI verification.


## Session 130
- **Session 130 — Bound and gzip full diagnostic captures:** full-mode Ctrl+Alt+L
  capture automatically stops after five seconds (or manually earlier), then
  compresses the restored capture to `.log.gz` using zlib. Original text is
  retained if compression fails; simple-mode behavior is unchanged. Static
  source review only; no build/runtime/XP/CI verification.
