#!/usr/bin/env python

import sys

if len(sys.argv) < 2:
    print  "Usage: " + sys.argv[0] + " <number of rows> [ <number of columns> [ <value to fill with> [ <value to fill diagonal with> ] ] ]"
    exit()

rows = int(sys.argv[1])

if len(sys.argv) == 2:
    columns = rows
    print "#",rows
else:
    columns = int(sys.argv[2])
    print "#",rows,columns

if len(sys.argv) == 4:
    fill_with = float(sys.argv[3])
else:
    fill_with = 1.0

if len(sys.argv) == 5:
    fill_diag_with = float(sys.argv[4])
else:
    fill_diag_with = columns*fill_with

for i in range(rows):
    for j in range(i):
        print fill_with,
    print fill_diag_with,
    for j in range(columns-i-2):
        print fill_with,
    if i != rows-1 :
        print fill_with


