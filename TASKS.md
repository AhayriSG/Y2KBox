# Y2KBox task queue

Several AIs/humans may work at the same time. **Claim a task before starting**:

1. `git pull --rebase origin main`
2. Edit the task's `Status` to `CLAIMED by <name/session> <date>` in this file,
   commit ("tasks: claim T05"), `git push origin main`.
   If the push is rejected, somebody else pushed first: `git pull --rebase`, re-read
   this file, pick another task if yours was taken.
3. Work on a branch `task/T05-short-name` (push it often - WIP must never be lost).
   Merge to `main` only when the *Definition of done* is met and the standard checks
   pass (HANDOFF.md section 7). Keep `main` always building.
4. When done: set `Status: DONE <date> (commit <hash>)`, add a line to WORKLOG.md,
   update the README status table, push.
5. **If you are about to run out of budget**: commit and push your branch, set the
   task to `PAUSED` with a precise "Resume here:" note (what is done, what is next,
   what is broken). Never leave a claimed task silent.

Status values: `TODO`, `CLAIMED by ...`, `PAUSED`, `DONE`. Priority: P0 first.

---

## T01 (P0) Fixed XP machine  -  Status: PAUSED (parts 1 and 2a done)
Goal: exactly one machine, no user-facing machine/CPU/RAM/SMP options.
* DONE part 1: default RAM 2 GiB, default 1 vCPU, `max_cpus` 2 (`hw/i386/pc.c`).
* DONE part 2a (`xp/strip/08-fixed-machine-memory.py`): minimal `pc_memory_init` (all RAM below 4 GiB,
  asserts above-4G == 0), memory-device/DIMM/NVDIMM/SP-MEM/HV-balloon hotplug handlers, CXL, device
  memory, `pc_max_used_gpa`, and in `acpi-build.c` the NVDIMM/memory-hotplug AML, CXL, SRAT device
  memory, HMAT, VIOT, VMGenID, VMClock, CEDT. `default.mak` now has DIMM, MEM_DEVICE, NVDIMM,
  ACPI_NVDIMM/HMAT/VIOT/CXL/VMGENID/VMCLOCK/MEMORY_HOTPLUG, HV_BALLOON_SUPPORTED = n
  (10 stale Kconfig `select` lines removed); 11 dead C files deleted. Checks green: e820 = one
  2 GiB entry, vectest OK, `-smp 2` boots, no pc-dimm/nvdimm devices.
* Resume here (part 2b): (1) delete the remaining ~18 versioned `pc-i440fx-X.Y` machine types in
  `hw/i386/pc_piix.c` + `hw/core/machine.c` compat arrays (`hw_compat_*`, `pc_compat_*` in
  `hw/i386/pc.c`/`include/hw/i386/pc.h`) and keep ONE machine type; (2) remove dead above-4G /
  hole64 / `max_ram_below_4g` / ram-slots/maxmem fields and code (`x86_machine_*`, `pc_init1`
  memory split in `hw/i386/pc_piix.c`, `hw/i386/x86.c`); (3) remove the `-m/-smp/-machine/-cpu`
  CLI paths (qemu-options.hx, system/vl.c, `-smp` parsing) now that defaults are fixed; (4) ACPI
  CPU hotplug AML (`build_cpus_aml`) is still needed for the CPU objects - keep it, drop the
  hotplug-only parts later; (5) re-run dead-source pass (HANDOFF 5b), standard checks.
* DoD: fresh configure+build; one machine in `-machine help`; standard checks (HANDOFF 7);
  `info mtree` shows 2 GiB below 4 GiB only.

## T02 (P0) XP guest CPU model: SSE2 maximum  -  Status: PAUSED (part 1 done)
* DONE: model `xp-p4` (Pentium 4 Northwood: family 15 model 2 stepping 9, SSE2, no SSE3+) added
  in `target/i386/cpu.c` and made the default (`TARGET_DEFAULT_CPU_TYPE` in `target/i386/cpu.h`).
  Verified in-guest with `xp/bench/cpuid.asm`.
* TODO: make it the ONLY model (delete the ~100 other models in `builtin_x86_defs`, `-cpu`
  option, CPU feature properties/ `max`/`host` types); then delete translator code for ISA the
  guest can never execute: SSE3, SSSE3, SSE4.1/4.2, AES/PCLMUL, AVX/AVX2/FMA/BMI, XSAVE,
  RDRAND, ... in `target/i386/tcg/` (decode-new.c.inc, emit.c.inc, translate.c, helpers);
  `gen_ldo_env_A0`/`gen_sto_env_A0` then always use pair atomicity. Keep x87/MMX/SSE/SSE2.
* DoD: build; CPUID shows no SSE3+; vectest OK; XP boot still works once T08 exists.

## T03 (P0) Hosts: Windows and Linux only  -  Status: PAUSED (pass 1 done, Linux-verified)
* Remove macOS/BSD/Solaris/Haiku/Emscripten/other-OS code: meson/configure branches,
  `util/` `ui/` `audio/` `net/` `block/` files for those OSes (cocoa, coreaudio, vmnet,
  kqueue, ...), headers, docs mentions.
* Verify BOTH hosts: native Linux build, and a Windows cross build
  (`x86_64-w64-mingw32-gcc`, see T07).
* DoD: both configure+build; no `darwin|freebsd|netbsd|openbsd|sunos|haiku` branches left.
* Done (xp/strip/07-linux-windows-hosts-only.py, Linux configure+build+checks green): cocoa,
  coreaudio, vmnet, apple-gfx/pvg, hw/vmapple, tap-bsd/solaris, os-wasm, emscripten.cfg;
  `supported_oses = ['windows','linux']`; macOS dependency blocks and the darwin/sunos/
  haiku/openbsd branches in the dependency and flags chains.
* Resume here: (1) remaining `host_os` dead branches/`bsd_oses` in meson.build (grep
  `darwin|bsd|sunos|haiku|emscripten`), configure (`host_os=` detection), util/, include/qemu/
  (`osdep.h` etc.), qapi/ (`@if CONFIG_DARWIN`...), chardev/, hw/9pfs, fsdev (macOS/BSD
  xattr code), system/, trace/ (dtrace/ftrace), options `cocoa/vmnet/coreaudio/pvg` in
  meson_options.txt; (2) **Windows build not verified** - see T07.

## T04 (P1) Inline PSHUFD / PSHUFLW / PSHUFHW / PUNPCK* / PACK* / SHUFPS  -  Status: TODO
Currently out-of-line helpers (`pshufd_xmm` ...). The x86 backend already has target
vector ops (`x86_shufps`, `x86_punpckl/h`, `x86_psrldq`, blend in
`tcg/x86_64/tcg-target-opc.h.inc`). Add inline lowering (or a new TCG shuffle op).
Measure with a new `xp/bench` loop (baseline: pshufd is a helper call).

## T05 (P1) Remove IOMMU / SEV / SGX / TDX / VFIO / KVM call-site leftovers  -  Status: TODO
Technique: see `xp/strip/05-remove-xen.py` (brace-matching helpers) + fresh build.
38 files mention IOMMU, 25 SEV/confidential, 20 SGX, 17 VFIO; also `kvm_enabled()`
branches, `include/system/kvm*.h`, `kvm_i386.h`, `hw/i386/kvm/clock.h`.

## T06 (P1) Remaining pruning  -  Status: TODO
Device models still compiled but unused, trace-events of deleted code, QAPI
schemas/commands for removed features, migration extras (COLO, postcopy, multifd -
keep savevm snapshots), vhost/vhost-user, block formats other than raw/qcow2/vvfat?,
net backends other than user/slirp(+socket?), `scripts/`, `python/`, `rust/`, `qga/`.
Use the dead-code technique in HANDOFF 5b. Re-run the boot + vectest checks.

## T07 (P1) Windows host build  -  Status: TODO
Cross-compile `qemu-system-i386.exe` and `qemu-img.exe` with mingw-w64
(`apt install mingw-w64`; deps via MSYS2 pkgs or a cross sysroot). Fix breakage caused
by pruning. Add a GitHub Actions workflow building Linux and Windows.

## T08 (P1) Boot Windows XP SP3 end to end  -  Status: TODO (needs an XP ISO from the owner)
Install to a qcow2 disk (`qemu-img create -f qcow2`), IDE CD-ROM, floppy; measure install time;
fix whatever breaks (ACPI HAL choice, SMP/APIC, TPR patching via kvmvapic option ROM,
RTC/PIT timing). Record the working command line in HANDOFF.

## T09 (P2) VBoxVGA device: VBE + HGSMI/VBVA  -  Status: TODO
Port from VirtualBox v5.2.x/v6.1.x (GPLv2; audit headers) `DevVGA*.cpp`, `HGSMI/`.
Start from QEMU `hw/display/vga*.c`, `vga-pci.c`, `vmware_vga.c` as templates.
Guest Additions 3.2.28 as bring-up baseline, then 5.2.x.

## T10 (P2) VMMDev + HGCM  -  Status: TODO
PCI 80ee:cafe, request protocol, HGCM service dispatch. Source: VirtualBox 5.2.x
`Devices/VMMDev`.

## T11 (P2) 3D host service  -  Status: TODO
Chromium-based `HostServices/SharedOpenGL` + `GuestHost/OpenGL` (exists in tag
v5.2.2, absent in 6.1.4). Port/replay GL stream on the host; consider a separate process.
Open question: Direct3D for XP guests vs OpenGL only (see HANDOFF 8).

## T12 (P2) Own front end: launcher + display  -  Status: TODO
Our own executable + wxWidgets GUI: pick a qcow2 disk (made with qemu-img), floppy image,
CD/ISO. **No RAM setting** (owner decision: always 2048 MB, automatic). Needs a display/input embedding path (DisplayChangeListener).

## T13 (P1) TCG performance for ALL guest instruction classes  -  Status: TODO
Scope (owner clarification): everything up to SSE2, not just SSE2. Suggested order - measure first
(extend `xp/bench` with protected-mode/ring-0 loops; keep a saved old binary for A/B; also
consider a real XP workload once T08 exists):
* flags/condition codes (lazy CC), integer ALU/shift/mul/div helpers, `rep movs/stos/cmps`
  string ops, `bswap/bt*`, `pushf/popf/cli/sti`;
* x87 (uses softfloat floatx80 helpers - likely very slow; XP-era apps use x87 heavily);
* softmmu: TLB fast-path size/associativity, page-crossing, segment-limit/privilege checks,
  A20/real-mode paths, `invlpg`, CR3 switches (XP context switches);
* control flow: TB chaining, indirect-jump lookup (`ret`/`call`), `sysenter/int 2e`, ring
  transitions, exceptions/interrupt delivery, `hlt` idle;
* system: PIT/RTC/HPET timers, `rdtsc`, I/O port dispatch, IDE PIO/DMA, VGA memory;
* keep XMM/MMX state in host registers across instructions; PSHUFD & shuffles inline (T04).
Every change: A/B numbers in WORKLOG + standard checks.

## T14 (P1) Threading model and thread pinning  -  Status: TODO
Define which host threads exist (vCPU, main loop, IO, audio, display/3D) and pin them
(vCPU thread to the fastest/performance core, others elsewhere) with Windows
(`SetThreadAffinityMask`/`SetThreadIdealProcessorEx`, priority) and Linux (`sched_setaffinity`)
APIs. Respect hybrid CPUs (P/E cores). Make it automatic (no user flags), with a safe fallback.
Measure with and without (1 vCPU default; 2-vCPU experimental mode if added).

## T15 (P2) Evaluate virtio for XP (opt-in)  -  Status: TODO
Owner idea: virtio-win 0.1.141 x86 floppy drivers for XP (VirtIO Ethernet, SCSI/block).
Re-enable `virtio-net`/`virtio-blk`/`virtio-scsi` (all virtio is OFF in `default.mak`; selects
may need restoring), install the drivers in an XP guest (needs T08) and benchmark disk/network vs
IDE + RTL8139 under TCG. Only adopt if clearly faster. Verify what that floppy really contains
for XP. RTL8139 stays the default.
