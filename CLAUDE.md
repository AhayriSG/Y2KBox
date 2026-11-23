# CLAUDE.md - Y2KBox (read this first, then HANDOFF.md)

Y2KBox is a downstream QEMU fork that emulates only 32-bit Windows XP guests (TCG, x86-64-v3 Windows/Linux hosts).
Context, decisions and design notes: `HANDOFF.md`. Work queue and claim protocol: `TASKS.md`. History: `WORKLOG.md`.

## Repository Analysis and Session Continuity

Distinguish a **new coding-agent session** from another prompt, task, or turn in the
same running session. A new session means a fresh agent runtime/sandbox without the
previous session's working context. A new user message or a context-compaction event
does not, by itself, make it a new session.

- **On the first repository analysis in a genuinely new session:** read this file,
  `HANDOFF.md`, `TASKS.md`, and `WORKLOAD_ACTIVE_SESSION.md`. Also read
  `WORKLOG.md` once to establish the project's historical context (the newest
  entries are at the bottom). If the file is too large for one pass, read it in
  chunks, prioritizing the latest entries, and use targeted older sections as needed.
- **On subsequent work in the same session:** do not reread the entire
  `WORKLOG.md`. Use `WORKLOAD_ACTIVE_SESSION.md` as the compact recent-context
  entry point, then check the relevant `HANDOFF.md` instructions and `TASKS.md`
  entries. Reopen only the specific `WORKLOG.md` sections needed to verify a
  historical implementation, decision, diagnostic result, or commit.
- **After context compaction in the same session:** recover from
  `WORKLOAD_ACTIVE_SESSION.md` and the relevant task/handoff sections; do not
  restart the full-history read merely because context was compacted.
- **At the end of each work session:** append the complete session record to
  `WORKLOG.md`, then refresh `WORKLOAD_ACTIVE_SESSION.md` so it summarizes the
  latest ten worklog sessions. Keep the summary concise and derived from the
  worklog; never edit, truncate, or replace the append-only history.
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
  `vectest` (`xp/bench/vectest.asm`) and the x87 differential test (`xp/bench/x87cmp.sh`) on **Linux and Windows**, and uploads
  the binaries (`qemu-system-i386[.exe]`, `qemu-img[.exe]`, `pc-bios/`, Windows runtime DLLs) as GitHub Actions Artifacts.
- **CI & Codacy feedback loop (never poll after pushing):**
  - The `UserPromptSubmit` hook (`scripts/session_status.sh`, registered in `.claude/settings.json`, also on `SessionStart`
    after compaction) injects GitHub Actions and Codacy status whenever `HEAD` has an unverified push, and is silent (0 tokens)
    once the HEAD run is `completed` and reported.
  - If `conclusion=failure`, inspect only the failed step (the hook prints job/step and annotations; for more use the run
    URL or `gh run view <run_id> --log-failed`) and fix the regression before starting new feature work.
  - **After pushing:** commit, push, and **finish your turn immediately**. Do not wait or poll for Actions or Codacy.
