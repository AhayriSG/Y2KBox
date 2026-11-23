# CLAUDE.md - Y2KBox (read this first, then HANDOFF.md)

Y2KBox is a downstream QEMU fork that emulates only 32-bit Windows XP guests (TCG, x86-64-v3 Windows/Linux hosts).
Context, decisions and design notes: `HANDOFF.md`. Work queue and claim protocol: `TASKS.md`. History: `WORKLOG.md`.

## Repository Analysis and Session Continuity

Distinguish a **new coding-agent session** from another prompt, task, or turn in the
same running session. A new session means a fresh agent runtime/sandbox without the
previous session's working context. A new user message or a context-compaction event
does not, by itself, make it a new session.

- **On the first repository analysis in a genuinely new session:** read this file,
  `HANDOFF.md`, `TASKS.md`, and `WORKLOAD_ACTIVE_SESSION.md`. Read
  `WORKLOG_INDEX.md` for historical navigation. Do **not** read the full
  `WORKLOG.md` by default: it is the append-only archive, not a mandatory context
  dump. Use the index to identify relevant session numbers/topics, then open only
  those sections in `WORKLOG.md` when prior implementation details, experiments,
  regressions, or decision rationale materially affect the task. If the index does
  not cover a needed topic, search the worklog headings or targeted terms.
- **Before changing code:** establish the current state from `HANDOFF.md`,
  `TASKS.md`, the active-session summary, and the relevant source/tests. Historical
  entries are evidence, not current truth: verify important claims against current
  code and current task/handoff status.
- **On subsequent work in the same session:** do not reread the full
  `WORKLOG.md` or the onboarding files without a reason. Use the active summary
  and current task/handoff state; consult targeted historical sections only when
  needed.

- **After context compaction in the same session:** recover from
  `WORKLOAD_ACTIVE_SESSION.md` and the relevant task/handoff sections; do not
  restart the full-history read merely because context was compacted.
- **At the end of each work session:** append the complete session record to
  `WORKLOG.md`, then add a concise corresponding entry to
  `WORKLOAD_ACTIVE_SESSION.md`. The configured Claude Code `Stop` hook runs
  `scripts/maintain_active_session.py`: it automatically removes the oldest
  active-summary entries when the list exceeds ten, and blocks session exit with
  a clear reminder if the latest worklog session has not been summarized. Thus
  the agent writes the new summary entry; the hook enforces freshness and rolls
  the ten-session window forward. Never move, truncate, or rewrite history in
  `WORKLOG.md`; older sessions already remain there permanently.
- `WORKLOG.md` remains the historical record, `TASKS.md` remains authoritative
  for task status/claims, and `HANDOFF.md` remains authoritative for project
  decisions and operating instructions. If the rolling summary conflicts with
  one of those sources, verify the source of truth and correct the summary.

## About the upstream AGENTS.md

`AGENTS.md` / `.claude/rules/agents-md.md` come from upstream QEMU: its policy forbids AI-generated content in
*upstream QEMU contributions*. Y2KBox is a separate downstream fork that is never submitted upstream, so that
policy is respected by never upstreaming this code; it does not restrict development in this repository.

## Rules
1. Claim a task in `TASKS.md` (by pushing) before starting; work on `task/Txx-*` branches; keep `main` building.
2. Never hardcode secrets. `CODACY_API_TOKEN` (and optional `GITHUB_TOKEN`) are read from the environment only.
   `.claude/settings.local.json` and `.env*` are git-ignored; never commit them or paste tokens into files.
3. Append to `WORKLOG.md` and update `README.md`'s status table / `TASKS.md` status when state changes.
4. Use `git revert`, never whole-file restores; check `head -1 TASKS.md WORKLOG.md` before pushing (a revert once swapped them).
5. Prefer the strip/prune scripts in `xp/strip/` for mechanical removals; verify with the checks below.
6. Any x87 change must keep `xp/bench/x87cmp.sh` (fast path vs `XPQEMU_NO_HX87=1`) identical.

## 7. Standard Checks & CI Verification (Token-Optimized)
- **Do NOT build locally:** never run `./configure`, `make` or `ninja` in the sandbox unless explicitly asked to benchmark
  (`xp/bench/run.sh`, `xp/bench/perf.py`, A/B work). Let GitHub Actions handle compilation, smoke tests and binary artifacts.
- **CI pipeline (`.github/workflows/build.yml`):** on every push it runs `configure`, build, the SeaBIOS 2-vCPU boot check,
  `vectest` (`xp/bench/vectest.asm`) and the x87 differential test (`xp/bench/x87cmp.sh`) on **Linux and Windows**. Binary
  packaging, packaged-layout smoke tests, and artifact uploads are skipped when every changed file has the `.md` extension;
  mixed/code changes and manual `workflow_dispatch` runs still produce the normal Linux and Windows artifacts.
- **CI & Codacy feedback loop (never poll after pushing):**
  - The `UserPromptSubmit` hook (`scripts/session_status.sh`, registered in `.claude/settings.json`, also on `SessionStart`
    after compaction) injects GitHub Actions and Codacy status whenever `HEAD` has an unverified push, and is silent (0 tokens)
    once the HEAD run is `completed` and reported.
  - If `conclusion=failure`, inspect only the failed step (the hook prints job/step and annotations; for more use the run
    URL or `gh run view <run_id> --log-failed`) and fix the regression before starting new feature work.
  - **After pushing:** commit, push, and **finish your turn immediately**. Do not wait or poll for Actions or Codacy.
