#!/bin/sh

# prints to stdout square banded matrix with given size filled with given values

if [ $# -lt 1 ]; then
  echo "Usage: $0 <number of rows/columns> [ <band width> [ <value to fill with> ] ]"
  exit 1
fi

ROWS=$1
echo "# $ROWS "
if [ $# -gt 1 ]; then
  BAND=$2
fi;
if [ $# -gt 2 ]; then
  FILL_WITH=$3
else
  FILL_WITH=1
fi

i=0
while [ $i -lt $((BAND)) ]; do
  j=0
  while [ $j -lt $((BAND+1+i)) ]; do
    echo -n "$FILL_WITH "
    j=$((j+1))
  done;
  echo ""
  i=$((i+1))
done
while [ $i -lt $((ROWS - BAND)) ]; do
  j=0
  while [ $j -lt $((BAND+BAND+1)) ]; do
    echo -n "$FILL_WITH "
    j=$((j+1))
  done;
  echo ""
  i=$((i+1))
done
i=$BAND
while [ $i -gt 0 ]; do
  j=0
  while [ $j -lt $((1+i)) ]; do
    echo -n "$FILL_WITH "
    j=$((j+1))
  done;
  echo ""
  i=$((i-1))
done


