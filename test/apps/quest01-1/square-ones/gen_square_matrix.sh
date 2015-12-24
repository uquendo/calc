#!/bin/sh

# prints to stdout matrix with given size filled with given values

if [ $# -lt 1 ]; then
  echo "Usage: $0 <number of rows> [ <number of columns> [ <value to fill with> ] ]"
  exit 1
fi

ROWS=$1
if [ $# -gt 1 ]; then
  COLUMNS=$2
  echo "# $ROWS $COLUMNS"
else
  COLUMNS=$ROWS
  echo "# $ROWS"
fi;
if [ $# -gt 2 ]; then
  FILL_WITH=$3
else
  FILL_WITH=1
fi

i=0
while [ $i -lt $ROWS ]; do
  j=0
  while [ $j -lt $COLUMNS ]; do
    echo -n "$FILL_WITH "
    j=$((j+1))
  done;
  echo ""
  i=$((i+1))
done

