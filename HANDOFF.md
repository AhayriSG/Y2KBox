# Y2KBox handoff  -  READ THIS FIRST

Files: `HANDOFF.md` (context, rules, design notes) - `TASKS.md` (claimable work queue) -
`WORKLOG.md` (what was done, chronologically) - `README.md` (public status table).
Repo: https://github.com/AhayriSG/Y2KBox , branch `main` (always keep it building).

## 0. If you are a new AI/human taking over (2 minutes)

1. Read sections 1-2 below, then `TASKS.md`. Pick an unclaimed task (highest priority
   first) and claim it directly on `main` (protocol at the top of TASKS.md).
2. Do NOT build locally (section 7): the status hook reports whether the latest pushed
   commit passed the checks.
3. Work directly on `main`, commit small, and push often. Do not create branches or
   pull requests. Before every write, fetch the latest `main` and the exact target file;
   verify the HEAD and file blob SHA, and abort/reapply if either has moved. Never force-push.
4. Before you run out of budget: push the current safe state, update TASKS.md
   (`PAUSED` + "Resume here"), and append to WORKLOG.md (section 10 has the checklist).
Several agents can work in parallel only by coordinating task claims and serializing
writes to `main`. If two tasks touch `hw/i386/pc*.c` (T01, T05, T06), coordinate through TASKS.md.

## 1. Project aim and non-negotiable constraints (owner decisions)

* **Only Windows XP 32-bit guests.** Binary: `qemu-system-i386` only. No 64-bit guests.
* **Guest hardware is fixed** (owner decisions, 2026-11-14):
  - guest RAM **2048 MB, set automatically, NOT user-configurable** (no RAM option at all,
    no 3.5 GB mode, no above-4G / 64-bit-hole logic);
  - guest CPU: model `xp-p4` (Pentium 4 Northwood class), **SSE2 is the maximum**
    (no SSE3/SSSE3/SSE4/AVX in the guest, so those translators can be deleted);
  - **1 vCPU by default** (uniprocessor); 2 vCPUs possible later as an experimental option
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
* Later (not now): own executable + wxWidgets GUI (T12).

## 2. Current state snapshot (update when it changes)

* Builds and passes the checks on Linux AND Windows (GitHub Actions, MSYS2 UCRT64) - keep CI green. macOS/BSD code removed (T03 pass 1).
* Accelerator policy: Linux product builds expose/use KVM only; Windows builds expose/use TCG only. Only i440FX `pc` machines (19 versioned
  variants still present). Default CPU `xp-p4` (verified in-guest: family 15, SSE2 yes,
  SSE3/SSSE3/SSE4/AVX no); default RAM 2 GiB below 4 GiB (e820 verified); 1 vCPU default. SeaBIOS boots under TCG with 2 vCPUs. **XP has not been
  booted yet** (T08, needs an XP ISO).
* Source tree 41 MB (upstream 171 MB). 91 enabled device symbols (~247 listed devices).
* Done: see `WORKLOG.md`. In progress: see `TASKS.md` (CLAIMED/PAUSED).
* **T09 / T10 draft on `main`:** `qemu-vmvga` (`vmvga` SVGA II + SVGA3D, `vmmouse`, `vmport` with TCG Ring-3
  `0x5658`/`0x5659` backdoor exemption) is wired as the default and only display device, with Linux
  `dxvk-native` (`SDL2`/`SDL3` sealed-memfd WSI) and Windows `d3d9.dll`/`d3d11.dll` (hidden HWND WSI, WARP
  fallback, optional DXVK Windows DLLs) host 3D backends. CI builds and smoke checks run on push; in-guest XP
  driver/3D verification awaits T08/T11.
* T13a host-x87 arithmetic/load and normal-finite FST m32/m64 fast paths are implemented. Windows five-run A/B medians: FST m32 2.474809 s -> 2.383368 s (+3.69%, 1.038x); FST m64 2.508467 s -> 2.359847 s (+5.92%, 1.063x). These are focused hot-cache results, not general workload gains; see TASKS.md and xp/bench/README.md. Remaining T13a operations are in progress.
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

**Q1 vCPUs / host threads / HAL (owner: unsure, "up to you").** Owner clarified he meant
the *host threads QEMU uses*, not the guest vCPU count, and doubts multiple vCPUs are a
good idea for XP. **Decision: 1 vCPU (uniprocessor) by default.** Reasons: XP's
uniprocessor ACPI HAL has the least synchronisation overhead; TCG with 1 vCPU needs no
MTTCG cross-vCPU locking/TLB-flush traffic; XP-era software gains little from 2 CPUs
relative to the emulation cost. `max_cpus` is 2 (XP Pro's licensed socket count) so a 2-vCPU
mode can be added later; if enabled, the user must choose the *ACPI Multiprocessor PC* HAL
during setup - document this in the README then. (Unverified: that XP setup auto-selects
the ACPI uniprocessor HAL for one CPU - verify in T08.) Host threads still matter: QEMU
already runs the vCPU, main-loop/IO, audio and (later) the 3D render on separate host
threads. **Thread pinning** (vCPU thread on a dedicated fast core, others elsewhere, Windows
and Linux affinity APIs) is task T14. Note: "host passthrough" CPU models are a KVM
concept and do not apply to TCG (the guest CPU is emulated); what we *can* do is use host
CPU features inside TCG (AVX2/BMI/etc. already assumed) and pass host resources such as the
GPU through our own 3D path.

**Q2 3D method (owner: unsure, then proposed qemu-vmvga).** Decision (updated 2026-11-14):
**primary = qemu-vmvga (VMware SVGA II + SVGA3D, DXVK backend)**, tasks T09-T11; VBoxVGA 5.2.x
(Chromium-based host service) is the frozen alternative, task T16. Reasoning in 8a: an
implementation already exists for the same QEMU generation, GPL-2.0-or-later, XP explicitly
supported, no 100k-line port of legacy VirtualBox/Chromium code. Still unverified until tested in an
XP guest (needs T08): D3D9 actually accelerated, D3D8/DirectDraw coverage, Windows-host backend.

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
wxWidgets launcher should expose independent network-adapter and system-disk-controller
selectors, with VirtIO Net and VirtIO Block as their defaults. The CD-ROM is deliberately
separate: retain the existing **PIIX IDE CD-ROM** as the default, preserving its optical
drive emulation behavior. T12 may offer an optional **VirtIO SCSI CD-ROM** choice after
the SCSI device implementation and XP driver path are restored and validated; do not
enable VirtIO SCSI in the current Block/Net restoration.

For a fresh XP installation, load the XP x86 `viostor` driver from
`virtio-win-0.1.141_x86.vfd` through text-mode Setup's F6 flow; install XP x86 NetKVM
after setup. An existing IDE installation must have `viostor` installed before switching
its boot disk to VirtIO Block. Owner-led disk/network performance comparisons remain
manual; do not add recurring CI performance benchmarks.
**Q4 RAM.** Decision (owner): always 2048 MB, automatic, no user setting. Implemented as
`mc->default_ram_size = 2 GiB` in `hw/i386/pc.c`; the above-4G/hole64 code is dead and is
removed by T01.

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
text RAM (MMIO slow path) vs integer loads. Passes on the old reference binary and
the new one, -smp 1 and -smp 2. SeaBIOS still boots. Not yet tested: real XP,
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

(Reference only: CI builds for you - see section 7. Build locally only for benchmarking / A-B performance work.)

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

- **Do NOT build locally:** never run `./configure`, `make` or `ninja` in the sandbox unless explicitly asked to benchmark
  (`xp/bench/run.sh`, `xp/bench/perf.py`, A/B performance work). Let GitHub Actions handle compilation, smoke tests and
  binary artifact uploads. (Same rule as `CLAUDE.md` section 7 - keep the two in sync.)
- **CI pipeline (`.github/workflows/build.yml`):** Linux CI builds the Linux KVM product and runs accelerator-policy,
  functional, x87 differential, packaging, and smoke checks; it does not run TCG performance benchmarks.
  Windows CI builds the TCG product and runs functional checks plus the x87 differential test
  (`xp/bench/x87cmp.sh`), then packages and smoke-tests the artifact. The completed T13d SSE2
  and T13a FST A/B measurements are historical results; they are no longer repeated on every
  artifact build. Keep their benchmark sources/scripts for deliberate manual A/B runs when
  evaluating future performance changes.
  `xp/bench/run.sh` reports raw median wall time without subtracting an empty-loop control. It rejects unexpected
  QEMU exit statuses, timeouts, and stderr output. Both jobs upload compiled binaries as GitHub Actions artifacts
  (`y2kbox-linux-x86_64`, `y2kbox-windows-x86_64`; run with `-L pc-bios`).
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
      ./qemu-system-i386 -L pc-bios -accel tcg -smp 2 -display none -monitor none -serial none \
        -chardev stdio,id=d -device isa-debugcon,iobase=0x402,chardev=d     # "Found 2 cpu(s)", "Booting from ..."
      nasm -fbin xp/bench/vectest.asm -o vectest.img   # run with isa-debug-exit + debugcon, expect "OK"
      xp/bench/x87cmp.sh build 2000                    # all 30 operations identical
      xp/bench/perf.py QEMU PCBIOS                     # performance (benchmark runs may build locally)

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
* **TCG Ring-3 backdoor exemption (T09 draft implemented):** `gen_check_io` -> `HELPER(check_io)`
  (`target/i386/tcg/system/seg_helper.c`) now exempts `0x5658` and `0x5659` (`size <= 4`) when
  `vmport_tcg_backdoor_enabled` is true (`hw/i386/vmport.c`). Verified on CI via `xp/bench/vmport_ring3_test.asm`
  (CPL=3, IOPL=0, 0xFF TSS IOPB: allows `0x5658`/`0x5659`, traps `#GP` on `0x565A`); still needs real XP driver
  verification in T08.
* **Host-side 3D backends (T10 draft implemented):**
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
    active (`display_backend_active == false`) unless `debug=on` or `Y2KBOX_3D_HEADLESS` is set.
* Risks: single small upstream; D3D9 is the ceiling for the XP driver (no DX10+); D3D8 and DirectDraw/D3D7
  coverage unknown (T11: try `d3d8to9` and a ddraw wrapper in the guest); performance of the device thread
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

## 9. References and resources for development

QEMU-based x86 emulation
* QEMU upstream: https://github.com/qemu/QEMU (TCG: `tcg/`, `target/i386/tcg/`,
  `accel/tcg/`; the new x86 decoder is `target/i386/tcg/decode-new.c.inc`).
* xemu (original Xbox, Pentium III class CPU + NV2A GPU emulation, QEMU based):
  https://github.com/xemu-project/xemu - reference for a trimmed single-purpose
  QEMU fork, a PIII-era CPU model, and host-GL-backed guest GPU emulation.

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
* **Code & State:** Commit and push the current safe state directly to `main` (no branches, PRs, or force-pushes). Always `git fetch` + `git rebase` before pushing due to parallel agents. Use `git revert` rather than file copies to avoid swapping files like `TASKS.md`/`WORKLOG.md` (verify with `head -1`).
* **TASKS.md:** Update status to `PAUSED` and add a "Resume here:" note.
* **WORKLOG.md:** Append what you did, evidence (numbers, commands), and what broke.
* **Documentation:** Update section 2 and the README table if state changed.

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


### SDL on-demand diagnostic capture (2026-11-21; updated)

The SDL **Ctrl+Alt+L** shortcut is an opt-in tester feature, available only when
the VM is launched with `-device vmvga,debug=on`. With debug off, the shortcut
is unavailable and the title bar does not advertise it.

The `debug` property enables the capability, not continuous output. The SDL title
also advertises the fullscreen toggle (Ctrl+Alt+F) and the grab-release chord when
applicable. The shortcut redirects stderr to `vmvga_freeze.log`, then activates VMVGA local diagnostics,
VMPort GuestRPC diagnostics, and the existing `vmware_*`, `vmport_*`, and
`vdagent*` trace events. Pressing it again disables those sources and restores the original stderr
descriptor. There are no routine start/stop markers or continuous diagnostic
messages in the console while capture is inactive. The first capture truncates
the file; later capture intervals in the same QEMU run append.

Do not add shell-level `2>` redirection or require `-trace "vdagent*"`; SDL
controls the relevant trace events for the capture interval. Exceptional file or
descriptor errors are still reported. The feature has not been locally built or
runtime-tested; keep T19 PAUSED until Linux/Windows CI and owner XP guest
verification are reported.
