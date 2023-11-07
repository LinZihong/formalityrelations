#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
	int n, m;
	n = strtol(argv[1], nullptr, 0);
	m = strtol(argv[2], nullptr, 0);

	char infile[80], outfile[80];
	sprintf(infile, "canoedge_%d_%d.el", n, m);
	sprintf(outfile, "erikised_%d_%d.el", n, m);

	freopen(infile, "r", stdin);
	freopen(outfile, "w", stdout);

	int order, e;
	while (cin >> order >> e) // main loop to process each graph
	{
		vector<int> edge_list[20] = {};
		int from, to;
		for(int i=1; i<=e; i++)
		{
			cin >> from >> to;
			edge_list[from].push_back(to);
		}
		printf("[");
		for (int i=1; i<=n; i++)
		{
			sort(edge_list[i].begin(), edge_list[i].end());
			printf("[");
			for (int j = 0; j < edge_list[i].size(); j++)
			{
				int end = edge_list[i][j];
				if (end <= n)
					printf("%d", end);
				else
					printf("p%d", end - n);
				if(j < edge_list[i].size() - 1)
					printf(",");
			}
			printf("]");
			if(i<n)
				printf(",");
		}
		printf("]\n");
	}

	fclose(stdin);
	fclose(stdout);
	
	return 0;
}