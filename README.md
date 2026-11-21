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

| Windows XP SP3 Desktop (ACPI PC HAL)<br/>VMware SVGA II (Experimental) | F1 Challenge '99-'02 | Joint Operations: Typhoon Rising 1.4.3.0
| :---: | :---: | :---: |
| <img src="https://github.com/user-attachments/assets/adcae51d-6939-4ed8-8e61-9a98fa4d94ce" width="310" alt="Windows XP SP3 desktop running in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/c36efbb4-8e7e-457c-b253-115691552d9f" width="310" alt="VMware SVGA II in Y2KBox (Experimental)"> | <img src="https://github.com/user-attachments/assets/ee1ba790-3550-4cbe-a479-358a126f7378" width="310" alt="VMware SVGA II and F1 Challenge '99-'02 in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/034245a2-0ba9-4eb8-80d0-b4a3779c543e" width="310" alt="VMware SVGA II and F1 Challenge '99-'02  in Y2KBox"> | <img src="https://github.com/user-attachments/assets/59ad794a-ad31-49a6-a209-9beaef453098" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising  in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/70f54da1-c64d-41ed-975f-5ea3da77173f" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising in Y2KBox">


## Host requirements

- **CPU ISA:** x86-64-v3 is the minimum. Older ISA levels are not supported.
- **Windows:** Windows 10 version 1803 (build 17134) or newer is strongly recommended. Version 1803 is the baseline we are targeting for future Windows TCG memory-management work, including potential use of newer virtual-memory APIs. It may become a formal minimum as those optimizations mature. Earlier Windows versions are not the target for future compatibility work.
- **Linux:** KVM is mandatory, and the user must have permission to access `/dev/kvm`. There is no supported Linux TCG fallback. Because guest VMware SVGA/vmport backdoor calls go through KVM, enable the host kernel parameter `kvm.enable_vmware_backdoor=1` (for example, add `options kvm enable_vmware_backdoor=1` to `/etc/modprobe.d/kvm.conf` and reload `kvm` / reboot; verify with `cat /sys/module/kvm/parameters/enable_vmware_backdoor`).

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
  -boot menu=on
```
The example expects `virtio-win-0.1.141_x86.vfd` in the current directory. Replace
or remove that floppy drive if you do not need it. To boot an installer ISO, attach
it to the CD-ROM drive with `-drive file="xpinstalldisc.iso",media=cdrom,readonly=on` and
select the CD-ROM from the boot menu.

VMware SVGA II 12.01.08 driver that compatible with Windows XP: [VMware Tools 10.0.12](https://packages.vmware.com/tools/frozen/windows/winPreVista.iso) *(Tip: choose **Custom** setup and uncheck everything except the **SVGA Driver** to avoid installing unneeded VMware background services).*
<br/>- `vmvga` (VMware SVGA II + SVGA3D, with `vgabios-vmware.bin`) is the default display device with **512 MB** of dedicated VRAM (plus 512 MB surface memory) and hardware cursor support at full Windows XP hardware acceleration. Note: 32-bit Windows XP's VMware XPDM driver (`vmx_svga.sys` 12.01.08) caps reported adapter VRAM in `dxdiag` to **128 MB** (`SVGA_VRAM_MAX_SIZE`) to fit 32-bit kernel system PTE limits even though the virtual device exposes the full 512 MB VRAM BAR1.
<br/>- Interactive display output uses **SDL** (`-display sdl`; VNC support is disabled, while `-display none` remains for headless checks). A wxWidgets front end is planned (T12).
<br/>- On Linux, launch with KVM (`-accel kvm`) and make sure `kvm.enable_vmware_backdoor=1` is enabled on the host kernel module (`qemu-system-i386` warns at startup if `/sys/module/kvm/parameters/enable_vmware_backdoor` is disabled). Linux binaries require the
usual shared libraries from your distribution, including glib2, pixman,
libslirp, libfdt, SDL2, and Vulkan (`libvulkan1`, plus `mesa-vulkan-drivers`
for lavapipe software fallback).

Diagnostic capture is available only when VMVGA is explicitly enabled with
`-device vmvga,debug=on`. This option is intended for testers; not for normal use.
With it enabled, the SDL title advertises **Ctrl+Alt+L**. Press once
to capture VMVGA, VMPort GuestRPC, and vdagent diagnostics to
`vmvga_freeze.log` in the launch working directory; press again to stop.
The VM continues running. The first capture truncates the file; later capture
intervals in the same QEMU run append. When `debug=on` is set but capture is inactive,
these diagnostic streams remain disabled: no continuous VMVGA/GuestRPC logging, no vdagent trace output,
and no capture start/stop messages in the command prompt. The hotkey is unavailable
unless `debug=on` is present. No separate `-trace "vdagent*"` option is needed;
the hotkey enables the relevant VMVGA, VMPort, and vdagent trace events only
for the selected interval. Do **not** use shell-level `2> vmvga_freeze.log`
redirection with this feature: it redirects stderr before QEMU starts and defeats interval capture.

### Windows XP guest recommendations

- **Display refresh rate:** Set the guest display to **maximum** for the selected resolution and VMware SVGA display mode. In XP, open Display Properties > Settings > Advanced > Monitor.
- **Mouse:** Open Control Panel > Mouse > Pointer Options and clear **Enhance pointer precision** to disable mouse acceleration.
- **Networking and retro web browsing:** The default `-nic user,model=rtl8139` configuration emulates an RTL8139 network adapter, for which Windows XP has an in-box driver. To browse [Protoweb](https://protoweb.org/wiki/servers/) from Internet Explorer, open Tools > Internet Options > Connections > LAN Settings, enable the proxy server, and enter **wayback.protoweb.org** with port **7851** for the current primary HTTP proxy. Protoweb periodically changes its server list, so check its [current server and proxy settings](https://protoweb.org/wiki/servers/) if the address or port stops working; alternate servers and optional automatic proxy configuration are listed there.
