#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
	int arr[] = {2, 3, 4};
	int set_size = sizeof(arr) / sizeof(int);
	int pow_size = 1 << set_size;
	for (int i = 0; i < pow_size; i++)
	{
		for (int j = 1; j <= set_size; j++)
		{
			printf("%d ", (i & (1 << (j-1))) > 0);
		}
		printf("\n");
	}
	return 0;
}