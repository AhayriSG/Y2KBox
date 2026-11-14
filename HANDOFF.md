# Y2KBox handoff (read this first)

Last updated: 2026-11-13. Branch `xp-fork` (== `main` in the Y2KBox repo).
Base commit: upstream QEMU `9d7d057a558b9a1c232cea390b4481dd20d9c9ce` (11.1.90).

## 1. Project aim (non-negotiable constraints from the owner)

* **Only Windows XP 32-bit guests.** Not 64-bit XP, not Vista+, not other OSes.
  Target binary: `i386-softmmu` only.
* **Emulation, not virtualization.** TCG/MTTCG only. No KVM, WHPX, HVF, NVMM,
  MSHV, Xen, Hyper-V, VT-d/AMD-Vi, SEV/SGX/TDX, VFIO. Do not depend on any host
  virtualization stack.
* **Hosts: x86-64 only, x86-64-v3 minimum** (AVX2, BMI1/2, FMA, F16C, LZCNT,
  MOVBE, POPCNT). Owner is fine excluding older CPUs. v4 was considered and
  rejected for now (see section 4). AVX-512 stays runtime-detected.
* **Performance is the point:** acceptable SSE2 speed under TCG/MTTCG.
* **3D: VBoxVGA** (VirtualBox Guest Additions XPDM driver + host 3D service).
  Owner explicitly rejected qemu-3dfx (per-game guest wrappers) and says
  virglrenderer/viogpu3d/SoftGPU/Wine9x are not mature enough for XP.
* **Fixed machine, no user-facing QEMU flags** (-smp, -cpu, -m, ...):
  - vCPU/thread count = max host threads (cap to what XP supports, see 6).
  - RAM: user picks in a GUI, **max 3.5 GB**; everything below 4 GB, so no
    above-4G RAM, no 64-bit PCI hole logic.
  - Later (NOT now): own executable + simple **wxWidgets GUI** asking for: a
    qemu-img-created `.qcow2` disk, RAM size (<=3.5 GB), floppy image, CD/ISO.
* Remove unrelated QEMU code freely, as long as XP-32 / TCG still works.

## 2. What is DONE (commits on `xp-fork`, oldest first)

1. Remove `tests/`.   2. Remove `docs/`.
3. Remove all non-x86 targets, boards, linux-user/bsd-user (`xp/strip/02-*.sh`).
4. i440FX `pc` machine only (`default.mak` disables q35, microvm, isapc, nitro,
   Xen-emu, SEV/SGX/TDX, VT-d, AMD-Vi, OVMF).
5. Full TCG: remove KVM/HVF/WHPX/NVMM/MSHV/Nitro accelerators, Xen, Hyper-V
   devices; options default to disabled (`xp/strip/03-*.sh`).
6. x86-64 hosts only, v3 baseline: removed aarch64/loongarch64/ppc64/riscv64/
   s390x/sparc64 TCG backends + TCI; `x86_version` default 3; startup check
   exits with a message on non-v3 CPUs; v3 features are compile-time constants in
   `tcg/x86_64`; fixed upstream typo (CPUID leaf 0x8000000 -> 0x80000000)
   (`xp/strip/04-*.sh`).
7. Remove Xen completely: stubs, headers, all call sites, QAPI/HMP/CLI
   (`xp/strip/05-remove-xen.py`).
8. SSE2 microbenchmark `xp/bench/` + `XPQEMU_NO_AVX512=1` env switch for A/B.

Verified after each step: fresh `configure` + full build OK; `-accel help` lists
only tcg; SeaBIOS boots under TCG with `-smp 2`. **XP itself has NOT been booted
yet.** Binary ~60 MB; source tree ~88 MB (from 171 MB); build graph 1930 -> 1716.

## 3. How to build / verify

    sudo apt install libfdt-dev libslirp-dev nasm   # plus glib/pixman dev, python3
    pip install meson ninja tomli
    # subprojects/keycodemapdb: if gitlab.com is blocked, clone
    #   https://github.com/qemu/keycodemapdb at the rev in its .wrap file
    mkdir build && cd build
    ../configure --target-list=i386-softmmu --disable-tools --disable-guest-agent --disable-werror
    ninja qemu-system-i386        # ~8-10 min on 2 cores
    ./qemu-system-i386 -accel help                # expect: tcg only
    ./qemu-system-i386 -L pc-bios -machine pc -accel tcg -m 64 -smp 2 -display none \
      -monitor none -serial none -chardev stdio,id=d -device isa-debugcon,iobase=0x402,chardev=d
      # expect SeaBIOS banner, "Found 2 cpu(s)", "Booting from ..."

Strip scripts are in `xp/strip/` (reproducible; `05` is not idempotent - `git
checkout` the touched files before re-running). Dev gotchas: the sandbox shell
was dash (use `bash` scripts), commands time out at 300 s (run ninja with
`setsid nohup ... &` and poll), always do a FRESH configure after changing
meson options (existing build dirs keep old option values).

## 4. Key findings (evidence, not guesses)

* **AVX-512 gives nothing for SSE2 guests**: 300M-iteration loop, reg-only
  3.13 s (on) vs 3.09 s (masked); with memory ops 8.11 s vs 8.14 s. The x86 TCG
  backend already has runtime-gated AVX-512 paths (vpternlog, 64-bit sar/rotate,
  min/max...), so v4 hosts benefit without v4 being mandatory. Stay on v3.
* **SSE2 register ops are already inline TCG vector ops** (add_vec, mul_vec,
  xor_vec, cmp_vec, shri_vec...). But: (a) every op is ld_vec from `env`, op,
  st_vec to `env` (XMM regs are not TCG globals); (b) **PSHUFD is an out-of-line
  helper** (`pshufd_xmm`); (c) **guest 128-bit loads are slow**.
* **128-bit load hotspot (diagnosed):** single-op microbenchmarks (200M iters):
  baseline 0.95 s, aligned store 0.95 s, aligned load 2.01 s, unaligned load
  1.97 s, `paddd xmm,[mem]` 2.03 s. Loads cost ~5 ns extra. Cause: the XP guest
  CPU has no AVX, so `gen_ldo_env_A0` (target/i386/tcg/translate.c:2159) uses
  `MO_ATOM_IFALIGN_PAIR`; the backend then loads into an integer register PAIR
  (`qemu_ld2_i128`), the frontend stores it to env as two 64-bit stores, and the
  next `ld_vec` reads 128 bits back -> **store-forwarding stall** (~15-20 cycles).
  Stores are fine. TCG has no vector-typed guest memory op.

## 5. DONE: `qemu_ld_vec` / `qemu_st_vec` (128-bit guest memory <-> XMM)

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

## 5b. Config-driven pruning (done) and how to repeat it

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

## 6. TODO after that (priority order)

1. Inline PSHUFD/PSHUFLW/PSHUFHW/PUNPCK*/PACK*/SHUFPS (x86 backend already has
   target-specific vec ops: x86_shufps, punpckl/h, psrldq, blend in
   `tcg-target-opc.h.inc`). Consider keeping hot XMM regs in host regs.
2. **Fixed machine config**: drop `-smp/-cpu/-m/-machine` choices; one XP CPU model
   (candidate: Core 2/Penryn-class, SSE up to 4.1, decide after testing XP);
   vCPUs = host threads. **Check the XP limit before capping**: my understanding
   (unverified) is XP Pro licenses 2 *sockets* and 32-bit Windows tops out at 32
   logical CPUs, so expose e.g. 2 sockets x N cores x 1 thread, MTTCG on.
   RAM <= 0xE0000000, below-4G only: simplify `pc_memory_init` (remove above-4G,
   hole64, max_ram_below_4g paths).
3. Remove remaining virt leftovers: IOMMU (38 files reference it), SEV/confidential
   (25), SGX (20), TDX, VFIO (17), unbuilt machine files (pc_q35.c, microvm*.c,
   isapc.c, nitro_enclave.c), all `kvm_enabled()` call sites + `kvm_i386.h`,
   `include/system/kvm*.h`, `hw/i386/kvm/clock.h`. Same technique as Xen: script
   with brace-matching helpers (`xp/strip/05-remove-xen.py`), fix, fresh build.
4. Prune devices (439 remain) and the ~19 versioned pc-i440fx machine types; keep:
   i440fx, IDE (PIIX), a NIC XP has drivers for (rtl8139/e1000/pcnet), AC97/SB16,
   PS/2, USB UHCI, ACPI, floppy, CD-ROM, qcow2/raw, VGA/VBE. Drop COLO, most of
   migration (keep savevm snapshots), vhost, virtio*, etc.
7. Boot an XP SP3 install end to end; then benchmark real workloads.
8. Delete dead v3 fallback branches in `tcg/x86_64/tcg-target.c.inc` (already
   compiled out via constants).
9. **VBoxVGA + 3D** (big): see section 7.
10. wxWidgets launcher (later, not now).

## 7. VBoxVGA / 3D research notes

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

## 8. Repository and workflow

Repo: https://github.com/AhayriSG/Y2KBox , branch `main` (the only branch).
History: owner's "Initial commit" -> snapshot import of upstream QEMU `9d7d057`
(tree identical to qemu/qemu 9d7d057a558b9a1c232cea390b4481dd20d9c9ce; upstream
history itself is not included) -> Y2KBox commits. To merge newer upstream QEMU:
add `qemu/qemu` as a remote and merge with `--allow-unrelated-histories`.

Work happens by committing to `main` and pushing (fast-forward, no force needed).
Keep README.md's status table and this file in sync with what you change.

## 9. References and resources for development

QEMU / x86 emulation
* QEMU upstream: https://github.com/qemu/QEMU (TCG: `tcg/`, `target/i386/tcg/`,
  `accel/tcg/`; the new x86 decoder is `target/i386/tcg/decode-new.c.inc`).
* xemu (original Xbox, Pentium III class CPU + NV2A GPU emulation, QEMU based):
  https://github.com/xemu-project/xemu - reference for a trimmed single-purpose
  QEMU fork, a PIII-era CPU model, and host-GL-backed guest GPU emulation.
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

