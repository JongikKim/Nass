#ifndef __CORE_GRAPH_H
#define __CORE_GRAPH_H

#include <string>
#include <map>
#include <vector>
#include <limits>
#include <iostream>

using namespace std;

typedef unsigned short vertex_t;

extern map<string, int> vlabel_map;
extern map<string, int> elabel_map;

extern vector<string> vertex_label;
extern vector<string> edge_label;

#define EPSILON numeric_limits<unsigned short>::max()

class coregraph
{
private:
	int* vlabels;  // vertex labels
	int** elabels; // adjacent matrix

	unsigned num_vertices;
	unsigned num_edges;

private:
    unsigned* vfreq;
    unsigned* efreq;

    unsigned** vertex_efreq;

private:
	vector<unsigned>* vlabel_index;

public:
	coregraph(unsigned n);
	~coregraph();

public:
	unsigned vsize(){ return num_vertices; }
	unsigned esize(){ return num_edges; }

public:
    int vlabel(int v){
        if(v == EPSILON) return EPSILON;
        return vlabels[v];
    }

    int elabel(int v1, int v2){
        if(v1 == EPSILON || v2 == EPSILON) return 0;
        return elabels[v1][v2];
    }

public: // accessor to vertex & edge labels
	int operator()(int v){ 
		if(v == EPSILON) return EPSILON;
		return vlabels[v];
	}
	int operator()(int v1, int v2){
		if(v1 == EPSILON || v2 == EPSILON) return 0;
		return elabels[v1][v2];
	}

public:
	void setVertexLabel(unsigned v, string label);
	void setEdgeLabel(unsigned v1, unsigned v2, string label);
	void setEdgeBond(unsigned v1, unsigned v2, int bond);

public:
    void labelScan();
	void labelRescan(unsigned sz_v, unsigned sz_e);
    
    unsigned* getVertexFrequencies(){ return vfreq; }
    unsigned* getEdgeFrequencies(){ return efreq; }
    unsigned** getVertexEdgeFrequencies(){ return vertex_efreq; }
	vector<unsigned>* getVertexLabelIndex(){ return vlabel_index; }

	vector<short>* adjacent;

public:
	unsigned sizeFilter(coregraph* rh);
	unsigned labelFilter(coregraph* rh);

public:
	void output(ostream& out, int number = 0);
};

#endif // __CORE_GRAPH_H
