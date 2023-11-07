#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "nausparse.h"

using namespace std;

#define MAX_EDGES 100
#define MAX_VERTS 100

int n, m, e;
int from[100] = {}, to[100] = {}; //starting and ending vertices of edges
int G[30][30] = {};
int subgraph_vertices[30] = {}; //indicate if a vertex is chosen in divide_and_quotient()

static DEFAULTOPTIONS_SPARSEDIGRAPH(gNautyOptions);
statsblk gNautyStats;
sparsegraph gNautyGraph, gNautyCanon;
int gNautyLab[MAX_VERTS];
int gNautyPtn[MAX_VERTS];
int gNautyOrbits[MAX_VERTS];

char s[2000];

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

	// for(v=0; v<nAir; ++v)
	// 	printf("relabel %d<-%d\n",v,gNautyLab[v]);
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
		printf("W([");
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
		printf("])");
		return sign;
}


int get_name_after_contraction(int start, int end, int vertex)
{
	if (start > end)
	{
		int temp = start;
		start = end;
		end = temp;
	}
	if (vertex < start) return vertex+1;
	if (start < vertex && vertex < end) return vertex;
	if (vertex > end) return vertex-1;
	return 1;
}

int contract_edge(int start, int end, int pos)
{
	int subG[30][30] = {};
	int sub_from[100] = {}, sub_to[100] = {};
	int sign = pos%2 ? -1 : 1;
	int g[MAX_EDGES][2];
	for (int i = 1; i <= e; i++)
	{
		if (i == pos) continue;
		int j = i < pos ? i : i-1;
		sub_from[j] = get_name_after_contraction(start, end, from[i]);
		sub_to[j] = get_name_after_contraction(start, end, to[i]);
		subG[sub_from[j]][sub_to[j]]++;
		if(subG[sub_from[j]][sub_to[j]] >= 2 || sub_from[j] == sub_to[j])
		{
		// printf("bad contraction: (%d, %d)\n", start, end);
			return 1;
		}
		g[j-1][0]=sub_from[j]-1;
		g[j-1][1]=sub_to[j]-1;
	}

	// printf("good contraction (%d, %d), sign = %d:\n", start, end, sign);
	// for (int i = 1; i <= e-1; i++)
	// 	printf("%d->%d ", sub_from[i], sub_to[i]);
	// printf("\n\n");	

	printf("+");
	sign *= canonicizeGraph(n-1, m, e-1, g);
	if(sign == -1) printf("*(%d)", sign);
	return 0;
}

int get_name_after_quotient(int l, int m1, int qn, int ground_vertex) // n+1 <= ground_vertex <= n+m
{
	int k = ground_vertex - n;
	if (k <= l) return qn + k;
	else if (l+1 <= k && k <= l+m1) return qn + l + 1;
	else return qn + k - (m1 - 1);
}

int compute_subgraph_and_quotient(int l, int m1, int n1, int qn, int *aerial_lookup)
{
	int sub_e = 0, q_e = 0;
	if (m1+n1 == m+n)
		return 1; // no vertex left

	int subG[30][30] = {}; //may even be unnecessary
	int sub_from[100] = {}, sub_to[100] = {};

	int qG[30][30] = {};
	int q_from[100] = {}, q_to[100] = {};

	int sign_factor = l*m1 + l + m1, sign; //from orientation alone

	int g[MAX_EDGES][2], q[MAX_EDGES][2] = {};

	for (int i = 1; i <= e; i++)
	{
		int start = from[i];
		int end = to[i];
		if (subgraph_vertices[start] == 1)
		{
			if ((end <= n && subgraph_vertices[end] == 1) || (end > n && end >= n+l+1 && end <= n+l+m1))
			{
				sub_e++;
				sub_from[sub_e] = aerial_lookup[start];
				if(end <= n)
				{
					sub_to[sub_e] = aerial_lookup[end];
				}
				else
				{
					sub_to[sub_e] = n1 + end - n - l;
				}

				subG[sub_from[sub_e]][sub_to[sub_e]] = 1;

				g[sub_e-1][0] = sub_from[sub_e] - 1;
				g[sub_e-1][1] = sub_to[sub_e] - 1;

				sign_factor += i - sub_e;
			}
			else
				return 2; // bad edge going out of ground in the quotient
		}
		else
		{
			q_e++;
			q_from[q_e] = aerial_lookup[start];
			if (end <= n)
			{
				if (subgraph_vertices[end] == 1)
					q_to[q_e] = qn + l + 1;
				else
					q_to[q_e] = aerial_lookup[end];
			}
			else
			{
				q_to[q_e] = get_name_after_quotient(l, m1, qn, end);
			}
			qG[q_from[q_e]][q_to[q_e]]++;
			if (qG[q_from[q_e]][q_to[q_e]] >= 2)
				return 4; // double edge in quotient

			q[q_e-1][0] = q_from[q_e] - 1;
			q[q_e-1][1] = q_to[q_e] - 1;
		}
	}

	if (sub_e != 2*n1 + m1 - 2)
		return 3; // bad number of edges in subgraph

	sign = sign_factor % 2 ? -1 : 1;
	
	// printf("sign: %d\n", sign);
	// printing the subgraph (for testing)
	// printf("subgraph (%d, %d): ", n1, m1);
	// for (int i = 1; i <= sub_e; i++)
	// 	printf("%d->%d ", sub_from[i], sub_to[i]);
	// printf("\n");
	printf("+");
	sign *= canonicizeGraph(n1, m1, sub_e, g);

	printf("*");

	// printf("quotient graph (%d, %d): ", qn, m-m1+1);
	// for (int i = 1; i <= q_e; i++)
	// 	printf("%d->%d ", q_from[i], q_to[i]);
	// printf("\n");
	sign *= canonicizeGraph(qn, m-m1+1, q_e, q);
	// printf("\n");

	if(sign == -1) printf("*(%d)", sign);

	return 0;
}

void divide_and_quotient()
{
	int pow_size = 1 << n;
	for (int i = 0; i < pow_size; i++)
	{
		int n1 = 0;
		int qn = 0; //number of aerial in quotient
		int aerial_lookup[30] = {};
		for (int j = 1; j <= n; j++)
		{
			subgraph_vertices[j] = ((i & (1 << (j-1))) > 0);
			if (subgraph_vertices[j] == 1)
			{
				// n1 += subgraph_vertices[j];
				aerial_lookup[j] = ++n1;
			}
			else
			{
				aerial_lookup[j] = ++qn;
			}
		}
		for(int l = 0; l <= m; l++)
		{
			for(int m1 = 0; m1 <= m-l; m1++)
			{
				int error = compute_subgraph_and_quotient(l, m1, n1, qn, aerial_lookup);
				// if(error == 0)
				// 	printf("good subgraph: subset=%d, l=%d, m1=%d, n1=%d\n", i, l, m1, n1);
			}
		}
	}
}

void reading_graph(int *from, int *to, int &n, int &m, int &k)
{
	cin.getline(s,1000);
	int len=strlen(s);
	int i=0;
	k=0;
	while(i<len)
	{
        //skipping the first [
		i++;
		while(s[i]=='[')
		{
            //new aerial
			n++;
			i++;
			while((s[i]>='0'&&s[i]<='9')||s[i]=='p')
			{
				k++;
				from[k]=n;
				int nr=0;
				while(s[i]>='0'&&s[i]<='9')
				{
					nr=nr*10+int(s[i])-48;
					i++;
				}
				if(s[i]==','||s[i]==']')
				{
					to[k]=nr;
					i++;
					continue;
				}
				nr=0;
				i++;
				while(s[i]>='0'&&s[i]<='9')
				{
					nr=nr*10+int(s[i])-48;
					i++;
				}
				if(m<nr)m=nr;
				from[k]=n;
				to[k]=-nr;
				i++;
			}
		}
	}

}

int main(int argc, char const *argv[])
{
	// prepare nauty:
	initNauty(&gNautyGraph);
	initNauty(&gNautyCanon);

	// ad hoc input format using T-code-like format
	// n m e followed by the list of edges
	// vertices are named 1, 2, ..., n, n+1, ..., n+m (aerial followed by ground)
	// cin >> n >> m >> e;
	// for (int i = 1; i <= e; i++)
	// {
	// 	cin >> from[i] >> to[i];
	// 	G[from[i]][to[i]] = 1;
	// }

	do
	{
		n = 0;
		m = 0;
		e = 0;
		memset(from, 0, sizeof(from));
		memset(to, 0, sizeof(to));
		memset(G, 0, sizeof(G));
		memset(subgraph_vertices, 0, sizeof(subgraph_vertices));
		reading_graph(from, to, n, m, e);
		if(!strcmp(s,"\0")) return 0;
		// cout << n << m << e;
		for (int i=1; i<=e; i++)
		{
			if (to[i] < 0)
				to[i] = -to[i] + n;
			// printf("%d %d ", from[i], to[i]);
		}

		// contract an edge
		for (int pos = 1; pos <= e; pos++)
		{
			if (from[pos] <= n && to[pos] <= n)
			{
				contract_edge(from[pos], to[pos], pos);
			}
		}

		divide_and_quotient();

		printf("\n");

		// from = {};
		// to = {};
	} while (strcmp(s,"\0"));

	return 0;
}
