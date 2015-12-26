#!/bin/bash

I=1
SIZE=(500 512 750 1000 1024 1250 1500 1750 2000 2048 3000 4000 4096 4500 5000 5120 5750 6000 6144 6500 6750 7000 7168 7500 7750 8000 8192 8250)
J=0
grep PERF "$1" | awk '{print $5}' | (
while read res; do
  if [[ $I -eq 1 ]]; then
    echo -n "${SIZE[${J}]} "
  fi;
  echo -n "$res "
  if [[ $I -eq 12 ]]; then
    I=1
    echo ""
    J=$((J+1))
  else
    I=$((I+1))
  fi
done
)
