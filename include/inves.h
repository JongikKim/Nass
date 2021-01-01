#ifndef __INVES_H
#define __INVES_H

#include "graph.h"
#include "dataset.h"

class vertexCompare
{
private:
	graph* src;
	graph* obj;
	DataSet* dataset;

public:
	vertexCompare(graph* src, graph* obj) : src(src), obj(obj)
	{
		dataset = DataSet::getInstance();
	}

	bool operator () (const int& u, const int& v)
	{
		unsigned freq1 = obj->vfreq[src->vlabel(u) - 1];
		unsigned freq2 = obj->vfreq[src->vlabel(v) - 1];

		if(freq1 == freq2)
			return src->vertexDegree(u) < src->vertexDegree(v);
		return freq1 < freq2;
	}
};

class Inves
{
public:
	static int COMPONENT_SIZE_THRESHOLD;

private:
	graph* x;
	graph* y;

//	int tau;

private:
	vector<int>* partition;
	vector<int> tinycomponents; // vertices in tiny connected components

private:
	int* source;
	int* target;

private: // initialized in incrementalPartitioning
	bool* F;
	int bridge_error; // errors in bridges
	int psize; // size of the current partition

public:
	int fail_position; // maching fail position

private: // partial status for incremental partitioning
	int* vertices_saved;
	int offset_saved;
	int p_saved;
public:
	bool tightLB;
	

public:
	Inves(graph* x, graph* y);
	~Inves();

public:
	unsigned incrementalLB(int tau);
	unsigned lowerBound(int tau); // tight LB & reordering

private:
	bool incrementalPartitioning(int* partition, int& offset);

private:
	void DFSOrder(int* vertices, unsigned v, bool* visited, unsigned subsize, vector<int>& order);
	void determineVertexOrder(int*, unsigned, vector<int>&, vector<int>*);
	void alternativeVertexOrder(int* vertices, unsigned subsize, vector<int>* component);
	int reorderPartition();
	int removeTinyComponents(vector<int>& component);

	void reorderGraph(int* vertices, int offset, int tau);

	bool rematch();
	bool inducedSI(int d);
	bool valid(int u, int v, int d);
};

#endif
