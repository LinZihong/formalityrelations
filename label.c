// Erik Panzer
// calling nauty for canonical labelling of Kontsevich digraph
// computes sign from permutation

#include <stdio.h>
#include "nausparse.h"

#define MAX_EDGES 100
#define MAX_VERTS 100

// ====================================================
// nauty canonical labelling:
// ====================================================

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
	for(v=nAir; v<nAir+nGround; ++v)
		if (gNautyLab[v]!=v) {
			printf("Error: ground vertex label changed (%d->%d)\n", v, gNautyLab[v]);
			exit(EXIT_FAILURE);
		}
	for(v=0; v<nAir; ++v)
		printf("relabel %d<-%d\n",v,gNautyLab[v]);

	// return canonicalized edge list, and compute edge permutation sign:
	int perm[MAX_EDGES];
	int k,i,pos,sign;
	k = 0;
	sign = 1;
	for (v=0; v<nAir; ++v)
		for (e=0; e<gNautyCanon.d[v]; ++e) {
			w = gNautyCanon.e[gNautyCanon.v[v]+e];
			// v->w is the k'th edge in the canonicalized output
			// where is it in the input?
			pos = -1;
			for(i=k; i<nEdges; ++i)
				if ((gNautyLab[v]==EdgeList[i][0]) && (gNautyLab[w]==EdgeList[i][1])) {
					pos = i;
					break;
				}
			if (pos<k) {
				printf("Error: could not find edge %d->%d (output) in input edgelist\n", v, w);
				exit(EXIT_FAILURE);
			}
			if (k!=pos) {
				sign = -sign;
				// swap k with pos
				EdgeList[pos][0] = EdgeList[k][0];
				EdgeList[pos][1] = EdgeList[k][1];
			}
			EdgeList[k][0] = v;
			EdgeList[k][1] = w;
			k++;
		}

	// output edge list:
	printf("Canonically labeled edge list:\n(%d)*W([", sign);
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
	printf("])\n");

	return sign;
}

int main(int argc, char *argv[])
{
	printf("Hello, World!\n - max number of vertices: %d\n - max number of edges: %d\n", MAX_VERTS, MAX_EDGES);

	// prepare nauty:
	initNauty(&gNautyGraph);
	initNauty(&gNautyCanon);

	// input graph from edge list:
	// example:
	int g[MAX_EDGES][2];
	int g2[MAX_EDGES][2];
	int e,sign,sign2;

	g[0][0] = 0;
	g[0][1] = 1;
	g[1][0] = 0;
	g[1][1] = 3;
	g[2][0] = 1;
	g[2][1] = 0;
	g[3][0] = 1;
	g[3][1] = 4;
	g[4][0] = 2;
	g[4][1] = 1;
	g[5][0] = 2;
	g[5][1] = 5;

	sign = canonicizeGraph(3,3,6,g);

	// test: swap the order of two edges (g[3]<->g[5])
	for(e=0;e<6;++e) {
		g2[e][0]=g[e][0];
		g2[e][1]=g[e][1];
	}
	g2[3][0]=g[5][0];
	g2[3][1]=g[5][1];
	g2[5][0]=g[3][0];
	g2[5][1]=g[3][1];

	sign2 = canonicizeGraph(3,3,6,g2);

	// output must be the same edge list, but with negative sign
	for(e=0;e<6;++e)
		if ((g[e][0]!=g2[e][0]) || (g[e][1]!=g2[e][1])) {
			printf("Error: canonical labels differ\n");
			exit(EXIT_FAILURE);
		}
	if (sign2!=-1) {
		printf("Error: same signs (should be opposite)\n");
		exit(EXIT_FAILURE);
	}

	printf("Canonicalization test successful\n");

	exit(EXIT_SUCCESS);
}
