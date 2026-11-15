#!/usr/bin/env python3
"""T02 part 2a: keep only the xp-p4 entry in builtin_x86_defs[]."""
p = 'target/i386/cpu.c'; s = open(p).read()
head = 'static const X86CPUDefinition builtin_x86_defs[] = {\n'
a = s.index(head) + len(head)
d = 0; k = a; start = None; elems = []
while True:
    c = s[k]
    if c == '{':
        if d == 0: start = k
        d += 1
    elif c == '}':
        d -= 1
        if d == 0: elems.append((start, k + 1))
        if d < 0: break
    k += 1
end = k                                   # index of the closing '}' of the array
keep = [e for e in elems if '.name = "xp-p4"' in s[e[0]:e[1]]]
assert len(keep) == 1, len(keep)
body = '    ' + s[keep[0][0]:keep[0][1]] + ',\n'
s = s[:a] + body + s[end:]
open(p, 'w').write(s)
print("kept 1 of", len(elems), "CPU models")
