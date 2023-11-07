#!/bin/bash

n=$1
m=$2
mode=$3 # -f for formality graphs, -k for kontsevich graphs

e=$((2*$1+$2-3))

if [[ "$mode" = "-k" ]]; then
	e=$((2*$1+$2-2))
fi

./graph $n $m $mode -rm
./sground $n $m

rm ground_${n}_$m.el filtered_${n}_$m.gr edges_${n}_$m.gr

if [ "$mode" = "-k" ]; then
	# mv canograph_${n}_$m.gr kontsevich_${n}_$m.gr
	mv canograph_${n}_$m.el ksv_n${n}m${m}.graphs
else
	# mv canograph_${n}_$m.gr sformality_${n}_$m.gr
	mv canograph_${n}_$m.el rel_n${n}m${m}.graphs
fi