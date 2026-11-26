# Contributing to Y2KBox

Thanks for your interest in contributing to Y2KBox.

## Contribution workflow

External contributions must be submitted through pull requests targeting
`main`. Direct pushes to `main` are reserved for repository maintainers and
coding agents explicitly authorized by the repository owner.

1. **Discuss substantial work first.** For a tracked task, coordinate with a
   maintainer before starting so work is not duplicated or in conflict with
   current priorities. Do not edit the shared task queue directly unless a
   maintainer asks you to.
2. **Use a feature branch.** Work in a fork or, if you have been granted write
   access, a dedicated branch in the repository. Keep the branch focused on one
   logical change.
3. **Open a pull request against `main`.** Explain the problem, the approach,
   relevant task or issue, compatibility impact, and the tests performed.
   Include screenshots or logs when they materially help review.
4. **Let GitHub Actions validate the proposed change.** Required PR checks must
   pass before merging. If a check fails, investigate and fix it rather than
   treating unrelated or pre-existing failures as approval.
5. **Review the build artifacts when applicable.** For code or build-system
   changes, maintainers should inspect the Linux and Windows artifacts produced
   by CI and run relevant smoke or functional tests. A successful compile alone
   is not sufficient evidence that the emulator works correctly.
6. **Wait for maintainer review and merge.** Contributors should not merge their
   own PRs unless a maintainer has explicitly granted that responsibility.

## CI and artifact expectations

The GitHub Actions workflow builds and packages Linux and Windows artifacts
for changes that trigger the platform jobs. Documentation-only changes may run
only the lightweight change-detection job; in that case, platform build jobs
are intentionally skipped and no binary artifacts are expected.

For changes affecting source code, build configuration, packaging, or runtime
behavior, check that the relevant Linux and Windows jobs ran and passed. Review
the uploaded artifacts and test the affected platform paths where practical.
Linux product validation uses KVM; Windows product validation uses TCG. A CI
build or smoke test does not by itself prove Windows XP guest compatibility,
visual correctness, or performance.

Treat artifacts from pull requests as untrusted executables. Inspect their
provenance and test them in an isolated environment; do not expose personal
files, credentials, or sensitive host resources to an untrusted build.

## Quality expectations

- Keep pull requests focused and reasonably small.
- Preserve existing project constraints documented in `AGENTS.md`,
  `HANDOFF.md`, and the relevant task entry in `TASKS.md`.
- Add or update tests when practical, and describe any validation that could
  not be performed.
- Do not claim tests, builds, or runtime behavior passed unless you observed
  the result.
- Do not force-push shared branches or rewrite shared history.
- Ask before broad refactors or changes that alter the supported guest,
  host, device, or graphics architecture.

Maintainers may merge a contribution only after reviewing the diff, required CI
checks, relevant build artifacts, and any additional runtime evidence warranted
by the risk and scope of the change.
