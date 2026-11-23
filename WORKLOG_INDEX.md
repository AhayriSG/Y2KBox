# WORKLOG index

This is a navigation aid for the append-only `WORKLOG.md`, not a second source
of truth. Use it to find historical evidence only when it is relevant to the
current task. Session numbers refer to headings in `WORKLOG.md`; some sessions
have follow-up subsections under the same number.

## How to use

1. Establish the current state from `CLAUDE.md`, `HANDOFF.md`, `TASKS.md`, and
   `WORKLOAD_ACTIVE_SESSION.md`.
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
| TCG performance, x87, XMM/SSE and benchmark methodology | 2–9, 19–28, 68, 77–79 | T04/T13 benchmark setup, x87 fast paths, XMM caching, TCG helper costs, differential testing and measured performance changes |
| VMware SVGA / SVGA3D, DXVK and D3D compatibility | 12, 29–32, 65–67, 80 | VMVGA integration, display/scanout issues, D3D9/D3D8 translation, shader/state compatibility and rendering regressions |
| SDL windowing, fullscreen, input and diagnostic capture | 33–36, 60–64 | Capture hotkey/file lifecycle, debug-mode controls, guest-driven window sizing, fullscreen behavior and diagnostic logging |
| VirtIO Block / Net and supported guest devices | 37–44 | VirtIO source restoration, Linux host-backend build gates, device defaults and PS/2 pointing-device policy |
| Scene-stall profiling, VMVGA FIFO, GART and FlatView | 45–59, 68–80 | T13e investigation history, instrumentation semantics, GART and memory transactions, FlatView algorithms, BQL contention and performance hypotheses |
| D3D9 dynamic-buffer uploads, DISCARD / NOOVERWRITE and lock profiling | 68–76, 80 | Buffer invalidation, in-flight range tracking, lock-mode selection, deferred uploads, merge/flush cadence and measured lock improvements |
| F1 Challenge performance investigation | 45–47, 68–79 | Crowded-grid slowdown, vCPU profiling, x87/TCG hotspots, owner validation and later shifts in bottlenecks |
| FIFA 2003 performance investigation | 45, 48–59, 80 | Scene-transition stalls, VMVGA/DXVK timing, memory topology costs and Session 80 optimizations |
| CI workflow, documentation and session-continuity automation | 35–36, 81–82 | Documentation changes, benchmark CI cleanup, Markdown-only artifact suppression and build-workflow behavior |

## Session 81 note

Session 81 changed CI artifact production for Markdown-only commits. Linux and
Windows configure/build/functional checks still run; only packaging, packaged
smoke tests, and binary uploads are skipped. See the full Session 81 entry in
`WORKLOG.md` and the current workflow in `.github/workflows/build.yml`.

## Session 82 note

Session 82 changes the coding-agent history-reading policy: new sessions read the
current handoff/task/recent-summary documents and this topic index, then consult
only relevant sections of the append-only worklog. The full worklog is no longer a
default context read. See Session 82 in `WORKLOG.md`.
