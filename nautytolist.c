// Erik Panzer
// code from 2020 to parse nauty strings into edge list
// parsedigraph6 adapted from parsegraph6 on 10 August 2022

#define MAX_VERTS 50
#define MAX_EDGES 100
int gnEdges = 0;
int gnVerts = 0;
int EDGELIST[MAX_EDGES][2];

// Nauty bitstring, packed into blocks of 6 bits, written as character+63
// returns pointer to the next token in s
char* parseNautyR(char *s, bool bits[], int nbits)
{
	char b, bit;
	int at = 0;
	while (at<nbits) {
		if ((s[0]<63) || (s[0]>126))
			return NULL;
		b = s[0] - 63;
		s++;
		for (bit=1<<5; (bit>0) && (at<nbits); bit >>= 1)
			bits[at++] = b & bit;
	}
	return s;
}

// returns pointer to the next token in s
char* parseNautyN(char *s, unsigned long long int *pN)
{
	if (s[0] == 0)
		return NULL;
	if (s[0] != 126) {
		if ((s[0] > 126) || (s[0] < 63))
			return NULL;
		*pN = s[0] - 63;
		return s+1;
	}
	bool bits[36];
	unsigned long long int N = 0;
	int k;
	if (s[1] != 126) {
		s = parseNautyR(s+1, bits, 18);
		if (s == NULL)
			return NULL;
		for (k=0; k<18; ++k)
			N = (N << 1) + bits[k];
		*pN = N;
		return s;
	}
	s = parseNautyR(s+2, bits, 36);
	if (s == NULL)
		return NULL;
	for (k=0; k<36; ++k)
		N = (N << 1) + bits[k];
	*pN = N;
	return s;
}

bool parsegraph6(char *s)
{
	// all characters in graph6 are between 63 and 126:
	int k;
	for (k=0; s[k]; ++k)
		if ((s[k]<63) || (s[k] > 126))
			return false;

	// vertex number:
	unsigned long long int N;
	s = parseNautyN(s, &N);
	if (s==NULL)
		return false;
	if (N>MAX_VERTS) {
		fprintf(stderr, "too many vertices: %llu\n", N);
		exit(EXIT_FAILURE);
	}
	gnVerts = N;

	// adjacency matrix:
	bool bits[(MAX_VERTS*(MAX_VERTS-1))/2];
	s = parseNautyR(s, bits, (gnVerts*(gnVerts-1))/2);
	if (s==NULL)
		return false;

	// edge list:
	gnEdges = 0;
	int v,w;
	k = 0;
	for (w=1; w<gnVerts; ++w)
	for (v=0; v<w; ++v, ++k)
		if (bits[k]) {
			if (gnEdges == MAX_EDGES) {
				fprintf(stderr, "too many edges\n");
				exit(EXIT_FAILURE);
			}
			EDGELIST[gnEdges][0] = v;
			EDGELIST[gnEdges][1] = w;
			gnEdges++;
		}

	return true;
}

bool parsedigraph6(char *s)
{
	// all characters in graph6 are between 63 and 126:
	int k;
	for (k=0; s[k]; ++k)
		if ((s[k]<63) || (s[k] > 126))
			return false;

	// vertex number:
	unsigned long long int N;
	s = parseNautyN(s, &N);
	if (s==NULL)
		return false;
	if (N>MAX_VERTS) {
		fprintf(stderr, "too many vertices: %llu\n", N);
		exit(EXIT_FAILURE);
	}
	gnVerts = N;

	// adjacency matrix:
	bool bits[MAX_VERTS*MAX_VERTS];
	s = parseNautyR(s, bits, gnVerts*gnVerts);
	if (s==NULL)
		return false;

	// edge list:
	gnEdges = 0;
	int v,w;
	k = 0;
	for (w=0; w<gnVerts; ++w)
	for (v=0; v<gnVerts; ++v, ++k)
		if (bits[k]) {
			if (gnEdges == MAX_EDGES) {
				fprintf(stderr, "too many edges\n");
				exit(EXIT_FAILURE);
			}
			EDGELIST[gnEdges][0] = v;
			EDGELIST[gnEdges][1] = w;
			gnEdges++;
		}

	return true;
}
