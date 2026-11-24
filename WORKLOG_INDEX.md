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
| TCG performance, x87, XMM/SSE and benchmark methodology | 2–9, 19–28, 68, 73–80 | T04/T13 benchmark setup, x87 fast paths, XMM caching, TCG helper costs, differential testing and measured performance changes |
| VMware SVGA / SVGA3D, DXVK, D3D and guest OpenGL | 12, 29–32, 65–67, 80, 99–109 | VMVGA integration, display/scanout issues, D3D9/D3D8 translation, shader/state compatibility, rendering regressions, and the guest OpenGL/SVGA3D investigation, including QindieGL and TitaniumGL wrapper experiments (Sessions 99–109) |
| SDL windowing, fullscreen, input and diagnostic capture | 33–36, 60–64 | Capture hotkey/file lifecycle, debug-mode controls, guest-driven window sizing, fullscreen behavior and diagnostic logging |
| Launcher architecture and user-managed VM instances | 112–113 | T12's planned SDL2/Dear ImGui instance manager, independently persisted per-instance virtual hardware and GPU choice, and the conditional VBoxVGA 5.2.x / SharedOpenGL research path |
| VirtIO Block / Net and supported guest devices | 37–44 | VirtIO source restoration, Linux host-backend build gates, device defaults and PS/2 pointing-device policy |
| Scene-stall profiling, VMVGA FIFO, GART and FlatView | 45–59, 68–80 | T13e investigation history, instrumentation semantics, GART and memory transactions, FlatView algorithms, BQL contention and performance hypotheses |
| D3D9 dynamic-buffer uploads, DISCARD / NOOVERWRITE and lock profiling | 68–76, 80 | Buffer invalidation, in-flight range tracking, lock-mode selection, deferred uploads, merge/flush cadence and measured lock improvements |
| F1 Challenge performance investigation | 45–47, 68–79 | Crowded-grid slowdown, vCPU profiling, x87/TCG hotspots, owner validation and later shifts in bottlenecks |
| FIFA 2003 performance investigation | 45, 48–59, 80 | Scene-transition stalls, VMVGA/DXVK timing, memory topology costs and Session 80 optimizations |
| CI workflow, documentation and session-continuity automation | 35–36, 81–98 | Documentation changes, Markdown-only CI job skipping, shared agent instructions, selective context loading, session-continuity automation, and model-specific local-build authorization |

## Session 81 note

Session 81 initially skipped packaging, packaged smoke tests, and artifact uploads
for Markdown-only commits while still running Linux/Windows builds and functional
checks. The later job-level condition in commit `8ab36ee` skips both platform jobs
entirely for Markdown-only commits; only the lightweight detector runs. Any
non-Markdown change and manual `workflow_dispatch` run the full platform jobs.
See `.github/workflows/build.yml`.

## Sessions 82–114 note

Session 82 introduced selective historical reads and the topic index. Session 83
centralized shared instructions in `AGENTS.md`; Session 84 clarified sequential
multi-agent handoff and moved XP guest acceptance to an owner-run checklist.
Session 85 added the owner-approved Gemini 4 Neon-only ordinary local-build
exception. Session 86 extended that same model-specific authorization to relevant,
bounded benchmark/A-B work while retaining explicit authorization for every other
agent. Session 87 clarified the fixed guest RAM rationale, XP x86 virtual/physical
memory limits, and the guest-facing Direct3D 9.0c / Shader Model 3.0 ceiling.
Session 88 reorganized the README so host requirements and the guest RAM
rationale live in the bottom FAQ, while the Fixed target summary stays guest-only.
Session 89 rewrote the README introduction to emphasize the XP gaming experience,
a mid-to-late 2000s performance target, and the distinction from cycle-accurate
hardware emulation, while acknowledging 86Box's contributors. Session 90 fixed the
product target to exactly one vCPU, removed QEMU's `-smp` CLI option, and aligned
T01, T14, README, and HANDOFF with the uniprocessor decision. Session 91 moved
the project-purpose material into the FAQ, distinguished Y2KBox from compatibility
layers, documented the one-vCPU/TCG trade-off, and separated the proprietary
hypervisor API rationale into its own FAQ entry.
Session 92 closed T09's integration/CI task while keeping XP driver, display/2D, D3D API, and game acceptance in the owner-run manual checklist; T10 remains separately paused. Session 93 moved the upstream QEMU 11.1.90 base commit and unchanged GPL-2.0 notice from the README Fixed target area into a dedicated provenance FAQ entry. Session 94 re-audited all 16 Markdown files and reconciled skill authorization, Claude-specific CI hook wording, serialized-write instructions, and Gemini 4 Neon-only local benchmark/build permission. Session 95 fixed the optional GitHub PAT Authorization-header argument in `scripts/session_status.sh`; this affects Claude's CI-status API reporting only, not sandbox GitHub authentication or GitHub Actions. Session 96 moved diagnostic logging guidance into the README FAQ, documented recording overhead and the tester profiling workflow, and clarified the optional compile-time VMVGA deep-trace switch. Session 97 merged the preservation-practicality rationale into the compatibility-layer FAQ, removing the separate closing entry while retaining the project's practical preservation and usability arguments. Session 98 moved that merged project-purpose answer to the final FAQ position, leaving the technical questions together above it. Session 99 added T25 to investigate guest OpenGL over VMware SVGA3D, with OpenGL 1.1 as the baseline and 2.1 as the maximum target; it distinguishes the Mesa SVGA guest GL protocol path from the separate Windows XP VMware Tools driver capability question. Session 100 began that investigation, clarified that Mesa translates GL operations into SVGA3D commands rather than using a separate OpenGL FIFO, identified existing legacy-command overlap in Y2KBox, and established that the supplied QEMU log cannot diagnose WGL/PFD selection. Session 101 clarified the README target as OpenGL 1.1 baseline / 2.1 maximum, with older 1.x compatibility included and guest OpenGL still unimplemented. Session 102 traced COD1's GLW_ChoosePFD failure to the id Tech 3 custom pixel-format selection path and added an XP-side probe that reproduces its filters and best-match rules. Session 103 matched Mesa's VGPU9 minimum hardware version and Shader Model 3.0 requirements against Y2KBox's advertised profile, identified remaining state-capability questions, and added a quick COD1 software-OpenGL test using `r_allowSoftwareGL 1`. Session 104 recorded the owner's GLview result: Windows XP is exposing Microsoft GDI Generic OpenGL 1.1.0 with only three extensions, and COD1 now passes WGL context creation before failing its feature check. Session 105 found VMware Workstation documentation explicitly stating that OpenGL apps in XP guests run in software-emulation mode, making GDI Generic expected rather than evidence of a broken Tools install. It identified WINVBE as a possible XP software-OpenGL 2.1 experiment, but not an accelerated or drop-in solution. Session 106 established that a conventional XP ICD requires the active display driver to advertise it through OPENGL_GETINFO, so a registry-only DLL install cannot preserve the VMware driver unless it already supports that escape. It recorded application-local standalone DLLs and a D3D9 translation stack only as speculative leads, not a known solution. Session 107 marked T25 paused for an independent reassessment that may challenge or replace those hypotheses; the README now describes OpenGL as a low-probability research objective. Session 108 added QindieGL as a concrete application-local OpenGL-to-D3D9 wrapper reference, documented its missing `wglGetExtensionsStringEXT` entry point and the owner's COD1 startup failure, and recorded a historical minimal-entry-point experiment that reached startup but retained severe graphics corruption. It also flagged QindieGL's GPL-3.0 license for review before any code reuse. Session 109 removed project-specific wrapper names from the README while retaining its generic unproven-wrapper wording, and added TitaniumGL as a separate T25 lead: COD1 created/made-current an OpenGL 1.4 context and saw numerous extensions, but still rejected the graphics card; hardware and software modes showed the same broad result. The `0 PFDs found` line and actual failed capability check remain unresolved. T25 remains paused. Session 110 clarified T24 as IMPLEMENTED: the owner confirmed the Windows window is non-resizable with its titlebar maximize control disabled; Linux/Windows CI and remaining XP checks for guest-driven resolution, Ctrl+Alt+F fullscreen, and mouse tracking are still outstanding. Session 111 added red/yellow/green visual status markers to all TASKS.md headings while retaining a canonical plain-text Status field below each heading; AGENTS.md now instructs agents to treat that field as authoritative. Session 112 initially described T12 as a choice between predefined graphics profiles. Session 113 corrected that interpretation: T12 is a user-managed instance manager, with users creating independent XP VMs and choosing virtual hardware, including the GPU, per instance. VMVGA/SVGA3D remains the supported path; VBoxVGA 5.2.x/SharedOpenGL remains a conditional research candidate, not an available option. Session 114 corrected T24 to DONE after the owner confirmed Linux/Windows SDL CI and all XP acceptance checks (guest-driven resolution, Ctrl+Alt+F fullscreen, and mouse tracking) were already verified. Session 115 recorded owner post-Session 80 Windows TCG captures (FIFA 2003 playable at ~85.7 FPS; Hitman 2 Sicily fade stalling on 2,913 unpresented sync ScreenTarget readbacks; F1 '99-'02 spending 3.711 s in 71,979 VB INVALIDATE DISCARD renames) and implemented steady-state FIFO D3D9 flip supersession with eager retired staging transfer, per-surface dynamic VB ring sub-allocation + guarded 2 MiB overlap-DISCARD, and O(1) GART alias unparent + FIFO-pass GART transaction batching (standalone local tests passed; CI and owner re-captures pending). See Sessions 82–115 in `WORKLOG.md` for details.
