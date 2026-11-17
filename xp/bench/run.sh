#!/bin/bash
# usage: run.sh image [env assignments...]   -> prints median wall seconds (N runs)
Q=${Q:-./build/qemu-system-i386}
img=$1; shift
N=${N:-3}; times=()
err=$(mktemp)
trap 'rm -f "$err"' EXIT
for i in $(seq "$N"); do
  : > "$err"
  s=$(date +%s.%N)
  env "$@" timeout 200 "$Q" -L "${PCBIOS:-./build/pc-bios}" -machine pc -accel tcg -m 16 -display none -monitor none \
      -serial none -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
      -drive "file=$img,format=raw,if=ide" >/dev/null 2>"$err"
  rc=$?
  e=$(date +%s.%N)
  if [ "$rc" -ne 1 ] || [ -s "$err" ]; then
    echo "benchmark run $i failed (QEMU exit=$rc; expected isa-debug-exit status 1)" >&2
    [ ! -s "$err" ] || cat "$err" >&2
    exit 1
  fi
  t=$(echo "$e - $s" | bc -l)
  times+=( "$t" )
done
printf "%.6f\n" "$best"
