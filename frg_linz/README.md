# Formality Relation Generation

The program under this folder, called `cformaliy`, is used to generate formality relations (Stokes' relations) from relation graph or graphs. In the output, the graphs are canonically labelled, and the induced edge ordering is used. To compile it, make sure nauty is installed (you can follow the instructions [here](https://pallini.di.uniroma1.it/index.html)) on your computer, and run the following

	g++ cformality.cpp -o cformality -l nausparse.o -l naugraph.o -l nautil.o -l naurng.o -l nauty.o -l schreier.o -I /path/to/nauty -L /path/to/nauty

Don't forget to replace /path/to/nauty with the path where you have nauty installed!

### How to use it

You can run it directly in your terminal, and for each line of relation graph (in kontsevint format) you feed in, it gives you the corresponding formaltiy relation.

Alternatively, use

	./cformality < input.graphs > output.frl

to specify the in- and out-files.

### How to batch generate relations from graphs

Maybe you came here from graph_generation_linz/README.md; you can go back and follow the instructions there now.

#### Note on the program

This program does essentially the same thing as the `genrel` programs by Radu Iacob. Currently it does not support leaving out the disconnected graphs (which we know trivially vanish) out of the relations, as does the `genrelcon` version of the latter. Some earlier tests show that it's quite a bit faster than an earlier version of `genrel`, but comparisons with the up-to-date version of `genrel` haven't been performed.