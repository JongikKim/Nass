#ifndef __GRAPH_H
#define __GRAPH_H

#include "coregraph.h"

#include <iostream>

using namespace std;

// Wrapper class for core graph
//
// Each node in the state-space tree keeps the following
// 1. SMALLER GRAPH
// - a vertex ordering
// - border, i.e., border position between mapped and unmapped vertices
//
// 2. LARGER GRAPH
// - border
//
// A graph wrapper is shared by all states. To distinguish states, each state
// replaces information mentioned above using the setState method.

struct mapping;
struct LabelFreq;
class graph
{
private:
	coregraph& src;
	vector<short>* adjacent;

private:
	unsigned begin; // begin position in the source graph
	unsigned end;   // end position + 1 in the source graph
	unsigned border; // border position between mapped and unmapped vertices

	unsigned nedges; // number of edges in unmapped vertices
	bool* mapping_info;

public:
    unsigned* vfreq;
    unsigned* efreq;

    unsigned** vertex_efreq;

private:
    unsigned** vertex_bridges;

public: // be careful not to use illegally
    unsigned vertexDegree(int vertex){ return vertex_efreq[vertex][elabel_map.size()]; }

private:
	vector<unsigned>* vlabel_index; // pointer to vlabel_index_mem
	vector<unsigned>* vlabel_index_mem;

public:
	vector<unsigned>& findVertices(unsigned label);

private:
	vertex_t* ordering; // vertex ordering of the source graph
	int O(unsigned v){ return ordering[v + begin]; } 

	vertex_t* reverse_order;
	bool mappingInfo(int v) { return reverse_order[v] < border; }

public:
	graph(coregraph& source, vertex_t* order = NULL);
	~graph();

public:
	unsigned vsize(){ return end - begin; }
	unsigned esize(){ return nedges; }

public:
	int vlabel(unsigned v){ return src(O(v)); }
	int elabel(unsigned v1, unsigned v2){ return src(O(v1), O(v2)); }

public: // methods for state-space tree expansion and mapped/unmapped parts
	void setState(unsigned border, LabelFreq* freq);
	void setState(mapping* m, LabelFreq* freq);
	mapping* generateNextState(int next_vertex, mapping* pm, unsigned bitmap_size);

	void selectMappedPart(){ begin = 0; end = border; }
	void selectUnmappedPart(){ begin = border; end = src.vsize(); }

public: // method for reordering
	void reorder(graph& rh);
	void reorder(vector<int>& order);

	void printOrder(){
		for(unsigned i = 0; i < src.vsize(); i++)
			cout << ordering[i] << " ";
		cout << endl;
	}


public: // methods for frequency calculation
	void countLabelFrequencies();
	void countBridgeFrequencies();
	void countVertexEdgeFrequencies();

	void countLabelsIncremental(bool restore);
	void countBridgesIncremental(bool restore);

public: // methods for counting errors
	unsigned mappedErrors(int u); // for deletion only
	unsigned mappedErrors(int u, graph& rh);
	unsigned bridgeErrors(graph& rh);
	unsigned unmappedErrors(graph& rh);
	unsigned edgeLabelErrors(unsigned v1, unsigned v2, graph& rh);
	static unsigned labelErrors(unsigned* arr_l, unsigned* arr_r, unsigned size);

//	unsigned sizeErrors(graph& rh);

public:
	void print();
};

#endif // __GRAPH_H
