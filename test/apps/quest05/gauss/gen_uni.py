#!/usr/bin/env python

# flake8 : noqa

import math
import sys

def f(x):
    return math.exp(-x*x)

if len(sys.argv) < 4:
    print "Usage: "+sys.argv[0]+" N a b"
    exit()

print "# "+sys.argv[1]

N = int(sys.argv[1])
a = float(sys.argv[2])
b = float(sys.argv[3])

for i in range(N+1):
    x = a + (b - a)/float(N)*i
    print x,f(x),1
