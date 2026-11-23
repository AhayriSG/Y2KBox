# Claude Code — Y2KBox

The shared, cross-agent repository policy lives in `AGENTS.md`; the Claude
rule `.claude/rules/agents-md.md` imports it. Follow that file as the
canonical source for project constraints, session onboarding, task claims,
safe checkpoints, build/CI policy, and verification.

This file contains only Claude Code-specific integration notes. Do not copy
shared policy here; update `AGENTS.md` instead.

## Claude hooks

Configured in `.claude/settings.json`:

- `UserPromptSubmit`: `scripts/session_status.sh` reports GitHub Actions and
  Codacy status for an unverified pushed HEAD. It is silent after that HEAD's
  completed result has been reported.
- `SessionStart` with matcher `compact`: runs the same status check after
  context compaction. Recover project context from
  `WORKLOAD_ACTIVE_SESSION.md` and the relevant `HANDOFF.md`/`TASKS.md`
  sections; do not restart full onboarding or reread the worklog.
- `Stop`: runs `scripts/maintain_active_session.py`. The agent must append
  the complete new session to `WORKLOG.md` and add its concise summary to
  `WORKLOAD_ACTIVE_SESSION.md`. The hook removes entries older than the
  latest ten and blocks session exit if the active summary does not match the
  latest ten worklog sessions. Never rewrite or prune `WORKLOG.md`.

After pushing, finish the turn. Do not manually wait for or poll CI/Codacy;
the status hook reports the result on a later prompt.

## Local secrets

- Never hardcode secrets. `CODACY_API_TOKEN` and optional `GITHUB_TOKEN` are
  read from the environment only.
- `.claude/settings.local.json` and `.env*` are git-ignored. Never commit
  them or paste tokens into repository files or conversation output.

## Safe repository operations

- Prefer the reproducible mechanical-removal scripts in `xp/strip/` for
  pruning work; inspect their effects and relevant checks before committing.
- Use `git revert` for reverting commits rather than whole-file restores.
  Before pushing, verify file identity carefully; in particular, check
  `head -1 TASKS.md WORKLOG.md` if performing shell-based edits to avoid
  accidentally swapping similarly named files.
