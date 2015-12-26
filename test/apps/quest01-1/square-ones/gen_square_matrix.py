#!/usr/bin/env python

import sys

if len(sys.argv) < 2:
    print  "Usage: "+sys.argv[0]+" <number of rows> [ <number of columns> [ <value to fill with> ] ]"
    exit

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

for i in range(rows):
    for j in range(columns):
        print fill_with,
    print


