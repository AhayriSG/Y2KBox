#!/usr/bin/env python3
"""T01 part 2b: keep ONE machine type (pc-i440fx-<latest>, alias 'pc'); delete the
older versioned machines and their now-unused compat tables."""
import re, subprocess
def rd(p): return open(p).read()
def wr(p, s): open(p, 'w').write(s)
p = 'hw/i386/pc_piix.c'; s = rd(p)
i = s.index('static void pc_i440fx_machine_11_1_options(MachineClass *m)')
s = s[:i].rstrip('\n') + '\n'
wr(p, s)

total = 0
for p in ('hw/i386/pc.c', 'hw/core/machine.c'):
    s = rd(p)
    s, n = re.subn(r'(?s)GlobalProperty ((?:pc|hw)_compat_\d+_\d+)\[\] = \{.*?\n\};\nconst size_t \1_len = G_N_ELEMENTS\(\1\);\n\n?', '', s)
    total += n; wr(p, s)
for p in ('include/hw/i386/pc.h', 'include/hw/core/boards.h'):
    s = rd(p)
    s, n = re.subn(r'extern GlobalProperty ((?:pc|hw)_compat_\d+_\d+)\[\];\nextern const size_t \1_len;\n', '', s)
    total += n; wr(p, s)
s = rd('hw/i386/pc.c')
s, n = re.subn(r'GlobalProperty (pc_compat_\d+_\d+)\[\] = \{\};\nconst size_t \1_len = G_N_ELEMENTS\(\1\);\n\n?', '', s)
total += n; wr('hw/i386/pc.c', s)
print("single machine type done; removed", total, "compat table definitions/declarations")
