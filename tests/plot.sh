#!/bin/bash
# Plot all test .dat output files using Gnuplot

test -n "$(which gnuplot)" || {
  echo "Please install gnuplot" 1>&2; exit 1; }

cd "$(dirname ""$0"")"
IFS="
"
files=$(ls -1 *.dat)

for file in $files
do
  ncols=$(( $(head -n1 "$file" | grep -o $'\t' | wc -l) + 1 ))
  series=""
  for i in $(seq 1 $ncols)
  do
    series="$series'$file' using $i with lines title columnhead"
    if [ $i -lt $ncols ]; then series="$series, "; fi;
  done
  if [ -n "$series" ]
  then
    echo "Plotting $file ..."
    echo "plot $series" | gnuplot --persist
  fi
done
