# Y2KBox Agent Guidelines

Y2KBox is a downstream QEMU fork for 32-bit Windows XP guests on x86-64-v3
Windows and Linux hosts. Linux product runtime is KVM-only; Windows product
runtime is TCG-only. VMVGA is the sole display/3D path.

## Source of truth

- `HANDOFF.md`: owner-approved architecture, constraints, and durable decisions.
- `TASKS.md`: task queue, claims, status, and exact resume points. Its colored heading markers are human-only cues; the plain-text `Status:` field immediately below each task heading is authoritative.
- `WORKLOAD_ACTIVE_SESSION.md`: concise recent-session context.
- `WORKLOG_INDEX.md`: navigation to relevant historical evidence.
- `WORKLOG.md`: append-only chronological record; never load it all by default.
- Current source and tests: verify implementation claims against these, not just
  old notes. Claude Code's configured status hook reports pushed CI/Codacy results
  on a later prompt. Other agents must not assume that hook is available; the owner
  or a later session reviews their pushed results. No agent should poll after pushing.

If sources conflict, do not silently choose the convenient version. Check current
code and the latest owner decision, then correct the stale document.

## Cross-agent continuity

The owner may switch between Claude, Gemini, Codex, or another coding agent
sequentially as sessions expire or tasks change. This is a supported normal
workflow: treat `AGENTS.md`, `HANDOFF.md`, `TASKS.md`,
`WORKLOAD_ACTIVE_SESSION.md`, and `WORKLOG.md` as the shared durable state.
Do not assume another provider's private conversation or runtime memory is
available. On takeover, resume from the recorded task status and exact
`Resume here:` instruction; do not repeat completed work or full onboarding
unless the repository state is genuinely unclear. Before handing off, record
all meaningful progress and the next action using the checkpoint procedure.
The default owner workflow is one active agent at a time; concurrent agents
must explicitly coordinate task claims and serialize writes to `main`.

## Fresh-session onboarding (token-efficient)

For a genuinely new agent runtime/sandbox:

1. Read this file.
2. Read only `HANDOFF.md` sections 0-3 for onboarding, project constraints,
   current state, and code map. Read deeper sections only when the task needs them.
3. Read the task-heading/status overview in `TASKS.md`, then only the complete
   entry for the task being claimed or continued. Do not load the entire task
   archive by default.
4. Read `WORKLOAD_ACTIVE_SESSION.md` and `WORKLOG_INDEX.md`.
5. Use the index to open only relevant `WORKLOG.md` session sections. Search
   specific terms/headings if the index does not cover the topic. Never read the
   full worklog by default.

Before changing code, inspect the relevant source and tests. Historical notes are
evidence and rationale, not proof that the current tree still behaves that way.

For later prompts in the same runtime, do not repeat onboarding. After context
compaction, recover from the active-session summary and the relevant task/handoff
sections; do not restart the full-history read.

## Repository workflow

- Work directly on `main`. Do not create branches or pull requests unless the
  owner explicitly changes this policy.
- Claim a task in `TASKS.md` before starting. Follow its claim/update protocol.
- Before every write, fetch the latest `main` and the exact target file. Verify
  both the branch HEAD and target blob SHA still match what you inspected. If
  either moved, abort that write, refetch, and reapply to the new content.
- Serialize writes to `main`; coordinate task overlap through `TASKS.md`.
- Commit small, coherent, safe checkpoints proactively. Never knowingly push
  broken or internally inconsistent changes. Never force-push.
- Before pausing: push the latest safe recoverable state; set the task to
  `PAUSED` with a precise `Resume here:` note; append completed work, evidence,
  and unresolved issues to `WORKLOG.md`; add the concise matching entry to
  `WORKLOAD_ACTIVE_SESSION.md`. If no safe checkpoint exists, do not push it;
  document how to recover it.
- Update `HANDOFF.md` only for durable decisions, constraints, or important
  current-state facts. Update `WORKLOG_INDEX.md` when a new historical topic
  deserves navigation. `README.md` is user-facing project documentation; do
  not assume it contains a task-status table.

## Build, tests, and CI

- **Standing local-build authorization — Gemini 4 Neon only:** Gemini 4 Neon
  may perform ordinary local QEMU configure/build and the relevant standard
  functional checks in its sandbox as part of validating its own changes. This
  is an owner-approved, model-specific exception, not a general permission for
  Gemini CLI, other Gemini models, Claude, Codex, or other agents. Do not infer
  that another agent inherits it. Use the repository's established build
  configuration and avoid unnecessary clean/full rebuilds when an incremental
  build or targeted check is sufficient.
- **All other agents:** do not run QEMU configure/build commands (`./configure`,
  `make`, or `ninja`) in the sandbox unless the owner explicitly authorizes
  that work for the current task.
- **Standing benchmarking/A-B authorization — Gemini 4 Neon only:** Gemini 4
  Neon may run the repository's established performance benchmark scripts and
  controlled A/B experiments (including `xp/bench/run.sh` and
  `xp/bench/perf.py`) when they are relevant to validating its own changes.
  This is a separate owner-approved, model-specific exception; it does not
  extend to Gemini CLI generally, other Gemini models, Claude, Codex, or other
  agents. Do not infer that another agent inherits it.
- Keep benchmark runs purposeful and bounded: use the established methodology,
  preserve comparable before/after results and configuration, avoid needless
  repetitions or unnecessarily long campaigns, and report the exact commands,
  workload, and results. Treat noisy or incomparable measurements as
  inconclusive rather than claiming a speedup. Any agent other than Gemini 4
  Neon still needs explicit owner authorization for performance benchmarks or
  A/B experiments.
- GitHub Actions remains the authoritative shared Linux/Windows build and CI
  result. A local build/test result is supplementary evidence, not CI status.
  Report the exact local configuration, commands, and checks run; distinguish
  local success/failure from GitHub Actions, and never claim CI passed based on
  local results. Do not push knowingly broken or internally inconsistent code.
- Markdown-only commits intentionally run only the lightweight CI change-
  detection job. Linux and Windows build/test jobs are skipped. Any non-`.md`
  change and manual `workflow_dispatch` run the normal platform jobs.
- After pushing, finish the turn immediately. Do not wait for or poll GitHub
  Actions or Codacy. Claude's configured status hook reports results on a later
  prompt; Gemini/Codex sessions should likewise end without polling and let the
  owner or a later session review the result.
- Never claim a test passed, a build compiled, or behavior works unless that
  result was actually observed. A green CI run proves only the checks it ran;
  it does not prove XP guest behavior, visual correctness, or performance gains.
- For x87 changes, preserve and run the differential check
  `xp/bench/x87cmp.sh` through the authorized workflow.

## AI-assisted contribution scope

The Y2KBox repository owner (`AhayriSG`) may explicitly authorize AI agents to
research, design, implement, modify, test, and document changes in this
repository, including core features and performance optimizations. Authorized
changes may be committed directly to Y2KBox `main`, subject to normal review,
correctness, and verification requirements.

This authorization applies **only** to `AhayriSG/Y2KBox`. It does not override
upstream QEMU policy, authorize AI-generated contributions to upstream QEMU, or
grant permission to contribute to another repository. Any upstream contribution
must independently follow that project's current rules.

## Security

Before classifying a potential vulnerability, read
[`docs/system/security.rst`](docs/system/security.rst) to determine whether it
falls within QEMU's security boundary.

Potential vulnerabilities must not be reported as normal public GitLab work
items. Follow the confidential procedure at
https://www.qemu.org/contribute/security-process/ instead.

Not every crash, assertion failure, or buffer overrun is a security
vulnerability. The relevant distinction is whether the issue can be exploited
in the virtualization use case to break guest isolation. If unsure, follow the
linked security documentation.
