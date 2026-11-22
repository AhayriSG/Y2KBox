# Y2KBox task queue

Several AIs/humans may work at the same time. **Claim a task before starting**:

1. Fetch the latest `main` and re-read this file before choosing work.
2. Edit the task's `Status` to `CLAIMED by <name/session> <date>` in this file,
   then commit and push directly to `main`. If another contributor moves `main`
   first, stop, fetch the new HEAD, re-read the queue, and coordinate before proceeding.
3. Work directly on `main`; do not create branches or pull requests. Before every
   write, fetch the latest `main` and the exact file being changed. Verify that both
   the branch HEAD and file blob SHA still match what you inspected; if either moved,
   abort that write, fetch again, and reapply the change to the new content. Commit
   and push small, coherent increments. Never force-push. Keep `main` building.
   (HANDOFF.md section 7: do not build locally or poll CI; the status hook reports it.)
4. When done: set `Status: DONE <date> (commit <hash>)`, add a line to WORKLOG.md,
   update the README status table, and push to `main`.
5. **If you are about to run out of budget**: commit and push the current safe state
   to `main`, set the task to `PAUSED` with a precise "Resume here:" note (what is
   done, what is next, what is broken). Never leave a claimed task silent.

Status values: `TODO`, `CLAIMED by ...`, `PAUSED`, `DONE`. Priority: P0 first.

---

## T01 (P0) Fixed XP machine  -  Status: PAUSED (parts 1, 2a, 2b done)
Goal: exactly one machine, no user-facing machine/CPU/RAM/SMP options.
* DONE part 1: default RAM 2 GiB, default 1 vCPU, `max_cpus` 2 (`hw/i386/pc.c`).
* DONE part 2a (`xp/strip/08-fixed-machine-memory.py`): minimal `pc_memory_init` (all RAM below 4 GiB,
  asserts above-4G == 0), memory-device/DIMM/NVDIMM/SP-MEM/HV-balloon hotplug handlers, CXL, device
  memory, `pc_max_used_gpa`, and in `acpi-build.c` the NVDIMM/memory-hotplug AML, CXL, SRAT device
  memory, HMAT, VIOT, VMGenID, VMClock, CEDT. `default.mak` now has DIMM, MEM_DEVICE, NVDIMM,
  ACPI_NVDIMM/HMAT/VIOT/CXL/VMGENID/VMCLOCK/MEMORY_HOTPLUG, HV_BALLOON_SUPPORTED = n
  (10 stale Kconfig `select` lines removed); 11 dead C files deleted. Checks green: e820 = one
  2 GiB entry, vectest OK, `-smp 2` boots, no pc-dimm/nvdimm devices.
* DONE part 2b (`xp/strip/09-single-machine-type.py`): only `pc-i440fx-11.2` (alias `pc`) remains; 88 compat
  tables (`pc_compat_*`, `hw_compat_*`) deleted. `-machine help` = pc + none.
* DONE part 2c-1: removed dead above-4G / `max_ram_below_4g` / `pci_hole64` / `gigabyte_align` /
  `enforce_amd_1tb_hole` / `pc_init1` memory-split code across `hw/i386/{pc,pc_piix,x86,acpi-build}.c`,
  `hw/pci-host/i440fx.c`, and `include/hw/i386/{pc,x86}.h`.
* Resume here (part 2c-2): remove remaining `ram-slots`/`maxmem` fields if unused; (3) remove the `-m/-smp/-machine/-cpu`
  CLI paths (qemu-options.hx, system/vl.c, `-smp` parsing) now that defaults are fixed; (4) rename the machine to something like `xp` (drop the version suffix); (5) ACPI
  CPU hotplug AML (`build_cpus_aml`) is still needed for the CPU objects - keep it, drop the
  hotplug-only parts later; (6) re-run dead-source pass (HANDOFF 5b), standard checks.
* DoD: fresh configure+build; one machine in `-machine help`; standard checks (HANDOFF 7);
  `info mtree` shows 2 GiB below 4 GiB only.

## T02 (P0) XP guest CPU model: SSE2 maximum  -  Status: PAUSED (parts 1 and 2a done)
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
* DoD: build; CPUID shows no SSE3+; vectest OK; XP boot still works once T08 exists.

## T03 (P0) Hosts: Windows and Linux only  -  Status: PAUSED (pass 1 done; Linux + Windows CI verified)
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

## T04 (P1) Inline PSHUFD / PSHUFLW / PSHUFHW / PUNPCK* / PACK* / SHUFPS  -  Status: DONE 2026-11-18 (implementation 746dae5; benchmark recorded df28ec6)
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

## T05 (P1) Remove IOMMU / SEV / SGX / TDX / VFIO call-site leftovers (preserve KVM)  -  Status: DONE 2026-11-19
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

## T06 (P1) Remaining pruning  -  Status: TODO
Device models still compiled but unused, trace-events of deleted code, QAPI
schemas/commands for removed features, migration extras (COLO, postcopy, multifd -
keep savevm snapshots), vhost/vhost-user, block formats other than raw/qcow2/vvfat?,
net backends other than user/slirp(+socket?), `scripts/`, `python/`, `rust/`, `qga/`.
Use the dead-code technique in HANDOFF 5b. Re-run the boot + vectest checks.

## T07 (P1) Windows host build  -  Status: DONE 2026-11-14 (CI green at 92dbf22, run 43695682586)
GitHub Actions (`.github/workflows/build.yml`) builds and checks BOTH hosts natively: Linux (ubuntu-24.04)
and Windows (MSYS2 UCRT64): configure, build `qemu-system-i386[.exe]` + `qemu-img[.exe]`, `-accel help`,
boot + `xp/bench/vectest.asm` (prints OK on both). Windows needed `pc-bios/qemu-nsis.ico` restored (agent fix
92dbf22). Keep CI green: the `UserPromptSubmit` hook (`scripts/session_status.sh`) reports the HEAD run - never poll; fix failures first. CI also uploads binaries as Artifacts and runs `xp/bench/x87cmp.sh`.
Follow-ups (not blocking): upload build artifacts; cache MSYS2 packages; add a Windows `vectest` timing run.

## T08 Boot Windows XP SP3 end to end  -  Status: TODO (needs an XP ISO from the owner)
Install to a qcow2 disk (`qemu-img create -f qcow2`), IDE CD-ROM, floppy; measure install time;
fix whatever breaks (ACPI HAL choice, SMP/APIC, TPR patching via kvmvapic option ROM,
RTC/PIT timing). Record the working command line in HANDOFF.

## T09 (P2) 3D: integrate qemu-vmvga (VMware SVGA II + SVGA3D)  -  Status: PAUSED (draft ready for CI & XP guest verification)
PRIMARY 3D plan (owner proposal 2026-11-14, evaluated and agreed; see HANDOFF 8a).
Repo: https://github.com/qemus/qemu-vmvga (GPL-2.0-or-later, 44-file overlay for QEMU 11.1.x: device
`vmvga`, SVGA3D vgpu9/vgpu10/vgpu11, DXVK-based rendering, `hw/i386/vmport.c`).
* **Draft implemented (needs CI verification on push + XP guest test in T08):**
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
* **Resume here:** check Linux + Windows CI results from this push; once an XP ISO is available (T08),
  verify XP loads VMware Tools 10.0.12 SVGA driver 12.01.08 at 1024x768x32 with 2D acceleration.

## T10 (P2) 3D host backend: Linux (DXVK-native + Vulkan) and Windows  -  Status: PAUSED (draft ready for CI & XP guest verification)
qemu-vmvga renders SVGA3D through DXVK (README: Vulkan-capable GPU + `dxvk-native` on the host).
* **Draft implemented (needs CI verification on push + XP guest test in T08/T11):**
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
* **Resume here:** check Linux (including packaged `dxvk-native` + lavapipe smoke check) and Windows CI
  builds from this push; validate live in-guest SVGA3D rendering on both hosts in T11; render-thread
  pinning remains T14.

## T11 XP guest driver validation and D3D coverage  -  Status: TODO (needs T08 + T09)
Install VMware Tools 10.0.12 (`winPreVista.iso`) in XP; verify SVGA3D D3D9 apps (e.g. DX9 sample/game);
find the gaps: Direct3D 8 and DirectDraw/Direct3D 7 apps (try `d3d8to9` and a ddraw wrapper in the guest),
shader model limits, resolution/mouse integration (vmmouse), performance on TCG. Record results.

## T16 (P3, frozen alternative) VBoxVGA 5.2.x path  -  Status: TODO (not planned; kept as fallback)
If qemu-vmvga proves unworkable on XP: VBoxVGA (VBE dispi + HGSMI/VBVA) + VMMDev (PCI 80ee:cafe) + HGCM +
the Chromium-based 3D host service from VirtualBox v5.2.x (GPLv2; the service is absent from 6.1.4).
Research notes: HANDOFF 8b. Sub-pieces if ever started: T16a VBoxVGA+HGSMI/VBVA, T16b VMMDev+HGCM,
T16c GL stream receiver/replay (consider a separate process).

## T12 (P2) Own front end: launcher + display  -  Status: TODO
Our own executable with an **SDL2 + Dear ImGui** frontend, using Xemu's `ui/xui/` as an architectural reference. Dear ImGui is the primary GUI choice; wxWidgets is a fallback alternative, not a parallel implementation target. Keep QEMU's existing **SDL2 display backend**: do not migrate to SDL3 or replace the VMVGA renderer, display path, fullscreen handling, or mouse/input behavior as part of frontend work. Reuse Xemu's relevant frontend patterns, but do not import its Xbox-specific systems or unnecessary dependencies wholesale. Keep the dependency footprint and SDL2/QEMU event-loop integration complexity as explicit design considerations; begin with Dear ImGui core and its SDL2 integration, adding components only when Y2KBox needs them. The only interactive display backend we plan to support is **SDL2**; do not spend time supporting VNC or `-display none` as user-facing display choices (`-display none` remains useful for headless CI/checks). The launcher should pick a qcow2 disk (made with qemu-img), floppy image, and CD/ISO. The initial frontend keeps RAM fixed at 2048 MB, automatic, with no RAM setting; possible later memory presets are tracked separately in T23. Add guest CD-ROM **insert/eject** controls so users can swap disc images in the emulated drive for multi-disc games. Add a **Debug** On/Off setting, default **Off**; it should enable Y2KBox's tester diagnostics without enabling continuous output. This maps to the `-y2kboxdebugmode on|off` command-line setting; keep it separate from the `-device vmvga` declaration. When enabled, SDL exposes Ctrl+Alt+L for interval-based diagnostic logging.

Add three independent device selectors:
* Network adapter: **VirtIO Net** as the default, with **RTL8139** as the fallback.
* System-disk controller: **VirtIO Block** as the default, with **PIIX IDE** as the fallback.
* CD-ROM controller: **PIIX IDE** as the default, with an optional **VirtIO SCSI CD-ROM** choice once the VirtIO SCSI device and XP driver path have been restored and validated. This is a separate optical-drive choice; do not couple it to the system-disk controller. Preserve the existing PIIX IDE CD-ROM behavior as the default.
* Pointing device: no selector; retain the PC machine's default **PS/2 IntelliMouse** as the only supported pointing device. Do not expose USB mouse or USB tablet choices in the launcher.

The launcher must make the `virtio-win-0.1.141_x86.vfd` driver floppy attachable during XP setup and warn that an existing XP installation cannot boot from VirtIO Block until its `viostor` driver is installed. It should also expose the appropriate guest driver guidance for VirtIO Net and, if the optional VirtIO SCSI CD-ROM path is implemented, its XP storage driver. Needs a display/input embedding path (DisplayChangeListener).
## T13 (P1) Windows TCG performance for ALL guest instruction classes  -  Status: TODO (baseline measured)
Primary performance target: Windows TCG. Linux product runtime is KVM-only, with no TCG fallback. TCG optimization and performance A/B benchmarks are Windows-only; Linux CI performs KVM build and functional validation. Scope (owner): everything up to SSE2, not just SSE2. Measure with `xp/bench/perf.py` (see
`xp/bench/README.md` for the baseline table). Measured hotspots and optimization work are tracked in the sub-tasks below. T13e's main FlatView topology bottleneck has been addressed by the ordered-tree renderer; its CI gate and residual FIFA/F1 workload follow-ups remain. T13a's existing guarded paths remain, while further x87 expansion is paused pending representative profiling. T13b/c and the broader TCG work remain P1.

* **T13e (P0) Profile and eliminate multi-second scene-transition stalls - Status: PAUSED (ordered-tree FlatView renderer implemented; owner equivalence and three-game validation reported; Windows/Linux CI pending).** The measured cross-game stall hotspot was repeated sorted-array insertion during recursive FlatView topology generation, triggered by GART map/unmap transaction commits. The ordered-tree renderer is now the default; the ineffective batched prototype and builder selector were removed. The legacy renderer remains only for `Y2KBOX_FLATVIEW_VERIFY=1`; leave verification unset during normal runs. Owner validation: Joint Operations' 1-3 s freezes/audio interruptions are no longer noticeable; FIFA 2003 is substantially improved but retains occasional ~1 s hitches; F1 Challenge '99-'02 scene transitions improved, while crowded-grid slowdown remains. Tree topology generation measured 1.713 s across 795 roots versus 17.965 s in the earlier batched capture (not a controlled A/B). **Resume:** verify Windows and Linux CI after promotion. If CI passes, close the original FlatView-stall fix and track the remaining FIFA hitches and F1 crowded-grid slowdown as separate profiling tasks; do not assume they share the same cause. Do not mark T13e DONE until CI validation is reported. No recurring CI performance benchmarks.
 
* **T13a x87 - Status: PAUSED 2026-11-21 (existing guarded arithmetic/load/store paths retained; further instruction expansion deferred until T13e identifies the dominant scene-stall cost).** Implemented: host-x87 fast paths for fadd/fsub/fsubr/fmul/fdiv/fdivr (ST0/FT0 and STN/ST0 forms), fsqrt on normal finite operands, integer-only fld m32/m64 for normal and zero operands, exact-load correction for the softfloat fallback, and normal-finite FST m32/m64 stores. Arithmetic results were bit-identical to softfloat across 7.2M randomized operations (xp/bench/x87cmp.sh; XPQEMU_NO_HX87=1 kill switch). Focused x87add/x87div microbenchmarks improved 3.0x/4.6x. Windows five-run FST A/B medians: m32 2.474809 s softfloat vs 2.383368 s host x87 (+3.69%, 1.038x); m64 2.508467 s vs 2.359847 s (+5.92%, 1.063x). These are isolated hot-cache loop results, not general application gains. Deferred: fild/fist/fistt, compares, frndint/fscale/fprem, transcendental operations, fadd m80, broader exceptional-operand paths, and any host-register-stack mapping. Revisit only if representative XP profiling justifies it.
* **T13b call/ret (~12 ns per call+ret pair).** `ret` = indirect jump through
  `tcg_gen_lookup_and_goto_ptr`/`lookup_tb_ptr`. Ideas: return-address shadow stack verified on `ret`,
  bigger/cheaper `tb_jmp_cache`, direct chaining for `call` targets; profile first.
* **T13c rep movs/stos/cmps/scas (~2 GB/s).** Fast path: when source/dest are plain RAM within a page
  (DF=0, no watchpoints) use host `memmove`/`memset` for the whole span; keep the generic loop for
  MMIO/page crossings/self-modifying-code pages.
* Remaining P1 profiling candidates: lazy flags/helper exits, SoftMMU TLB lookup and page-crossing costs, TB lookup/chaining/cache sizing, TB invalidation/SMC behavior, interrupt/ring transitions, `sysenter/int 2e`, timers, storage I/O, VGA/SVGA3D memory paths, and MTTCG scaling. Prioritize from measured XP workload evidence rather than presumed generic impact.
* Keep XMM/MMX state in host registers across instructions; PSHUFD & shuffles inline (T04).
Every change: A/B numbers in WORKLOG + standard checks + CI green.


* **T13d legacy XMM register caching - Status: DONE 2026-11-18 (implementation and targeted Linux/Windows A/B measurements complete; no meaningful speedup demonstrated).**
  Initial implementation: `c272958`. `DisasContext` owns an EBB-temp cache for eligible
  legacy XMM register-only operations: lazy loads, dirty tracking, and flush/free barriers
  before non-cacheable instructions, exceptions, and TB exits. MMX, VEX/YMM, memory operands,
  and unsupported vector operations retain their existing paths. Eligible classes include
  packed integer arithmetic/compare/min-max/multiply, logical ops, immediate PSRLD, and the
  T04 shuffle/unpack/pack operations. `shuffletest.asm` adds a MOVD cache-barrier regression
  case. Linux CI compared the same 100M-iteration `sse2.asm` image against the pre-cache
  artifact from run 44156398064 (median of 3 runs): 0.877822 s before, 0.872720 s after,
  +0.58% (1.006x), effectively near-neutral.
  Review of `tcg/x86_64/tcg-target.c.inc` showed that Win64 can allocate only XMM0-XMM4:
  XMM5 is reserved as scratch and XMM6-XMM15 are withheld under the Windows x64 ABI.
  The translator preflights each cacheable instruction's full operand set and flushes
  before exceeding five live cached guest XMM values, preventing an oversized cache from
  forcing vector-register spills. Linux behavior is unchanged. Windows CI compared the
  pre-cache artifact from run 44156398064 with the capacity-aware build using the same
  300M-iteration `sse2.asm` image (median of three runs): 3.166927 s before, 3.132343 s
  after, +1.09% (1.011x). Both executable preflights and all six benchmark runs succeeded.
  The small positive deltas on shared CI runners are effectively near-neutral, not
  demonstrated performance gains; they also do not establish a broad SSE2 improvement or
  regression. Keep the cache and Win64 five-register pressure guard as implemented.
  **T13d follow-up / next optimization gate:** do not expand the XMM cache yet. First profile
  the generated host code and determine whether the remaining cost is guest XMM state traffic,
  TCG register allocation/spills, translation/dispatch overhead, or another source. Then build
  a benchmark that isolates the identified cost (and retain a representative end-to-end workload
  as a guard). Only pursue a cache-policy or lowering change when the profile and targeted A/B
  measurement support it; do not infer a bottleneck from the near-neutral SSE2 microbenchmark.

## T14 (P1) Threading model and thread pinning  -  Status: TODO
Define which host threads exist (vCPU, main loop, IO, audio, display/3D) and pin them
(vCPU thread to the fastest/performance core, others elsewhere) with Windows
(`SetThreadAffinityMask`/`SetThreadIdealProcessorEx`, priority) and Linux (`sched_setaffinity`)
APIs. Respect hybrid CPUs (P/E cores). Make it automatic (no user flags), with a safe fallback.
Measure with and without (1 vCPU default; 2-vCPU experimental mode if added).

## T15 (P3) Restore and enable VirtIO Block + Net for XP  -  Status: DONE 2026-11-21
The minimal VirtIO implementation and build wiring have been restored from the exact upstream QEMU 11.1.90 baseline (`9d7d057a558b9a1c232cea390b4481dd20d9c9ce`): shared VirtIO core/config/bus/QMP, PCI transport, VirtIO Block + Net frontends and implementations, `net_rx_pkt`, and the missing public headers. Meson wiring includes only the selected Block/Net device sources. `CONFIG_VIRTIO`, `CONFIG_VIRTIO_PCI`, `CONFIG_VIRTIO_BLK`, and `CONFIG_VIRTIO_NET` are enabled; unrelated VirtIO devices remain disabled.

Linux and Windows CI both pass on the validated source state at `d95d75703577a65e913d2451d0f01bfbb915a477`. Owner confirms QEMU launches and boots Windows XP with VirtIO enabled and reports a substantial initial improvement in interactive performance. The standard XP launch profile now uses VirtIO Block for the system disk and VirtIO Net for networking. This task's source restoration, device registration/build validation, and launch-profile integration are complete; the owner will continue manual workload-specific performance and reliability comparisons. **Do not add recurring CI performance benchmarks.**

The Linux build fixes are part of the final scope: `vhost_vdpa` is disabled to avoid the pruned host vhost-vDPA source; `vhost_kernel`, `vhost_user`, and `vhost_crypto` are disabled to prevent unresolved generic host-vhost references; `vhost_net` remains disabled. Plain guest VirtIO Block/Net use QEMU userspace queues and do not require host vhost acceleration. The VirtIO Net PCI frontend no longer requests the unavailable optional `efi-virtio.rom`; UEFI/PXE network boot is out of scope. Linux KVM and Windows TCG remain separate and unchanged.

* Standard launch defaults: `virtio-blk-pci` for the system disk and `virtio-net-pci` for networking.
* Keep **PIIX IDE** and **RTL8139** enabled as independent fallbacks. A system-disk controller change must not implicitly change the network adapter, or vice versa.
* Keep the CD-ROM on the existing **PIIX IDE** path. Do not enable VirtIO SCSI or a VirtIO CD-ROM in this task. T12 may later expose an optional VirtIO SCSI CD-ROM choice after its device implementation and XP driver path are restored and validated; PIIX IDE remains the default.
* For a fresh XP installation, load the XP x86 `viostor` driver from `virtio-win-0.1.141_x86.vfd` through text-mode Setup's F6 flow. Install the XP x86 NetKVM driver after setup. Existing IDE installations must have `viostor` installed before switching their boot disk to VirtIO Block.
* Owner-led disk and network performance comparisons remain manual and workload-specific. Keep benchmark scripts available for deliberate A/B runs, but do not add recurring performance benchmarks to CI.
## T17 (P1) Restore Linux KVM accelerator; keep Windows TCG-only  -  Status: CLAIMED by ChatGPT 2026-11-19
Earlier host/accelerator stripping removed the KVM implementation (see the historical
Session 1 entry in WORKLOG.md). Restore the Linux KVM build and runtime path, including
the required accelerator sources, x86 KVM hooks, machine/interrupt/clock integration,
headers, stubs, Meson/Kconfig wiring, and any shared dependencies removed with it.
Do not blindly copy an upstream directory: compare against this fork's QEMU base and
current XP-only configuration, then restore the smallest complete dependency set.

Resume checkpoint (2026-11-19): The Linux x86 KVM accelerator, x86 backend,
and PC KVM interrupt/clock integration have been restored with Linux-only Meson
wiring. Windows remains TCG-only. CI exposed two follow-up dependency issues:
KVM nested-state migration data was over-pruned with Xen members, and the disabled
TDX stub imported a Linux-only UAPI header; both were corrected. The next Linux
build exposed the optional RAPL energy-MSR implementation still present in
target/i386/kvm/kvm.c even though xp/strip/03-remove-virt-accels.sh intentionally
removed vmsr_energy.c/.h. Remove that unsupported optional integration and its
orphaned KVM properties/state; do not remove the core KVM backend. KVM runtime
validation on a KVM-capable Linux host remains outstanding.
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
* Preserve the XP machine/device constraints and keep IOMMU, SEV, SGX, TDX, and VFIO
  disabled unless a concrete dependency proves otherwise.
* DoD: Linux product CI builds with KVM support and reports KVM as the only supported
  runtime accelerator; a KVM-capable Linux host boots the XP guest with `-accel kvm`.
  A Linux host without accessible `/dev/kvm` is rejected by the launcher with the
  documented actionable prompt (no fallback). Windows CI builds and runs the TCG checks
  without KVM dependencies. Record exact host/guest validation and do not claim KVM
  runtime success from compile-only CI.


## T18 (P2) Evaluate Windows TCG virtual-memory backend APIs  -  Status: TODO
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


## T19 (P1) General SDL diagnostic logging capture  -  Status: PAUSED (generalized implementation committed; awaiting CI + XP guest verification)
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
* On activation, redirect stderr to `y2kbox-log.log` and enable general
  performance counters, VMVGA local diagnostics, VMPort GuestRPC diagnostics,
  and the relevant VMVGA/VMPort/vdagent trace events. On stop, disable those
  diagnostic sources and restore original stderr. The VM keeps running. The first
  activation truncates the file; later intervals append. Do not emit routine
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

## T20 (P2) Full BIN/CUE CD-image parsing  -  Status: TODO
Implement full BIN/CUE parsing for Y2KBox's emulated CD-ROM, not merely a limited
or strict-subset parser. Support the complete CUE-sheet syntax and all defined
CUE flags (including flags such as `DCP`), with correct track/index and sector
layout handling. Use the CD-ROM references in HANDOFF.md section 9 (libcdio,
86Box, DuckStation, and the CUE-sheet format references) when designing and
validating the parser. Keep this distinct from frontend disc swapping: guest-
visible insert/eject of images in the emulated CD-ROM is tracked under T12.

## T21 (P2) Verify ReShade injection compatibility  -  Status: TODO
Investigate and verify that ReShade injection is compatible with Y2KBox's
Windows/SDL2 display path and the planned SDL2 + Dear ImGui frontend. Identify any
limitations or conflicts in the current host-window/display lifecycle and
implement only the compatibility work actually needed. This compatibility
investigation takes priority over integrating librashader.

## T22 (P3) Consider librashader integration  -  Status: TODO (after T12)
After the SDL2 + Dear ImGui frontend is implemented, consider integrating librashader
as an optional shader/post-processing path. This is deliberately very low
priority; first establish ReShade injection compatibility (T21). Do not let
librashader integration delay the frontend or higher-priority compatibility
work. See HANDOFF.md section 9 for librashader and emulator integration
references.

## T23 (P3) Configurable RAM and vGPU memory presets  -  Status: TODO (after T12)
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

## T24 (P1) Fixed-size SDL guest display window  -  Status: PAUSED (implementation committed; awaiting CI + XP guest verification)
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
* DoD: Linux and Windows SDL builds pass CI; owner verifies windowed guest
  resolution changes, disabled maximize/manual resize, fullscreen toggling,
  and mouse tracking in an XP guest.

