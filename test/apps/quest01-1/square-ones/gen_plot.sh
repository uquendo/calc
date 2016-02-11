#!/bin/sh

infile=$1
outfile=$2

gnuplot << EOF
set term pdfcairo size 10.00in,6.00in
set key left
set xlabel "log N"
set ylabel "time / N^3"
set output "$outfile"
plot '$infile' using (log(\$1)):((\$2)/((\$1)*(\$1)*(\$1))) with linespoints t "s-gcc", \
     '$infile' using (log(\$1)):((\$3)/((\$1)*(\$1)*(\$1))) with linespoints t "st-gcc", \
     '$infile' using (log(\$1)):((\$4)/((\$1)*(\$1)*(\$1))) with linespoints t "v-gcc", \
     '$infile' using (log(\$1)):((\$5)/((\$1)*(\$1)*(\$1))) with linespoints t "b-gcc", \
     '$infile' using (log(\$1)):((\$6)/((\$1)*(\$1)*(\$1))) with linespoints t "s-clang", \
     '$infile' using (log(\$1)):((\$7)/((\$1)*(\$1)*(\$1))) with linespoints t "st-clang", \
     '$infile' using (log(\$1)):((\$8)/((\$1)*(\$1)*(\$1))) with linespoints t "v-clang", \
     '$infile' using (log(\$1)):((\$9)/((\$1)*(\$1)*(\$1))) with linespoints t "b-clang", \
     '$infile' using (log(\$1)):((\$10)/((\$1)*(\$1)*(\$1))) with linespoints t "s-intel", \
     '$infile' using (log(\$1)):((\$11)/((\$1)*(\$1)*(\$1))) with linespoints t "st-intel", \
     '$infile' using (log(\$1)):((\$12)/((\$1)*(\$1)*(\$1))) with linespoints t "v-intel", \
     '$infile' using (log(\$1)):((\$13)/((\$1)*(\$1)*(\$1))) with linespoints t "b-intel"
EOF
