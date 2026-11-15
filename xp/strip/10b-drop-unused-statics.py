#!/usr/bin/env python3
"""Delete 'static ... NAME = {...};' definitions that gcc reports as defined-but-unused.
usage: 10b-drop-unused-statics.py FILE BUILD_DIR OBJECT_TARGET   (loops until clean)"""
import re, subprocess, sys
path, build, target = sys.argv[1:4]
for it in range(8):
    subprocess.run(['touch', path])
    out = subprocess.run(['ninja', '-C', build, target], capture_output=True, text=True)
    text = out.stdout + out.stderr
    names = sorted(set(re.findall(r"[‘'](\w+)[’'] defined but not used", text)))
    if not names:
        print("clean after", it, "rounds"); break
    s = open(path).read(); n = 0
    for name in names:
        m = re.search(r'(?m)^static [^\n;=(]*\b%s\b(\[\w*\])? = ' % re.escape(name), s)
        if not m: continue
        i = m.end(); depth = 0; k = i
        while True:
            c = s[k]
            if c == '{': depth += 1
            elif c == '}':
                depth -= 1
                if depth == 0: break
            k += 1
        k = s.index(';', k) + 1
        while s[k:k+1] == '\n': k += 1
        s = s[:m.start()] + s[k:]; n += 1
    open(path, 'w').write(s)
    print("round", it, ": removed", n, "of", len(names), "unused statics")
