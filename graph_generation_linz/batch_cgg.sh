#!/bin/bash

mode=$3 # -f for formality graphs, -k for kontsevich graphs

# for $n in {1..$n}
# do
# 	for $m in {1..$m}
# 	do
# 		# bash cgg.sh $i $j $mode
# 		echo "$i $j"
# 	done
# done

e=$((2*$1+$2-3))

if [[ mode = "-k" ]]; then
	e=$((2*$1+$2-2))
fi

n=0


while [[ $n -le $1 ]]; do
	m=0
	while [[ $m -le $2 ]]; do
		e=$((2*$n+$m-3))

		if [[ "$mode" = "-k" ]]; then
			e=$((2*$n+$m-2))
			fileName="ksv_n${n}m${m}.graphs"
		else
			fileName="rel_n${n}m${m}.graphs"
		fi

		if [[ $e -ge 0 ]]; then
			echo "running $n $m:"
			if [[ -f "graph_database/$fileName" ]]; then
				echo "already exists"
			else
				bash scgg.sh $n $m $mode
				mv $fileName graph_database
			fi
		fi
		
		((m++))
	done
	((n++))
done