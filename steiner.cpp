#include<iostream>
#include<vector>
#include<list>
#include<array>
#include<algorithm>

// Graph representation:
//   vertex = {0, ..., N-1} for input N <= 32

// subset of vertices, represented by a bit vector
typedef unsigned int	vertex_subset;
//typedef unsinged __int64 vertex_subset; // for N <= 64

void print_subset_binary(int N, vertex_subset V)
{
	std::cout << "{";
	for(int i = 0; i<N ; i++)
		if (V & (1 << i)) std::cout << i <<" ";
	std::cout << "}";
}

// edge is a pair of vertices with weight
class edge
{
public:
	int a, b; 	// endvertices
	int w;		// weight

	edge(int aa, int bb, int ww = 1) : a(aa), b(bb), w(ww) {}
};

bool compare_edge(edge e, edge f) { return e.w < f.w; }

// List of edge
typedef std::vector<edge> edge_list;

// Output format
class output_form
{
public:
	vertex_subset	V;		// vertex set for this
	edge_list	MST;		// minimum spanning tree for V, if CONNECTED; empty, otherwise
	int		weight;		// wegith of minimum "Steiner" tree for V, at last
	vertex_subset	steiner;	// V + SteinerPoints of a minimum Steiner tree for V

	output_form(vertex_subset VV = 0) : V(VV), MST(), weight(0), steiner(0) {}
};

// disjoint set structure
class disjoint_set
{
protected:
	std::vector<int> parent, rank;
public:
	disjoint_set(int N) : parent(N), rank(N, 1) 
	{
		for(int i=0; i<N; i++) parent[i] = i;
	}

	// find operation
	int find(int u)	
	{
		if (u == parent[u]) return u;
		else return parent[u] = find(parent[u]);
	}

	// union operation
	int merge(int u, int v)
	{
		u = find(u), v = find(v);
		if(u==v) return u;
		if(rank[u] > rank[v]) return parent[u] = v;
		else if(rank[u] < rank[v]) return parent[v] = u;
		else {rank[v]++; return parent[u] = v;}
	}
};

// modified Kruskal's algorithm for subsets V of vertices {0, ..., N-1}
// assuming the edges in E are already sorted in their weights
// T will store the edges in a minimum spanning tree for V, if possible
// This function returns a MST for V and its weight, if the subgraph of G=(V,E) induced by V is connected
// or returns -1, otherwise.
int min_spanning_tree_for_subset(int N, vertex_subset V, const edge_list E, edge_list & T)
{
	int n = 0; // n = |V|
	for (int i = 0; i < N; i++) // count # of 1's in V
		n += ((V & (1 << i)) > 0);
	
	// trivial cases
	if (n < 2) return 0;

	disjoint_set components(N);
	int wT = 0;

	for(edge e : E)
	{
		if(components.find(e.a) != components.find(e.b)) // edge e = (a,b) does not make a cycle
		{
			if((V & (1 << e.a)) > 0 && (V & (1 << e.b)) > 0) // only if both a and b belong to V
			{
				T.push_back(e); 
				wT += e.w; 
				components.merge(e.a, e.b);
			}
		}

		if (T.size() == n-1) return wT;
	}

	// V is not connected in the induced subgraph
	T.clear();
	return -1;
}

int main()
{
	int N = 0;
	edge_list G;

	// read an input graph G from standard input
	// in adjacency matrix
	std::cin >> N;
	for (int i=0; i<N; i++)
	{
		for(int j = 0; j<N; j++)
		{
			int w;
			std::cin >> w;
			if (j > i && w != 0) G.push_back(edge(i,j,w));
		}
	}

	// sort the edges in weights
	std::sort(G.begin(), G.end(), compare_edge);

	// for every subset V of {0, ..., N-1}
	// check if the subgraph of G induced by V is connected and
	// if so, compute a minimum spanning tree for the subgraph
	vertex_subset maxV = (1 << N) - 1;	// 2^N - 1
	std::vector<output_form> Steiner(maxV+1);	
	for (vertex_subset V = 0; V <= maxV; V++)
	{
		Steiner[V].V = Steiner[V].steiner = V;
		Steiner[V].weight = min_spanning_tree_for_subset(N, V, G, Steiner[V].MST);
	}

	// Pull down MSTs for those subsets that are NOT connected 
	// We need to enumerate all subsets in the decreasing order of # its vertices
	// Compute the list of vertex_subset's in that order by dynamic programming
	std::list<vertex_subset> *S = new std::list<vertex_subset> [N+1];
	for (vertex_subset V = 0; V <= maxV; V++)
	{
		// count # of 1's
		int ones = 0;
		int v = V;
		for (int i = 0; i<N ; i++, v >>= 1) ones += v%2;
		S[ones].push_back(V);
	}

	for (int ones = N; ones > 2; ones--)
		for(vertex_subset V : S[ones])
			for(vertex_subset I = 1; I<=maxV; I<<=1)
				if ((V & I) != 0) // && Steiner[V-I].MST.empty())	// V contains vertex i = (log I)
				{
					if (Steiner[V - I].weight == -1 || Steiner[V - I].weight > Steiner[V].weight)
						Steiner[V - I].steiner = Steiner[V].steiner, Steiner[V - I].weight = Steiner[V].weight;
				}

	delete [] S;			

	// Print out the results
	for(vertex_subset V = 0; V <= maxV; V++)
	{
		print_subset_binary(N, V);
		std::cout << std::endl;
		std::cout << "Weight of MST: "<< Steiner[V].weight << std::endl;
		std::cout << "MST: ";
		if(Steiner[V].steiner != V)
			std::cout << "--> ", print_subset_binary(N, Steiner[V].steiner);
		else
			for(edge e : Steiner[V].MST)
				std::cout << "(" << e.a <<", " << e.b <<") ";
		std::cout << std::endl << std::endl;
	}
	std::cout.flush();
			

	return 0;
}