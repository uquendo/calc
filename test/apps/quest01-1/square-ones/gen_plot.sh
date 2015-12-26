#!/bin/sh

infile=$1
outfile=$2

gnuplot << EOF
set term pdfcairo size 10.00in,6.00in
set key left
set output "$outfile"
plot '$infile' using 1:2 with linespoints t "s-gcc", \
     '$infile' using 1:3 with linespoints t "st-gcc", \
     '$infile' using 1:4 with linespoints t "v-gcc", \
     '$infile' using 1:5 with linespoints t "b-gcc", \
     '$infile' using 1:6 with linespoints t "s-clang", \
     '$infile' using 1:7 with linespoints t "st-clang", \
     '$infile' using 1:8 with linespoints t "v-clang", \
     '$infile' using 1:9 with linespoints t "b-clang", \
     '$infile' using 1:10 with linespoints t "s-intel", \
     '$infile' using 1:11 with linespoints t "st-intel", \
     '$infile' using 1:12 with linespoints t "v-intel", \
     '$infile' using 1:13 with linespoints t "b-intel"
EOF
