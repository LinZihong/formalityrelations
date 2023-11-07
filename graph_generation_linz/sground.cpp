#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "nausparse.h"
using namespace std;

// ====================================================
// nauty canonical labelling:
// ====================================================

#define MAX_EDGES 100
#define MAX_VERTS 100

static DEFAULTOPTIONS_SPARSEDIGRAPH(gNautyOptions);
statsblk gNautyStats;
sparsegraph gNautyGraph, gNautyCanon;
int gNautyLab[MAX_VERTS];
int gNautyPtn[MAX_VERTS];
int gNautyOrbits[MAX_VERTS];

void initNauty(sparsegraph *pG)
{
	pG->nv  = 0;
	pG->nde = 0;


	pG->vlen = MAX_VERTS;
	pG->dlen = pG->vlen;
	pG->elen = MAX_VERTS*MAX_EDGES;

	pG->v = (size_t*)malloc(pG->vlen*sizeof(size_t));
	pG->d = (int*)malloc(pG->dlen*sizeof(int));
	pG->e = (int*)malloc(pG->elen*sizeof(int));

	for (int v=0; v<MAX_VERTS; ++v)
		pG->v[v] = v*MAX_EDGES;

	pG->w    = NULL;
	pG->wlen = 0;

	gNautyOptions.getcanon = TRUE;
	gNautyOptions.defaultptn = FALSE;
}

// assumes that vertices are labled 0,...,nAir-1,nAir,...,nAir+nGround-1
// graph gets canonically labelled, edges sorted lexicographically, and the relative sign is returned
int canonicizeGraph(int nAir, int nGround, int nEdges, int EdgeList[][2])
{
	int v,w,e;

	if (nAir+nGround>MAX_VERTS) {
		printf("Error: too many vertices\n");
		exit(EXIT_FAILURE);
	}
	if (nEdges>MAX_EDGES) {
		printf("Error: too many edges\n");
		exit(EXIT_FAILURE);
	}

	// start with isolated vertices:
	for(v=0; v<nAir+nGround; ++v)
		gNautyGraph.d[v] = 0;
	gNautyGraph.nv = nAir+nGround;

	// add edges:
	for(e=0; e<nEdges; ++e)
	{
		v = EdgeList[e][0];
		w = EdgeList[e][1];
		if ((v<0) || (w<0) || (v>=nAir+nGround) || (w>=nAir+nGround)) {
			printf("Error: vertex index out of bounds (edge %d->%d)\n",v,w);
			exit(EXIT_FAILURE);
		}
		if (v>=nAir) {
			printf("Error: outgoing edge from aerial vertex\n");
			exit(EXIT_FAILURE);
		}
		gNautyGraph.e[gNautyGraph.v[v]+gNautyGraph.d[v]++] = w;
	}
	gNautyGraph.nde = nEdges;

	// vertex partiton: {0,..,nAir-1},{nAir},{nAir+1},...,{nAir+nGround-1}
	for(v=0; v<nAir+nGround; ++v)
	{
		gNautyLab[v] = v;
		if (v>=nAir-1)
			gNautyPtn[v] = 0;
		else
			gNautyPtn[v] = 1;
	}

	// compute canonical labelling:
	sparsenauty(&gNautyGraph,gNautyLab,gNautyPtn,gNautyOrbits,&gNautyOptions,&gNautyStats,&gNautyCanon);
	// sort edge lists:
	sortlists_sg(&gNautyCanon);

	// check that the ground vertices have not moved:
	// for(v=nAir; v<nAir+nGround; ++v)
	// 	if (gNautyLab[v]!=v) {
	// 		printf("Error: ground vertex label changed (%d->%d)\n", v, gNautyLab[v]);
	// 		exit(EXIT_FAILURE);
	// 	}
	// for(v=0; v<nAir; ++v)
	// 	printf("relabel %d<-%d\n",v,gNautyLab[v]);

	// return canonicalized edge list, and compute edge permutation sign:
	// int perm[MAX_EDGES];
	// int k,i,pos,sign;
	// k = 0;
	// sign = 1;
	// for (v=0; v<nAir; ++v)
	// 	for (e=0; e<gNautyCanon.d[v]; ++e) {
	// 		w = gNautyCanon.e[gNautyCanon.v[v]+e];
	// 		// v->w is the k'th edge in the canonicalized output
	// 		// where is it in the input?
	// 		pos = -1;
	// 		for(i=k; i<nEdges; ++i)
	// 			if ((gNautyLab[v]==EdgeList[i][0]) && (gNautyLab[w]==EdgeList[i][1])) {
	// 				pos = i;
	// 				break;
	// 			}
	// 		if (pos<k) {
	// 			printf("Error: could not find edge %d->%d (output) in input edgelist\n", v, w);
	// 			exit(EXIT_FAILURE);
	// 		}
	// 		if (k!=pos) {
	// 			sign = -sign;
	// 			// swap k with pos
	// 			EdgeList[pos][0] = EdgeList[k][0];
	// 			EdgeList[pos][1] = EdgeList[k][1];
	// 		}
	// 		EdgeList[k][0] = v;
	// 		EdgeList[k][1] = w;
	// 		k++;
	// 	}

	// output edge list:
	printf("[");
	for (v=0; v<nAir; ++v) {
		if (v>0)
			printf(",");
		printf("[");
		for (e=0; e<gNautyCanon.d[v]; ++e) {
			w = gNautyCanon.e[gNautyCanon.v[v]+e];
			if (e>0)
				printf(",");
			// output aerial vertices as 1,..,nAir
			// output terrestrial vertices as p1,...,pnGround
			if (w<nAir)
				printf("%d", w+1);
			else
				printf("p%d", w-nAir+1);
		}
		printf("]");
	}
	printf("]\n");

	return 0;
}

void write_canonicalized_edge_list(int n, int m, int *from, int *to, int *vertex_names, int e)
{
	int g[MAX_EDGES][2];

	for (int i = 1; i <= e; i++)
	{
		g[i-1][0] = vertex_names[from[i]];
		g[i-1][1] = vertex_names[to[i]];
	}

	canonicizeGraph(n, m, e, g);
}

int write_grounded(int *from, int *to, int *out_degree, int n, int m, int e)
{
	int num_graphs = 0;
	vector<int> zero_out_vertex;
	for(int i=1;i<=n+m;i++)
	{
		if(out_degree[i] == 0)
			zero_out_vertex.push_back(i);
	}
	sort(zero_out_vertex.begin(), zero_out_vertex.end());
	do
	{
		// output the ground vertices if necessary
		int vertex_names[200];
		memset(vertex_names, -1, sizeof(vertex_names));
		for (int i = 0; i < m; i++)
		{
			vertex_names[zero_out_vertex[i]] = n + i;
		}
		int count = 0;
		for (int i = 1; i <= n+m ; i++)
		{
			if(vertex_names[i] == -1)
			{
				vertex_names[i] = count++;
			}
			// cout << vertex_names[i] << " ";
		}
		// if(mode == 0)
		// 	write_edge_list(from, to, vertex_names, e);
		// else if (mode == 1)
		// 	write_kontsevint_edge_list(n, from, to, vertex_names, e);
		// else
		// {
		// 	write_nauty(n, m, from, to, vertex_names, e);
		// }
		write_canonicalized_edge_list(n, m, from, to, vertex_names, e);
		num_graphs++;
		reverse(zero_out_vertex.begin()+m, zero_out_vertex.end());
	} while (next_permutation(zero_out_vertex.begin(),zero_out_vertex.end()));
	return num_graphs;
}

int main(int argc, char const *argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);

	// prepare nauty:
	initNauty(&gNautyGraph);
	initNauty(&gNautyCanon);

	int n, m;
	n = strtol(argv[1], nullptr, 0);
	m = strtol(argv[2], nullptr, 0);

	char infile[80], outfile[80];
	sprintf(infile, "edges_%d_%d.gr", n, m);
	sprintf(outfile, "ground_%d_%d.el", n, m);

	freopen(infile, "r", stdin);
	freopen(outfile, "w", stdout);

	int order, e;
	int count = 0;
	while (cin >> order >> e) // main loop to process each graph
	{
		int from[200], to[200];
		int out_degree[100] = {};
		for(int i=1; i<=e; i++)
		{
			cin >> from[i] >> to[i];
			out_degree[from[i]]++;
		}
		count += write_grounded(from, to, out_degree, n, m, e);
	}

	cerr << "Finished canonical grounding: " << count << " graphs written\n removing duplicates...\n";

	fclose(stdin);
	fclose(stdout);

	char srt[100];
	sprintf(srt, "sort -u ground_%d_%d.el > canograph_%d_%d.el", n, m, n, m);
	system(srt);

	return 0;
}