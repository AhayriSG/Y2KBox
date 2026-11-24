# Y2KBox task queue

Different coding agents may take over sequentially; this shared task queue is the handoff point. Concurrent contributors are supported only when deliberately coordinated. **Claim an agent task before starting**:

1. Fetch the latest `main` and re-read this file before choosing work.
2. Edit the task's `Status` to `CLAIMED by <name/session> <date>` in this file,
   then commit and push directly to `main`. If another contributor moves `main`
   first, stop, fetch the new HEAD, re-read the queue, and coordinate before proceeding.
3. Work directly on `main`; do not create branches or pull requests. Before every
   write, fetch the latest `main` and the exact file being changed. Verify that both
   the branch HEAD and file blob SHA still match what you inspected; if either moved,
   abort that write, fetch again, and reapply the change to the new content. Commit
   and push small, coherent increments. Never force-push. Keep `main` building.
   (HANDOFF.md section 7: Gemini 4 Neon may perform ordinary local builds and
   relevant, bounded benchmarks/A-B; other agents need explicit authorization.
   Do not poll CI.)
4. When done: set `Status: DONE <date> (commit <hash>)`, add the evidence and
   result to `WORKLOG.md`, and push to `main`. Update `README.md` only if a
   user-facing requirement, setup step, or project fact changed; it has no task table.
5. **If you are about to run out of budget**: commit and push the current safe state
   to `main`, set the task to `PAUSED` with a precise "Resume here:" note (what is
   done, what is next, what is broken). Never leave a claimed task silent.

Status values: `TODO`, `CLAIMED by ...`, `IN PROGRESS`, `IMPLEMENTED` (implementation complete; acceptance evidence pending), `PAUSED`, `DONE`. `IMPLEMENTED` tasks have no known source work remaining and are not available to claim; finish their stated verification before marking them `DONE`. Priority: P0 first.

**Visual heading markers (human aid only):** `🔴` = not started (`TODO`); `🟡` = partial, claimed, in progress, paused, or implemented with verification pending (`CLAIMED`, `IN PROGRESS`, `PAUSED`, `IMPLEMENTED`); `🟢` = fully completed and verified (`DONE`). Keep the full canonical status in the plain-text `**Status:**` field immediately below every task heading. The heading marker and its label are redundant display aids, never the source of truth; agents must parse and update the `Status` field. In particular, `IMPLEMENTED` remains yellow until acceptance evidence is complete and the task becomes `DONE`.

---

## T01 (P0) Fixed XP machine — 🟡 PAUSED
**Status:** PAUSED (parts 1, 2a, 2b done)
Goal: exactly one machine, no user-facing machine/CPU/RAM/SMP options.
* DONE part 1: fixed RAM 2 GiB and exactly 1 vCPU, with `default_cpus = max_cpus = 1` (`hw/i386/pc.c`). The QEMU `-smp` CLI option and its parser have been removed; multi-vCPU/SMP configurations are unsupported.
* DONE part 2a (`xp/strip/08-fixed-machine-memory.py`): minimal `pc_memory_init` (all RAM below 4 GiB,
  asserts above-4G == 0), memory-device/DIMM/NVDIMM/SP-MEM/HV-balloon hotplug handlers, CXL, device
  memory, `pc_max_used_gpa`, and in `acpi-build.c` the NVDIMM/memory-hotplug AML, CXL, SRAT device
  memory, HMAT, VIOT, VMGenID, VMClock, CEDT. `default.mak` now has DIMM, MEM_DEVICE, NVDIMM,
  ACPI_NVDIMM/HMAT/VIOT/CXL/VMGENID/VMCLOCK/MEMORY_HOTPLUG, HV_BALLOON_SUPPORTED = n
  (10 stale Kconfig `select` lines removed); 11 dead C files deleted. Checks green: e820 = one
  2 GiB entry, vectest OK, no pc-dimm/nvdimm devices. Historical pre-restriction testing showed `-smp 2` could boot; that is not supported by the current product target.
* DONE part 2b (`xp/strip/09-single-machine-type.py`): only `pc-i440fx-11.2` (alias `pc`) remains; 88 compat
  tables (`pc_compat_*`, `hw_compat_*`) deleted. `-machine help` = pc + none.
* DONE part 2c-1: removed dead above-4G / `max_ram_below_4g` / `pci_hole64` / `gigabyte_align` /
  `enforce_amd_1tb_hole` / `pc_init1` memory-split code across `hw/i386/{pc,pc_piix,x86,acpi-build}.c`,
  `hw/pci-host/i440fx.c`, and `include/hw/i386/{pc,x86}.h`.
* Resume here (part 2c-2): remove remaining `ram-slots`/`maxmem` fields if unused; (3) remove the remaining `-m/-machine/-cpu`
  CLI paths now that defaults are fixed (`-smp` has already been removed); (4) rename the machine to something like `xp` (drop the version suffix); (5) ACPI
  CPU hotplug AML (`build_cpus_aml`) is still needed for the CPU objects - keep it, drop the
  hotplug-only parts later; (6) re-run dead-source pass (HANDOFF 5b), standard checks.
* DoD: fresh configure+build; one machine in `-machine help`; standard checks (HANDOFF 7);
  `info mtree` shows 2 GiB below 4 GiB only; the machine exposes exactly one CPU and does not accept `-smp`.

## T02 (P0) XP guest CPU model: SSE2 maximum — 🟡 PAUSED
**Status:** PAUSED (parts 1 and 2a done)
* DONE: model `xp-p4` (Pentium 4 Northwood: family 15 model 2 stepping 9, SSE2, no SSE3+) added
  in `target/i386/cpu.c` and made the default (`TARGET_DEFAULT_CPU_TYPE` in `target/i386/cpu.h`).
  Verified in-guest with `xp/bench/cpuid.asm`.
* DONE part 2a (`xp/strip/10-only-xp-p4-cpu.py`, `10b-drop-unused-statics.py`): `builtin_x86_defs` holds only
  `xp-p4` (46 other models deleted) and 17 now-unused cache/version tables; `cpu.c` 11085 -> 6205 lines;
  `-cpu help` = xp-p4, base, max. CPUID output unchanged.
* Resume here (part 2b): delete the `max`/`base` CPU types and the `-cpu` option / CPU feature-property
  machinery (`x86_cpu_...` feature words, `FeatureWordInfo` tables for features we can never expose);
  then delete translator code for ISA the
  guest can never execute: SSE3, SSSE3, SSE4.1/4.2, AES/PCLMUL, AVX/AVX2/FMA/BMI, XSAVE,
  RDRAND, ... in `target/i386/tcg/` (decode-new.c.inc, emit.c.inc, translate.c, helpers);
  `gen_ldo_env_A0`/`gen_sto_env_A0` then always use pair atomicity. Keep x87/MMX/SSE/SSE2.
* DoD: build; CPUID shows no SSE3+; vectest OK; retain the owner-verified XP SP3 boot baseline.

## T03 (P0) Hosts: Windows and Linux only — 🟡 PAUSED
**Status:** PAUSED (pass 1 done; Linux + Windows CI verified)
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
  meson_options.txt; (2) Windows build is verified by CI (T07).

## T04 (P1) Inline PSHUFD / PSHUFLW / PSHUFHW / PUNPCK* / PACK* / SHUFPS — 🟢 DONE
**Status:** DONE 2026-11-18 (implementation 746dae5; benchmark recorded df28ec6)
Legacy XMM forms of PSHUFD/PSHUFLW/PSHUFHW, PUNPCK*, PACK* and SHUFPS now
have inline native x86 TCG lowering. MMX and VEX/YMM forms retain their helper paths.
Dedicated boot-sector functional tests run in Linux/Windows CI:
`xp/bench/shuffletest.asm` and `xp/bench/packunpacktest.asm`.
CI validation: PASS on Linux and Windows (run 44156398064); both
`shuffletest.asm` and `packunpacktest.asm` print `OK` on both hosts.
The Linux CI workflow downloads the pre-T04 baseline artifact (run
44148654097) and compares the raw runtime of one identical 300M-iteration PSHUFD
image on the baseline and current binaries, on the same runner. Each result is the
median of five successful runs. No empty-loop subtraction is used: TCG translation
and dispatch make that a different workload, not an additive overhead. The earlier
negative net times were invalid and are not a performance result. The runner rejects
unexpected QEMU exit codes, timeouts and stderr output. Linux CI benchmark result (AMD EPYC 9V74, same runner/build environment):
baseline median 3.083675 s; T04 median 3.166926 s; relative change -2.70%
(0.974x). This single isolated dependent-PSHUFD microbenchmark shows no
measurable improvement; the small difference may be within shared-runner noise
and is not evidence of a broad regression. Linux result recorded; Windows
functional CI remains independent and is not required for this Linux measurement.

## T05 (P1) Remove IOMMU / SEV / SGX / TDX / VFIO call-site leftovers (preserve KVM) — 🟢 DONE
**Status:** DONE 2026-11-19
* Removed PC-machine IOMMU hotplug registration/state and the PC ACPI DMAR/IVRS
  table builders. Removed the unused AMD-IOMMU stub wiring and its Kconfig options.
  The generic x86 IOMMU stub/interface remains because the KVM split-irqchip IOAPIC
  path still consults it; with no IOMMU device configured it resolves to no device.
* Removed SEV kernel-loader hash bookkeeping, the SEV pflash encryption hook, and
  unused OVMF SEV metadata declarations. Generic CPU CPUID feature gating remains.
* Removed the TDX TDVF-region setup call. Firmware in guest-private memory still
  takes the direct-load path, now selected by
  `machine_require_guest_memfd_private()` rather than SEV/TDX feature predicates.
  The disabled `is_tdx_vm()` predicate remains only for shared CPU feature gating.
* Removed SGX EPC DSDT/SRAT generation and the unused PCMachineState EPC member.
  SGX capability-query and CPUID stubs remain to keep unsupported features masked.
* Removed the dead VFIO compatibility property and its `hw/vfio/types.h` include
  from `pc_piix.c`; VFIO remains disabled in the XP device configuration.
* KVM implementation and required interfaces were not removed by T05. Linux KVM
  restoration remains the separate T17 task.

## T06 (P1) Remaining pruning — 🔴 TODO
**Status:** TODO
Device models still compiled but unused, trace-events of deleted code, QAPI
schemas/commands for removed features, migration extras (COLO, postcopy, multifd -
keep savevm snapshots), vhost/vhost-user, block formats other than raw/qcow2/vvfat?,
net backends other than user/slirp(+socket?), `scripts/`, `python/`, `rust/`, `qga/`.
Use the dead-code technique in HANDOFF 5b. Re-run the boot + vectest checks.

## T07 (P1) Windows host build — 🟢 DONE
**Status:** DONE 2026-11-14 (CI green at 92dbf22, run 43695682586)
GitHub Actions builds and checks Linux (ubuntu-24.04) and Windows (MSYS2 UCRT64): configure,
build `qemu-system-i386[.exe]` + `qemu-img[.exe]`, accelerator/device checks, functional tests,
and the x87 differential test. Windows needed `pc-bios/qemu-nsis.ico` restored (92dbf22).
The current workflow skips both platform jobs for Markdown-only commits; any non-Markdown
change and manual dispatch run the full platform checks and artifact pipeline. The Claude
status hook reports the HEAD run; agents must not poll CI after pushing.

## Owner-run XP guest validation (manual checklist; not an AI-agent task)

XP SP3 boot, desktop, and game launch are already owner-verified. The remaining
checks require a person operating the guest and judging real application behavior;
they are deliberately not claimable or independently completable by coding agents.
Agents may prepare test steps, inspect supplied logs/captures, and analyze reported
results, but must not mark these checks complete without the owner's confirmation.

- Install VMware Tools 10.0.12 (`winPreVista.iso`), selecting the XP SVGA driver
  12.01.08; record any installation or driver issues.
- Confirm 2D/display behavior, resolution changes, mouse integration, and D3D9
  acceleration in representative XP applications.
- Check D3D8 and DirectDraw/Direct3D 7 coverage (including `d3d8to9` or a
  DirectDraw wrapper where useful), shader limits, and representative game behavior.
- Check actual Windows-host visual correctness and performance in the owner's
  representative workloads. Distinguish owner observations from CI/build evidence.
- Record the exact guest setup, application/game, scene, host, and observed result
  in `WORKLOG.md`; update this note only when the owner supplies new evidence.

## T09 (P2) 3D: integrate qemu-vmvga (VMware SVGA II + SVGA3D) — 🟢 DONE
**Status:** DONE 2026-11-24 (integration committed; CI-verified; runtime acceptance tracked separately)
PRIMARY 3D plan (owner proposal 2026-11-14, evaluated and agreed; see HANDOFF 8a).
Repo: https://github.com/qemus/qemu-vmvga (GPL-2.0-or-later, 44-file overlay for QEMU 11.1.x: device
`vmvga`, SVGA3D vgpu9/vgpu10/vgpu11, DXVK-based rendering, `hw/i386/vmport.c`).
* **T09 integration and CI scope is complete.** XP driver, display/2D behavior, D3D API, and game compatibility acceptance belongs to the owner-run XP guest validation checklist above; it is not an unfinished AI-agent task.
  * Overlaid `hw/display/vmware_vga*.c`, `hw/display/include/*`, and `hw/i386/vmport.{c,-vmvga.h}`;
    restored `hw/i386/vmmouse.c` and `pc-bios/vgabios-vmware.bin` from `35a2e66`.
  * Wired `hw/display/meson.build`, `hw/i386/meson.build`, `pc-bios/meson.build`, `hw/pci/pci.c`
    (`VGA_VMWARE` -> `"vmvga"`), `hw/i386/pc_piix.c` (`m->default_display = "vmware"`), and
    `system/vl.c` (`VGA_VMWARE` accepts `"vmvga"` and `"vmware-svga"`).
  * Set `vmvga` as the default and only display device in `configs/devices/i386-softmmu/default.mak`
    (`CONFIG_VGA_CIRRUS=n`, `CONFIG_VGA_PCI=n`, `CONFIG_VMWARE_VGA=y`, `CONFIG_VMMOUSE=y`, `CONFIG_VMPORT=y`).
  * Added `vmport_tcg_backdoor_enabled` (`system/globals.c`, `include/system/system.h`, set in
    `vmport_realizefn`) and exempted ports `0x5658`/`0x5659` (`size <= 4`) in `HELPER(check_io)`
    (`target/i386/tcg/system/seg_helper.c`).
  * Added `xp/bench/vmport_ring3_test.asm` (CPL=3, IOPL=0, 0xFF TSS IOPB test for `0x5658`/`0x5659`
    vs `#GP` on `0x565A`) and Linux/Windows CI checks (`-vga help`, `-device help`, `vmport_ring3_test.asm`).
* **Completion:** no further T09 integration work is pending. The owner-run validation checklist above is the separate place to record ongoing XP driver, display/2D, D3D API, and game-compatibility observations. Do not repeat completed integration/build checks unless the source changes.

## T10 (P2) 3D host backend: Linux (DXVK-native + Vulkan) and Windows — 🟡 PAUSED
**Status:** PAUSED (integration committed; owner-run guest validation pending)
qemu-vmvga renders SVGA3D through DXVK (README: Vulkan-capable GPU + `dxvk-native` on the host).
* **Integration implemented and CI-verified;** live guest rendering and API compatibility require the owner-run XP guest validation note above.
  * **Linux:** `vmware_vga_dxvk_wsi.c` synthesizes a sealed-memfd `libSDL2-2.0.so.0` WSI shim (`"SDL2"`),
    or `libSDL3.so.0` (`"SDL3"`) implementing the 21 `wsi_platform_sdl3_funcs.h` symbols when QEMU has
    already loaded SDL2 (`--enable-sdl`). `vmware_vga_dxvk.c` searches `<exec_dir>/`, `<exec_dir>/lib/`,
    then system paths, pre-loading `libdxvk_dxgi.so.0` (`RTLD_GLOBAL`). CI installs `libvulkan1` +
    `mesa-vulkan-drivers` (lavapipe), packages `dxvk-native` 3.1.1 `.so.0` libs into `dist/`, and runs
    a headless 3D smoke check (`Y2KBOX_3D_HEADLESS=1` with `lvp_icd.x86_64.json`).
  * **Windows (`CONFIG_WIN32`):** Uses a hidden `WS_POPUP | WS_DISABLED` `"STATIC"` `HWND` in
    `vmware_vga_dxvk_wsi.c` (`"Win32"`) and `LoadLibraryA`/`GetProcAddress`/`FreeLibrary` on `"d3d9.dll"` /
    `"d3d11.dll"` (supporting inbox Windows Direct3D with `D3D_DRIVER_TYPE_WARP` fallback on D3D11, as
    well as optional DXVK Windows DLLs placed beside `qemu-system-i386.exe`).
  * **Headless benchmark safety:** `vmsvga3d_renderer_realize()` skips DXVK/D3D init under `-display none`
    (via `display_backend_active` in `system/vl.c`) unless Y2KBox debug mode (`-y2kboxdebugmode on`) or `Y2KBOX_3D_HEADLESS` is set,
    preserving empty `stderr` and zero overhead in non-3D CI benchmarks.
* **Resume here:** use the owner-run XP guest validation note above for live guest rendering and D3D API checks. CI's
  headless smoke check does not prove Windows-host visual correctness or game compatibility;
  render-thread pinning remains T14.

## T16 (P3, frozen alternative) VBoxVGA 5.2.x path — 🔴 TODO
**Status:** TODO (not planned; kept as fallback)
If qemu-vmvga proves unworkable on XP: VBoxVGA (VBE dispi + HGSMI/VBVA) + VMMDev (PCI 80ee:cafe) + HGCM +
the Chromium-based 3D host service from VirtualBox v5.2.x (GPLv2; the service is absent from 6.1.4).
Research notes: HANDOFF 8b. Sub-pieces if ever started: T16a VBoxVGA+HGSMI/VBVA, T16b VMMDev+HGCM,
T16c GL stream receiver/replay (consider a separate process).

## T12 (P2) Own front end: launcher + display — 🔴 TODO
**Status:** TODO
Our own executable with an **SDL2 + Dear ImGui** frontend, using Xemu's `ui/xui/` as an architectural reference. Dear ImGui is the primary GUI choice; wxWidgets is a fallback alternative, not a parallel implementation target. Keep QEMU's existing **SDL2 display backend**: do not migrate to SDL3 or replace the VMVGA renderer, display path, fullscreen handling, or mouse/input behavior as part of frontend work. Reuse Xemu's relevant frontend patterns, but do not import its Xbox-specific systems or unnecessary dependencies wholesale. Keep the dependency footprint and SDL2/QEMU event-loop integration complexity as explicit design considerations; begin with Dear ImGui core and its SDL2 integration, adding components only when Y2KBox needs them. The only interactive display/windowing backend we plan to support is **SDL2**; do not spend time supporting VNC or `-display none` as user-facing display choices (`-display none` remains useful for headless CI/checks).

The launcher should act as an **instance manager**, not a chooser among application-defined machine presets. Users should be able to create and manage multiple independent Windows XP VM instances. During instance creation, the user gives the instance a name and chooses its virtual hardware, including the virtual GPU when more than one supported GPU backend exists. Y2KBox stores each instance's configuration independently and associates it with that instance's own system qcow2 disk image and guest installation. The instance list should show the user's instances and their relevant configuration; it must not be a hard-coded list of vendor/demo profiles. Users may create as many instances as their host storage and resources reasonably allow.

**Per-instance virtual GPU direction:**
* **VMware SVGA II / SVGA3D:** the existing VMVGA device and VMware XP guest-driver stack. This remains the currently supported/default graphics path and the Direct3D 9-focused option. Existing instances using it must continue to work independently of experimental backends.
* **VBoxVGA 5.2.x / SharedOpenGL (proposed, unproven):** a possible additional virtual GPU choice for newly created instances, paired with the matching VirtualBox 5.2.x Guest Additions/driver stack and Chromium-based SharedOpenGL host service. It is a research candidate, not an implemented Y2KBox backend. VirtualBox 5.2.x documentation describes experimental Windows XP guest 3D support, but that does not establish that its device/protocol/host-service stack has been ported to or works in Y2KBox. Keep T16's research and feasibility gates; do not advertise or offer this GPU as selectable until a minimal end-to-end proof of concept succeeds and licensing/source compatibility have been audited.
* The virtual GPU is an instance-level hardware choice, not a global application setting or a fixed preset. Do not silently change the GPU of existing instances when another backend is added. Preserve each instance's selected hardware configuration.

The launcher should let users select a qcow2 disk (made with qemu-img), floppy image, and CD/ISO for an instance. Add guest CD-ROM **insert/eject** controls so users can swap disc images in the emulated drive for multi-disc games. The initial frontend keeps RAM fixed at 2048 MB, automatic, with no RAM setting; possible later memory presets are tracked separately in T23. Add a **Debug** On/Off setting, default **Off**; it should enable Y2KBox's tester diagnostics without enabling continuous output. This maps to the `-y2kboxdebugmode on|off` command-line setting; keep it separate from the `-device vmvga` declaration. When enabled, SDL exposes Ctrl+Alt+L for interval-based diagnostic logging.

Add three independent per-instance device selectors:
* Network adapter: **VirtIO Net** as the default, with **RTL8139** as the fallback.
* System-disk controller: **VirtIO Block** as the default, with **PIIX IDE** as the fallback.
* CD-ROM controller: **PIIX IDE** as the default, with an optional **VirtIO SCSI CD-ROM** choice once the VirtIO SCSI device and XP driver path have been restored and validated. This is a separate optical-drive choice; do not couple it to the system-disk controller. Preserve the existing PIIX IDE CD-ROM behavior as the default.
* Pointing device: no selector; retain the PC machine's default **PS/2 IntelliMouse** as the only supported pointing device. Do not expose USB mouse or USB tablet choices in the launcher.

The launcher must make the `virtio-win-0.1.141_x86.vfd` driver floppy attachable during XP setup and warn that an existing XP installation cannot boot from VirtIO Block until its `viostor` driver is installed. It should also expose the appropriate guest driver guidance for VirtIO Net and, if the optional VirtIO SCSI CD-ROM path is implemented, its XP storage driver. Needs a display/input embedding path (DisplayChangeListener).
## T13 (P1) Windows TCG performance for ALL guest instruction classes — 🟡 IN PROGRESS
**Status:** IN PROGRESS (T13e-F1 active; other subtracks retain their statuses)

Primary performance target: Windows TCG. Linux product runtime is KVM-only; do not
use Linux TCG timings as a product performance target. Scope is all XP guest
instruction classes up to SSE2, not only SSE2. Use `xp/bench/perf.py` and the
baseline/methodology in `xp/bench/README.md`. Historical profiling, implementation
details, and measurements are indexed under TCG performance in `WORKLOG_INDEX.md`
(especially Sessions 73-80); do not duplicate that full history here.

* **T13e — cross-game scene-transition stalls: DONE (CI-verified).** The original
  multi-second FlatView topology rebuild bottleneck was removed by the ordered-tree
  renderer; the legacy implementation remains available only through
  `Y2KBOX_FLATVIEW_VERIFY=1`. Owner reports the original Joint Operations freezes
  are no longer noticeable and FIFA 2003 is substantially improved. Residual FIFA
  hitches and F1 crowded-grid slowdown are separate follow-ups, not evidence that
  the original FlatView fix failed. See WORKLOG Sessions 74 and 80.
* **T13e-F1 — F1 Challenge '99-'02 crowded-grid slowdown: IN PROGRESS.** Session 75
  reduced measured DXVK buffer-lock time by about 53x using in-flight range tracking
  and safe DISCARD/NOOVERWRITE selection. Sessions 77-79 removed major x87, PIO,
  VAPIC SMC, and TB-lookup costs; owner measured 32.7 FPS average in the Albert Park
  capture at Session 79. Session 80 removed additional VMVGA/BQL and memory-topology
  stalls. The remaining crowded-grid behavior needs fresh, comparable profiling.
  Do not widen deferred-update batching or change guest-visible update ordering:
  previous captures showed each update consumed before another could merge.
* **Current resume:** start with the latest owner-supplied capture. Establish its
  exact game, scene, weather, resolution, and duration before comparing against a
  matching baseline. Inspect the complete `Y2KBOX-PERF`/`Y2KBOX-PROF` records;
  `cpu.exec` aggregate timing alone does not identify a bottleneck. Keep F1 crowded-
  grid profiling separate from FIFA residual hitches and the Hitman 2 D3D8 static-
  background issue requiring owner visual confirmation; agents may analyze supplied captures. Make no source change until a repeatable hotspot
  is isolated.
* **T13a x87: PAUSED.** Existing guarded host-x87 fast paths remain: arithmetic,
  square root, exact integer loads, and normal-finite FST m32/m64 stores, with
  softfloat fallback and `XPQEMU_NO_HX87=1` differential kill switch. The
  `xp/bench/x87cmp.sh` comparison passed across 7.2M randomized operations.
  Focused microbenchmarks are not general application gains. Do not expand without
  representative XP workload evidence. See WORKLOG Session 77 and
  `xp/bench/README.md`.
* **T13b call/ret and TB lookup: PARTIAL.** Sessions 78-79 enlarged and optimized
  the jump cache, made flush O(1), and added a zero-frame fast lookup path. Revisit
  only if current profiling shows this remains material; see WORKLOG Sessions 78-79.
* **T13c REP string operations: TODO.** Consider whole-span host `memmove`/`memset`
  only for plain RAM within a page, DF=0, and no watchpoints. Preserve the generic
  path for MMIO, page crossings, and self-modifying-code pages.
* **T13d legacy XMM register caching: DONE.** The cache and Win64 five-register
  pressure guard are retained. Linux and Windows A/B results were effectively
  near-neutral; no broad speedup was demonstrated. Do not expand without generated-
  code profiling and a targeted benchmark. See WORKLOG Sessions 68 and 77-79.
* Other candidates (lazy flags/helper exits, SoftMMU/TLB, TB chaining/invalidation,
  interrupts/ring transitions, sysenter/int 2e, timers, storage, SVGA3D, MTTCG)
  must be prioritized from measured Windows XP workload evidence, not assumptions.
* For every change: record comparable A/B measurements and methodology in
  `WORKLOG.md`, retain relevant regression tests, and obtain the normal CI result.
  Do not add recurring CI performance benchmarks.

## T14 (P1) Threading model and thread pinning — 🔴 TODO
**Status:** TODO
Define which host threads exist (vCPU, main loop, IO, audio, display/3D) and pin them
(vCPU thread to the fastest/performance core, others elsewhere) with Windows
(`SetThreadAffinityMask`/`SetThreadIdealProcessorEx`, priority) and Linux (`sched_setaffinity`)
APIs. Respect hybrid CPUs (P/E cores). Make it automatic (no user flags), with a safe fallback.
Measure scheduling and affinity changes with the guest fixed at exactly 1 vCPU; do not introduce a multi-vCPU test mode.

## T15 (P3) Restore and enable VirtIO Block + Net for XP — 🟢 DONE
**Status:** DONE 2026-11-21
The minimal VirtIO implementation and build wiring have been restored from the exact upstream QEMU 11.1.90 baseline (`9d7d057a558b9a1c232cea390b4481dd20d9c9ce`): shared VirtIO core/config/bus/QMP, PCI transport, VirtIO Block + Net frontends and implementations, `net_rx_pkt`, and the missing public headers. Meson wiring includes only the selected Block/Net device sources. `CONFIG_VIRTIO`, `CONFIG_VIRTIO_PCI`, `CONFIG_VIRTIO_BLK`, and `CONFIG_VIRTIO_NET` are enabled; unrelated VirtIO devices remain disabled.

Linux and Windows CI both pass on the validated source state at `d95d75703577a65e913d2451d0f01bfbb915a477`. Owner confirms QEMU launches and boots Windows XP with VirtIO enabled and reports a substantial initial improvement in interactive performance. The standard XP launch profile now uses VirtIO Block for the system disk and VirtIO Net for networking. This task's source restoration, device registration/build validation, and launch-profile integration are complete; the owner will continue manual workload-specific performance and reliability comparisons. **Do not add recurring CI performance benchmarks.**

The Linux build fixes are part of the final scope: `vhost_vdpa` is disabled to avoid the pruned host vhost-vDPA source; `vhost_kernel`, `vhost_user`, and `vhost_crypto` are disabled to prevent unresolved generic host-vhost references; `vhost_net` remains disabled. Plain guest VirtIO Block/Net use QEMU userspace queues and do not require host vhost acceleration. The VirtIO Net PCI frontend no longer requests the unavailable optional `efi-virtio.rom`; UEFI/PXE network boot is out of scope. Linux KVM and Windows TCG remain separate and unchanged.

* Standard launch defaults: `virtio-blk-pci` for the system disk and `virtio-net-pci` for networking.
* Keep **PIIX IDE** and **RTL8139** enabled as independent fallbacks. A system-disk controller change must not implicitly change the network adapter, or vice versa.
* Keep the CD-ROM on the existing **PIIX IDE** path. Do not enable VirtIO SCSI or a VirtIO CD-ROM in this task. T12 may later expose an optional VirtIO SCSI CD-ROM choice after its device implementation and XP driver path are restored and validated; PIIX IDE remains the default.
* For a fresh XP installation, load the XP x86 `viostor` driver from `virtio-win-0.1.141_x86.vfd` through text-mode Setup's F6 flow. Install the XP x86 NetKVM driver after setup. Existing IDE installations must have `viostor` installed before switching their boot disk to VirtIO Block.
* Owner-led disk and network performance comparisons remain manual and workload-specific. Keep benchmark scripts available for deliberate A/B runs, but do not add recurring performance benchmarks to CI.
## T17 (P1) Restore Linux KVM accelerator; keep Windows TCG-only — 🟡 PAUSED
**Status:** PAUSED (Linux KVM implementation and CI build restored; host runtime validation outstanding)
Earlier host/accelerator stripping removed the KVM implementation (see the historical
Session 1 entry in WORKLOG.md). Restore the Linux KVM build and runtime path, including
the required accelerator sources, x86 KVM hooks, machine/interrupt/clock integration,
headers, stubs, Meson/Kconfig wiring, and any shared dependencies removed with it.
Do not blindly copy an upstream directory: compare against this fork's QEMU base and
current XP-only configuration, then restore the smallest complete dependency set.

Restore checkpoint (2026-11-19; historical build issues resolved): The Linux x86
KVM accelerator, x86 backend, and PC KVM interrupt/clock integration were restored
with Linux-only Meson wiring. Follow-up CI failures involving over-pruned KVM nested
state, a Linux-only header in the disabled TDX stub, and stale optional RAPL energy-MSR
references were addressed; subsequent Linux/Windows CI passed. Windows remains
TCG-only. The remaining acceptance gap is runtime validation on a KVM-capable Linux
host; compile-only CI is not sufficient.
* Linux: compile and ship KVM as the only supported runtime accelerator. The launcher
  must check that `/dev/kvm` exists and is accessible before starting QEMU; if not, show a
  clear actionable error such as: `KVM is required. Enable hardware virtualization/KVM
  and grant your user access to /dev/kvm, then restart Y2KBox.` Do not silently fall back
  to TCG. Internal TCG-only Linux CI/development builds may exist, but are not product
  builds and do not define a supported Linux runtime.
* Optional KVM RAPL guest energy-MSR emulation is outside the XP product scope;
  its vmsr_energy backend was removed by the original pruning pass and must not be
  partially referenced by the restored KVM implementation.
* Windows: remain TCG/MTTCG-only. Do not restore WHPX or other Windows accelerators.
* TCG performance work and optimization acceptance are Windows-only.
* **Resume here:** run the supported Linux product build on a KVM-capable Linux host,
  confirm `/dev/kvm` access and successful XP guest launch with `-accel kvm`, and
  record the exact host/kernel/guest result. Do not repeat the completed source
  restoration or reintroduce the removed optional RAPL integration without new evidence.
* Preserve the XP machine/device constraints and keep IOMMU, SEV, SGX, TDX, and VFIO
  disabled unless a concrete dependency proves otherwise.
* DoD: Linux product CI builds with KVM support and reports KVM as the only supported
  runtime accelerator; a KVM-capable Linux host boots the XP guest with `-accel kvm`.
  A Linux host without accessible `/dev/kvm` is rejected by the launcher with the
  documented actionable prompt (no fallback). Windows CI builds and runs the TCG checks
  without KVM dependencies. Record exact host/guest validation and do not claim KVM
  runtime success from compile-only CI.


## T18 (P2) Evaluate Windows TCG virtual-memory backend APIs — 🔴 TODO
**Status:** TODO
Investigate whether Windows guest-memory allocation, reservation, mapping, or remapping
is a measurable cost in the Windows TCG execution path before implementing a new
backend. Candidate APIs include `VirtualAlloc2` and `MapViewOfFile3` (Windows 10
version 1803+), particularly their address requirements and placeholder reservation /
replacement support. These APIs provide more control over virtual address-space layout;
they are not assumed to make translated guest instruction execution faster.

* **Profile first:** use representative TCG workloads and generated-host-code / runtime
  profiling to determine whether guest-memory allocation, mapping, remapping, TLB/page
  behavior, or address-space layout is a material bottleneck. Separate startup/setup
  costs from steady-state guest execution.
* **Prototype only if justified:** compare a narrowly scoped Windows implementation
  against the existing memory path. Preserve the current implementation as a fallback
  until the new path's compatibility and reliability are established.
* **Benchmark:** use repeatable A/B measurements across memory-intensive and
  representative end-to-end workloads; include startup and memory-footprint effects.
  Do not accept the change based solely on API novelty or synthetic allocation timings.
* **Decision gate:** if profiling does not identify a relevant bottleneck, leave the
  existing backend unchanged and close/defer this task without implementation.
* **OS policy:** Windows 10 1803+ is the encouraged technical baseline documented in
  README.md, not an unconditional requirement introduced by this task. Do not raise
  the formal minimum OS version without a separate compatibility decision.
* **Scope:** Windows TCG only. Linux remains KVM-only; no Linux TCG product path.


## T19 (P1) General SDL diagnostic logging capture — 🟡 PAUSED
**Status:** PAUSED (CI and basic repeated-capture behavior verified; capture-content coverage still under investigation)
Add an SDL2 host-side keyboard shortcut that captures a selected diagnostic
interval across Y2KBox, without continuously generating diagnostic output.

* Debug mode is a separate opt-in setting, default **off**, configured independently
  of device creation with `-y2kboxdebugmode on` (or explicitly disabled with
  `-y2kboxdebugmode off`). This is the command-line equivalent of T12's future
  **Debug: On/Off** GUI setting. Do not put the debug property inside the
  `-device vmvga` declaration or expose the internal VMVGA property to users.
* With debug mode off, Ctrl+Alt+L is unavailable and no diagnostic capture is
  performed. With it on, the SDL title advertises the shortcut; debug mode enables
  diagnostic capability but does not itself start continuous output.
* Use the existing SDL host-hotkey modifier chord and add **Ctrl+Alt+L** to toggle
  capture; consume the key so it is not sent to the guest.
* On activation, create a new timestamped `y2kbox-log-YYYYMMDD-HHMMSS.log`
  file (adding a numeric suffix on collisions), redirect stderr to it, and enable
  general performance counters, VMVGA local diagnostics, VMPort GuestRPC
  diagnostics, and the relevant VMVGA/VMPort/vdagent trace events. Reset all
  `Y2KBOX-PERF` counters and timing baselines at capture start; report only that
  interval's measurements when stopping. This resets diagnostic statistics only,
  not the VM or guest. On stop, disable those diagnostic sources and restore
  original stderr. The VM keeps running. Never append sessions or overwrite an
  existing capture. Do not emit routine
  capture start/stop markers or diagnostic output to the console while inactive.
* Show generic diagnostic-logging availability and active state in the SDL title
  only when debug mode is enabled. Report actual file/descriptor errors, not
  routine logging status.
* No separate `-trace "vdagent*"` command-line option should be required.
  Remove shell-level `2>` redirection guidance; it prevents interval capture.
* DoD: Linux and Windows SDL builds compile in CI; static review confirms all
  diagnostic sources are quiet while inactive and stderr is restored on stop;
  owner verifies hotkey, output coverage, and repeated intervals in an XP guest.
  No branch or PR.

## T20 (P2) Full BIN/CUE CD-image parsing — 🔴 TODO
**Status:** TODO
Implement full BIN/CUE parsing for Y2KBox's emulated CD-ROM, not merely a limited
or strict-subset parser. Support the complete CUE-sheet syntax and all defined
CUE flags (including flags such as `DCP`), with correct track/index and sector
layout handling. Use the CD-ROM references in HANDOFF.md section 9 (libcdio,
86Box, DuckStation, and the CUE-sheet format references) when designing and
validating the parser. Keep this distinct from frontend disc swapping: guest-
visible insert/eject of images in the emulated CD-ROM is tracked under T12.

## T21 (P2) Verify ReShade injection compatibility — 🔴 TODO
**Status:** TODO
Investigate and verify that ReShade injection is compatible with Y2KBox's
Windows/SDL2 display path and the planned SDL2 + Dear ImGui frontend. Identify any
limitations or conflicts in the current host-window/display lifecycle and
implement only the compatibility work actually needed. This compatibility
investigation takes priority over integrating librashader.

## T22 (P3) Consider librashader integration — 🔴 TODO
**Status:** TODO (after T12)
After the SDL2 + Dear ImGui frontend is implemented, consider integrating librashader
as an optional shader/post-processing path. This is deliberately very low
priority; first establish ReShade injection compatibility (T21). Do not let
librashader integration delay the frontend or higher-priority compatibility
work. See HANDOFF.md section 9 for librashader and emulator integration
references.

## T23 (P3) Configurable RAM and vGPU memory presets — 🔴 TODO
**Status:** TODO (after T12)
After the SDL2 + Dear ImGui frontend is implemented, add simple user-facing memory
presets in the launcher. RAM candidates remain 512, 1024, and 2048 MB; the
current machine stays fixed at 2048 MB until that GUI work is implemented.

For Options -> vGPU, expose two distinct memory controls rather than one
ambiguous "VRAM" setting:

* **Framebuffer VRAM** (`vgamem_mb` / `SVGA_REG_VRAM_SIZE` / PCI BAR1):
  initial/default value **128 MiB**. Candidate choices: 16, 32, 64, and
  128 MiB. This controls the legacy framebuffer aperture and its PCI MMIO
  reservation; 128 MiB is the practical target for the project's 32-bit
  Windows XP guest configuration.
* **3D Surface Memory** (Y2KBox's SVGA3D surface-memory budget):
  initial/default value **512 MiB**. Candidate choices: 64, 128, 256, 512,
  and 768 MiB. This budget is distinct from the BAR1 framebuffer and backs
  SVGA3D surface/resource allocations.

The device reports the combined value through `SVGA_REG_MEMORY_SIZE`, which
is currently calculated as framebuffer VRAM plus the surface-memory budget.
Keep the combined value at or below **1024 MiB**; validate the pair rather
than allowing a GUI selection that exceeds this limit. With the current
128 MiB + 512 MiB defaults, `SVGA_REG_MEMORY_SIZE` is **640 MiB**. The
largest listed 3D Surface Memory preset (768 MiB) fits with the 128 MiB
framebuffer exactly at the 1024 MiB ceiling.

The 128 MiB framebuffer default is chosen to avoid reserving an unnecessarily
large BAR1 aperture for the 32-bit XP target. The precise XPDM driver's
mapping/clamping behavior should still be confirmed against the VMware Tools
12.01.08 guest driver and by checking the reported local framebuffer memory
and PCI BAR1 mapping in the XP guest; do not present the driver-specific
128 MiB behavior as a universal SVGA hardware limit.

The current device defaults are implemented separately from this future GUI
task. The GUI must expose the two pools accurately, retain the combined
1024 MiB limit, and explain that the 3D surface budget is not the same as
dxdiag's local framebuffer-memory figure.

## T24 (P1) Fixed-size SDL guest display window — 🟡 IMPLEMENTED
**Status:** IMPLEMENTED (Windows window behavior owner-verified; CI + remaining XP checks outstanding)
Make the SDL host window follow the VMVGA guest display surface rather than
allowing host-side window geometry to influence the guest.

* Windowed SDL windows remain decorated but are non-resizable; the titlebar
  maximize/resize affordance must be disabled by the non-resizable window style.
* Guest display-surface changes resize the window programmatically.
* Host `SDL_WINDOWEVENT_RESIZED` and `SDL_WINDOWEVENT_SIZE_CHANGED` events
  must not call `qemu_console_set_ui_info()` or change guest display geometry.
  If a host window manager changes the window dimensions anyway, restore the
  current guest surface dimensions.
* Fullscreen desktop dimensions are presentation-only and never feed back into
  the guest resolution. Keep `Ctrl+Alt+F` as the interactive fullscreen toggle.
* Do not alter mouse emulation or coordinate mapping to compensate for
  host-window resizing.
* **Owner-confirmed Windows behavior (2026-11-24):** the built Windows
  application presents a decorated, non-resizable window, and the titlebar
  maximize control is disabled as intended. This confirms the fixed-window
  behavior; it does not by itself verify guest-driven resolution changes,
  `Ctrl+Alt+F` fullscreen switching, or mouse tracking.
* **Remaining acceptance checks:** Linux and Windows SDL CI; owner verification
  in XP of guest-driven resolution changes, `Ctrl+Alt+F` fullscreen toggling,
  and mouse tracking. Do not change the implementation unless one of these
  checks exposes a real defect.
* DoD: Linux and Windows SDL builds pass CI and the remaining owner-run XP
  checks above are confirmed.

## T25 (P2) Investigate guest OpenGL support through VMware SVGA3D — 🟡 PAUSED
**Status:** PAUSED (low feasibility; awaiting independent reassessment)
Establish whether a guest-visible OpenGL path for Windows XP is feasible while preserving the existing VMware SVGA II / SVGA3D display driver, with **OpenGL 1.1 as the baseline and OpenGL 2.1 as the maximum target**. OpenGL is not currently implemented as a guest-facing API.

**Current disposition (2026-11-24): PAUSED — low feasibility; awaiting independent reassessment.** No implementation work is authorized. The next reviewer should independently challenge the assumptions and investigate alternative architectures, rather than simply continue the current standalone-DLL / GL4ES / ANGLE hypothesis. A guest-side application-local wrapper DLL is one unproven possibility, not an established solution or the presumed direction. Request a written architecture, XP compatibility, licensing, and feasibility assessment before proposing any prototype. Do not modify the guest, replace or patch the VMware display driver, change the host renderer, or begin a broad OpenGL implementation as part of this paused task. See HANDOFF.md section 8g.

* **Owner-run XP diagnosis completed (2026-11-24):** COD1 was launched with `+set r_allowSoftwareGL 1`. It selected a generic software pixel format, created a WGL context, and made it current, but then exited with the missing-video-card-features fatal error. GLview independently reports Microsoft / GDI Generic / OpenGL 1.1.0, exposing only `GL_EXT_bgra`, `GL_EXT_paletted_texture`, and `GL_WIN_swap_hint`; its reported limits and extension-check details are recorded in HANDOFF.md section 8d and WORKLOG.md Session 104. This establishes that XP is currently using Microsoft's generic software OpenGL implementation, not a VMware OpenGL ICD. COD1 now gets past PFD selection/context creation; its remaining failure is consistent with missing required GL functionality. Pixel-format tab details were not captured. Do not require the owner to install Visual Studio or compile `xp/tools/glprobe.c` in the guest; the GLview result already establishes the active renderer.
* **Audit the guest/host contract:** Mesa's SVGA Gallium driver is an OpenGL implementation that translates GL/Gallium state into SVGA3D commands; it does not send raw OpenGL calls through a separate OpenGL command FIFO. Initial VGPU9 comparison is encouraging: Mesa's legacy path reports GLSL feature level 1.20 (the GLSL version associated with OpenGL 2.1), requires SVGA hardware version WS8_B1 and vertex/pixel Shader Model 3.0, and uses the legacy SVGA3D command family. Y2KBox reports WS8_B1, VS/PS 3.0, and four render targets; its command table has handlers for the core legacy surface/context, render/texture state, shader, draw, clear, query, and presentation commands used by this path. This removes an obvious protocol-generation/shader-version blocker, but does not prove semantic completeness. Continue auditing exact state behavior and capability gaps; current candidates include legacy features such as line stipple and logic operations, which must not be advertised unless correctly implemented or emulated.
* **Standard XP ICD discovery constraint:** Windows XP's OpenGL runtime does not simply load an arbitrary ICD because a registry entry exists. The active display driver must advertise OpenGL through the OPENGL_GETINFO escape and identify the ICD; WINVBE documents this handshake for NT 3.5 through XP, and ReactOS's loader implementation corroborates the QUERYESCSUPPORT / ExtEscape(OPENGL_GETINFO) sequence. Therefore a conventional ICD cannot be added alongside VMware's driver unless that driver already supports the escape or is modified. The active GDI Generic result and VMware's documented XP software-OpenGL behavior provide no evidence of such support. Do not pursue a registry-only ICD installation or modify/replace the VMware display driver.
* **Unproven architecture leads (historical research, not an implementation plan):** A per-application standalone OpenGL DLL or app-local wrapper might bypass Windows XP's normal ICD discovery path. The id Tech 3 `r_glDriver` mechanism, GL4ES, and a historical ANGLE D3D9 backend were discussed as possible leads, but none has been demonstrated to work with COD1, Windows XP, or Y2KBox. Other architectures may be better. The detailed evidence, caveats, and references are in HANDOFF.md sections 8f–8g.
* **QindieGL lead (researched 2026-11-24; not a change in task status):** [QindieGL](https://github.com/crystice-softworks/QindieGL) is an application-local OpenGL-to-Direct3D 9 wrapper and a concrete source-level reference for the standalone-DLL idea. In its current source, `wglGetExtensionsStringARB(HDC)` is implemented and registered, but `wglGetExtensionsStringEXT(void)` is neither implemented, registered, nor advertised. The owner's COD1 run with QindieGL logged an unknown-procedure warning for `wglGetExtensionsStringEXT` and then exited. Adding a registry extension flag did not change that result: a registry value cannot supply a missing exported WGL procedure. A historical DxWnd discussion reports that an experimental rebuild with a minimal EXT entry point allowed COD1 to start, but produced substantial graphical corruption (especially the skybox). This is evidence that the missing entry point can block startup, not evidence that a correct renderer is close. QindieGL's source is GPL-3.0; review license compatibility before copying or adapting any code into Y2KBox, whose license is GPL-2.0. No patch, build, guest test, or implementation is authorized by this research. See HANDOFF.md section 8h.
* **TitaniumGL lead (owner-run COD1 test, 2026-11-24; not a change in task status):** [TitaniumGL](https://titaniumgl.uni.hu/keret.cgi?/index.html) is another application-local OpenGL implementation/wrapper worth independent evaluation. With its Win32 `opengl32.dll` beside COD1, the game loaded the wrapper, selected pixel format 1, acquired a DC, created and made current a GL context, and logged `GLVERSION 1.4` plus `GL RENDERER VMware SVGA II/119MB VRAM`. It listed numerous `GL_EXT`, `GL_ARB`, `WGL_ARB`, `WGL_EXT`, `GL_OES`, and `GL_SGIS` extensions; the full list was not captured. The log also reported `0 PFDs found`, `MCD acceleration found`, and successful loading/calling of `system32\\opengl32.dll`, then reached `RE_SHUTDOWN (1)` after COD1 rejected the graphics card with the same missing-features message. The owner observed the same broad result in TitaniumGL's hardware-accelerated and software-renderer modes. These observations establish wrapper loading and context creation, not correct implementation of every advertised extension, successful gameplay, or a functioning accelerated D3D9 path. The `0 PFDs found` line merits investigation but is not yet proven causal because context creation subsequently succeeded. The precise XP requirements, release provenance, source availability, and license have not yet been independently verified. See HANDOFF.md section 8i.
* **Disposition:** T25 remains paused. No prototype, guest modification, driver replacement, source change, build, or guest test is currently authorized. Resume only after an independent reviewer provides a fresh feasibility assessment and the owner approves a concrete next step.
