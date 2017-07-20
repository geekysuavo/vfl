#!/usr/bin/env python

from sys import argv

L = [l.strip().split() for l in open(argv[1]).readlines()]
D = [[float(f) for f in l] for l in L]
S = sorted(D, key = lambda x: x[0])

K = [[s[0]] + s[1] for s in zip(range(1, len(S) + 1), S)]

print('\n'.join([('{} ' * 5).format(*k).strip() for k in K]))

