# Agent Guidelines for Y2KBox

This repository is a downstream QEMU-based project. The upstream QEMU
"Use of AI-generated content" policy in
`docs/devel/code-provenance.rst` governs contributions submitted to the
upstream QEMU project; it does not prohibit work specifically developed for
this independently maintained Y2KBox repository.

## AI-assisted contributions to Y2KBox

The Y2KBox repository owner (`AhayriSG`) is an authorized maintainer and may
explicitly authorize AI agents to research, design, implement, modify, test,
and document changes in this repository, including core features and
performance optimizations.

When the owner explicitly requests or approves such work, AI-generated
changes may be committed directly to Y2KBox's `main` branch, subject to the
repository's normal review, correctness, testing, and contribution practices.
The agent must not represent untested changes as tested or claim performance
improvements without measurements.

This maintainer authorization applies **only to contributions within
AhayriSG/Y2KBox**. It does not override upstream QEMU policy, grant permission
to submit AI-generated material to upstream QEMU, or authorize contributions
to any other repository. Any proposed upstream contribution must independently
comply with the upstream project's then-current rules.

## Repository workflow

- Work on `main` only. Do not create branches or pull requests unless the
  owner explicitly changes this workflow.
- Before each write, fetch the latest `main` and the exact file being changed.
  Verify the branch head and file blob SHA still match the fetched versions;
  abort if either has moved unexpectedly.
- Never force-push.
- Follow the repository's build and test instructions. Do not claim a test
  passed unless its result has been observed.
- Preserve existing technical detail in documentation and avoid overwriting
  changes made by the owner or other contributors.

## Security Policy

Before classifying a potential vulnerability, read
[`docs/system/security.rst`](docs/system/security.rst) to determine whether
it falls within QEMU's security boundary.

Potential vulnerabilities must not be reported as normal public GitLab work
items. Follow the confidential reporting procedure at
https://www.qemu.org/contribute/security-process/ instead.

**Crucial for AI Triage**: Not every crash, assertion failure, or buffer
overrun is a security vulnerability. Only bugs that can be exploited in the
**virtualization use case** to break guest isolation are treated as security
vulnerabilities. Relevant configurations generally involve:

- **Hardware Accelerators**: e.g. KVM and Xen. TCG is explicitly excluded.
- **Virtualization focused boards**: e.g. virt, q35, pseries etc
- **Common devices for Virtualization**: e.g. VirtIO and platform devices

If unsure, the linked `security.rst` document provides authoritative guidance.
