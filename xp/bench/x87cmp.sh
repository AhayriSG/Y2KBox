#!/bin/bash
# x87 differential test: same binary, host-x87 fast path vs softfloat (XPQEMU_NO_HX87=1), per-operation checksums.
# usage: xp/bench/x87cmp.sh BUILD_DIR [TRIALS]   (needs nasm). Exit 1 if any operation differs. Used by CI.
D=${1:-build}; T=${2:-2000}
HERE="$(cd "$(dirname "$0")" && pwd)"
B="$D/qemu-system-i386"; [ -x "$B.exe" ] && B="$B.exe"
nasm -fbin -DTRIALS=$T "$HERE/x87diff_ops.asm" -o "$D/x87ops.img" || exit 1
run() {   # $1 = output file, rest = env assignments
  out=$1; shift; rm -f "$out.raw"
  env "$@" timeout 200 "$B" -L "$D/pc-bios" -accel tcg -display none -monitor none -serial none -no-reboot \
      -device isa-debug-exit,iobase=0xf4,iosize=0x04 -drive file="$D/x87ops.img",format=raw,if=ide \
      -chardev file,id=d,path="$out.raw" -device isa-debugcon,iobase=0x402,chardev=d >/dev/null 2>&1
  grep -E '^[0-9A-F]{8}' "$out.raw" | tr -d '\r' | cut -c1-8 | head -46 > "$out"
}
run "$D/x87_soft.txt" XPQEMU_NO_HX87=1
run "$D/x87_fast.txt" X=1
names=(fadd0,1 fadd1,0 fsub0,1 fsub1,0 fsubr0,1 fsubr1,0 fmul0,1 fmul1,0 fdiv0,1 fdiv1,0 fdivr0,1 fdivr1,0 fsqrt faddm64 fmulm64 fsubm64 fdivm64 fsubrm32 faddm32 fdivrm32 fldm64 fldm32 fstm64 fstm32 fchs fabs frndint fxch fcom fucom fsubrm64 fdivrm64 fmulm32 fsubm32 fdivm32 fcomm64 fcomm32 fildm32 fiaddm32 fldst1 fstst1 fld1 fldz fcomi fucomi lazy_sw)
n=$(wc -l < "$D/x87_soft.txt")
[ "$n" -eq 46 ] || { echo "x87cmp: expected 46 checksums, got $n (test did not complete)"; exit 1; }
paste -d' ' "$D/x87_soft.txt" "$D/x87_fast.txt" | awk -v names="${names[*]}" 'BEGIN{split(names,nm," ")} {if ($1!=$2) {bad++; printf "DIFF %-9s soft=%s fast=%s\n", nm[NR], $1, $2}} END{if (bad) {print "x87cmp: " bad " operation(s) differ"; exit 1} else print "x87cmp: all 46 operations identical (fast vs softfloat)"}'
