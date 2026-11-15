#!/usr/bin/env python3
"""usage: perf.py QEMU PCBIOS -> net ns/iteration per benchmark (min of 3, minus empty loop of equal length)"""
import re, subprocess, sys, time, os
Q, BIOS = sys.argv[1], sys.argv[2]
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), "micro"))
N = {'alu':300e6,'imul':300e6,'div':100e6,'branch':300e6,'memalu':100e6,'pushpop':100e6,'callret':100e6,
     'strmov':3.75e6,'strset':3.75e6,'x87add':30e6,'x87div':30e6,'x87trig':15e6}
def build(name, n):
    s = open(name + '.asm').read()
    s = re.sub(r"mov ebp, \d+", "mov ebp, %d" % n, s)
    open('/tmp/_b.asm', 'w').write(s)
    subprocess.run(['nasm', '-fbin', '/tmp/_b.asm', '-o', '/tmp/_b.img'], check=True)
def run():
    best = 1e9
    for _ in range(3):
        t = time.time()
        subprocess.run([Q, '-L', BIOS, '-accel', 'tcg', '-display', 'none', '-monitor', 'none', '-serial', 'none', '-no-reboot',
                        '-device', 'isa-debug-exit,iobase=0xf4,iosize=0x04', '-drive', 'file=/tmp/_b.img,format=raw,if=ide'],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=250)
        best = min(best, time.time() - t)
    return best
print("%-8s %10s %10s %12s" % ("bench", "iters", "net secs", "net ns/iter"), flush=True)
for name, n in N.items():
    n = int(n)
    build('empty', n); te = run()
    build(name, n); tb = run()
    print("%-8s %10d %10.2f %12.2f" % (name, n, tb - te, (tb - te) / n * 1e9), flush=True)
