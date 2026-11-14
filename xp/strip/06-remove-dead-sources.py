#!/usr/bin/env python3
"""Delete C sources that are not part of the qemu-system-i386 build.
usage: 06-remove-dead-sources.py BUILD_DIR   (a configured+built dir)
Scope is limited to config-driven code (hw/, target/, disas/ non-x86, contrib/,
qga/, ebpf/, non-x86 firmware dirs). Host-dependent backends (ui, audio, net,
block, util, crypto, chardev, ...) are NOT touched: they look dead only because
of missing host libraries. Removes the file names from the meson.build next to
them; empty files() lists are valid meson."""
import os, re, subprocess, sys
build = sys.argv[1]
src = os.getcwd()
cmds = subprocess.check_output(['ninja', '-C', build, '-t', 'commands', 'qemu-system-i386']).decode()
built = set()
for m in re.finditer(r'-c\s+(\S+\.c)\b', cmds):
    p = os.path.realpath(os.path.join(build, m.group(1)))
    if p.startswith(src + '/'): built.add(os.path.relpath(p, src))
tracked = subprocess.check_output(['git', 'ls-files']).decode().split('\n')
SCOPE = ('hw/', 'target/', 'contrib/', 'qga/', 'ebpf/', 'pc-bios/')
dead = [f for f in tracked if f.endswith('.c') and f not in built and f.startswith(SCOPE)]
dead += [f for f in tracked if f.startswith('disas/') and f.endswith('.c') and f not in built
         and os.path.basename(f) not in ('capstone.c', 'disas-common.c', 'disas-host.c', 'disas-mon.c')
         and not f.endswith(('i386.c', 'x86.c'))]
print("removing", len(dead), "dead C files")
for f in dead:
    subprocess.run(['git', 'rm', '-q', f], check=True)
    d, b = os.path.split(f)
    mb = os.path.join(d, 'meson.build')
    if not os.path.exists(mb): continue
    s = open(mb).read(); name = re.escape("'%s'" % b)
    total = 0
    for pat in (name + r'\s*,\s*', r',\s*' + name, name):   # all occurrences
        s, n = re.subn(pat, '', s); total += n
    if total: open(mb, 'w').write(s)
