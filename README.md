<div align="center">

<img width="272" height="281" alt="Y2KBox logo (alpha)" src="https://github.com/user-attachments/assets/af59c3c3-da2c-469f-84e0-1c605ce8d1e8" alt="Y2KBox Logo" width="200" />

# Y2KBox

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d0d7875bcbfb40428c030025425f775f)](https://app.codacy.com/gh/AhayriSG/Y2KBox/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![build](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml/badge.svg)](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/AhayriSG/Y2KBox/blob/main/LICENSE)
</div>

Y2KBox is a QEMU fork built to bring back the Windows XP gaming experience, with
3D acceleration and a performance target inspired by a mid-to-late 2000s PC.

The goal isn't to turn XP into a modern gaming powerhouse running everything at
1080p and 144 Hz. It's to deliver a reasonably fast, compatible virtual PC that
can run most games from the Windows XP era with the kind of performance and
experience you'd expect from the hardware of that time.

Y2KBox also isn't intended to be a cycle-accurate, low-level hardware emulator.
Projects such as 86Box already serve that cycle-accurate emulation purpose
exceptionally well for earlier PC eras, thanks to the dedication and talent of
their contributors.

**Contributors / coding agents:** start with [AGENTS.md](AGENTS.md), the shared
repository policy for Claude, Gemini, Codex, and other agents. For a fresh session,
read only the onboarding sections of [HANDOFF.md](HANDOFF.md), the task overview and
relevant entry in [TASKS.md](TASKS.md), [WORKLOAD_ACTIVE_SESSION.md](WORKLOAD_ACTIVE_SESSION.md),
and [WORKLOG_INDEX.md](WORKLOG_INDEX.md). Consult only relevant sections of the
append-only [WORKLOG.md](WORKLOG.md); never load the full archive by default.
[CLAUDE.md](CLAUDE.md) and [GEMINI.md](GEMINI.md) are short tool-specific entrypoints.
At handoff, append the session to the worklog and add its concise summary to the
rolling ten-session file. The Claude Code Stop hook enforces that summary; it does
not apply to other agents. Contributors work directly on `main`; coordinate task
claims and serialize commits.

## Fixed target

- **Guest OS:** 32-bit Windows XP.
- **Guest memory:** 2048 MiB, assigned automatically; there is no RAM setting in the
  current build.
- **Guest CPU:** xp-p4 (Pentium 4 Northwood class), with SSE2 as the maximum exposed
  instruction set; exactly 1 vCPU (uniprocessor). Multi-vCPU/SMP configurations are not
  supported.
- **Guest 3D APIs:** Direct3D 9.0c with Shader Model 3.0 as the maximum advertised
  shader model. OpenGL 1.1 is the baseline target, with OpenGL 2.1 as the maximum
  target. OpenGL support is not currently implemented and remains a **low-probability
  research objective**. The standard Windows XP VMware display-driver path appears
  to expose only Microsoft's GDI Generic software renderer. Alternative approaches,
  potentially including guest-side application-local wrapper DLLs, have not yet
  been proven viable. A different solution may exist and remains open to investigation.
  Direct3D 10/11, Shader Model 4.0 or newer, and OpenGL versions newer than 2.1
  are outside the guest-facing graphics target. Host-side rendering may use
  newer APIs internally.



| Windows XP SP3 Desktop (ACPI PC HAL)<br/>VMware SVGA II (Experimental) | F1 Challenge '99-'02 | Joint Operations: Typhoon Rising 1.4.3.0
| :---: | :---: | :---: |
| <img src="https://github.com/user-attachments/assets/adcae51d-6939-4ed8-8e61-9a98fa4d94ce" width="310" alt="Windows XP SP3 desktop running in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/c36efbb4-8e7e-457c-b253-115691552d9f" width="310" alt="VMware SVGA II in Y2KBox (Experimental)"> | <img src="https://github.com/user-attachments/assets/ee1ba790-3550-4cbe-a479-358a126f7378" width="310" alt="VMware SVGA II and F1 Challenge '99-'02 in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/034245a2-0ba9-4eb8-80d0-b4a3779c543e" width="310" alt="VMware SVGA II and F1 Challenge '99-'02  in Y2KBox"> | <img src="https://github.com/user-attachments/assets/59ad794a-ad31-49a6-a209-9beaef453098" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising  in Y2KBox"><br/><img src="https://github.com/user-attachments/assets/70f54da1-c64d-41ed-975f-5ea3da77173f" width="310" alt="VMware SVGA II and Joint Operations Typhoon Rising in Y2KBox">



## Download & Usage

Build-triggering CI runs upload Linux and Windows builds as Actions Artifacts.
Documentation-only commits that change only `.md` files intentionally skip the
Linux and Windows jobs entirely, so those runs produce no binaries or artifacts.
Any non-Markdown change and manual workflow dispatch run the full build and artifact
pipeline. Open the [latest successful build run](https://github.com/AhayriSG/Y2KBox/actions/workflows/build.yml)
and choose the latest successful run where the Linux and Windows jobs completed (a Markdown-only run shows those jobs as skipped), then download the artifact for your platform:

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
<br/>- Interactive display output uses **SDL** (`-display sdl`; VNC support is disabled, while `-display none` remains for headless checks). A Dear ImGui frontend integrated with SDL2 is planned (T12).
<br/>- On Linux, launch with KVM (`-accel kvm`) and make sure `kvm.enable_vmware_backdoor=1` is enabled on the host kernel module (`qemu-system-i386` warns at startup if `/sys/module/kvm/parameters/enable_vmware_backdoor` is disabled). Linux binaries require the
usual shared libraries from your distribution, including glib2, pixman,
libslirp, libfdt, SDL2, and Vulkan (`libvulkan1`, plus `mesa-vulkan-drivers`
for lavapipe software fallback).



### Windows XP guest recommendations

- **Display refresh rate:** Set the guest display to **maximum** for the selected resolution and VMware SVGA display mode. In XP, open Display Properties > Settings > Advanced > Monitor.
- **Mouse:** Y2KBox's supported pointing device is the PC machine's default **PS/2 IntelliMouse**; keep this device and do not attach USB mouse or tablet devices. USB absolute-coordinate/tablet and USB HID mouse paths can cause tracking, camera-recentering, or DirectInput compatibility problems in some XP-era games. In XP, open Control Panel > Mouse > Pointer Options and clear **Enhance pointer precision** to disable mouse acceleration.
- **Networking and retro web browsing:** The standard launch profile uses `-netdev user,id=net0 -device virtio-net-pci,netdev=net0`. Windows XP needs the XP x86 **NetKVM** driver from `virtio-win-0.1.141_x86.vfd`; install it in the guest before relying on VirtIO networking. RTL8139 remains available as an independent fallback with `-nic user,model=rtl8139` and uses XP's in-box driver. To browse [Protoweb](https://protoweb.org/wiki/servers/) from Internet Explorer, open Tools > Internet Options > Connections > LAN Settings, enable the proxy server, and enter **wayback.protoweb.org** with port **7851** for the current primary HTTP proxy. Protoweb periodically changes its server list, so check its [current server and proxy settings](https://protoweb.org/wiki/servers/) if the address or port stops working; alternate servers and optional automatic proxy configuration are listed there.
- **CD-ROM and CD images (BIN/CUE):** The emulated CD-ROM stays on the PIIX IDE controller by default (`-drive id=cdrom,if=ide,media=cdrom,readonly=on`), independently of the system-disk controller. Its slower optical-drive behavior and direct control of the emulated CD device are intentional; VirtIO Block does not replace this CD-ROM path. A future frontend option (T12) may offer a VirtIO SCSI CD-ROM as an alternative, but that requires restoring and validating the VirtIO SCSI device and XP driver path first; it is not enabled in current builds. Until Y2KBox's emulated CD-ROM supports the full BIN/CUE format—including the complete set of CUE flags (such as `DCP`)—and supports inserting/ejecting disc images in the emulated drive for multi-disc swapping, we recommend using **Daemon Tools 3.47** inside the Windows XP guest. First transfer the BIN/CUE image files from the host into the XP guest, then mount the image with Daemon Tools.

## FAQ

### What are the host requirements?

- **CPU ISA:** x86-64-v3 is the minimum. Older ISA levels are not supported.
- **Windows:** Windows 10 version 1803 (build 17134) or newer is strongly recommended. Version 1803 is the baseline we are targeting for future Windows TCG memory-management work, including potential use of newer virtual-memory APIs. It may become a formal minimum as those optimizations mature. Earlier Windows versions are not the target for future compatibility work.
- **Linux:** KVM is mandatory, and the user must have permission to access `/dev/kvm`. There is no supported Linux TCG fallback. Because guest VMware SVGA/vmport backdoor calls go through KVM, enable the host kernel parameter `kvm.enable_vmware_backdoor=1` (for example, add `options kvm enable_vmware_backdoor=1` to `/etc/modprobe.d/kvm.conf` and reload `kvm` / reboot; verify with `cat /sys/module/kvm/parameters/enable_vmware_backdoor`).

### How do I capture diagnostic logs and performance information?

Diagnostic logging is an opt-in tester feature and is **off by default**.
Choose the diagnostic level with `-y2kboxdebugmode off|simple|on` (the
default is `off`):

- `off`: diagnostics disabled; Ctrl+Alt+L is unavailable.
- `simple`: compact capture with bounded samples of repetitive VMVGA events
  and interval summaries. It preserves explicit warnings for malformed or
  unknown commands, rejected operations, invalid register accesses, allocation
  failures, stalls, and other important state changes.
- `on`: Full logging, with the selected per-event VMVGA SYNC, fence, and
  3D-command stream, including CPU-shadow vertex-position and UV bounds for
  supported float vertex formats.

Simple mode samples the first four SYNC writes/results, fence events, and
FIFO-run-end records, plus the first eight selected 3D-command records. It also
writes interval-scoped VMVGA flight-recorder totals when capture stops.
The `Y2KBOX-PERF command` records retain per-command counts and timing
measurements; sampling repetitive event lines does not discard command-level
performance data. Simple mode is intended to reduce routine log volume, not
to hide warnings or errors. Some malformed FIFO packets are reported through
`VMVGA-FIFO-STALL` diagnostics.

With debug mode enabled, the SDL title advertises **Ctrl+Alt+L**. Press it to
start a capture in a new timestamped
`y2kbox-log-YYYYMMDD-HHMMSS.log` file in the launch working directory.
Existing files are never overwritten, and the VM continues running. Each
capture starts with fresh `Y2KBOX-PERF` counters and timing baselines; it does
not restart the VM or guest state.

- **Simple capture** continues until Ctrl+Alt+L is pressed again. It remains a
  plain-text `.log` file.
- **Full capture** automatically stops after two seconds. Once started, it
  cannot be manually stopped with Ctrl+Alt+L; the hotkey is not a cancel
  command during Full capture. The SDL title indicates the active two-second
  capture.

When Full capture stops, Y2KBox attempts to save one PNG screenshot of the
guest display surface, using the same filename as the log with a `.png`
extension. This is a CPU-display-surface snapshot, not a sequence of frames.
The PNG is unavailable in builds configured without libpng or Pixman, and a
snapshot can also fail; check the log's `Y2KBOX-DIAG pixel_snapshot_result`
record. Full logs are then compressed asynchronously by a background worker
using zlib compression level 9 and saved as `.log.gz`. The original `.log`
is removed only after compression succeeds; if compression fails, it is
preserved. The PNG remains alongside the compressed log.

The hotkey enables the relevant VMVGA, VMPort GuestRPC, vdagent, and
performance diagnostics only for the selected capture interval. When debug mode
is enabled but capture is inactive, these diagnostic streams remain disabled.
No separate `-trace "vdagent*"` option is needed. Do **not** use shell-level
`2> y2kbox-log.log` redirection with this feature: it redirects stderr before
QEMU starts and defeats interval capture.

**Performance guidance:** Recording diagnostics adds overhead from timestamp
collection, counter updates, and diagnostic output. For performance analysis
and comparisons, use **only `PERF STATS` from Simple mode** as the reference.
Full mode enables substantially more instrumentation and can affect timing, so
ignore its `PERF STATS` when evaluating normal emulation performance. Full
logs are for detailed technical investigation—event traces and state snapshots
can help diagnose graphics or rendering problems, but Full-mode performance
statistics are not a benchmarking reference. For comparisons, reproduce the
same game scene, resolution, settings, and similar capture duration.

**Helping optimize games:** If a game runs poorly, share the Simple-mode log
and describe the game, scene, resolution, settings, and reproduction steps.
Developers or an LLM-assisted coding agent can analyze the measurements and
add targeted profiling counters if the existing statistics do not cover the
suspected bottleneck. Performance counters help locate execution cost, but
visual glitches may require Full-mode logs, the PNG screenshot, or additional
targeted diagnostics.

**Advanced graphics diagnostics:** Developers investigating VMware SVGA graphics
issues can optionally enable `VMVGA_TRACE_DEEP_GB_UPDATE` at compile time to
capture detailed successful GB-UPDATE operations. This trace is **disabled by
default** because it can generate substantial logging volume. It is intended
for targeted graphics investigations, not ordinary tester logging; the regular
diagnostic hotkey does not require it.

### Which version of QEMU is Y2KBox based on?

Y2KBox is based on upstream QEMU **11.1.90**, at commit [`9d7d057a558b9a1c232cea390b4481dd20d9c9ce`](https://github.com/qemu/qemu/commit/9d7d057a558b9a1c232cea390b4481dd20d9c9ce). The upstream QEMU license remains **GPL-2.0**, unchanged; see the repository's `COPYING` and `LICENSE` files.

### Why is Y2KBox limited to one vCPU?

Some Windows XP-era PCs had multiple processor cores, so a dual-core configuration would certainly be period-appropriate. However, Y2KBox deliberately targets a fixed, single-vCPU configuration.

With QEMU's TCG software CPU emulator, running multiple guest vCPUs introduces additional coordination and synchronization between those virtual CPUs. Cross-vCPU operations and shared-state coordination can add overhead, potentially reducing performance in games that primarily depend on one main thread. That overhead is workload-dependent; it does not mean that two vCPUs are always slower, or that dual-core XP systems were unrealistic.

The decision is about Y2KBox's intended configuration and performance target. Rather than introduce a multi-vCPU option with additional complexity and potentially inconsistent results, the project focuses on making its single-vCPU environment as fast and compatible as possible.

Also, **one guest vCPU does not mean QEMU is restricted to one host thread**. QEMU can still use separate host threads for its main loop, I/O, audio, and rendering.

### Why not make a proprietary hypervisor API mandatory?

Modern CPUs provide hardware virtualization through technologies such as Intel VT-x and AMD-V. QEMU can expose that acceleration through host hypervisor interfaces—for example, **WHPX** on Windows and **Hypervisor.framework (HVF)** on macOS. Those interfaces are supplied by their respective operating-system vendors and are not open-source components that this project can independently maintain. Making one of them a hard requirement for CPU execution would put a fundamental part of Y2KBox's future in the hands of a platform vendor: its availability, behavior, compatibility, and long-term support would be outside the project's control.

Y2KBox therefore takes a different approach on Windows: use QEMU's **TCG software CPU emulator**, rather than requiring WHPX or another proprietary hypervisor interface. This costs performance and makes optimization work more demanding, but it keeps the CPU execution engine within the project's open-source codebase and avoids making a vendor-controlled host API a prerequisite for running the guest. The same principle is why the Linux product uses **KVM**: it is an open-source Linux kernel virtualization interface, rather than a proprietary platform API. Linux KVM is currently mandatory for Linux builds; Windows builds use TCG. macOS/HVF is not a supported host target at present.

This is an engineering and preservation choice, not a claim that hardware virtualization is inherently undesirable. KVM remains the right fit for the Linux product, while Windows TCG gives Y2KBox a path that does not depend on WHPX being available or supported indefinitely. The trade-off is deliberate: prioritize an implementation the project can inspect, maintain, optimize, and carry forward over relying on a closed host interface for a core capability.

### Why is the current guest RAM target 2 GiB?

The 2048 MiB default is a Y2KBox product decision, not a hard 2 GiB physical-memory
limit in 32-bit Windows XP. 32-bit XP can address up to 4 GiB of physical address
space, although PCI devices and other memory-mapped hardware can reserve part of that
range, leaving less than 4 GiB available to the guest OS. A future GUI option for
4 GiB of guest RAM may be considered; it is not available today.

The familiar **2 GiB user / 2 GiB kernel split is virtual address space, not a split
of installed RAM**. By default, each 32-bit XP process gets up to 2 GiB of user-mode
virtual address space, while the other 2 GiB is reserved for the system. Installing
more guest RAM does not by itself increase the address space available to a game.

On supported 32-bit Windows XP Professional installations, the /3GB Boot.ini
switch can change the split to 3 GiB for user mode and 1 GiB for the kernel. The
game executable must also have the LARGEADDRESSAWARE flag set to use the extra
user address space. This is an application-specific option, not a general
recommendation: reducing kernel virtual address space can affect drivers and other
system resources, so compatibility must be tested.

More physical RAM can still help Windows keep working data resident, reduce paging,
and cache file data, depending on the workload. It does not automatically become
a game-owned extra gigabyte, and it does not guarantee that a game will stop
stuttering.

### Why build a Windows XP emulator when compatibility layers already exist?

Compatibility layers such as Wine and various game-specific wrappers are valuable tools for running older Windows software on modern operating systems. They solve a different problem, though: adapting applications to work without running the original Windows environment.

Y2KBox takes a different approach. It provides an actual Windows XP environment, including its operating-system behavior, virtual hardware, guest drivers, and graphics stack. This matters for software that expects XP-specific behavior or interacts with hardware and drivers in ways that compatibility layers may not reproduce consistently.

The goal is not to replace Wine, wrappers, or community compatibility projects. Those solutions can be excellent choices for individual games. Y2KBox exists for people who want a more complete, self-contained Windows XP environment that they can preserve, configure once, and return to later.

Owning original hardware is only the beginning of preservation. It also means finding working components, maintaining aging hardware, dealing with storage and display quirks, and keeping the whole setup usable on modern systems. Emulation can remove much of that friction while adding conveniences that original hardware never offered: portability, snapshots, easy backups, configurable devices, repeatable environments, and integration with modern workflows.

Y2KBox aims to spare users from having to solve the same compatibility problems over and over inside the guest or on the host. Ideally, they should not need a chain of guest-side wrappers, per-game community patches, or fragile workarounds every time a host operating-system update breaks something. Nor should using the guest require users to modify the host's graphics-driver stack, install opaque kernel drivers, or edit system registries just to get an old game running.

Y2KBox aims to provide a complete **Windows XP experience in a box**: an SSE2-capable x86 environment with VMware SVGA/SVGA3D and the graphics capabilities associated with the era, including **Direct3D 8.1 and Direct3D 9.0c**.

The longer-term vision is to make running this software feel as straightforward as using a modern emulator such as PCSX2: launch it, configure the guest once, and get on with the experience.

The objective is straightforward: make the Windows XP and Y2K-era software experience **portable, maintainable, and accessible**—so preservationists can spend less time repairing the environment and more time preserving, exploring, and enjoying the software itself.
