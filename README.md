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

**Contributors / LLMs:** read [HANDOFF.md](HANDOFF.md), [TASKS.md](TASKS.md), and
[WORKLOAD_ACTIVE_SESSION.md](WORKLOAD_ACTIVE_SESSION.md) before starting. Follow the
session-aware reading rules in [CLAUDE.md](CLAUDE.md): a fresh coding-agent session
reads `WORKLOG.md` to establish history; continued work in the same session uses the
rolling summary and consults only relevant historical worklog sections. Append every
completed/paused session to [WORKLOG.md](WORKLOG.md) and refresh the rolling summary.
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
  -netdev user,id=net0 ^
  -device virtio-net-pci,netdev=net0 ^
  -drive id=fld01,if=floppy,format=raw,file="virtio-win-0.1.141_x86.vfd" ^
  -drive id=system,if=none,file=xp.qcow2,format=qcow2 ^
  -device virtio-blk-pci,drive=system ^
  -drive id=cdrom,if=ide,media=cdrom,readonly=on ^
  -boot menu=on
```
The example expects [virtio-win-0.1.141_x86.vfd](https://fedorapeople.org/groups/virt/virtio-win/direct-downloads/archive-virtio/virtio-win-0.1.141-1/) in the current directory. It uses
VirtIO Block for the system disk and VirtIO Net for networking; the floppy provides
the XP `viostor` and NetKVM drivers during setup. Keep the floppy attached (`virtio-win-0.1.141_x86.vfd`) until
both drivers are installed, then remove that drive if desired.
Press **F6** during text-mode Windows XP Setup to load an additional storage driver,
and select the XP x86 **VirtIO Block (`viostor`)** driver;
otherwise XP setup will lack the boot-start storage driver and fail to start. The CD-ROM is
explicitly attached to the PIIX IDE controller. To boot an installer ISO, replace the blank CD-ROM line with
`-drive id=cdrom,if=ide,media=cdrom,readonly=on,file="xpinstalldisc.iso"`
and select the CD-ROM from the boot menu.

VMware SVGA II 12.01.08 driver that compatible with Windows XP: [VMware Tools 10.0.12](https://packages.vmware.com/tools/frozen/windows/winPreVista.iso) *(Tip: choose **Custom** setup and uncheck everything except the **SVGA Driver** to avoid installing unneeded VMware background services).*
<br/>- `vmvga` (VMware SVGA II + SVGA3D, with `vgabios-vmware.bin`) is the default display device with **128 MiB framebuffer VRAM** plus a separate **512 MiB 3D surface-memory budget**, and hardware cursor support at full Windows XP hardware acceleration. The framebuffer size is the local VRAM exposed through BAR1; the surface-memory budget is separate and is not additional `dxdiag` dedicated/display memory. The 32-bit Windows XP VMware XPDM driver (`vmx_svga.sys` 12.01.08) reports at most **128 MiB** of adapter VRAM (`SVGA_VRAM_MAX_SIZE`).
<br/>- Interactive display output uses **SDL** (`-display sdl`; VNC support is disabled, while `-display none` remains for headless checks). A wxWidgets front end is planned (T12).
<br/>- On Linux, launch with KVM (`-accel kvm`) and make sure `kvm.enable_vmware_backdoor=1` is enabled on the host kernel module (`qemu-system-i386` warns at startup if `/sys/module/kvm/parameters/enable_vmware_backdoor` is disabled). Linux binaries require the
usual shared libraries from your distribution, including glib2, pixman,
libslirp, libfdt, SDL2, and Vulkan (`libvulkan1`, plus `mesa-vulkan-drivers`
for lavapipe software fallback).

Diagnostic logging is an opt-in tester feature and is **off by default**.
Enable Y2KBox debug mode with `-y2kboxdebugmode on` (or explicitly disable it
with `-y2kboxdebugmode off`; the default is off). This is the command-line
equivalent of the future T12 GUI's **Debug: On/Off** setting.
With debug mode enabled, the SDL title advertises **Ctrl+Alt+L**. Press once
to capture general Y2KBox diagnostics—including performance counters, VMVGA,
VMPort GuestRPC, and vdagent traces—to a new timestamped
`y2kbox-log-YYYYMMDD-HHMMSS.log` file in the launch working directory; press
again to stop. The VM continues running. Every capture interval gets its own
file, and existing files are never overwritten. At the start of each interval,
all `Y2KBOX-PERF` counters and timing baselines reset, so each file reports only
work measured during that capture; the VM and guest state are not restarted.
When debug mode is enabled but capture is inactive, the diagnostic streams
remain disabled: no continuous VMVGA/GuestRPC logging, no vdagent trace output,
and no routine capture start/stop messages in the command prompt. The hotkey
is unavailable when debug mode is off. No separate `-trace "vdagent*"`
option is needed; the hotkey enables the relevant diagnostic trace events only
for the selected interval. Do **not** use shell-level `2> y2kbox-log.log`
redirection with this feature: it redirects stderr before QEMU starts and
defeats interval capture.

### Windows XP guest recommendations

- **Display refresh rate:** Set the guest display to **maximum** for the selected resolution and VMware SVGA display mode. In XP, open Display Properties > Settings > Advanced > Monitor.
- **Mouse:** Y2KBox's supported pointing device is the PC machine's default **PS/2 IntelliMouse**; keep this device and do not attach USB mouse or tablet devices. USB absolute-coordinate/tablet and USB HID mouse paths can cause tracking, camera-recentering, or DirectInput compatibility problems in some XP-era games. In XP, open Control Panel > Mouse > Pointer Options and clear **Enhance pointer precision** to disable mouse acceleration.
- **Networking and retro web browsing:** The standard launch profile uses `-netdev user,id=net0 -device virtio-net-pci,netdev=net0`. Windows XP needs the XP x86 **NetKVM** driver from `virtio-win-0.1.141_x86.vfd`; install it in the guest before relying on VirtIO networking. RTL8139 remains available as an independent fallback with `-nic user,model=rtl8139` and uses XP's in-box driver. To browse [Protoweb](https://protoweb.org/wiki/servers/) from Internet Explorer, open Tools > Internet Options > Connections > LAN Settings, enable the proxy server, and enter **wayback.protoweb.org** with port **7851** for the current primary HTTP proxy. Protoweb periodically changes its server list, so check its [current server and proxy settings](https://protoweb.org/wiki/servers/) if the address or port stops working; alternate servers and optional automatic proxy configuration are listed there.
- **CD-ROM and CD images (BIN/CUE):** The emulated CD-ROM stays on the PIIX IDE controller by default (`-drive id=cdrom,if=ide,media=cdrom,readonly=on`), independently of the system-disk controller. Its slower optical-drive behavior and direct control of the emulated CD device are intentional; VirtIO Block does not replace this CD-ROM path. A future wxWidgets option (T12) may offer a VirtIO SCSI CD-ROM as an alternative, but that requires restoring and validating the VirtIO SCSI device and XP driver path first; it is not enabled in current builds. Until Y2KBox's emulated CD-ROM supports the full BIN/CUE format—including the complete set of CUE flags (such as `DCP`)—and supports inserting/ejecting disc images in the emulated drive for multi-disc swapping, we recommend using **Daemon Tools 3.47** inside the Windows XP guest. First transfer the BIN/CUE image files from the host into the XP guest, then mount the image with Daemon Tools.

## Project goals

Y2KBox exists to make preserving and revisiting the Windows XP-era computing experience easier. The goal is not simply to boot an old operating system: it is to provide a portable, reproducible, plug-and-play environment for the games and software people want to keep using, with the hardware behavior and compatibility those programs expect.

That means aiming for a complete **Windows XP experience in a box**: an SSE2-capable x86 environment with VMware SVGA/SVGA3D and the graphics capabilities associated with the era, including **Direct3D 8.1 and Direct3D 9.0c**. The longer-term vision is to make running this software feel as straightforward as using a modern emulator such as PCSX2: launch it, configure the guest once, and get on with the experience.

### Why not make a proprietary hypervisor API mandatory?

Modern CPUs provide hardware virtualization through technologies such as Intel VT-x and AMD-V. QEMU can expose that acceleration through host hypervisor interfaces—for example, **WHPX** on Windows and **Hypervisor.framework (HVF)** on macOS. Those interfaces are supplied by their respective operating-system vendors and are not open-source components that this project can independently maintain. Making one of them a hard requirement for CPU execution would put a fundamental part of Y2KBox's future in the hands of a platform vendor: its availability, behavior, compatibility, and long-term support would be outside the project's control.

Y2KBox therefore takes a different approach on Windows: use QEMU's **TCG software CPU emulator**, rather than requiring WHPX or another proprietary hypervisor interface. This costs performance and makes optimization work more demanding, but it keeps the CPU execution engine within the project’s open-source codebase and avoids making a vendor-controlled host API a prerequisite for running the guest. The same principle is why the Linux product uses **KVM**: it is an open-source Linux kernel virtualization interface, rather than a proprietary platform API. Linux KVM is currently mandatory for Linux builds; Windows builds use TCG. macOS/HVF is not a supported host target at present.

This is an engineering and preservation choice, not a claim that hardware virtualization is inherently undesirable. KVM remains the right fit for the Linux product, while Windows TCG gives Y2KBox a path that does not depend on WHPX being available or supported indefinitely. The trade-off is deliberate: prioritize an implementation the project can inspect, maintain, optimize, and carry forward over relying on a closed host interface for a core capability.

### Make preservation practical

People can sometimes buy original hardware cheaply, but owning the machine is only the beginning. It also means finding working components, maintaining aging hardware, dealing with storage and display quirks, and keeping the whole setup usable on modern systems. Emulation can remove much of that friction while adding conveniences that original hardware never offered: portability, snapshots, easy backups, configurable devices, repeatable environments, and integration with modern workflows.

Y2KBox aims to spare users from having to solve the same compatibility problems over and over inside the guest or on the host. Ideally, they should not need a chain of guest-side wrappers, per-game community patches, or fragile workarounds every time a host operating-system update breaks something. Nor should using the guest require users to modify the host's graphics-driver stack, install opaque kernel drivers, or edit system registries just to get an old game running.

The objective is straightforward: make the Windows XP and Y2K-era software experience **portable, maintainable, and accessible**—so preservationists can spend less time repairing the environment and more time preserving, exploring, and enjoying the software itself.

