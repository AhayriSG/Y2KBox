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

## Isolated PSHUFD XMM benchmark

`pshufd.asm` runs a dependent register-form `PSHUFD xmm,xmm,imm8` loop.
Build an instruction and empty-loop control with the same iteration count:

    nasm -fbin -DLOOPS=1000000000 pshufd.asm -o pshufd.img
    nasm -fbin -DLOOPS=300000000 -DEMPTY pshufd.asm -o empty.img
    Q=build/qemu-system-i386 PCBIOS=build/pc-bios ./run.sh pshufd.img
    Q=build/qemu-system-i386 PCBIOS=build/pc-bios ./run.sh empty.img

Use the minimum of five runs for each image and subtract the empty-loop
time. `run.sh` prints six decimal places. Keep the host, QEMU build, CPU affinity, and iteration count identical
when comparing revisions. This isolates the guest instruction loop but is not
a substitute for a broader SSE2 workload benchmark.

## General-instruction microbenchmarks (`micro/*.asm`, `perf.py`)

Real-mode boot sectors (register loop counter `ebp`; keep loop state off the code page - a counter
on the code page triggers self-modifying-code retranslation and makes everything 50x slower).
`perf.py QEMU PCBIOS` builds each with an iteration count, subtracts an empty loop of the same
length, and prints net ns/iteration (min of 3 runs). The sandbox is noisy: use >= 1 s runs.

Baseline (2026-11-14, sandbox, xp-p4 CPU, TCG, 1 vCPU), net ns per iteration:

| bench | what | ns/iter | note |
|-------|------|---------|------|
| alu | 10 ALU insns incl. adc/ror/neg | ~0 | at noise floor: ~native |
| imul / div | mul/imul/div chains | ~0 / ~1 | fine |
| branch | 3 conditional jumps | 2.5 | |
| memalu | 6 memory-operand ALU ops | 4.5 | ~0.75 ns/op |
| pushpop | 3 push + 3 pop | 4.5 | |
| **callret** | 2x (call, inc, ret) | **24.6** | ~12 ns per call+ret pair |
| **strmov / strset** | `rep movsd` / `rep stosd`, 1 KB | **509 / 536** | ~2 GB/s |
| **x87add** | fld m64; fmul m64; faddp | **72** | ~24 ns per x87 insn |
| **x87div** | fld; fdiv; fsqrt; faddp | **157** | |
| x87trig | fld; fsin; faddp | 84 | |
