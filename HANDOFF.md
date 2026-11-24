# Y2KBox handoff  -  READ THIS FIRST

Files: `HANDOFF.md` (context, rules, design notes) - `TASKS.md` (claimable work queue) -
`WORKLOAD_ACTIVE_SESSION.md` (latest ten sessions) - `WORKLOG_INDEX.md` (historical navigation) -
`WORKLOG.md` (append-only chronological archive) - `README.md` (user-facing project overview).
Repo: https://github.com/AhayriSG/Y2KBox , branch `main` (always keep it building).

## 0. If you are a new AI/human taking over (2 minutes)

1. Read `AGENTS.md` first. For onboarding, read only sections 0-3 of this file,
   the task-heading/status overview in `TASKS.md`, and the complete entry for the
   task you will claim or continue. Read `WORKLOAD_ACTIVE_SESSION.md` and
   `WORKLOG_INDEX.md`. Do not read this entire handoff, the entire task archive,
   or the full `WORKLOG.md` by default. Use the index to open only relevant
   historical sessions, and verify important old claims against current source
   and current owner decisions. Claim an unclaimed task (highest priority first)
   directly on `main` using the protocol at the top of `TASKS.md`.
2. Follow the model-specific local build policy in section 7: Gemini 4 Neon has
   standing authorization for ordinary local configure/build and standard checks;
   all other agents need explicit owner authorization. Gemini 4 Neon also has
   standing authorization for relevant, bounded benchmark/A-B work; all other
   agents need explicit authorization. Claude's configured status hook reports pushed CI/Codacy results on a later prompt; Gemini/Codex sessions must finish without polling and leave result review to the owner or a later session.
3. Work directly on `main`, commit small, and push often. Do not create branches or
   pull requests. Before every write, fetch the latest `main` and the exact target file;
   verify the HEAD and file blob SHA, and abort/reapply if either has moved. Never force-push.
4. Checkpoint proactively; do not wait until the execution budget is nearly exhausted.
   Push a coherent, safe, recoverable state directly to `main` (never knowingly push
   broken or internally inconsistent changes), then update `TASKS.md` with the current
   status and an exact "Resume here:" instruction, append completed work/evidence/
   outstanding issues to `WORKLOG.md`, and add the concise matching entry to
   `WORKLOAD_ACTIVE_SESSION.md`. If no safe checkpoint exists, do not push broken
   changes merely to preserve progress; document the unpushed work and recovery steps.
The normal workflow is sequential handoff between agents (for example, Claude to Gemini or Codex); the shared files are the durable continuity record, not a shared live memory. Parallel work is allowed only when deliberately intended, with task claims coordinated and writes to `main` serialized. If two tasks touch `hw/i386/pc*.c` (T01, T05, T06), coordinate through TASKS.md.

## 1. Project aim and non-negotiable constraints (owner decisions)

* **Only Windows XP 32-bit guests.** Binary: `qemu-system-i386` only. No 64-bit guests.
* **Guest hardware is fixed** (owner decisions, 2026-11-14):
  - guest RAM **2048 MB, set automatically, NOT user-configurable** (no RAM option at all,
    no 3.5 GB mode, no above-4G / 64-bit-hole logic);
  - guest CPU: model `xp-p4` (Pentium 4 Northwood class), **SSE2 is the maximum**
    (no SSE3/SSSE3/SSE4/AVX in the guest, so those translators can be deleted);
  - **Exactly 1 vCPU** (uniprocessor); multi-vCPU/SMP configurations are outside the product target.
    (decision and reasoning in section 4, Q1);
  - no user-facing QEMU flags (-smp, -cpu, -m, -machine ...). Our own front end will only
    ask for: qcow2 disk (made with qemu-img), floppy image, CD/ISO image.
* **Host accelerator policy (owner decision, 2026-11-19; clarified 2026-11-19):** Linux
  builds and runtime are KVM-only. KVM is mandatory: if `/dev/kvm` is missing or inaccessible,
  the launcher must stop with a clear message telling the user to enable KVM and grant the
  required access. There is no supported Linux TCG runtime fallback. Windows builds are
  TCG/MTTCG-only; WHPX and every other Windows hardware-accelerator path remain out of scope.
  TCG performance development is exclusively for Windows. The XP guest/device target is unchanged.
  * **Current restoration status:** the Linux x86 KVM implementation and build wiring
    have been restored on main. T17 remains in progress until KVM runtime validation on a KVM-capable host is
    completed. Do not claim
    Linux runtime success from compile-only CI.
  * TCG optimization work is exclusively Windows-focused. Internal TCG-only Linux CI/
    development builds may be used when useful, but they are not supported Linux product
    builds or a runtime fallback; Linux TCG timings are not a product performance target.
  * The XP PC machine does not expose IOMMU hotplug or emit DMAR/IVRS tables. The x86 IOMMU
    stub/interface remains only for the generic KVM split-irqchip path. SEV kernel-loader and
    pflash hooks, TDX TDVF setup, and SGX EPC ACPI state are removed. CPU feature-masking
    stubs remain so unsupported features are not exposed. VFIO is disabled in the XP device
    config; removing its dead machine compatibility hook must not remove generic KVM support.
    Optional KVM RAPL guest energy-MSR emulation is unsupported; its vmsr_energy backend was
    removed during XP pruning.
  * **KVM trace-generation checkpoint (2026-11-19):** the restored KVM trace-event
    directories are registered in root `meson.build`. Their directory-local `trace.h`
    wrappers were initially missing, causing undeclared trace calls and linker failures;
    the three wrappers have since been restored from the exact upstream baseline. Linux
    KVM build/CI validation is restored; KVM runtime validation on a KVM-capable host
    remains a separate requirement before T17 can be closed.
* **Host:** x86-64, **x86-64-v3 minimum**, **Windows and Linux only** (no macOS, BSD,
  Solaris, ...). AVX-512 stays runtime-detected (measured: no gain for SSE2 guests).
* **SDL display policy (owner decision, 2026-11-22):** SDL is a fixed-size
  virtual monitor, not a host-resizable desktop window. In windowed mode, create
  a decorated but non-resizable SDL window; the guest display surface is the sole
  source of its client-area dimensions. The titlebar maximize/resize affordance
  must be unavailable/disabled through SDL's non-resizable window style.
  Guest resolution changes may resize the window programmatically. Host
  `SDL_WINDOWEVENT_RESIZED` and `SDL_WINDOWEVENT_SIZE_CHANGED` notifications
  must never be forwarded to `qemu_console_set_ui_info()` or otherwise change
  guest display geometry; if a window manager changes the window size anyway,
  restore the current guest surface dimensions. Fullscreen desktop sizing is
  host-display presentation only and must not change the guest resolution.
  Interactive fullscreen toggling remains `Ctrl+Alt+F`. Do not modify mouse
  emulation or coordinate mapping to compensate for host-side resizing.

* **Performance is the point, for ALL guest instruction classes** (integer, flags, x87,
  MMX/SSE/SSE2, string ops, memory/MMU, interrupts and ring transitions, I/O), not only
  SSE2. SSE2 was merely the first thing measured. See T13.
* **3D: VMware SVGA3D via qemu-vmvga** (https://github.com/qemus/qemu-vmvga: VMware SVGA II
  device with SVGA3D rendered through DXVK; XP driver = VMware Tools 10.0.12, SVGA driver 12.01.08).
  Owner proposed this on 2026-11-14 instead of the VBoxVGA 5.2.x plan; evaluated in section 8a.
  **VBoxVGA 5.2.x remains the documented fallback/alternative** (task T16, notes in 8b).
  Rejected by owner: qemu-3dfx (per-game guest wrappers); virglrenderer / viogpu3d /
  SoftGPU / Wine9x are "not mature enough" for XP (they remain useful references).
* Delete unrelated QEMU code freely as long as the Windows XP-32 guest continues to work
  under Windows TCG and Linux KVM product builds/runtime remain functional. Linux KVM is
  mandatory for the Linux product; Linux TCG is not a supported fallback.
* Later (not now): own SDL2 + Dear ImGui frontend (T12), with Dear ImGui as the chosen primary UI. Do not start a wxWidgets implementation in parallel; reconsider alternatives only if a concrete blocker emerges.

## 2. Current state snapshot (update when it changes)

* Builds and passes the checks on Linux AND Windows (GitHub Actions, MSYS2 UCRT64) - keep CI green. macOS/BSD code removed (T03 pass 1).
* Accelerator policy: Linux product builds expose/use KVM only; Windows builds expose/use TCG only. Only
  `pc-i440fx-11.2` (alias `pc`) remains. Default CPU `xp-p4` (verified in-guest: family 15,
  SSE2 yes, SSE3/SSSE3/SSE4/AVX no); default RAM 2 GiB below 4 GiB (e820 verified);
  exactly 1 vCPU; `max_cpus` is 1 and the `-smp` CLI option is removed. SeaBIOS booted with 2 vCPUs in historical testing before this restriction; that configuration is no longer supported. The owner has booted an XP SP3
  desktop and run XP games. Remaining VMware driver, 3D API, and per-game compatibility
  checks require owner-run guest validation (see the manual checklist in TASKS.md)
  alongside the active agent investigations.
* Source tree 41 MB (upstream 171 MB). 91 enabled device symbols (~247 listed devices).
* Done: see `WORKLOG.md`. In progress: see `TASKS.md` (CLAIMED/PAUSED).
* **T09 / T10 integration on `main`:** `qemu-vmvga` (`vmvga` SVGA II + SVGA3D,
  `vmmouse`, `vmport` with TCG Ring-3 `0x5658`/`0x5659` backdoor exemption) is the
  default and only display device, with Linux `dxvk-native` and Windows
  `d3d9.dll`/`d3d11.dll` host backends. T09 integration and its CI checks are
  complete; T10 host-backend integration is also committed and CI-verified.
  XP driver, display/2D, D3D API, and game compatibility observations are tracked
  separately in the owner-run guest validation checklist in TASKS.md.
* T13a host-x87 arithmetic/load and normal-finite FST m32/m64 fast paths are implemented. Windows five-run A/B medians: FST m32 2.474809 s -> 2.383368 s (+3.69%, 1.038x); FST m64 2.508467 s -> 2.359847 s (+5.92%, 1.063x). These are focused hot-cache results, not general workload gains; see TASKS.md and xp/bench/README.md. Further x87 expansion remains PAUSED; retain the existing guarded fast paths and differential test, and revisit only if representative profiling justifies more work.
* T13d legacy XMM register caching (`c272958`) and the Win64 five-register pressure guard are implemented. Linux A/B: 0.877822 s before vs 0.872720 s after, +0.58% (1.006x). Windows A/B: 3.166927 s before vs 3.132343 s after, +1.09% (1.011x). Both are effectively near-neutral shared-runner measurements, not demonstrated speedups. Windows benchmark setup now uses awk and the UCRT64 runtime directory on `PATH`; both executable preflights and all six benchmark runs succeeded.
* Known "still ON because machine code calls them": memory hotplug, DIMM/NVDIMM, CXL,
  HMAT, VIOT, VMGENID, VMCLOCK, ERST, ACPI CPU/memory hotplug (T01 removes them).

## 3. Code map (where things live)

* TCG core: `tcg/` ; x86 host backend: `tcg/x86_64/` ; x86 guest frontend:
  `target/i386/tcg/` (`decode-new.c.inc`, `emit.c.inc`, `translate.c`).
* PC machine: `hw/i386/pc.c`, `pc_piix.c`, `x86-common.c`, `acpi-build.c`.
* Memory core: `system/physmem.c`, `system/memory.c`. Devices: `hw/*/`.
* Scripted removals: `xp/strip/*` (reproducible). Benchmarks/tests: `xp/bench/`.
* Option defaults: `meson_options.txt` (`x86_version` = 3), device set:
  `configs/devices/i386-softmmu/default.mak`.

## 4. Decisions on the former open questions (owner answers + rationale)

**Q1 vCPU count / host threads / HAL (owner decision, 2026-11-24).** Y2KBox has a strict
**one-vCPU uniprocessor target**. `default_cpus` and `max_cpus` are both 1, and the QEMU
`-smp` command-line option is removed; multi-vCPU/SMP configurations are not supported.
This is a product-scope decision, not a claim that multiple vCPUs are technically impossible
or slower in every workload. Windows XP-era dual-core systems were period-appropriate, but
multi-vCPU TCG/MTTCG execution adds cross-vCPU coordination and synchronization that can
create overhead, particularly for workloads dominated by one guest thread. The impact is
workload-dependent; do not describe two vCPUs as universally slower. Y2KBox chooses the
simpler fixed uniprocessor target and keeps the XP guest on the uniprocessor ACPI PC HAL.
Do not add a second vCPU or experimental SMP mode without an explicit owner decision to
revise this target.
Host threads remain distinct from guest vCPUs: QEMU also runs the main loop/IO, audio,
and 3D rendering on host threads. **Thread pinning** (vCPU thread on a dedicated
fast core, others elsewhere, Windows and Linux affinity APIs) is task T14 and must retain
the fixed one-vCPU guest configuration. Note: "host passthrough" CPU models are a KVM
concept and do not apply to TCG (the guest CPU is emulated); what we *can* do is use host
CPU features inside TCG (AVX2/BMI/etc. already assumed) and pass host resources such as the
GPU through our own 3D path.

**Q2 3D method (owner: unsure, then proposed qemu-vmvga).** Decision (updated 2026-11-14):
**primary = qemu-vmvga (VMware SVGA II + SVGA3D, DXVK backend)**, integration tasks T09-T10 plus owner-run validation; VBoxVGA 5.2.x
(Chromium-based host service) is the frozen alternative, task T16. Reasoning in 8a: an implementation already exists for the same QEMU generation, GPL-2.0-or-later,
XP explicitly supported, no 100k-line port of legacy VirtualBox/Chromium code. XP now boots in the
owner's environment; remaining D3D9 acceleration, D3D8/DirectDraw coverage, and Windows-host
visual correctness require owner-run validation per workload (checklist in TASKS.md).

**Q3 NIC / storage drivers (owner proposed VirtIO as a faster path).** The standard XP
launch profile now uses **VirtIO Net + VirtIO Block**: `virtio-net-pci` for networking and
`virtio-blk-pci` for the system disk. Linux and Windows CI both pass on the validated
source state at `d95d75703577a65e913d2451d0f01bfbb915a477`; owner reports that QEMU launches
and boots XP with VirtIO enabled and that initial interactive performance is substantially
better. The minimal guest VirtIO source set was restored from upstream QEMU 11.1.90
(`9d7d057a558b9a1c232cea390b4481dd20d9c9ce`), including shared core, PCI transport,
Block/Net frontends and implementations, and required headers.

The earlier Linux failures were host-backend build-gate issues, not guest VirtIO queue
requirements. The product explicitly disables `vhost_kernel`, `vhost_user`,
`vhost_crypto`, `vhost_net`, and `vhost_vdpa`; plain guest VirtIO uses QEMU userspace
queues. The optional `efi-virtio.rom` request was removed because this BIOS-based XP
product does not ship a UEFI PXE ROM. Keep Linux KVM and Windows TCG policies unchanged.

Keep **PIIX IDE** and **RTL8139** enabled as independent fallbacks. The future T12
SDL2 + Dear ImGui frontend should expose independent network-adapter and system-disk-controller
selectors, with VirtIO Net and VirtIO Block as their defaults. The CD-ROM is deliberately
separate: retain the existing **PIIX IDE CD-ROM** as the default, preserving its optical
drive emulation behavior. T12 may offer an optional **VirtIO SCSI CD-ROM** choice after
the SCSI device implementation and XP driver path are restored and validated; do not
enable VirtIO SCSI in the current Block/Net restoration.

For pointing input, the supported product device is the PC machine's default **PS/2 IntelliMouse**.
Do not add USB mouse (`usb-mouse`) or USB tablet (`usb-tablet`) selectors or recommend
those devices for XP games; their relative/absolute input behavior can cause tracking,
camera-recentering, and legacy DirectInput compatibility problems. Keep the shared USB HID
implementation available for USB keyboard support, but do not remove or fork upstream HID
source merely to hide its optional mouse/tablet models. They are not part of Y2KBox's
supported device profile and must not be instantiated by product presets.

For a fresh XP installation, load the XP x86 `viostor` driver from
`virtio-win-0.1.141_x86.vfd` through text-mode Setup's F6 flow; install XP x86 NetKVM
after setup. An existing IDE installation must have `viostor` installed before switching
its boot disk to VirtIO Block. Owner-led disk/network performance comparisons remain
manual; do not add recurring CI performance benchmarks.
**Q4 RAM.** Current decision (owner): always 2048 MiB, automatic, no user setting.
Implemented as `mc->default_ram_size = 2 GiB` in `hw/i386/pc.c`; the above-4G/hole64
code is dead and is removed by T01. This is a product default, not a claim that 32-bit
XP cannot use more physical RAM. A future GUI option up to 4 GiB may be considered,
but actual XP-visible RAM can be lower because PCI/MMIO ranges occupy part of the
32-bit physical address space. More guest RAM does not enlarge a normal 32-bit
process's default 2 GiB user virtual address space; XP Professional's /3GB boot
switch and an executable's LARGEADDRESSAWARE flag are separate, compatibility-sensitive
requirements for a process to use up to 3 GiB.

## 5. Findings and design notes (evidence)

### 5a. SSE2 performance diagnosis
* AVX-512 on/off: reg loop 3.13 s vs 3.09 s; mem loop 8.11 s vs 8.14 s -> no gain.
* Register SSE2 ops are already inline TCG vector ops (add_vec, mul_vec, xor_vec,
  cmp_vec, shri_vec...), but each is `ld_vec env -> op -> st_vec env` (XMM regs are not
  TCG globals; keeping them in host regs is T13). T04 now lowers legacy XMM
  PSHUFD/PSHUFLW/PSHUFHW, PUNPCK*, PACK*, and SHUFPS through native host vector ops;
  MMX and VEX/YMM forms retain their existing helper paths. Dedicated shuffle/pack
  functional tests pass on Linux and Windows CI. Linux CI's isolated PSHUFD benchmark
  measured 3.083675 s baseline vs 3.166926 s T04 (median of five, AMD EPYC 9V74;
  -2.70%, 0.974x). This does not show a speedup; the small difference from one shared
  runner is not conclusive evidence of a regression. See xp/bench/README.md.
* T13d (implementation commit `c272958`): eligible legacy XMM register-only operations now
  keep values in TCG EBB vector temporaries across consecutive instructions. The cache lazily
  loads guest XMM state, tracks dirty registers, and flushes/frees before non-cacheable
  instructions, exceptions, and TB exits. MMX, VEX/YMM, memory operands, and unsupported vector
  operations keep their previous paths. The cache covers packed integer arithmetic, compare,
  min/max/multiply, logical operations, immediate PSRLD, and the T04 shuffle/unpack/pack family.
  The shuffle functional test adds a MOVD barrier case. Linux CI compared the same 100M-iteration
  `sse2.asm` image against the pre-cache artifact (run 44156398064), median of three runs:
  0.877822 s before vs 0.872720 s after (+0.58%, 1.006x). The Win64 refinement caps live
  cached guest XMM values at five and preflights each instruction's complete operand set.
  Windows CI compared the same 300M-iteration image against the pre-cache artifact (median
  of three runs): 3.166927 s before vs 3.132343 s after (+1.09%, 1.011x); both executable
  preflights and all six benchmark runs succeeded. Both host results are effectively
  near-neutral shared-runner measurements, not demonstrated performance improvements.
* The load hotspot (fixed, below): int-pair load + two 64-bit env stores + 128-bit
  reload = store-forwarding stall (~15-20 cycles).

### 5a1. General-instruction baseline (see xp/bench/README.md)
Near native: integer ALU, imul/div, branches, memory-operand ops, push/pop. Slow: x87 (softfloat), call/ret, rep string ops. Plan in TASKS T13a-c.

### 5a3. DONE (part 1): x87 host fast path (T13a)
Code: `target/i386/tcg/fpu_helper.c` (block "XP fork (T13a)"). Kill switch: env `XPQEMU_NO_HX87=1`.
* **Idea:** host is x86-64, so its x87 unit has exactly the guest's semantics. Measured: `fldt,fldt,faddp,fstpt` =
  2.6 ns (softfloat ~24 ns), BUT `fnclex` costs ~60 ns on this CPU - never use it per operation.
* **Fast path conditions:** both operands normal finite (exp not 0/0x7fff, integer bit set); guest CW has all
  exceptions masked (`fpuc & 0x3f == 0x3f`); precision field != reserved 01. Otherwise softfloat (unchanged).
  Host CW = guest `fpuc & 0xf00 | 0x3f`, cached per host thread (`fldcw` only when it changes).
* **Flags invariant:** host sticky SW bits are always a subset of the guest's; after each op the host bits are
  OR-ed into the guest SW via `fpu_set_exception`. Anything that clears/loads guest exception flags
  (fclex, fninit, do_fldenv, do_fxrstor, cpu reset, vmstate post_load) calls `x86_hx87_flags_changed()`,
  which bumps a global epoch; each host thread runs `fnclex` lazily when its epoch/owner differs (also covers
  two vCPUs sharing one host thread). If you add a new place that clears guest FPU flags, call it there.
* **Inline asm note:** AT&T `fsub/fdiv` forms with a `%st(i)` DESTINATION are swapped by GNU as; the code uses
  only the `... %st(1), %st` (destination st0) forms, which are unambiguous. Verified by the tests.
* **Accuracy bug found in stock QEMU:** `fld m32/m64` honoured precision control (rounded + inexact flag). Real
  x87 loads are exact; XP/Direct3D runs at 24-bit PC, so upstream gave wrong values. Fixed (`fx_exact_conv*`).
  Other conversions may share this bug (`fild` etc.) - see T13a TODO.
* **Tests:** `xp/bench/x87diff.asm` (+ `x87diff_ops.asm` per-op variant, `x87cmp.sh`): random operands x 12
  precision/rounding modes x 30 ops; compare the same binary with and without `XPQEMU_NO_HX87=1`. Expected
  checksum for TRIALS=20000 is in `xp/bench/x87diff.expected.txt`. Run it after ANY x87 change.
* **Results:** x87add 65.3 -> 21.7 ns, x87div 153.9 -> 33.5 ns per iteration (3.0x / 4.6x).
* **FST m32/m64 store fast path (commit `bb8a17f`):** for normal finite ST0 values,
  native host `fstps` / `fstpl` performs destination-format rounding under the guest RC;
  host exception bits are merged into guest status. Exceptional/non-normal inputs and
  unsupported control-word cases retain the softfloat conversion. Windows CI's randomized
  differential test passed after this change. The dedicated same-binary A/B benchmark uses
  5M non-popping stores per width and five-run medians. First successful Windows result:
  FST m32 2.474809 s softfloat vs 2.383368 s host-x87 (+3.69%, 1.038x); FST m64
  2.508467 s vs 2.359847 s (+5.92%, 1.063x). Both results support retaining the fast path,
  but the constant-ST0/hot-destination microbenchmark does not establish a general x87 or
  application-level speedup.

### 5a4. P0: Windows TCG scene-transition stalls (T13e)

**Current result (2026-11-22): the main FlatView topology bottleneck has been
identified and substantially reduced.** GART map/unmap transaction commits were
expensive because each commit regenerated shared FlatView memory topology.
Recursive `render_memory_region()` repeatedly inserted ranges into a sorted
array, shifting large portions of that array for millions of insertions.

The ordered-tree renderer in `system/memory.c` is now the unconditional
default. It preserves the recursive priority/overlap traversal, stores ranges
in a GLib `GSequence` while rendering, then flattens once into the normal
FlatView array before the existing simplification and dispatch stages. The
failed batched-array prototype and `Y2KBOX_FLATVIEW_BUILDER` selector were
removed. The legacy renderer remains only as the reference for
`Y2KBOX_FLATVIEW_VERIFY=1`, which compares output before and after
simplification. The owner reports that this equivalence check passed. Keep
verification unset during normal gameplay and performance captures.

Owner's separate performance capture measured 1.713 s of topology generation
across 795 roots, versus 17.965 s in the earlier batched-prototype capture.
GART map commits measured 1.508 s across 138 operations and unmap commits
0.235 s across 159 operations. Maximum `cpu.exec` duration was 85.693 ms,
with no slices above 100 ms, compared with 399.384 ms and 144 such slices in
the earlier capture. These captures were not perfectly controlled A/B runs,
so treat the size of the improvement as strong evidence, not a precise
benchmark ratio. The batched prototype was ineffective: 3.88 million batches
averaged about one range and copied approximately 23.68 billion range entries.

Owner's game validation:
- Joint Operations: previous 1-3 second freezes and accompanying audio
  interruptions were no longer noticeable.
- FIFA 2003: major improvement; occasional roughly one-second hitches remain.
- F1 Challenge '99-'02: scene-transition stalls improved, but crowded starting
  grids still cause sustained slideshow-like performance until other cars
  leave view.

The remaining FIFA hitches and F1 crowded-grid slowdown are separate profiling
targets. Do not assume they share the FlatView cause or attribute them to
TCG, DXVK, SVGA3D, shader compilation, or scheduling without targeted
evidence. The tree's temporary nodes/payloads add allocation and peak-memory
overhead; this was not separately measured.

**Validation still outstanding:** no local build/test was run and Windows/Linux
CI was not checked after promotion. Do not claim CI passed or mark T13e DONE
until the required CI validation is reported. Chronological profiling captures,
instrumentation stages, abandoned prototypes, and commit history are in
`WORKLOG.md`; current task status and resume criteria are in `TASKS.md`.

### 5a2. DONE: `qemu_ld_vec` / `qemu_st_vec` (128-bit guest memory <-> XMM)

New TCG ops (include/tcg/tcg-opc.h) used by `gen_ldo_env_A0`/`gen_sto_env_A0`
(target/i386/tcg/translate.c). Generators: `tcg_gen_qemu_{ld,st}_v128`
(tcg/tcg-op-ldst.c, wrappers in include/tcg/tcg-op-mem.h). Backend
(tcg/x86_64/tcg-target.c.inc): `tgen_qemu_ld_vec/st_vec` emit VMOVDQA (known
16-aligned) or VMOVDQU directly to/from the vector reg; constraints
`C_O1_I1(x,L)` / `C_O0_I2(x,L)`; slow path reuses the i128 helper calls and moves
RAX:RDX <-> xmm (`TCGLabelQemuLdst.is_vec/vec_reg`). Limitations: little-endian
only (asserted); plugin memory callbacks are not emitted for these ops.

**Measured (old binary vs new, same host, min of 2, 200M iters unless noted):**

| image | old | new |
|-------|-----|-----|
| aligned load `movdqa xmm,[m]` | 1.56 s | 0.58 s |
| unaligned load `movdqu` | 1.55 s | 0.64 s |
| `paddd xmm,[m]` | 1.60 s | 0.65 s |
| aligned store | 0.66 s | 0.59 s |
| mixed mem loop (300M) | 6.20 s | 2.82 s |
| register-only loop (300M) | 2.31 s | 2.31 s |

**Tested** with `xp/bench/vectest.asm` (prints OK/FAIL<n> on port 0x402): unaligned
loads/stores across 4 KB page boundaries, aligned ops, misaligned MOVDQA raising
#GP (load and store), memory-operand ALU op vs register form, and loads from VGA
text RAM (MMIO slow path) vs integer loads. Historically passed on the old reference
binary and the new one with `-smp 1` and `-smp 2`; the latter is no longer a supported
product configuration. SeaBIOS still boots. Not yet tested: real XP,
AVX-guest atomicity path (MO_ATOM_IFALIGN), a stress test under MTTCG.


### 5b. Config-driven pruning (done) and how to repeat it

Because there is exactly ONE configuration, the build tells us what is dead:
`ninja -t commands qemu-system-i386` gives the exact compiled C files and
`ninja -t deps` the exact headers. `xp/strip/06-remove-dead-sources.py BUILD_DIR`
deletes unbuilt C files in config-driven dirs only (hw/, target/, qga/, ebpf/,
non-x86 disas) and strips their names from meson.build. **Do not** apply this to
host-dependent backends (ui/, audio/, net/, block/, util/, crypto/, chardev/,
io/): they look dead only because this sandbox lacks host libraries. `default.mak`
now lists the XP device set; switching a family off needs any conflicting Kconfig
`select` lines removed (see git history for the 12 removed). Lesson learned: run the
boot/functional test after pruning - removing `efi-e1000.rom` broke the default NIC.


## 6. Build and environment

(Reference only: CI builds for you - see section 7 for Gemini 4 Neon's model-specific local build and benchmark authorizations.)

    sudo apt install libfdt-dev libslirp-dev nasm pkg-config   # + glib2/pixman dev, python3
    pip install meson ninja tomli
    # subprojects/keycodemapdb: if gitlab.com is blocked, clone
    #   https://github.com/qemu/keycodemapdb at the rev in its .wrap file
    mkdir build && cd build
    ../configure --target-list=i386-softmmu --disable-tools --disable-guest-agent --disable-werror
    ninja qemu-system-i386        # ~10 min on 2 cores
    # qemu-img (needed for qcow2): configure WITHOUT --disable-tools, `ninja qemu-img`

Always do a FRESH configure after changing `meson_options.txt` / `default.mak` defaults
or meson files (old build dirs keep old option values). Strip script `05` is not
idempotent (`git checkout` touched files first).

## 7. Standard Checks & CI Verification (Token-Optimized)

- **Local build policy (model-specific):** Gemini 4 Neon has standing owner
  authorization to run ordinary local QEMU configure/build and relevant standard
  functional checks in its sandbox while validating its own changes. This does
  not extend to Gemini CLI generally, other Gemini models, Claude, Codex, or
  other agents; they must obtain explicit owner authorization before running
  `./configure`, `make`, or `ninja`. Use the established build configuration
  and avoid unnecessary clean/full rebuilds. Gemini 4 Neon also has standing
  owner authorization for relevant, bounded performance benchmarks and
  controlled A/B experiments using established scripts/methodology, including
  `xp/bench/run.sh` and `xp/bench/perf.py`. Preserve comparable before/after
  evidence, avoid needless repetitions or unnecessarily long campaigns, and
  report exact commands, workload, configuration, and results. This separate
  exception applies only to Gemini 4 Neon; all other agents still need explicit
  owner authorization before benchmarking or A/B work.
- **Verification and reporting:** GitHub Actions remains the authoritative
  shared Linux/Windows CI result. Local build/test results are supplementary:
  report the exact configuration, commands, and checks performed, and clearly
  distinguish local results from CI. Never claim CI passed based on a local
  build. Review the complete diff and never knowingly push broken or internally
  inconsistent changes. Before pausing an incomplete task, follow the checkpoint and handoff sequence in section 0 above. Update `HANDOFF.md` only for lasting decisions, constraints, or important current-state facts; keep task status in `TASKS.md`, chronological evidence in `WORKLOG.md`, and the concise recent-session context in `WORKLOAD_ACTIVE_SESSION.md`. If there is no safe checkpoint, leave broken/unverified changes unpushed and state exactly what remains to be recovered or verified.
- **CI pipeline (`.github/workflows/build.yml`):** Linux CI builds the Linux KVM product and runs accelerator-policy,
  functional, x87 differential, packaging, and smoke checks; it does not run TCG performance benchmarks.
  Windows CI builds the TCG product and runs functional checks plus the x87 differential test
  (`xp/bench/x87cmp.sh`), then packages and smoke-tests the artifact. The completed T13d SSE2
  and T13a FST A/B measurements are historical results; they are no longer repeated on every
  artifact build. Keep their benchmark sources/scripts for deliberate manual A/B runs when
  evaluating future performance changes.
  The workflow detects changed paths first: when every changed file ends in `.md`, only the
  lightweight `changes` job runs; Linux and Windows build/test jobs are skipped entirely,
  so no binaries or artifacts are produced. Any non-Markdown change and manual
  `workflow_dispatch` run execute the normal Linux and Windows build, functional checks,
  packaging, packaged-layout smoke tests, and artifact uploads. `xp/bench/run.sh` reports raw median
  wall time without subtracting an empty-loop control. It rejects unexpected QEMU exit statuses,
  timeouts, and stderr output. Normal runs upload `y2kbox-linux-x86_64` and
  `y2kbox-windows-x86_64` (`-L pc-bios`).
- **CI & Codacy feedback loop (never poll after pushing):**
  - The `UserPromptSubmit` hook (`scripts/session_status.sh`, registered in `.claude/settings.json`; also runs on
    `SessionStart` after compaction) injects GitHub Actions and Codacy status whenever `HEAD` has an unverified push, and
    exits silently once the HEAD run is `completed` and reported (cache `/tmp/y2kbox_ci_checked_<sha>`).
  - If `conclusion=failure`, inspect only the failed step (the hook prints job/step names and annotations; more detail via
    the run URL or `gh run view <run_id> --log-failed`) and fix the regression before starting new feature work.
  - **After pushing:** commit, push, and **finish your turn immediately**. Do not wait or poll for Actions or Codacy.
- **Secrets:** `CODACY_API_TOKEN` (and optional `GITHUB_TOKEN`) come from the environment only, never from files.
  `.claude/settings.local.json` and `.env*` are git-ignored. Codacy's API is not reachable from the Claude sandbox
  network (allowlist), only from the owner's machine, so the Codacy half of the hook runs there.
- **Manual reference (humans / benchmarking only)** - what the CI checks do:

      ./qemu-system-i386 -accel help                        # only: tcg
      ./qemu-system-i386 -L pc-bios -accel tcg -display none -monitor none -serial none \
        -chardev stdio,id=d -device isa-debugcon,iobase=0x402,chardev=d     # "Found 2 cpu(s)", "Booting from ..."
      nasm -fbin xp/bench/vectest.asm -o vectest.img   # run with isa-debug-exit + debugcon, expect "OK"
      xp/bench/x87cmp.sh build 2000                    # all 30 operations identical
      xp/bench/perf.py QEMU PCBIOS                     # performance (local build/benchmark execution: Gemini 4 Neon only under standing authorization; every other agent needs explicit owner authorization)

## 8. 3D research notes

### 8a. PRIMARY: qemu-vmvga (VMware SVGA II / SVGA3D)  - evaluation 2026-11-14 (read from the repo)
* Repo https://github.com/qemus/qemu-vmvga : a 44-file **overlay** for a QEMU tree (`hw/display/vmware_vga.c`,
  `vmware_vga_3d*.c`, `vmware_vga_vgpu9/10/11.c`, `vmware_vga_dxvk*.c`, `vmware_vga_gmr.c`,
  `hw/display/include/svga*.h` (public VMware SVGA3D headers), `hw/i386/vmport.c`). Prebuilt binary is based on
  QEMU 11.1.1 (our base is 11.1.90, so the overlay should apply with modest adaptation). License:
  GPL-2.0-or-later (compatible). Derived from QEMU's own `vmware_vga.c` (Zaborowski) plus later work
  (Christopher Eric Lentocha).
* Provides the full legacy 2D FIFO command set and SVGA3D (D3D9-level vgpu9; vgpu10/11 for newer guests);
  3D is rendered via DXVK -> Vulkan. README host requirements: Vulkan-capable GPU + `dxvk-native`.
  Usage: `-device vmvga` and `-machine ...,vmport=on` (VMware backdoor port).
* **XP is explicitly supported:** README table: Windows XP / Server 2003 -> SVGA driver 12.01.08 from VMware
  Tools 10.0.12 (`https://packages.vmware.com/tools/frozen/windows/winPreVista.iso`; also Win2000 11.09.01).
  Other rows (Vista 8.16.01, 7/8 8.17.03, 10/11 9.17.11, 95/98/ME 10.7.0) are not our concern.
* **TCG Ring-3 backdoor exemption (T09 integration implemented):** `gen_check_io` -> `HELPER(check_io)`
  (`target/i386/tcg/system/seg_helper.c`) now exempts `0x5658` and `0x5659` (`size <= 4`) when
  `vmport_tcg_backdoor_enabled` is true (`hw/i386/vmport.c`). Verified on CI via `xp/bench/vmport_ring3_test.asm`
  (CPL=3, IOPL=0, 0xFF TSS IOPB: allows `0x5658`/`0x5659`, traps `#GP` on `0x565A`). This completes the T09 integration/CI scope; real XP driver and application acceptance is recorded separately in the owner-run manual checklist in TASKS.md.
* **Host-side 3D backends (T10 integration implemented):**
  * **Linux:** `hw/display/vmware_vga_dxvk_wsi.c` synthesizes a sealed-memfd `libSDL2-2.0.so.0` WSI shim (`"SDL2"`),
    or `libSDL3.so.0` (`"SDL3"`) implementing the 21 `wsi_platform_sdl3_funcs.h` symbols if `--enable-sdl` already
    loaded SDL2 (`RTLD_NOLOAD`). `vmware_vga_dxvk.c` searches `<exec_dir>/`, `<exec_dir>/lib/`, then system paths,
    pre-loading `libdxvk_dxgi.so.0` (`RTLD_GLOBAL`). CI ships `dxvk-native` 3.1.1 `.so.0` libs into `dist/` and runs
    a headless smoke check with `Y2KBOX_3D_HEADLESS=1` + `lvp_icd.x86_64.json` (lavapipe).
  * **Windows (`CONFIG_WIN32`):** `vmware_vga_dxvk_wsi.c` creates a hidden `WS_POPUP | WS_DISABLED` `"STATIC"`
    `HWND` (`"Win32"`), and `vmware_vga_dxvk.c` loads `"d3d9.dll"` / `"d3d11.dll"` via `LoadLibraryA` with
    `D3D_DRIVER_TYPE_WARP` fallback on D3D11 (working with inbox Windows Direct3D or optional DXVK Windows DLLs
    placed beside `qemu-system-i386.exe`).
  * **Headless benchmark safety:** `vmsvga3d_renderer_realize()` skips 3D backend init when `-display none` is
    active (`display_backend_active == false`) unless Y2KBox diagnostics (`-y2kboxdebugmode simple|on`) or `Y2KBOX_3D_HEADLESS` is set.
* Risks: single small upstream; D3D9 is the ceiling for the XP driver (no DX10+); D3D8 and DirectDraw/D3D7
  coverage unknown (owner-run validation: try `d3d8to9` and a ddraw wrapper in the guest); performance of the device thread
  under TCG; Vulkan requirement. Benefits: vmmouse/absolute pointer comes with the VMware stack.


### 8b. ALTERNATIVE (frozen, task T16): VBoxVGA 5.2.x
* 86Box PR #6183 ("Add Initial Support for VBoxVGA Emulation") extends the Bochs
  VBE device; it is early/stalled, shows no 3D, XP BSODs without one register; it
  reports Guest Additions < 4.0.0 only (3.2.28 best) because of 86Box's missing
  SSE (not a VBoxVGA limit - QEMU TCG has SSE2).
* **Licensing:** VirtualBox **7.x is GPL-3.0-only - do not copy** into this
  GPLv2 tree. The **5.2.x / 6.1.x** headers checked (DevVGA.cpp, DevVGA_VBVA.cpp,
  VMMDev.cpp, crservice.cpp) say **GPL version 2** (plus MIT for the Bellard VGA
  core). Audit every file's header before porting; not legal advice.
* Official mirror https://github.com/VirtualBox/virtualbox has tags v5.2.2, v6.0.x,
  v6.1.x, v7.x. **The Chromium-based 3D host service (`HostServices/SharedOpenGL`,
  `GuestHost/OpenGL`) exists in v5.2.2 but not in v6.1.4**. XPDM display driver is
  in `src/VBox/Additions/WINNT/Graphics/Video/{disp,mp}/xpdm`; it is 2D/VBVA/VHWA;
  3D goes through the Chromium path (HGCM via VMMDev, PCI 80ee:cafe).
  Use sparse + blobless clone (`git clone --filter=blob:none --no-checkout
  --depth 1 --branch v5.2.2 ...`; never `git grep` on a blobless clone - it
  downloads everything and times out).
* Needed pieces: VBoxVGA (VBE dispi + HGSMI + VBVA, see `DevVGA_VBVA.cpp`, `HGSMI/`),
  VMMDev + HGCM, GL stream receiver/replay (crserver). Open questions: Direct3D for
  XP guests (vs OpenGL only), which GA version to target (plan: 5.2.x, with 3.2.28
  as the bring-up baseline).



### 8c. Owner's Gemini conversation (summarised; NOT verified, some claims are outdated or wrong)
The owner shared a conversation arguing VBoxVGA is "the" XP 3D option and that qemu-vmvga would need
reverse-engineering. Checked against the repo: qemu-vmvga already implements SVGA3D (so that part is
wrong), and it lists an XP driver. Useful, plausible points kept: with either approach the guest driver is a
fixed protocol client, so the host side (renderer, display resize, batching) can be modernised without
touching guest binaries, but the binary protocol must be matched exactly; VirtualBox 6.1 removed the legacy
VBoxVGA 3D path, tying that route to Guest Additions <= 5.2.x. Claims about XP D3D8/9 via the Chromium
bridge were not verified. Treat that conversation as opinion, this section as evidence.

### 8d. Windows XP guest OpenGL diagnostic result (owner-run, 2026-11-24)

The owner tested COD1 with `+set r_allowSoftwareGL 1` and inspected the guest with GLview's OpenGL and Extensions tabs. The active implementation is **Microsoft / GDI Generic / OpenGL 1.1.0**. It exposes only three extensions: `GL_EXT_bgra`, `GL_EXT_paletted_texture`, and `GL_WIN_swap_hint`. This is Microsoft's generic software OpenGL implementation, not a VMware OpenGL ICD and not evidence that guest OpenGL calls are reaching Y2KBox's accelerated SVGA3D path.

COD1's software-allowed path successfully selected a generic pixel format, created a WGL context, and made it current, then shut down with its missing-video-card-features fatal error. The failure is therefore past pixel-format selection and WGL context creation; the exposed GL implementation lacks functionality COD1 requires. This does not identify a defect in the host SVGA3D renderer because the guest is not currently exposing an accelerated OpenGL implementation.

**GLview-reported details** (owner transcription; pixel-format tab was not captured):

- Extension-check groups: OpenGL 1.1 group 0/2 (`GL_EXT_vertex_array`, `GL_EXT_texture_object`; transcribed as `GL_EX_texture_object` by the owner); 1.2 group 1/11 (only `GL_EXT_bgra` supported; unsupported: `GL_EXT_texture3d`, `GL_EXT_packed_pixels`, `GL_EXT_rescale_normal`, `GL_EXT_separate_specular_color`, `GL_EXT_texture_edge_clamp`, `GL_SGIS_texture_lod`, `GL_EXT_draw_range_elements`, `GL_EXT_blend_color`, `GL_EXT_blend_subtract`, `GL_EXT_blend_minmax`); 1.4 group 1/15 (only `GL_WIN_swap_hint` supported); 1.3, 1.5, 2.0, and 2.1 groups all 0 supported. These are the viewer's extension checks, not a formal conformance certification.
- Limits: maximum viewport 16384x16384; maximum texture size 1024x1024; auxiliary buffers 0; sub-pixel bits 3; attribute stack 16; model-view stack 32; name stack 128; projection and texture stacks 10 each; six clip planes; eight light sources; display-list nesting 64; pixel-map table 65536; evaluator order 30.
- Line width and point size ranges are both 0.5–10.0 with 0.125 granularity. Texture environment modes: `GL_EYE_LINEAR`, `GL_SPHERE_MAP`; texture-environment combine modes: `GL_MODULATE`, `GL_DECAL`, `GL_BLEND`, `GL_REPLACE`.

**Next step:** investigate why the XP VMware Tools/SVGA driver stack loads GDI Generic rather than a VMware OpenGL ICD, including driver installation and ICD registration. Do not require the owner to install Visual Studio or compile the optional `xp/tools/glprobe.c` utility in the guest. GPU Caps Viewer is optional corroboration; the GLview test need not be repeated. A Mesa Windows software renderer, if tried, must remain app-local and be treated as a software fallback, not as proof of SVGA3D acceleration.


### 8e. VMware's documented XP OpenGL limitation and software fallback research (2026-11-24)

The owner's GLview result (section 8d) is consistent with VMware's documented product behavior, not evidence of an incomplete VMware Tools installation:

- VMware Workstation Player 15 User's Manual, **Using Accelerated 3D Graphics**, states that DirectX 9 accelerated graphics apply to Windows XP, Vista, and 7 guests, while **OpenGL applications run in software-emulation mode**: https://sw5-prod-media-files.s3.eu-central-1.amazonaws.com/media/pdf/3d/31/4a/workstation-player-15-windows-user-guide.pdf
- VMware Workstation 6 User's Manual documents the same limitation for its experimental 3D support for Windows 2000/XP guests: OpenGL applications run in software emulation mode: https://ist.mit.edu/sites/default/files/migration/products/vmware/ws6-manual.pdf
- Broadcom's legacy-guest support article confirms VMware Tools 10.0.12 is the final Tools release supporting Windows 2000, XP, and Server 2003: https://knowledge.broadcom.com/external/article/316388

Taken together with the owner's Microsoft / GDI Generic / OpenGL 1.1.0 report, this strongly indicates the expected XP OpenGL path is Microsoft's generic software implementation, not a missing VMware ICD. The Workstation Player manual is product-level evidence; it does not document every internal detail of the exact VMware Tools 10.0.12 package. Nonetheless, **do not ask the owner to reinstall VMware Tools, hunt for `vm3dgl.dll`, or repeat GLview absent new contradictory evidence.** The supported XP configuration provides DirectX 9 acceleration but not accelerated VMware OpenGL.

**Potential software-only diagnostic:** [WINVBE](https://github.com/ages2001/WINVBE) v1.3 documents Windows XP support and an OpenGL 2.1 / GLSL 1.20 ICD (`WVBGL32.DLL`) built around Mesa 7.8.2. Its README says the ICD replaces the GDI Generic 1.1 implementation and supports programs that require multitexturing or shaders. However, WINVBE is a complete alternate VBE display-driver stack, not an app-local ICD DLL that can simply be dropped into the VMware driver installation. Its documented NT install replaces the active display driver; its known limitations explicitly say Direct3D and OpenGL run on the CPU and that there is no hardware 2D/3D acceleration. It could be evaluated only in a disposable VM snapshot as a software-OpenGL compatibility experiment; it is not a solution for Y2KBox's SVGA3D acceleration goal and may displace the existing VMware D3D9 path. WINVBE's overall license is GPL-3.0, while its embedded Mesa 7.8.2 code is MIT; do not reuse WINVBE code in this GPL-2.0-only project without a separate licensing review.

**Updated architectural conclusion:** the host SVGA3D implementation cannot make OpenGL appear inside XP by itself. A Windows XP-compatible guest-side OpenGL ICD is required to translate WGL/OpenGL calls into the SVGA3D protocol. Mesa's current `vmwgfx`/Gallium SVGA driver is Linux-specific and remains a reference for command translation and semantics, not a drop-in XP ICD. T25 should focus on the feasibility and scope of that guest ICD plus the host SVGA3D state/command coverage it requires.

## 8f. Windows XP ICD discovery constraint and non-replacing D3D9 route (2026-11-24)

The earlier proposal of a conventional XP OpenGL ICD that can simply be registered alongside the VMware display driver was too optimistic. Windows XP's ICD discovery path requires cooperation from the active display driver:

- WINVBE's maintainer documents that Windows NT 3.5 through XP's OPENGL32 runtime asks the active display driver for an ICD through the OPENGL_GETINFO escape. The driver identifies itself, and the INF registers the ICD DLL under OpenGLDrivers. The README also distinguishes the later pixel-format model: Windows 2000 and later ask the ICD for pixel formats. See https://github.com/ages2001/WINVBE (sections "OpenGL ICD" and "Windows NT").
- ReactOS's OpenGL loader provides corroborating implementation detail: it first asks GDI whether the driver supports OPENGL_GETINFO via QUERYESCSUPPORT, then queries the driver for the ICD name/version using ExtEscape before it reads the OpenGLDrivers registry entry and loads the DLL: https://github.com/reactos/reactos/blob/master/dll/opengl/opengl32/icdload.c. ReactOS is corroboration, not a substitute for Microsoft's XP source.
- Therefore adding an ICD registry key or DLL alone is not enough on XP. A standard ICD can coexist with the VMware display driver only if that driver already implements and advertises the required escape. The owner's current GDI Generic result and VMware's documented XP software-emulation behavior show no active VMware ICD. No evidence currently establishes that VMware Tools 10.0.12's XP display driver supports OPENGL_GETINFO. Do not assume a registry-only installation can activate a third-party ICD, and do not modify or replace the VMware display driver to force this route.

**The only plausible route that preserves the VMware display driver is an application-loaded OpenGL implementation rather than a conventional system ICD:**

- Mesa's Windows llvmpipe documentation describes placing a replacement opengl32.dll and its WGL/Gallium DLL beside an application. This demonstrates an app-local deployment pattern that bypasses the normal system ICD selection, but the documented llvmpipe renderer is CPU software only: https://docs.mesa3d.org/drivers/llvmpipe.html. This is not proof that the same deployment works unchanged on XP; XP DLL loading and redirection still need verification.
- The original Quake III Windows renderer accepts a separately loaded OpenGL DLL through its r_glDriver setting. It classifies a DLL whose name does not contain "opengl32" as a standalone driver, and its standalone path uses the driver's qwglSwapBuffers entry point rather than GDI's SwapBuffers: https://github.com/id-Software/Quake-III-Arena/blob/master/code/win32/win_glimp.c. COD1 is id Tech 3-derived, but its exact executable/renderer fork must be checked before relying on the same cvar, loader, and swap behavior. This is a promising COD1-specific way to load a custom DLL without replacing the Windows display driver; it is not a generic solution for every game.
- A custom standalone DLL could use the existing Direct3D 9 runtime as its transport. Microsoft documents that Direct3D 9 operates on the Windows XP display driver model (XPDM): https://learn.microsoft.com/en-us/windows/win32/direct3d9/xpdm-vs-wddm. VMware Workstation Player 15 documents accelerated DirectX 9 for XP guests while OpenGL applications run in software emulation mode: https://sw5-prod-media-files.s3.eu-central-1.amazonaws.com/media/pdf/3d/31/4a/workstation-player-15-windows-user-guide.pdf. Thus a GL-to-D3D9 layer could potentially reuse the existing VMware D3D9 guest driver and Y2KBox's existing SVGA3D D3D9 path, avoiding a second raw SVGA command-submission mechanism.
- Candidate front end: GL4ES translates much of desktop OpenGL 1.5 / 2.1 to GLES 1.1 / 2.0 and its project history records Windows platform support, but it is not a ready-made WGL standalone driver: https://github.com/ptitSeb/gl4es and https://github.com/ptitSeb/gl4es/blob/master/CHANGELOG.md. Candidate GLES2 backend: historical ANGLE releases provided a Direct3D 9 renderer, but ANGLE's D3D9 backend has since been removed (2026 removal: https://chromium.googlesource.com/angle/angle/%2B/c1f9769be2c257fb0edcd7f735d6dd9be6d01d99), and Qt's Windows guidance warns that ANGLE is not fully reliable on Windows XP: https://wiki.qt.io/Qt_5_on_Windows_ANGLE_and_OpenGL. The GL4ES + old ANGLE combination is therefore an investigation lead, not a known-good or drop-in stack. WGL lifecycle, pixel-format behavior, window integration, extension coverage, XP toolchain compatibility, and licensing/version pinning remain to be verified.

**Decision gate:** do not begin a full ICD or renderer implementation. First prove only that COD1 can load a separate standalone DLL and that this DLL can create and present a Direct3D 9 HAL device through the existing VMware XP driver. If either fails, stop. If both pass, build the smallest GL/WGL compatibility slice needed to test COD1's reported requirements before considering broader OpenGL 1.1–2.1 coverage. No VMware driver replacement, WINVBE installation, guest modification, emulator source change, build, or guest test was performed in this investigation.

## 8g. T25 paused pending independent OpenGL feasibility reassessment (2026-11-24)

The current OpenGL investigation has reached a low-confidence feasibility point. **T25 is paused; no implementation work is authorized.** The goal remains OpenGL 1.1 baseline / OpenGL 2.1 maximum while preserving the existing VMware XP display-driver and D3D9 path, but the available research does not establish a viable accelerated OpenGL architecture.

The previous section's standalone-DLL route is only one possible lead. It must not be treated as the answer, the only possible architecture, or an instruction for the next contributor to continue that specific design. A guest-side application-local wrapper DLL could potentially bypass the conventional XP ICD discovery path, but neither its compatibility with COD1's exact renderer nor an accelerated D3D9-backed implementation has been demonstrated. GL4ES plus historical ANGLE D3D9 is likewise speculative. A future reviewer may reject these ideas entirely and propose a better route.

**Request for the next independent reviewer:**

- Reassess the problem from first principles. Verify the XP graphics-driver and WGL constraints, the exact COD1 renderer behavior where relevant, and the existing VMware D3D9 acceleration path.
- Explore alternative architectures, including but not limited to application-local wrappers, translation layers, and other guest-side approaches that preserve the VMware display driver. Do not assume the current candidates are exhaustive.
- Separate proven facts, product/documentation constraints, unverified hypotheses, and practical unknowns. Evaluate Windows XP compatibility, required APIs/toolchains, application coverage, performance prospects, maintenance burden, and licensing.
- Deliver a concise architecture proposal with evidence, feasibility/risk assessment, and the smallest decisive validation experiment. Explain why rejected alternatives fail.
- Do not write implementation code, modify the guest, install or replace display drivers, change the host renderer, or run builds/tests as part of the reassessment unless the owner separately authorizes a specific experiment.

The owner asked for a fresh independent assessment, not an automatic handoff to a named agent. No other agent has been assigned by this update. The project documentation should make it easy for a future contributor or model to take up the question without inheriting the current hypothesis as a requirement.

## 8h. QindieGL application-local wrapper lead and COD1 result (2026-11-24)

[QindieGL](https://github.com/crystice-softworks/QindieGL) is an application-local
OpenGL wrapper that translates to Microsoft Direct3D 9.0c. Its source is a useful
concrete reference for the standalone `opengl32.dll` architecture, but it is an
older, limited OpenGL implementation (up to OpenGL 1.4) and is not established as
a supported COD1 configuration. Its repository is GPL-3.0; do not copy or adapt
its code into Y2KBox without an explicit license-compatibility review, because
Y2KBox is GPL-2.0.

**Owner-run COD1 test in Y2KBox (2026-11-24):** the owner imported QindieGL's
registry settings, placed its Win32 `opengl32.dll` beside the COD1 executable,
and tried both safe-mode and normal launch shortcuts. Both silently exited to
the desktop. The QindieGL log showed Direct3D initialization, adapter-mode and
format enumeration, accepted depth/texture formats, and then:

```text
wglCreatecontext: creating pure device with hardware vertex processing
WARNING: wglGetProcaAddress: queried unknown proc 'wglGetExtensionsStringExT' (stub = 0xBAD00001)
```

The log's procedure-name spelling/capitalization may itself be a formatting
artifact; the relevant WGL procedure is `wglGetExtensionsStringEXT`. Do not
overstate this log as proof that a D3D9 device was successfully created or that
a frame was presented. It shows the wrapper reached context-creation setup and
then returned a stub for an unrecognized WGL procedure.

**Source-level cause:** QindieGL's `code/d3d_extension.cpp` implements
`wglGetExtensionsStringARB(HDC)` and registers that ARB entry point in its
`glext_EntryPoints` table. It does not implement or register
`wglGetExtensionsStringEXT(void)`, and its generated WGL extension strings
advertise `WGL_ARB_extensions_string` and `WGL_EXT_swap_control`, but not
`WGL_EXT_extensions_string`. Its `wglGetProcAddress` wrapper logs unknown
names and returns a synthetic stub address. The owner's addition of a
`WGL_EXT_extensions_string` registry flag did not alter the warning or launch
result. A registry value cannot create a missing function implementation or
entry-point mapping.

**Historical COD1 experiment:** a [DxWnd discussion](https://sourceforge.net/p/dxwnd/discussion/general/thread/307b85b604/)
records the same missing-`wglGetExtensionsStringEXT` warning. A participant
produced an experimental QindieGL rebuild with a minimal EXT entry point; the
tester reported that COD1 then started, but in-game graphics were substantially
corrupted, especially the skybox. Treat this only as evidence that the missing
entry point can block startup. It is not a validated fix, and getting past
startup did not produce correct rendering.

**Research implications and limits:**

- Inspect the historical experimental patch and compare its implementation
  with the EXT function's semantics before considering any change. The EXT
  function has no HDC parameter and is not automatically interchangeable with
  the ARB function's HDC-taking signature; context/DC handling and extension
  advertisement need deliberate review.
- The next useful research step, if T25 is later reopened, is to inspect the
  historical patch and QindieGL's current lifecycle/extension handling, then
  determine whether a narrowly scoped isolated experiment is justified. Do not
  assume the minimal stub is sufficient or that it addresses the reported
  rendering corruption.
- Keep startup, context creation, successful D3D9 device creation, frame
  presentation, and correct in-game rendering as separate validation gates.
- This discovery does not change T25's PAUSED status or authorize a patch,
  build, guest modification, or test. It is a concrete lead for the independent
  reassessment, not a reason to resume implementation automatically.


## 8i. TitaniumGL application-local OpenGL test and COD1 result (2026-11-24)

[TitaniumGL project page](https://titaniumgl.uni.hu/keret.cgi?/index.html)
describes an OpenGL implementation/wrapper that uses Direct3D 9 on Windows.
The owner tested its Win32 `opengl32.dll` as an application-local DLL beside
Call of Duty 1 (COD1) in the Windows XP guest.

**Owner-observed COD1 log and behavior (2026-11-24):**

```text
pixelformat 1 selected
getting DC: succeeded
creating GL context: succeeded
making context: succeeded
0 PFDs found
MCD acceleration found
calling library system32\opengl32.dll succeeded

GLVERSION 1.4
GL RENDERER VMware SVGA II/119MB VRAM
[large GL_EXT / GL_ARB / WGL_ARB / WGL_EXT / GL_OES / GL_SGIS extension list]
...
RE_SHUTDOWN (1)
```

The owner reports that COD1 still displayed its missing-video-card-features
error after the extension list and shut down the renderer. The complete extension
list and exact per-extension capability values were not saved. The owner also
tested TitaniumGL's software-renderer mode and observed the same broad result
as with hardware acceleration; changing that mode did not get COD1 past its
graphics-card rejection.

**What this establishes, and what it does not:**

- The application-local DLL is being loaded by COD1, and the game successfully
  acquired a DC, created a GL context, and made it current.
- COD1 saw OpenGL 1.4 and a renderer string identifying VMware SVGA II with
  119 MB VRAM, along with a much larger advertised extension set than the
  native XP GDI Generic OpenGL 1.1 path.
- The log's `0 PFDs found` line is notable, but it is not yet established as
  the cause of failure: the same log subsequently reports successful context
  creation and making the context current. Do not infer what TitaniumGL means
  by this counter without source or additional diagnostics.
- Advertised extensions do not prove that every associated function is
  implemented correctly or that all limits/formats satisfy COD1's checks.
  Context creation is not equivalent to successful renderer initialization,
  frame presentation, or correct in-game rendering.
- The owner reports the same COD1 rejection in both TitaniumGL hardware and
  software rendering modes. This weakens the hypothesis that the failure is
  caused solely by the selected rendering mode, but does not identify the
  failing capability check.
- The log line about calling `system32\\opengl32.dll` succeeded is recorded
  as reported; its exact loader/forwarding semantics have not been established.

**Research limits and next steps if T25 is reopened:**

- Obtain the exact TitaniumGL build/version and inspect its XP requirements,
  source availability, provenance, and license before considering further use.
  The project page and the owner's successful DLL/context test do not by
  themselves establish that every current release supports XP.
- Capture the complete TitaniumGL log and COD1's exact fatal/console output,
  especially the capability checks immediately preceding `RE_SHUTDOWN (1)`.
  The owner was tired and did not capture the full extension list; do not ask
  for it again unless a later test makes it necessary.
- Investigate the `0 PFDs found` message and COD1's actual feature-check logic
  rather than guessing from the advertised extension names. Compare required
  extensions, function pointers, texture limits/formats, and other queried
  capabilities against what TitaniumGL really implements.
- Keep TitaniumGL and QindieGL as separate experimental leads. QindieGL's
  missing `wglGetExtensionsStringEXT` and historical startup-with-corruption
  report are documented in section 8h; those findings do not explain
  TitaniumGL's different observed failure.
- This test does not change T25's PAUSED status and does not authorize a
  prototype, guest modification, driver replacement, source change, build, or
  additional test. It is evidence for the independent reassessment only.

## 9. References and resources for development

QEMU-based x86 emulation
* QEMU upstream: https://github.com/qemu/QEMU (TCG: `tcg/`, `target/i386/tcg/`,
  `accel/tcg/`; the new x86 decoder is `target/i386/tcg/decode-new.c.inc`).
* xemu (original Xbox, Pentium III class CPU + NV2A GPU emulation, QEMU based):
  https://github.com/xemu-project/xemu - reference for a trimmed single-purpose
  QEMU fork, a PIII-era CPU model, and host-GL-backed guest GPU emulation.
  Specific TCG research candidates (all were OPEN proposals when reviewed on
  2026-11-21; do not describe them as merged or proven fixes):
  - #3028, https://github.com/xemu-project/xemu/pull/3028 - xemu-specific fix for
    broad whole-page TB invalidation when the store executes from the current TB's
    page. Y2KBox's QEMU 11.1.90 SoftMMU path already performs precise TB/range
    overlap checks in `tb_invalidate_phys_page_range__locked()`; do not port this
    change without first finding a different broad-invalidation path in our tree.
  - #3140, https://github.com/xemu-project/xemu/pull/3140 - skip MMU indexes with
    no populated entries during `tlb_reset_dirty()`. Y2KBox already sets the
    `cpu->neg.tlb.c.dirty` bit on TLB population but still scans every mode in
    `tlb_reset_dirty()`; measure call frequency/duration before adapting.
  - #3141, https://github.com/xemu-project/xemu/pull/3141 - avoid generic indirect
    CPU-operation dispatch and `curr_cflags()` work in `helper_lookup_tb_ptr()` for
    xemu's single x86 CPU target. Potential steady-state indirect-branch optimization;
    maintainer requested profiling evidence. Any adaptation must preserve Y2KBox's
    target selection and debug/single-step flag behavior.
  These are narrow optimizations from a different Xbox/UMA workload. Compare the
  exact patches with this QEMU 11.1.90 source and profile before adapting them.

Misc x86 emulation
* xbased (same focus as xemu): https://github.com/crazyriddler/xbased
* Bochs PR "Improve glxgears support for GeForce":
  https://github.com/bochs-emu/Bochs/pull/574 - guest GL on an emulated GPU.

3D: VMware SVGA3D (primary plan)
* qemu-vmvga: https://github.com/qemus/qemu-vmvga - VMware SVGA II/SVGA3D device for QEMU with DXVK rendering
  (section 8a). Also useful: open-vm-tools / Mesa `svga3d_*.h` headers (already in the overlay).

x87 / soft-float references (for T13a and any FPU fast path)
* PCSX2 PR #12001, PS2 FPU soft-float interpreter: https://github.com/PCSX2/pcsx2/pull/12001 and its recompiler
  PRs https://github.com/DiscoStarslayer/pcsx2-reliquary/pull/17 , /pull/18 , /pull/29 . The PS2 EE FPU is NOT
  IEEE-754/x87 (no NaN/Inf, truncating), so the semantics are not reusable, but the technique - turning a
  soft-float interpreter into a recompiler with branch-free SSE sequences and cheap flag handling - is a useful
  model for an exact fast path for the common x87 cases.
* xemu `fpu/`: https://github.com/xemu-project/xemu/tree/master/fpu - checked: it is QEMU's softfloat
  (floatx80 included), an older copy of the same code as ours; no Xbox-specific x87 changes found (the history
  was not examined in depth). Reference for comparison only.

Real-GPU emulation route (alternative/third 3D idea)
* PCBox PR #202 "Added NVIDIA GeForce2 MX (NV11) emulation support": https://github.com/PCBox/PCBox/pull/202
  (merged; 6 files, +130/-6, on top of PCBox's NV10/Celsius emulation, Vulkan 3D; NV11 PCI ID 0x0110; stock
  NVIDIA Detonator drivers on XP give hardware T&L D3D7/8 and OpenGL; also notes a Windows XP BSOD (0x50) fix by
  disabling PCI D3hot for NV11; author says it was AI-assisted and tested locally). PCBox is an 86Box fork
  (GPL). Different approach from paravirtual SVGA3D: emulate real hardware so unmodified vendor drivers work.
  Relevant if the VMware driver's Direct3D 7/8 / DirectDraw coverage proves insufficient. Not evaluated in
  depth (read the PR summary only); the NV register/method emulation is large.
* **NVPlay / nvplayground:** https://github.com/starfrost013/nvplayground - a DOS/DJGPP-based
  low-level NVIDIA GPU research and raw-I/O test tool. Useful as a hardware-behavior reference
  for early NVIDIA architectures, particularly RIVA 128 (NV3) and RIVA TNT (NV4); NV1x/Celsius
  support is currently basic and largely untested. It is a research/test utility, not an emulator
  or a drop-in component for Y2KBox.

VBoxVGA / VirtualBox (alternative 3D path)
* 86Box PR #6183 "Add Initial Support for VBoxVGA Emulation":
  https://github.com/86Box/86Box/pull/6183 (VBE side of the device; see sec. 7).
* VirtualBox display device source (modern, 7.x, GPL-3.0 - read only, do not copy):
  https://github.com/CE1CECL/vbox/blob/master/src/VBox/Devices/Graphics/DevVGA.cpp
  Prefer the official mirror tags v5.2.2 / v6.1.x (GPLv2) for porting:
  https://github.com/VirtualBox/virtualbox

Guest-side 3D stacks for old Windows (alternatives / building blocks)
* wine9x (Wine wrapper for Win9x/XP era): https://github.com/JHRobotics/wine9x
* SoftGPU (guest software 3D driver): https://github.com/JHRobotics/softgpu
* DXVK: https://github.com/doitsujin/DXVK ; d3d8to9: https://github.com/crosire/d3d8to9/
  (D3D8/9 -> modern API translation ideas for an XP D3D path).
* **D7VK**: https://github.com/WinterSnowfall/d7vk - a DXVK spin-off targeting Direct3D 7/6/5. It uses a modified DXVK D3D9 Vulkan backend together with Wine's DirectDraw implementation or the native Windows DirectDraw implementation as a proxy layer. DirectDraw immediate-mode applications are in scope; Direct3D retained-mode applications are not supported.
* viogpu3d (Virtio GPU 3D for Windows) PR:
  https://github.com/virtio-win/kvm-guest-drivers-windows/pull/943
* virglrenderer: https://github.com/utmapp/virglrenderer (host-side virtual GL;
  guest drivers are WDDM, so not directly usable by XP).
* Yttrium (D3D GPU driver for KVM/QEMU Windows guests):
  https://github.com/arehnman/yttrium-virtio-gpu


CD-ROM, BIN/CUE parsing, and disc-drive behavior
* libcdio C library: https://github.com/libcdio/libcdio-C - reference implementation/library for optical-disc and CUE/BIN handling.
* 86Box CD-ROM implementation: https://github.com/86Box/86Box/tree/master/src/cdrom - reference for CD-ROM device and media behavior.
* DuckStation CUE parser: https://github.com/stenzek/duckstation/blob/master/src/util/cd_image_cue.cpp - a mature emulator CUE-sheet parser.
* Cue sheet (.cue) format overview: https://emulation.gametechwiki.com/index.php/Cue_sheet_(.cue)
* GNU ccd2cue CUE-sheet format documentation: https://www.gnu.org/software/ccd2cue/manual/html_node/CUE-sheet-format.html#CUE-sheet-format
* Copy protection overview: https://emulation.gametechwiki.com/index.php/Copy_protection
* Disc-drive emulation overview: https://emulation.gametechwiki.com/index.php/Disc_Drive_emulation

Shader and post-processing integration
* librashader: https://github.com/SnowflakePowered/librashader - Rust library for integrating libretro-style shader pipelines into standalone applications.
* ares librashader integration: https://github.com/ares-emulator/ares/pull/1397
* PCSX2 librashader integration: https://github.com/PCSX2/pcsx2/pull/14433
* DuckStation Slang post-processing implementation: https://github.com/stenzek/duckstation/blob/master/src/util/postprocessing_shader_slang.cpp - reference for a standalone emulator's Slang shader integration.



## Handoff Rules & Sandbox Gotchas

### 1. Session Handoff Protocol (Before Budget Ends)
* **Never leave `main` broken.**
* **Code & State:** Commit and push the current safe state directly to `main` (no branches, PRs, or force-pushes). The normal workflow is one active agent at a time with serialized writes. Before every write, fetch the latest `main` and the exact target file; verify both the branch HEAD and target blob SHA still match what was inspected. If either moved, abort, refetch, and reapply to the new content. Do not use a blind fetch/rebase as a substitute for this check. Use `git revert` for reverting commits rather than whole-file restores; verify file identity before shell-based edits.
* **TASKS.md:** Update status to `PAUSED` and add a "Resume here:" note.
* **WORKLOG.md:** Append what you did, evidence (numbers, commands), and what broke.
  Keep it as the complete historical record; never prune old sessions.
* **WORKLOG_INDEX.md:** Add or adjust topic-to-session pointers when the work
  introduces a substantial new area or makes an existing mapping useful to update.
  This is navigation only; do not duplicate session summaries.
* **Active session summary:** Add a concise matching session entry to
  `WORKLOAD_ACTIVE_SESSION.md`. The Claude Code `Stop` hook runs
  `scripts/maintain_active_session.py`, automatically drops entries older than
  the latest ten, and blocks session exit if the newest worklog session is missing
  from the summary. Do not manually move or delete old history from `WORKLOG.md`.
* **Documentation:** Update section 2 when durable current-state facts change. Update the
  README only when user-facing setup, requirements, or project information changes; it has
  no task-status table.

### 2. Environment & Shell Constraints
* **Shell & Execution:** The sandbox shell is `dash` (no `<(...)`, no `{a,b}` expansion, no `${var:0:7}`). Always run scripts via explicit `bash` execution. Single commands time out at 300 seconds (run long jobs like ninja via `setsid nohup ... &` and poll).
* **Network & Auth:** Network allowlist blocks `app.codacy.com` and artifact downloads. GitHub API works, but unauthenticated limits (60/h) exhaust quickly—use an environment token. **Never write `ghp_` tokens to files.**

### 3. Pruning & Dead Code Rules
* **Location:** Prune/dead-code scripts live in `xp/strip/`.
* **Safety Checks:** Only prune files *after* confirming the config truly excludes them (a removed option ROM once broke the default NIC). Re-run standard checks afterward.
* **Host-Dependent Backends:** Host-dependent backends (ui/audio/net/block/util/crypto) look "dead" in a library-poor sandbox—**do not delete them** based on that signal; the dead-code method applies only to config-driven directories.

### 4. Benchmarking & Development
* **Benchmarks:** The sandbox is noisy. Use run durations of at least 1 second, with a minimum of 3 runs. Keep loop counters off the code page (Self-Modifying Code / SMC makes execution ~50x slower).
* **Validation:** Always compare a changed binary against the *same* binary with its kill switch enabled, rather than an old build.


* CI performance policy: Linux CI is a KVM build plus functional validation job;
  it does not run TCG timing benchmarks. Windows CI is the only ongoing host for
  TCG performance A/B measurements. Historical Linux benchmark results remain
  documented for context but are not repeated.


### General SDL diagnostic logging capture (2026-11-22; T19)

Y2KBox has one supported renderer, VMVGA, so its local diagnostic hooks are
always part of the product. The debug-mode setting is nevertheless separate
from device declaration: select diagnostics with `-y2kboxdebugmode off|simple|on`
(default: `off`), independently of `-device vmvga`. The levels select disabled,
compact capture, and full selected event logging. This is the command-line equivalent
of the future T12 SDL2 + Dear ImGui frontend's **Debug: On/Off** setting. It enables diagnostic
capabilities but does not start continuous output. Do not use
`-device vmvga,debug=on`; keep device creation and debug configuration separate.

When debug mode is enabled, the SDL title advertises **Ctrl+Alt+L**. Pressing
the shortcut creates a new timestamped `y2kbox-log-YYYYMMDD-HHMMSS.log`
file in the launch working directory, redirects stderr to it, and starts a
diagnostic capture interval: general performance
counters, VMVGA local diagnostics, VMPort GuestRPC diagnostics, and the existing
`vmware_*`, `vmport_*`, and `vdagent*` trace events. Pressing it again stops
those sources and restores the original stderr descriptor. At each capture
start, `Y2KBOX-PERF` counters and timing baselines are reset; the stop summary
therefore covers only that capture interval. This resets diagnostic statistics
only; the VM and guest continue running throughout.

Every capture interval creates a separate file. If a timestamped filename
already exists, a numeric suffix is added rather than overwriting it. Debug
mode off means the shortcut is unavailable
and the title does not advertise it. Debug mode on but capture inactive still
produces no continuous VMVGA/GuestRPC diagnostics, vdagent trace output, or
routine capture start/stop messages in the console. The hotkey consumes the
host key chord; it does not inject Ctrl+Alt+L into XP. No separate
`-trace "vdagent*"` option is needed. Do not redirect stderr from the shell
when using this feature, because QEMU must control stderr redirection for the
capture interval. Exceptional file/descriptor errors are still reported.

Full (`on`) captures have a hard two-second maximum. The capture stops
automatically at the limit, or earlier if Ctrl+Alt+L is pressed again. Once
stderr has been flushed and restored, Y2KBox hands the completed full log to a background worker for zlib compression and saves it as `y2kbox-log-YYYYMMDD-HHMMSS.log.gz`
(with the same collision suffix policy). The original `.log` is removed only
after compression succeeds; if compression fails, it is retained for recovery.
The SDL title indicates the two-second auto-stop while a full capture is active.
Simple captures are unchanged: no automatic timeout and no automatic compression.
The gzip stream uses zlib compression level 9 (`wb9`, maximum compression). Compression runs off the QEMU main thread so the VM can continue while the completed log is compressed. Worker-thread failures leave the original `.log` available for manual recovery.
For performance investigations, start with Simple mode: its PERFSTATS and
`Y2KBOX-PERF` aggregate summaries may be sufficient to identify a major
bottleneck; use Full mode when per-event detail is needed.

Simple-mode sampling is not intended to suppress explicit warnings. The VMVGA
legacy FIFO path emits capture-gated `VMVGA-WARN` records for identified invalid,
unknown, rejected, and allocation-failure cases (including the explicit
`NOP_ERROR` FIFO command) that were previously only `VPRINT` diagnostics (and
therefore normally compiled out). The generic unknown-FIFO warning is emitted
only after both the legacy command switch and SVGA3D dispatcher decline an
unhandled non-3D command; SVGA3D records its own out-of-range command IDs and
in-range IDs without metadata. Unknown register writes are warned only from
the register switch's `default` case. This placement matters: warnings before
dispatch incorrectly labelled recognized commands and valid register writes
as unknown, producing a large warning flood in the owner's 2026-11-24 simple
capture. Session 125 corrected those two placements. Session 126 also moved
unknown-register-read warnings into the read switch's `default` case: register
index 22 is `SVGA_REG_BUSY`, which already has a valid handler, so it must not
be reported as unknown. Existing FIFO/3D stalls and abnormal SYNC/fence
conditions retain their independent reporting paths.

The D3D9 `SCREEN-READBACK phase=supersede-flip` event is emitted for eligible
valid-to-valid full-screen flips where obsolete old-frame readback work is
superseded to avoid synchronous readback/BQL stalls. It is retained per event
in `on` mode. In `simple` mode, only the first two events per capture are
printed; capture stop reports the total, first and last old SID, counts for up
to four distinct SIDs, and an event count for SIDs beyond the tracked set.
This is log-volume control, not a claim that supersession itself is a bottleneck
or that every superseded readback would otherwise have completed usefully.

This is targeted coverage, not a universal severity classifier. QEMU/DXVK
error-reporting paths that write to stderr remain in the capture independently
of simple-mode event sampling, but internal fallbacks that return silently
cannot be guaranteed to appear. Repeated-warning aggregation is not yet
implemented; a genuinely unsupported command that repeats can still produce
repeated lines. Full source-wide severity audit remains future work.

Owner-run Windows XP captures on 2026-11-24 verified both capture levels:
the approximately 48 KB simple capture identified `capture_mode=simple` and
reported 734 superseded flips with `event-lines=2` (SID 2: 367; SID 1: 367).
The approximately 1.5 MB full capture identified `capture_mode=on` and
reported 766 superseded flips with `event-lines=766`, confirming every event
was retained individually. The owner also confirmed that the false unknown
register-read warning for valid `SVGA_REG_BUSY` was absent from both captures.
These are separate runs, so their event totals are not expected to match.

This is owner-side Windows XP capture verification, not CI evidence. No local
build/test was run for Sessions 124–127. Keep T19 IMPLEMENTED pending
Linux/Windows CI and any remaining repeated-interval acceptance checks.

Session 128 expanded only the opt-in `on` capture. Full mode now emits a
`VMVGA-FIFO` event for every FIFO command, with a capture-local sequence number,
decoded name when available, command ID, FIFO offsets, remaining word count,
and SYNC state. It also records operands for legacy 2D `SURFACE_COPY`,
`SURFACE_FILL`, and `RECT_COPY` operations; existing `RECT_FILL` and
`UPDATE` rectangle records remain. Full mode additionally enables the deep
D3D9 draw and state/target traces that were previously disabled by compile-time
developer switches. These include detailed draw planning/submission and
render-target/texture state activity. The developer switches remain disabled
for simple captures; the runtime full-mode selection enables them.

This is intended to make 2D/3D activity easier to correlate around the NFS
Underground 2 minimap flicker. It cannot identify a rendered object semantically
as a minimap or HUD, and the minimap may be rendered through textured 3D draws
rather than the 2D FIFO. Use the command sequence and neighboring operation
records alongside existing readback/presentation events. Full-mode logs will
be substantially larger; simple mode and inactive/default-off behavior are
unchanged. No rendering or synchronization behavior was intentionally changed.
Session 128 received static source review only; no build, runtime capture, XP
verification, or CI was performed. The next owner test should be one full-mode
capture spanning the visible flicker, not a separate test for each trace type.

Session 129 adds the first coordinated T26 resource-level layer, still only in
the opt-in `on` capture:

- `VMVGA-D3D9-BUFFER-DEFER` records each guest write range and the accumulated
  deferred range/update count. `VMVGA-D3D9-BUFFER-UPLOAD` records direct full
  buffer DISCARDs and deferred native range uploads, including guest range,
  native Lock range/flags, mode, decision reason, flush cause, ring capacity,
  and copied bytes.
- `VMVGA-RESOURCE-UPDATE` records guest-backed image/buffer subresources,
  update boxes, row/plane layout, byte count and backend after the MOB transfer
  has updated the CPU shadow but before native upload completion. No pixel data
  is dumped.
- `VMVGA-D3D9-GEOMETRY` adds a per-range summary correlated with the FIFO
  sequence, target, viewport, primitive/index range and stream count. It now
  decodes raw vertex-input position and TEXCOORD0 UV bounds from the CPU shadow for
  indexed and non-indexed draws using FLOAT1–FLOAT4 declaration elements. These
  are raw vertex-input values, not transformed screen-space coordinates. Unsupported
  formats, unavailable shadows, instancing, invalid indices and oversized draws
  report an explicit `bounds_reason`; decoding is capped at 65,536 vertices and
  performs no GPU readback or forced synchronization.
- `VMVGA-FRAME` marks guest `SVGA_3D_CMD_PRESENT` command boundaries with a capture-local frame ID. The marker names the frame being closed and advances the ID for subsequent draws. This is a guest command boundary, not a host SDL presentation/vblank frame; retain the separate FIFO/event `seq`.
- `VMVGA-D3D9-STATE-SNAPSHOT` emits one effective guest-context snapshot per validated D3D9 draw. It includes render-target SIDs with live dimensions/format, viewport/scissor, shader IDs, every valid texture/sampler and render state, and bound texture SIDs with live dimensions, format and mip count. It carries both `frame` and `seq`; it describes guest state at draw submission, not proof that native host state application succeeded.
- Full-mode capture also writes one PNG beside the log when the build already has optional libpng and Pixman (`CONFIG_PNG` and `CONFIG_PIXMAN`). It copies the last CPU-side guest DisplaySurface to RGB and does not force SDL/OpenGL GPU readback; active direct GPU scanout is skipped. Builds without those existing features record that the snapshot is unavailable; the log appends `pixel_snapshot_result=saved`, `not-saved`, or `unavailable` after capture. No image dependency was added.
- For the Joint Operations D3D8 environment-mapped canopy investigation,
  Full mode additionally emits `VMVGA-D3D9-TEXGEN-MATRIX` for WORLD, VIEW and
  TEXTURE0; `VMVGA-D3D9-TEXGEN-STATE`, `...-MATERIAL` and `...-LIGHT`
  for transformed-coordinate draws; and `VMVGA-CUBEMAP-SHADOW` with bounded
  level-zero CPU-shadow face checksums. These are diagnostic snapshots, not
  proof of native GPU contents or correct cube-face orientation. The records
  are capture-gated and must not change rendering or scheduling.
- `VMVGA-RENDER-CORRELATION` adds full-only readback submit/ready/retire
  markers carrying the last observed FIFO sequence. Because readback completion
  can be asynchronous, this is a nearby correlation hint, not a guaranteed
  originating command ID.

Simple-mode and inactive/default-off log formats are preserved; new high-volume
records are gated by active full capture. No GPU readback, forced synchronization,
or rendering/upload-policy change was introduced intentionally. Do not identify
the minimap from surface IDs or texture dimensions alone.

T26 remains IN PROGRESS. Static source review only; no QEMU build, runtime test,
owner capture, or CI was performed. Next owner step: one full-mode NFSU2 capture
spanning visible flicker, then inspect the new surface 29/35 uploads, surfaces
404 and 410–446 updates, draw summaries and readback correlations together.
