#!/bin/sh
# Remove every non-x86 target, board tree and the user-mode emulators.
# Run from the repository root. Idempotent-ish: skips paths already gone.
set -eu
TARGET_ARCHS="alpha arm avr hexagon hppa loongarch m68k microblaze mips or1k ppc riscv rx s390x sh4 sparc tricore xtensa"
HW_ARCHS="$TARGET_ARCHS sparc64"

for a in $TARGET_ARCHS; do [ -d target/$a ] && git rm -rq target/$a; done
for a in $HW_ARCHS;     do [ -d hw/$a ]     && git rm -rq hw/$a;     done
for d in linux-user bsd-user common-user; do [ -d $d ] && git rm -rq $d; done

for a in $TARGET_ARCHS; do
  sed -i "/^subdir('$a')\$/d" target/meson.build
  sed -i "/^source $a\/Kconfig\$/d" target/Kconfig
done
for a in $HW_ARCHS; do
  sed -i "/^subdir('$a')\$/d" hw/meson.build
  sed -i "/^source $a\/Kconfig\$/d" hw/Kconfig
done
sed -i "/^subdir('common-user')\$/d;/^subdir('bsd-user')\$/d;/^subdir('linux-user')\$/d" meson.build

# Keep only the i386 softmmu target/device configs.
for f in configs/targets/*.mak; do
  [ "$(basename $f)" = "i386-softmmu.mak" ] || git rm -q "$f"
done
for d in configs/devices/*; do
  [ "$(basename $d)" = "i386-softmmu" ] || git rm -rq "$d"
done

# trace-events directory list in meson.build still names removed trees.
for a in $HW_ARCHS; do
  sed -i "/^ *'hw\/$a',\$/d;/^ *'target\/$a',\$/d" meson.build
done
# Sub-entries such as target/arm/hvf, target/mips/tcg, target/s390x/kvm.
for a in $TARGET_ARCHS; do
  sed -i "/^ *'target\/$a\/[a-z0-9_]*',\$/d" meson.build
done
