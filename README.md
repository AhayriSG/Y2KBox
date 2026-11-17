<div align="center">

<img width="272" height="281" alt="Y2KBox logo (alpha)" src="https://github.com/user-attachments/assets/af59c3c3-da2c-469f-84e0-1c605ce8d1e8" alt="Y2KBox Logo" width="200" />

# Y2KBox

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0d7875bcbfb40428c030025425f775f)](https://app.codacy.com/gh/AhayriSG/Y2KBox/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![build](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml/badge.svg)](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/AhayriSG/Y2KBox/blob/main/LICENSE)
</div>

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, using **TCG / MTTCG only** (no KVM/WHPX/HVF/Xen).
Long-term goals: fast TCG for all guest instruction classes, **VMware SVGA3D
(qemu-vmvga) 3D acceleration** for XP (VBoxVGA 5.2.x kept as the alternative), and a small wxWidgets launcher.

**Contributors / LLMs: read [HANDOFF.md](HANDOFF.md) first**, then pick work from
[TASKS.md](TASKS.md) (claim by pushing) and log what you did in [WORKLOG.md](WORKLOG.md).
Several AIs can work in parallel.

Fixed target: guest RAM 2048 MB (automatic, not configurable), guest CPU SSE2 maximum
(Pentium 4 class), 1 vCPU by default; hosts: Windows and Linux (x86-64-v3 or newer).

Base: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).
License: GPL-2.0, unchanged (see COPYING / LICENSE).

## Download

Every green CI run uploads Linux and Windows builds as Actions Artifacts. Open the
[latest successful run](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
and download the artifact for your platform:

- Linux: `qemu-system-i386`, `qemu-img`, and `pc-bios/`.
- Windows: `qemu-system-i386.exe`, `qemu-img.exe`, `pc-bios/`, and the required
  runtime DLLs, including SDL2.

Extract the **entire** archive. Keep `pc-bios/` beside the executable; QEMU finds
it automatically, so you do not need `-L` when launching from that directory.

Create a 10 GB QCOW2 disk image:

```console
qemu-img create -f qcow2 xp.qcow2 10G
```

Check the build version:

```console
qemu-system-i386 --version
```

Example Windows Command Prompt launch for a Windows XP VM:

```bat
qemu-system-i386 -nodefaults -rtc base=localtime -monitor stdio -display sdl ^
  -name "Windows XP" ^
  -M pc,kernel-irqchip=off ^
  -device VGA ^
  -audiodev sdl,id=audio0,in.voices=0 ^
  -device AC97,audiodev=audio0 ^
  -nic user,model=rtl8139 ^
  -drive id=fld01,if=floppy,format=raw,file="virtio-win-0.1.141_x86.vfd" ^
  -drive file=xp.qcow2,format=qcow2 ^
  -drive media=cdrom,readonly=on ^
  -boot menu=on -trace "vdagent*"
```

The example expects `virtio-win-0.1.141_x86.vfd` in the current directory. Replace
or remove that floppy drive if you do not need it. To boot an installer ISO, attach
it to the CD-ROM drive with `-drive file="xp.iso",media=cdrom,readonly=on` and
select the CD-ROM from the boot menu.

A wxWidgets front end is planned (T12). Linux binaries require the usual shared
libraries from your distribution, including glib2, pixman, libslirp, libfdt, and SDL2.

| Windows XP SP3 (ACPI PC HAL) - Desktop |
| :--------------------------------------------------------------------------------------------: |
| <img src="https://github.com/user-attachments/assets/adcae51d-6939-4ed8-8e61-9a98fa4d94ce" width="500" alt="Windows XP SP3 desktop running in Y2KBox"> |
