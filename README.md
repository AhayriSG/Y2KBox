# Y2KBox

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, using **TCG / MTTCG only** (no KVM/WHPX/HVF/Xen).
Long-term goals: fast SSE2 under TCG, VirtualBox-style **VBoxVGA 3D acceleration**
for XP, and a small wxWidgets launcher.

**Start here: [HANDOFF.md](HANDOFF.md)** - aim, decisions, what is done, what is
in progress, how to build/verify, and exact next steps (written for other LLMs
and humans picking this up).

Base: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).
License: GPL-2.0, unchanged (see COPYING / LICENSE).
