# SSE2 TCG microbenchmark

`sse2.asm` is a 512-byte real-mode boot sector. It enables SSE, runs LOOPS
iterations of a fixed mix (paddd, pmullw, pxor, pshufd, pcmpeqb, pand, por,
psrld, paddw, psubb), optionally with 128-bit loads/stores (`-DMEM`), and exits
through isa-debug-exit. `run.sh` times it from the host (min of N runs).

    nasm -fbin -DLOOPS=300000000 sse2.asm -o reg.img
    nasm -fbin -DLOOPS=300000000 -DMEM sse2.asm -o mem.img
    Q=build/qemu-system-i386 PCBIOS=build/pc-bios ./run.sh reg.img X=1
    ... ./run.sh reg.img XPQEMU_NO_AVX512=1     # A/B with AVX-512 masked

## Baseline (sandbox: AVX-512 capable x86-64, 1 core, TCG, -smp 1)

| image | AVX-512 on | AVX-512 masked |
|-------|-----------|----------------|
| reg, 300M iters (12 guest insns/iter) | 3.13 s | 3.09 s |
| mem, 300M iters (15 guest insns/iter) | 8.11 s | 8.14 s |

AVX-512 makes no measurable difference for SSE2 guest code.

## What the generated code shows (`-d op_opt,out_asm`)

* Register-form SSE2 ops are already inline TCG vector ops (add_vec, mul_vec,
  xor_vec, cmp_vec, shri_vec, ...), but every one is ld_vec from `env`, op,
  st_vec back to `env` (guest XMM regs are not TCG globals).
* PSHUFD is an out-of-line helper call (`pshufd_xmm`).
* 128-bit guest loads/stores are `qemu_ld2_i128`/`qemu_st2_i128` (integer
  register pairs): memory -> xmm -> two GPRs -> env -> ld_vec. There is no
  vector variant of the guest memory ops. This makes a vector memory op ~5-6 ns.

Candidate work: a qemu_ld_vec/qemu_st_vec op for the x86_64 backend; inline
PSHUFD/PSHUFLW/PSHUFHW/PUNPCK*/PACK*/SHUFPS; keeping hot XMM regs in host regs.
