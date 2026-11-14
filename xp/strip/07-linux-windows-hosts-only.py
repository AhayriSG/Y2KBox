#!/usr/bin/env python3
"""Hosts: Windows and Linux only. Removes macOS/BSD/Solaris/Haiku/WASM code paths.
Verified by a fresh Linux configure+build; a Windows cross build is task T07."""
import re, subprocess
def rd(p): return open(p).read()
def wr(p, s): open(p, 'w').write(s)
def sub1(s, old, new, p=''):
    assert old in s, (p, old[:60]); return s.replace(old, new, 1)
def between(s, a, b, new='', p=''):
    i = s.index(a); j = s.index(b, i) + len(b); return s[:i] + new + s[j:]

p = 'meson.build'; s = rd(p)
s = re.sub(r"supported_oses = \[[^\]]*\]", "supported_oses = ['windows', 'linux']", s, count=1)
s = between(s, "cocoa = dependency('appleframeworks',", "required: get_option('cocoa'))\n", "cocoa = not_found  # macOS removed\n", p)
s = between(s, "vmnet = dependency('appleframeworks', modules: 'vmnet'", "    warning('vmnet.framework API is outdated, disabling')\n  endif\nendif\n", "vmnet = not_found  # macOS removed\n", p)
s = between(s, "coreaudio = not_found\nif not get_option('coreaudio').auto()", "required: get_option('coreaudio'))\nendif\n", "coreaudio = not_found  # macOS removed\n", p)
s = sub1(s, "  elif host_os == 'emscripten'\n    blockdev_ss.add(files('os-wasm.c'))\n", "", p)
s = between(s, "elif host_os == 'darwin'\n  coref = dependency('appleframeworks', modules: 'CoreFoundation')", "required: get_option('pvg'))\n", "", p)
s = between(s, "elif host_os == 'sunos'\n  socket = [cc.find_library('socket'),", "emulator_link_args = cc.get_supported_link_arguments('-Wl,-z,wxneeded')\n  endif\n", "", p)
s = between(s, "if host_os == 'darwin'\n  # Disable attempts to use ObjectiveC", "elif host_os == 'windows'\n  # plugins use delaylib", "if host_os == 'windows'\n  # plugins use delaylib", p)
s = s.replace("    'hw/vmapple',\n", "")
wr(p, s)

for p, old in (('hw/meson.build', "subdir('vmapple')\n"), ('hw/Kconfig', "source vmapple/Kconfig\n")):
    wr(p, sub1(rd(p), old, '', p))
p = 'hw/display/meson.build'; s = rd(p)
s = re.sub(r"system_ss\.add\(when: \[pvg, 'CONFIG_MAC_PVG_[A-Z]+'\][^\n]*\n", '', s); wr(p, s)
p = 'net/meson.build'; s = rd(p)
s = sub1(s, "elif host_os in bsd_oses\n  system_ss.add(files('tap.c', 'tap-bsd.c'))\nelif host_os == 'sunos'\n  system_ss.add(files('tap.c', 'tap-solaris.c'))\n", "", p)
s = between(s, "vmnet_files = files(", "system_ss.add(when: vmnet, if_true: vmnet_files)\n", "", p)
wr(p, s)
p = 'ui/meson.build'; s = rd(p); wr(p, sub1(s, "system_ss.add(when: cocoa, if_true: files('cocoa.m'))\n", "", p))
p = 'audio/meson.build'; s = rd(p); wr(p, sub1(s, "  ['coreaudio', coreaudio, files('coreaudio.m')],\n", "", p))

for f in ['audio/coreaudio.m', 'ui/cocoa.m', 'hw/display/apple-gfx-mmio.m', 'hw/display/apple-gfx-pci.m',
          'hw/display/apple-gfx.h', 'hw/display/apple-gfx.m', 'net/vmnet-bridged.m', 'net/vmnet-common.m',
          'net/vmnet-host.c', 'net/vmnet-shared.c', 'net/vmnet_int.h', 'net/tap-bsd.c', 'net/tap-solaris.c',
          'os-wasm.c', 'include/system/os-wasm.h', 'configs/meson/emscripten.txt']:
    subprocess.run(['git', 'rm', '-q', f], check=True)
subprocess.run(['git', 'rm', '-rq', 'hw/vmapple'], check=True)
subprocess.run("sed -i '/cocoa\\|coreaudio\\|vmnet\\|\\bpvg\\b/d' scripts/meson-buildoptions.sh", shell=True)
print("done")
