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
| VMware SVGA / SVGA3D, DXVK and D3D compatibility | 12, 29–32, 65–67, 80 | VMVGA integration, display/scanout issues, D3D9/D3D8 translation, shader/state compatibility and rendering regressions |
| SDL windowing, fullscreen, input and diagnostic capture | 33–36, 60–64 | Capture hotkey/file lifecycle, debug-mode controls, guest-driven window sizing, fullscreen behavior and diagnostic logging |
| VirtIO Block / Net and supported guest devices | 37–44 | VirtIO source restoration, Linux host-backend build gates, device defaults and PS/2 pointing-device policy |
| Scene-stall profiling, VMVGA FIFO, GART and FlatView | 45–59, 68–80 | T13e investigation history, instrumentation semantics, GART and memory transactions, FlatView algorithms, BQL contention and performance hypotheses |
| D3D9 dynamic-buffer uploads, DISCARD / NOOVERWRITE and lock profiling | 68–76, 80 | Buffer invalidation, in-flight range tracking, lock-mode selection, deferred uploads, merge/flush cadence and measured lock improvements |
| F1 Challenge performance investigation | 45–47, 68–79 | Crowded-grid slowdown, vCPU profiling, x87/TCG hotspots, owner validation and later shifts in bottlenecks |
| FIFA 2003 performance investigation | 45, 48–59, 80 | Scene-transition stalls, VMVGA/DXVK timing, memory topology costs and Session 80 optimizations |
| CI workflow, documentation and session-continuity automation | 35–36, 81–94 | Documentation changes, Markdown-only CI job skipping, shared agent instructions, selective context loading, session-continuity automation, and model-specific local-build authorization |

## Session 81 note

Session 81 initially skipped packaging, packaged smoke tests, and artifact uploads
for Markdown-only commits while still running Linux/Windows builds and functional
checks. The later job-level condition in commit `8ab36ee` skips both platform jobs
entirely for Markdown-only commits; only the lightweight detector runs. Any
non-Markdown change and manual `workflow_dispatch` run the full platform jobs.
See `.github/workflows/build.yml`.

## Sessions 82–94 note

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
Session 92 closed T09's integration/CI task while keeping XP driver, display/2D, D3D API, and game acceptance in the owner-run manual checklist; T10 remains separately paused. Session 93 moved the upstream QEMU 11.1.90 base commit and unchanged GPL-2.0 notice from the README Fixed target area into a dedicated provenance FAQ entry. Session 94 re-audited all 16 Markdown files and reconciled skill authorization, Claude-specific CI hook wording, serialized-write instructions, and Gemini 4 Neon-only local benchmark/build permission. See Sessions 82–94 in `WORKLOG.md` for the policy evolution and related decisions.
