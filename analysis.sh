#!/bin/sh
#An example of how to use erd_analyzer. Runs erd_analyzer with some "script" operates of files with a certain depth<WHATEVER> prefix. Saves plotfile and runs gnuplot.
WHATEVER=$1
LOW=$2
HIGH=$3
echo "load prefix depth$WHATEVER
set low $LOW
set high $HIGH
join 35-Cl 37-Cl
color Si purple
color H brown
color C gray
color N green
color O blue
color Cl red
sort
results
xrange [-200:1000]
yrange [0:80]
plotinfo
results
plot plot$WHATEVER" |erd_analyzer
gnuplot -persist plot$WHATEVER.plt
