#!/bin/sh
# x86-64 hosts only, x86-64-v3 baseline.
#  - delete non-x86 TCG backends, TCI and their host/util glue
#  - default x86_version=3 and enforce it at startup
#  - fold v3-guaranteed features into compile-time constants
# AVX-512 (VL/BW/DQ/VBMI2) stays runtime-detected: it is not part of v3 and
# the backend already has gated fast paths for it.
set -eu

for d in aarch64 loongarch64 ppc64 riscv64 s390x sparc64 tci; do git rm -rq tcg/$d; done
git rm -q tcg/tci.c
for d in aarch64 loongarch64 ppc64 riscv64; do git rm -rq host/include/$d; done
git rm -q util/cpuinfo-aarch64.c util/cpuinfo-loongarch.c util/cpuinfo-ppc.c util/cpuinfo-riscv.c

python3 - <<'PY'
import re
# tcg/meson.build: no TCI
p='tcg/meson.build'; s=open(p).read()
a=s.index("if get_option('tcg_interpreter')"); b=s.index("tcg_ss.add(when: libdw")
s=s[:a]+s[b:]; open(p,'w').write(s)

# meson.build: x86_64 hosts only
p='meson.build'; s=open(p).read()
s=re.sub(r"supported_cpus = \[[^\]]*\]", "supported_cpus = ['x86_64']", s, count=1)
a=s.index("tcg_arch = host_arch\nif have_tcg"); b=s.index("  accelerators += 'CONFIG_TCG'\nendif", a)
s=s[:a]+"tcg_arch = host_arch\nif have_tcg\n  add_project_arguments('-iquote', meson.current_source_dir() / 'tcg' / tcg_arch,\n                        language: all_languages)\n\n"+s[b:]
open(p,'w').write(s)

# util/meson.build: only the x86 cpuinfo
p='util/meson.build'; s=open(p).read()
a=s.index("if cpu == 'aarch64'\n  util_ss.add(files('cpuinfo-aarch64.c'))"); b=s.index("endif\n", s.index("s390x_pci_mmio.c"))+len("endif\n")
s=s[:a]+"util_ss.add(files('cpuinfo-i386.c'))\n"+s[b:]
open(p,'w').write(s)

# default baseline: x86-64-v3; drop the interpreter option
p='meson_options.txt'; s=open(p).read()
s=s.replace("option('x86_version', type : 'combo', choices : ['0', '1', '2', '3', '4'], value: '1',",
            "option('x86_version', type : 'combo', choices : ['0', '1', '2', '3', '4'], value: '3',")
s=re.sub(r"option\('tcg_interpreter'.*?\n(?:\s+description:.*\n)?", "", s, count=1)
open(p,'w').write(s)

# startup enforcement + fix the extended-leaf query typo
p='util/cpuinfo-i386.c'; s=open(p).read()
s=s.replace("__get_cpuid_max(0x8000000, 0)","__get_cpuid_max(0x80000000, 0)")
chk='''
    {
        /* XP fork: x86-64-v3 is the hard baseline (AVX2, BMI1/2, FMA, F16C,
         * LZCNT, MOVBE, POPCNT). The binary is built with -march flags that
         * assume it, so fail early with a readable message. */
        const unsigned need = CPUINFO_MOVBE | CPUINFO_LZCNT | CPUINFO_POPCNT |
                              CPUINFO_BMI1 | CPUINFO_BMI2 | CPUINFO_AVX1 |
                              CPUINFO_AVX2;
        unsigned ma, mb, mc = 0, md;
        if (__get_cpuid_max(0, 0) >= 1) {
            __cpuid(1, ma, mb, mc, md);
        }
        if ((info & need) != need || !(mc & bit_FMA) || !(mc & bit_F16C)) {
            static const char msg[] =
                "qemu: this build requires an x86-64-v3 host CPU "
                "(AVX2, BMI1/2, FMA, F16C, LZCNT, MOVBE).\\n";
            if (write(2, msg, sizeof(msg) - 1) < 0) {
                /* nothing useful to do */
            }
            _exit(1);
        }
    }
'''
s=s.replace("#endif\n\n    info |= CPUINFO_ALWAYS;", "#endif\n"+chk+"\n    info |= CPUINFO_ALWAYS;")
open(p,'w').write(s)

# compile-time constants for everything v3 guarantees
p='tcg/x86_64/tcg-target-has.h'; s=open(p).read()
for n in ('bmi1','popcnt','avx1','avx2','movbe'):
    s=re.sub(r"#define have_%s( +)\(cpuinfo & CPUINFO_\w+\)"%n, lambda m:"#define have_%s%s1  /* guaranteed by x86-64-v3 */"%(n,m.group(1)), s)
open(p,'w').write(s)
p='tcg/x86_64/tcg-target.c.inc'; s=open(p).read()
s=s.replace("#define have_bmi2       (cpuinfo & CPUINFO_BMI2)","#define have_bmi2       1  /* guaranteed by x86-64-v3 */")
s=s.replace("#define have_lzcnt      (cpuinfo & CPUINFO_LZCNT)","#define have_lzcnt      1  /* guaranteed by x86-64-v3 */")
open(p,'w').write(s)
PY

# stale generated option list in configure's helper
sed -i '/tcg[-_]interpreter/d' scripts/meson-buildoptions.sh

# summary line no longer knows about the interpreter
python3 - <<'PY'
p='meson.build'; s=open(p).read()
old="""  if get_option('tcg_interpreter')
    summary_info += {'TCG backend':   'TCI (TCG with bytecode interpreter, slow)'}
  else
    summary_info += {'TCG backend':   'native (@0@)'.format(cpu)}
  endif
"""
new="""  summary_info += {'TCG backend':   'native (@0@, x86-64-v@1@ baseline)'.format(cpu, get_option('x86_version'))}
"""
open(p,'w').write(s.replace(old,new))
PY
