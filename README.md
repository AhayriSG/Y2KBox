<div align="center">

<img width="272" height="281" alt="Y2KBox logo (alpha)" src="https://github.com/user-attachments/assets/af59c3c3-da2c-469f-84e0-1c605ce8d1e8" alt="Y2KBox Logo" width="200" />

# Y2KBox

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0d7875bcbfb40428c030025425f775f)](https://app.codacy.com/gh/AhayriSG/Y2KBox/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![build](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml/badge.svg)](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/AhayriSG/Y2KBox/blob/main/LICENSE)
</div>

A QEMU fork that emulates exactly one thing: **32-bit Windows XP guests**, on
**x86-64-v3 (or newer) hosts**, with **Linux KVM required** and **Windows TCG / MTTCG only**. Linux users must enable KVM and have access to `/dev/kvm` before running Y2KBox; there is no supported Linux TCG runtime fallback. TCG performance work is exclusively aimed at Windows.
Long-term goals: fast TCG for all guest instruction classes, **VMware SVGA3D
(qemu-vmvga) 3D acceleration** for XP (VBoxVGA 5.2.x kept as the alternative), and a small wxWidgets launcher.

**Contributors / LLMs: read [HANDOFF.md](HANDOFF.md) first**, then pick work from
[TASKS.md](TASKS.md) (claim on `main`) and log what you did in [WORKLOG.md](WORKLOG.md).
Contributors work directly on `main`; coordinate task claims and serialize commits.

Fixed target: guest RAM 2048 MB (automatic, not configurable), guest CPU SSE2 maximum
(Pentium 4 class), 1 vCPU by default; hosts: Windows and Linux (x86-64-v3 or newer).

Base: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).
License: GPL-2.0, unchanged (see COPYING / LICENSE).


## Download & Usage

Every green CI run uploads Linux and Windows builds as Actions Artifacts. Open the
[latest successful run](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
and download the artifact for your platform:

- Linux: `qemu-system-i386`, `qemu-img`, `pc-bios/`, and bundled `dxvk-native`
  3.1.1 `.so.0` libraries for SVGA3D.
- Windows: `qemu-system-i386.exe`, `qemu-img.exe`, `pc-bios/`, and the required
  runtime DLLs, including SDL2 (uses inbox Windows `d3d9.dll`/`d3d11.dll` by
  default, or optional DXVK Windows DLLs placed beside `qemu-system-i386.exe`).

Extract the **entire** archive. Keep `pc-bios/` beside the executable; QEMU finds
it automatically, so you do not need `-L` when launching from that directory.

Create a 10 GB QCOW2 disk image:

```console
qemu-img create -f qcow2 xp.qcow2 10G
```

Example Windows Command Prompt launch for a Windows XP VM:

```bat
qemu-system-i386 -nodefaults -rtc base=localtime -monitor stdio -display sdl ^
  -name "Windows XP" ^
  -M pc,kernel-irqchip=off ^
  -device vmvga ^
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
it to the CD-ROM drive with `-drive file="xpinstalldisc.iso",media=cdrom,readonly=on` and
select the CD-ROM from the boot menu.

VMware SVGA II 12.01.08 driver that compatible with Windows XP: [VMware Tools 10.0.12](https://packages.vmware.com/tools/frozen/windows/winPreVista.iso) *(Tip: choose **Custom** setup and uncheck everything except the **SVGA Driver** to avoid installing unneeded VMware background services).*
<br/>- If you experience mouse cursor issues, or graphical glitches in certain games, drop the hardware acceleration level down by one notch in the Windows XP guest: right-click the Windows XP desktop and select Properties. Navigate to Settings > Advanced > Troubleshoot.
<br/>- Currently, the VMware SVGA II device is hardcoded to 32MB of dedicated VRAM. (Note: This will be increased to 512MB in an upcoming update).
<br/>- A wxWidgets front end is planned (T12). `vmvga` (VMware SVGA II + SVGA3D, with
`vgabios-vmware.bin`) is the default display device. Linux binaries require the
usual shared libraries from your distribution, including glib2, pixman,
libslirp, libfdt, SDL2, and Vulkan (`libvulkan1`, plus `mesa-vulkan-drivers`
for lavapipe software fallback).

| Windows XP SP3 Desktop (ACPI PC HAL)<br/>VMware SVGA II (Experimental) | F1 Challenge '99-'02 | Joint Operations: Typhoon Rising 1.4.3.0
| :---: | :---: | :---: |
| <img src="https://github.com/user-attachments/assets/adcae51d-6939-4ed8-8e61-9a98fa4d94ce" width="310" alt="Windows XP SP3 desktop running in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/c36efbb4-8e7e-457c-b253-115691552d9f" width="310" alt="VMware SVGA II in Y2KBox (Experimental)"> | <img src="https://github.com/user-attachments/assets/ee1ba790-3550-4cbe-a479-358a126f7378" width="310" alt="VMware SVGA II and F1 Challenge '99-'02 in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/034245a2-0ba9-4eb8-80d0-b4a3779c543e" width="310" alt="VMware SVGA II and F1 Challenge '99-'02  in Y2KBox"> | <img src="https://github.com/user-attachments/assets/59ad794a-ad31-49a6-a209-9beaef453098" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising  in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/70f54da1-c64d-41ed-975f-5ea3da77173f" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising in Y2KBox">

