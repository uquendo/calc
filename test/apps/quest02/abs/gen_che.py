#!/usr/bin/env python

# flake8 : noqa

import math
import sys

def f(x):
    return math.fabs(x)+math.sin(x)

if len(sys.argv) < 4:
    print "Usage: "+sys.argv[0]+" N a b"
    exit()

print "# "+sys.argv[1]
print sys.argv[2]+" "+sys.argv[3]

N = int(sys.argv[1])
a = float(sys.argv[2])
b = float(sys.argv[3])

for i in range(N+1):
    x = -math.cos(((2.0*i+1.0)/(1.0*N+1.0))*math.pi/2.0)
    print f(x)
