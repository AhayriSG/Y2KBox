# SSE2 TCG microbenchmark

`sse2.asm` is a 512-byte real-mode boot sector. It enables SSE, runs LOOPS
iterations of a fixed mix (paddd, pmullw, pxor, pshufd, pcmpeqb, pand, por,
psrld, paddw, psubb), optionally with 128-bit loads/stores (`-DMEM`), and exits
through isa-debug-exit. `run.sh` times it from the host (median of N runs).

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
The CI A/B test builds one identical 300-million-iteration image and runs it
with the pre-T04 and current QEMU binaries on the same Linux runner:

    nasm -fbin -DLOOPS=300000000 pshufd.asm -o pshufd.img
    Q=build/qemu-system-i386 PCBIOS=build/pc-bios N=5 ./run.sh pshufd.img

`run.sh` reports the median of five successful wall-clock runs (six decimal
places). Compare the raw medians; do **not** subtract a separate empty-loop
measurement. QEMU TCG's translation-block shape, dispatch and host scheduling
make an empty loop a different workload, not a stable additive timing overhead.
The earlier subtraction produced negative net times and is invalid.

Keep the host, QEMU build, CPU affinity, image and iteration count identical
when comparing revisions. This measures the isolated dependent PSHUFD loop,
not a broad SSE2 workload or the user's local CPU performance.

### T04 Linux CI result (2026-11-18)

Runner: AMD EPYC 9V74, x86-64, Ubuntu 24.04 Azure runner. Same runner,
same 300M-iteration image, five runs per binary; `run.sh` reports the median.

| build | median wall time |
|-------|------------------:|
| Pre-T04 baseline (`c5ef98e`) | 3.083675 s |
| T04 | 3.166926 s |

Relative change: **-2.70%** (0.974x; T04 is 0.083251 s slower in this run).
This result does not show a speedup for the isolated dependent PSHUFD loop.
Because this is one shared-host CI sample, the small difference should not be
treated as a definitive regression. It says nothing conclusive about broader
SSE2 workloads or other host CPUs.

## T13d legacy XMM register-cache A/B benchmark

The per-TB legacy XMM cache is implemented in commit `c272958`. It keeps
eligible register-only legacy XMM values in TCG EBB vector temporaries across
consecutive instructions, with lazy loads and dirty writeback. Cache barriers
flush and free the temporaries before non-cacheable instructions, exceptions,
and TB exits. MMX, VEX/YMM, memory operands, and unsupported vector operations
remain on their existing paths. The functional shuffle test includes a MOVD
barrier case.

Linux CI compares one identical 100-million-iteration `sse2.asm` workload
using the pre-cache Linux artifact from run 44156398064 and the current build
on the same runner. Each side is measured three times and compared by median
wall time. Windows CI additionally compares the pre-cache Windows artifact from
the same run against the current build using an identical 300-million-iteration
image and three-run medians. The runner uses `awk` for timestamp subtraction
(no `bc` dependency); the Windows job adds the UCRT64 runtime directory to
`PATH` before launching the not-yet-packaged current executable. Both are mixed
register-only SSE2 workloads, not broad application benchmarks or measurements
of the user's local CPU. These are separate host measurements; do not compare
their absolute runtimes across operating systems.

The Win64 cache budget is five live guest XMM values: the x86-64 TCG backend
reserves XMM5 as a scratch register and does not allocate XMM6-XMM15 under the
Windows x64 ABI. Before emitting a cacheable instruction, the translator
preflights its complete operand set and flushes the current cache if adding
those operands would exceed five live values. This avoids forcing the TCG
allocator to spill a larger set of cached vectors. Linux retains its existing
cache behavior.

### T13d Linux CI result (2026-11-18)

Baseline artifact: pre-cache build from Actions run 44156398064. Same
100M-iteration `sse2.asm` image, three runs per build, median wall time:

| build | median wall time |
|-------|------------------:|
| Before XMM cache | 0.877822 s |
| After XMM cache | 0.872720 s |

Relative change: **+0.58%** (1.006x; 0.005102 s faster). This is a marginal
difference on a shared CI runner, effectively near-neutral rather than a
demonstrated performance improvement. It is not evidence of a broad SSE2
speedup or regression, and does not predict performance on other host CPUs.
Linux benchmark completed successfully. The first Windows attempt did not
produce a measurement: the runner lacked `bc`, and the current build's UCRT64
DLL directory was not on `PATH`. Both CI setup issues are corrected; a valid
Windows A/B result is still pending. No local build/test was run.

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
