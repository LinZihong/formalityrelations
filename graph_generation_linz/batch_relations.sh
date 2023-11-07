for i in rel_*.graphs;
do
	file=$(echo "$i" | cut -d'.' -f 1 | cut -c5-)
	echo "running $i"
	./cformality  < $i > ${file}.frl
done
