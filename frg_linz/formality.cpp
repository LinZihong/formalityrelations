#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
using namespace std;

int n, m, e;
int from[100] = {}, to[100] = {}; //starting and ending vertices of edges
int G[30][30] = {};
int subgraph_vertices[30] = {}; //indicate if a vertex is chosen in divide_and_quotient()

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
	for (int i = 1; i <= e; i++)
	{
		if (i == pos) continue;
		int j = i < pos ? i : i-1;
		sub_from[j] = get_name_after_contraction(start, end, from[i]);
		sub_to[j] = get_name_after_contraction(start, end, to[i]);
		subG[sub_from[j]][sub_to[j]]++;
		if(subG[sub_from[j]][sub_to[j]] >= 2)
		{
			// printf("bad contraction: (%d, %d)\n", start, end);
			return 1;
		}
	}

	printf("good contraction (%d, %d), sign = %d:\n", start, end, sign);
	for (int i = 1; i <= e-1; i++)
		printf("%d->%d ", sub_from[i], sub_to[i]);
	printf("\n\n");

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
		}
	}

	if (sub_e != 2*n1 + m1 - 2)
		return 3; // bad number of edges in subgraph

	sign = sign_factor % 2 ? -1 : 1;
	
	printf("sign: %d\n", sign);
	// printing the subgraph (for testing)
	printf("subgraph (%d, %d): ", n1, m1);
	for (int i = 1; i <= sub_e; i++)
		printf("%d->%d ", sub_from[i], sub_to[i]);
	printf("\n");

	printf("quotient graph (%d, %d): ", qn, m-m1+1);
	for (int i = 1; i <= q_e; i++)
		printf("%d->%d ", q_from[i], q_to[i]);
	printf("\n\n");

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

int main(int argc, char const *argv[])
{
	// ad hoc input format using T-code-like format
	// n m e followed by the list of edges
	// vertices are named 1, 2, ..., n, n+1, ..., n+m (aerial followed by ground)
	cin >> n >> m >> e;
	for (int i = 1; i <= e; i++)
	{
		cin >> from[i] >> to[i];
		G[from[i]][to[i]] = 1;
	}

	// contract an edge
	for (int pos = 1; pos <= e; pos++)
	{
		if (from[pos] <= n && to[pos] <= n)
			contract_edge(from[pos], to[pos], pos);
	}

	divide_and_quotient();

	return 0;
}
