#!/usr/bin/env python

from sys import argv

f = open(argv[1])
L = f.readlines()
f.close()

L = [l.strip().split() for l in L]

dat = [[float(l[1]), float(l[2])] for l in L if l[0] == "0"]
dstr = '\n'.join([' '.join(map(str, d)) for d in dat])

print(dstr)

