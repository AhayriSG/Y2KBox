# Y2KBox handoff  -  READ THIS FIRST

Files: `HANDOFF.md` (context, rules, design notes) - `TASKS.md` (claimable work queue) -
`WORKLOG.md` (what was done, chronologically) - `README.md` (public status table).
Repo: https://github.com/AhayriSG/Y2KBox , branch `main` (always keep it building).

## 0. If you are a new AI/human taking over (2 minutes)

1. Read sections 1-2 below, then `TASKS.md`. Pick an unclaimed task (highest priority
   first) and **claim it by pushing** (protocol at the top of TASKS.md).
2. Build and run the standard checks (section 7) BEFORE changing anything, so you know
   the baseline is green.
3. Work on a `task/Txx-name` branch, commit small, push often.
4. Before you run out of budget: push, update TASKS.md (`PAUSED` + "Resume here"),
   append to WORKLOG.md (section 10 has the checklist).
Several agents can work in parallel: tasks are written to be independent (different
files). If two tasks touch `hw/i386/pc*.c` (T01, T05, T06) coordinate through TASKS.md.

## 1. Project aim and non-negotiable constraints (owner decisions)

* **Only Windows XP 32-bit guests.** Binary: `qemu-system-i386` only. No 64-bit guests.
* **Guest hardware is fixed:**
  - guest RAM **2048 MB**, all below 4 GB (GUI may later offer up to 3.5 GB; not now);
  - guest CPU: **SSE2 is the maximum** (a Pentium-4-class model; no SSE3/SSSE3/SSE4/AVX
    in the guest - so those translators can be deleted);
  - vCPU count = **host thread count** (see open question Q1 in section 4);
  - no user-facing QEMU flags (-smp, -cpu, -m, -machine ...). Our own front end will
    pick: qcow2 disk (created with qemu-img), RAM, floppy image, CD/ISO.
* **Emulation, not virtualization:** TCG/MTTCG only. No KVM/WHPX/HVF/Xen/Hyper-V/
  IOMMU/SEV/SGX/TDX/VFIO.
* **Host:** x86-64, **x86-64-v3 minimum**, **Windows and Linux only** (no macOS, BSD,
  Solaris, ...). AVX-512 stays runtime-detected (measured: no gain for SSE2 guests).
* **Performance** is the point: SSE2/x87/memory paths under TCG/MTTCG.
* **3D: VBoxVGA** (VirtualBox Guest Additions XPDM driver + a host 3D service).
  Rejected by owner: qemu-3dfx (per-game guest wrappers); virglrenderer / viogpu3d /
  SoftGPU / Wine9x are "not mature enough" for XP (they remain useful references).
* Delete unrelated QEMU code freely as long as XP-32 + TCG keep working.
* Later (not now): own executable + wxWidgets GUI (T12).

## 2. Current state snapshot (update when it changes)

* Builds on Linux (native). Windows host build: not yet verified (T07).
* `qemu-system-i386 -accel help` -> only `tcg`. Only i440FX `pc` machines (19 versioned
  variants still present). SeaBIOS boots under TCG with 2 vCPUs. **XP has not been
  booted yet** (T08, needs an XP ISO).
* Source tree 41 MB (upstream 171 MB). 91 enabled device symbols (~247 listed devices).
* Done: see `WORKLOG.md`. In progress: see `TASKS.md` (CLAIMED/PAUSED).
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

## 4. Open questions / decisions pending

* **Q1 vCPU limit.** Owner wants "all host threads". My understanding (UNVERIFIED): XP Pro
  licenses 2 sockets (Home: 1) and 32-bit Windows supports at most 32 logical CPUs, so
  expose e.g. 2 sockets x N cores x 1 thread (cap 32). Verify before capping; also check
  that XP's ACPI-APIC multiprocessor HAL is selected.
* Q2 Direct3D for XP guests via VBox Guest Additions, or OpenGL only? Which GA version
  (plan: 5.2.x; 3.2.28 as bring-up baseline)?
* Q3 Which NIC to default to (rtl8139 has an in-box XP driver; e1000 needs a driver disk).
* Q4 Is a 3.5 GB RAM option wanted later (needs the PCI hole at 0xE0000000)?

## 5. Findings and design notes (evidence)

### 5a. SSE2 performance diagnosis
* AVX-512 on/off: reg loop 3.13 s vs 3.09 s; mem loop 8.11 s vs 8.14 s -> no gain.
* Register SSE2 ops are already inline TCG vector ops (add_vec, mul_vec, xor_vec,
  cmp_vec, shri_vec...), but each is `ld_vec env -> op -> st_vec env` (XMM regs are not
  TCG globals; keeping them in host regs is T13). **PSHUFD is an out-of-line helper
  call** (T04).
* The load hotspot (fixed, below): int-pair load + two 64-bit env stores + 128-bit
  reload = store-forwarding stall (~15-20 cycles).

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

## 7. Standard checks ("green" = all pass; run after every change)

    ./qemu-system-i386 -accel help                       # only: tcg
    # SeaBIOS boot, 2 vCPUs:
    ./qemu-system-i386 -L pc-bios -machine pc -accel tcg -m 64 -smp 2 -display none \
      -monitor none -serial none -chardev stdio,id=d -device isa-debugcon,iobase=0x402,chardev=d
        # expect "Found 2 cpu(s)" and "Booting from ..."
    # memory-op functional test (build with: nasm -fbin xp/bench/vectest.asm -o vectest.img)
    ./qemu-system-i386 -L pc-bios -machine pc -accel tcg -m 16 -smp 2 -display none \
      -monitor none -serial none -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
      -drive file=vectest.img,format=raw,if=ide -chardev stdio,id=d \
      -device isa-debugcon,iobase=0x402,chardev=d        # expect "OK"
    # perf (A/B against a saved binary): xp/bench/run.sh

## 8. VBoxVGA / 3D research notes

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

VBoxVGA / VirtualBox
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
* viogpu3d (Virtio GPU 3D for Windows) PR:
  https://github.com/virtio-win/kvm-guest-drivers-windows/pull/943
* virglrenderer: https://github.com/utmapp/virglrenderer (host-side virtual GL;
  guest drivers are WDDM, so not directly usable by XP).
* Yttrium (D3D GPU driver for KVM/QEMU Windows guests):
  https://github.com/arehnman/yttrium-virtio-gpu


## 10. Handoff rules and sandbox gotchas

Before you stop (budget ending) do ALL of: (1) commit + push your branch; (2) TASKS.md:
status `PAUSED` with "Resume here:"; (3) append to WORKLOG.md what you did, evidence
(numbers, commands), and what broke; (4) update section 2 and the README table if state
changed; (5) never leave `main` broken.

Gotchas seen so far: the sandbox shell was `dash` (use `bash` scripts); a single command
is limited to 300 s (run ninja via `setsid nohup ... &` and poll); prune files only after
confirming the config really excludes them, then re-run the standard checks (a removed
option ROM once broke the default NIC); host-dependent backends (ui/audio/net/block/
util/crypto) look "dead" in a library-poor sandbox - do not delete them by that signal.
