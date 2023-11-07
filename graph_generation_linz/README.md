# Canonical Graph Generation

The software contained in this directory is designed to generate connected, canonically labelled Kontsevich graphs and relation graphs. By a Kontsevich graph, we mean an admissible graph in Kontsevich's sense with 2n+m-2 edges, where n and m are the number of aerial and ground vertices, respectively. A relation graph means one with 2n+m-3 edges, named this way because it gives rise to a Stokes' relation.

To generate the graphs for a single n and m, everything is packed into the `scgg.sh` script. The `batch_cgg.sh` script generates all the graphs with (n,m) smaller than some (N,M). But before running these scripts, there are two things to be done.

### Preparatory steps!

First, we need to make sure nauty is installed. You can follow the instructions [here](https://pallini.di.uniroma1.it/index.html). From the nauty folder, copy the following executables to the current folder, or make symbolic links to them: geng, directg, pickg, showg. Our generation toolkit depends on these nauty "gtools".

Then we need to compile the C++ codes. Run the following commands in your terminal

	g++ graph.cpp -o graph
	g++ sground.cpp -o sground -l nausparse.o -l naugraph.o -l nautil.o -l naurng.o -l nauty.o -l schreier.o -I /path/to/nauty -L /path/to/nauty

Don't forget to replace /path/to/nauty with the path where you have nauty installed!

### Example

To generate a relation graph with n=3, m=2, use

	bash scgg.sh 3 2 -f
	
This generates a file `ksv_n3m2.graphs` in the current folder.

To generate a Kontsevich graph with n=3, m=2, use

	bash scgg.sh 3 2 -k

This generates a file `rel_n3m2.graphs` in the current folder.

The output will be in kontsevint's edge-list format, respectively.

### How do I batch generate?

First create a folder called graph_database under the current directory.

To generate all (permissible) formality graphs with n<=N, m<=M, use the `batch_cgg.sh` script. For example, for all graphs with n<=4, m<=3 you can use

	bash batch_cgg.sh 4 3 -f

Replace -f with -k if you want Kontsevich graphs.

The generated graphs will be moved into the graph_database folder. This script skips a particular (n,m) if the corresponding file already exists in graph_database.

### How do I generate the relations from a relation graph?

You are referred to the README under frg_linz.

### How do I batch generate the relations.

*Please first follow the README under frg_linz.*

Now put the `batch_relations.sh` script under the folder containing relation graphs. Running the script will call `cformality` on all relation graph files `rel_*.graphs` and generate the corresponding relation files.