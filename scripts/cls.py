#!/usr/bin/env python

from sys import argv

y = argv[1]
f = open(argv[2])
L = f.readlines()
f.close()

L = [l.strip().split() for l in L]
L = [l for l in L if len(l) == 4]

dat = [[float(l[1]), float(l[2])] for l in L if l[3] == y]
dstr = '\n'.join([' '.join(map(str, d)) for d in dat])

print(dstr)

