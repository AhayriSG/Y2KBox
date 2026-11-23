# Gemini CLI — Y2KBox

Follow `AGENTS.md` as the canonical repository policy. It defines the
project constraints, token-efficient session onboarding, task-claim protocol,
safe checkpoints, the Gemini 4 Neon-only local-build exception, CI behavior,
and verification standards. The exception applies only when the active model
is specifically Gemini 4 Neon; it does not extend to other Gemini models or
other agents. Performance benchmarking/A-B work still requires explicit owner
authorization. Report local checks separately from GitHub Actions.

Before starting a task, use the fresh-session reading sequence in `AGENTS.md`:
read `HANDOFF.md` sections 0-3, the task overview and only the relevant task
entry in `TASKS.md`, `WORKLOAD_ACTIVE_SESSION.md`, and
`WORKLOG_INDEX.md`. Open only relevant historical sections from
`WORKLOG.md`; never load the full archive by default.

When taking over work from Claude, Codex, or another agent, use the shared task status and exact resume point; do not assume access to that agent's private session memory or repeat completed onboarding. The owner normally uses one active agent at a time.

Do not duplicate shared instructions here. Make universal policy changes in
`AGENTS.md`, durable project decisions in `HANDOFF.md`, task-status changes
in `TASKS.md`, and historical records in `WORKLOG.md`.
