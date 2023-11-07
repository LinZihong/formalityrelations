#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>
using namespace std;

int main(int argc, char const *argv[])
// n = number of aerial vertices
// m = number of ground vertices
// -k | -f Kontsevich graph (2n+m-2 edges) or [by default ]Formality graph (2n+m-3 edges)
// -rm remove temporary files i.e. the undirected and directed graphs
{
	int n, m;

	// cout << "input n, m:";
	// cin >> n >> m;
	n = strtol(argv[1], nullptr, 0);
	m = strtol(argv[2], nullptr, 0);

	int edges = 2*n + m - 3; //number of edges we want, option -f by default
	if(strcmp(argv[argc-2], "-k") == 0 || strcmp(argv[argc-1], "-k") == 0)
		edges += 1; //one more edge for option -k

	char geng[100];
	sprintf(geng, "./geng %d %d:%d -c undirected_%d_%d.gr", n+m, edges/2, edges, n, m);
	system(geng);

	// char watercluster[100];
	// sprintf(watercluster, "./watercluster Z < undirected_%d_%d.gr > directed_%d_%d.gr", n, m, n, m);
	// system(watercluster);

	char directg[100];
	sprintf(directg, "./directg -e%d undirected_%d_%d.gr directed_%d_%d.gr", edges, n, m, n, m);
	system(directg);

	char pickg[100];
	sprintf(pickg, "./pickg -d0 -m%d: directed_%d_%d.gr filtered_%d_%d.gr", m, n, m, n, m);
	system(pickg);

	if(strcmp(argv[argc-1], "-rm") == 0)
	{
		char rm[100];
		sprintf(rm, "rm undirected_%d_%d.gr directed_%d_%d.gr", n, m, n, m);
		system(rm);
	}

	char showg[100];
	sprintf(showg, "./showg -e -o1 -q filtered_%d_%d.gr edges_%d_%d.gr", n, m, n, m);
	system(showg);

	return 0;
}