#!/bin/bash

mat_dir=/tmp
bin_dir_prefix=/home/nor/.wip/priv/edu/progr-and-unix/num/calc-build-basic-
verbose_level=6
precision=64

for size in 500 512 750 1000 1024 1250 1500 1750 2000 2048 3000 4000 4096 4500 5000 5120 5750 6000 6144 6500 6750 7000 7168 7500 7750 8000 8192 8250; do
  ./gen_square_matrix.py ${size} > ${mat_dir}/data${size}.dat
  for compiler in gcc clang intel; do
    for algo in num-cpp-s num-cpp-st num-cpp-v num-cpp-b ; do
      ${bin_dir_prefix}${compiler}/bin/quest01-1  -A ${mat_dir}/data${size} -B ${mat_dir}/data${size} -C ${mat_dir}/result \
        --verbose=${verbose_level} --precision=${precision} --algorithm=${algo}
    done;
  done;
  rm ${mat_dir}/data${size}.dat;
done;




