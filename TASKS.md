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

## T01 (P0) Fixed XP machine  -  Status: CLAIMED by Claude (session 1) 2026-11-14
Goal: exactly one machine, no user-facing machine/CPU/RAM/SMP options.
* Guest RAM **fixed at 2048 MB**, entirely below 4 GB: delete above-4G RAM, 64-bit PCI
  hole, `max_ram_below_4g`, ram-slots/maxmem logic in `hw/i386/pc.c` `pc_memory_init`.
* vCPUs = number of host threads (check the XP limit first, see HANDOFF 4); MTTCG on.
* Delete: memory hotplug, DIMM/NVDIMM/MEM_DEVICE, CXL, NUMA/HMAT, VIOT, VMGENID,
  VMCLOCK, ERST, ACPI CPU/memory hotplug, versioned `pc-i440fx-X.Y` machines
  (keep ONE machine type), then re-enable-off the matching `default.mak` lines
  (see the "NOTE: ... still ON" comment there) and delete newly dead sources.
* Definition of done: fresh configure+build; `qemu-system-i386 -machine help` shows
  one machine; SeaBIOS boots; `xp/bench/vectest.asm` prints OK; `info mtree` shows
  2 GiB RAM below 4 GiB; README/WORKLOG updated.

## T02 (P0) XP guest CPU model: SSE2 maximum  -  Status: TODO
* Add one CPU model (Pentium 4 / "Northwood"-like: FPU,VME,DE,PSE,TSC,MSR,PAE,MCE,CX8,
  APIC,SEP,MTRR,PGE,MCA,CMOV,PAT,PSE36,CLFLUSH,MMX,FXSR,SSE,SSE2) as the only/default
  model; remove user `-cpu` selection and all other models.
* Then delete translator code for ISA the guest can never execute: SSE3, SSSE3, SSE4.1/4.2,
  AES/PCLMUL, AVX/AVX2/FMA/BMI, XSAVE/XSAVEOPT, RDRAND... in
  `target/i386/tcg/` (decode-new.c.inc, emit.c.inc, translate.c, helpers). Make
  `gen_ldo_env_A0`/`gen_sto_env_A0` always use pair atomicity.
* DoD: build; XP-era SSE2 test passes; `-cpu` option gone; CPUID leaf 1 shows no SSE3+.

## T03 (P0) Hosts: Windows and Linux only  -  Status: CLAIMED by Claude (session 1) 2026-11-14
* Remove macOS/BSD/Solaris/Haiku/Emscripten/other-OS code: meson/configure branches,
  `util/` `ui/` `audio/` `net/` `block/` files for those OSes (cocoa, coreaudio, vmnet,
  kqueue, ...), headers, docs mentions.
* Verify BOTH hosts: native Linux build, and a Windows cross build
  (`x86_64-w64-mingw32-gcc`, see T07).
* DoD: both configure+build; no `darwin|freebsd|netbsd|openbsd|sunos|haiku` branches left.

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
Our own executable + wxWidgets GUI: pick a qcow2 disk (made with qemu-img), RAM (the
guest RAM is fixed at 2048 MB for now - revisit if the owner wants a slider, max 3.5 GB),
floppy image, CD/ISO. Needs a display/input embedding path (DisplayChangeListener).

## T13 (P2) TCG performance, round 2  -  Status: TODO
Keep hot XMM regs in host registers across instructions; x87 speed; softmmu TLB fast
path; MTTCG scaling for 2+ vCPUs; real workloads. Always A/B against a saved binary.
