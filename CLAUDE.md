# CLAUDE.md - Y2KBox (read this first, then HANDOFF.md)

Y2KBox is a downstream QEMU fork that emulates only 32-bit Windows XP guests (TCG, x86-64-v3 Windows/Linux hosts).
Context, decisions and design notes: `HANDOFF.md`. Work queue and claim protocol: `TASKS.md`. History: `WORKLOG.md`.

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
