#!/bin/bash
cd "$(dirname "$0")"
nasm -fbin -DTRIALS=${1:-2000} x87diff_ops.asm -o /tmp/ops.img || exit 1
B=/home/claude/b6/qemu-system-i386; cd /home/claude/b6
run() { timeout 100 env "$@" $B -L pc-bios -accel tcg -display none -monitor none -serial none -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04 -drive file=/tmp/ops.img,format=raw,if=ide -chardev stdio,id=d -device isa-debugcon,iobase=0x402,chardev=d 2>&1 | grep -E "^[0-9A-F]{8}$" | head -30; }
run XPQEMU_NO_HX87=1 > /tmp/ops_soft.txt
run X=1 > /tmp/ops_fast.txt
names=(fadd0,1 fadd1,0 fsub0,1 fsub1,0 fsubr0,1 fsubr1,0 fmul0,1 fmul1,0 fdiv0,1 fdiv1,0 fdivr0,1 fdivr1,0 fsqrt faddm64 fmulm64 fsubm64 fdivm64 fsubrm32 faddm32 fdivrm32 fldm64 fldm32 fstm64 fstm32 fchs fabs frndint fxch fcom fucom)
paste -d' ' /tmp/ops_soft.txt /tmp/ops_fast.txt | awk -v names="${names[*]}" 'BEGIN{split(names,n," ")} {d=($1==$2?"same":"DIFF"); if (d=="DIFF") bad++; printf "%2d %-10s %s %s %s\n", NR, n[NR], $1, $2, d} END{print "differing ops:", bad+0}'
