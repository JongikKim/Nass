#include "inves.h"
#include "nass.h"

#include <algorithm>

Inves::Inves(graph* x, graph* y)
{
	this->x = x;
	this->y = y;

	this->partition = NULL;

	source = NULL; // source will be set in incrementalPartitioning
	target = new int[x->vsize()]; // reserve max size
	F = new bool[y->vsize()];

	// save local states for stopping and resuming partitioning
	vertices_saved = NULL;
	offset_saved = p_saved = 0;
	tightLB = false;
}

Inves::~Inves()
{
	delete [] F;
	delete [] target;

	delete [] vertices_saved;
}

int Inves::COMPONENT_SIZE_THRESHOLD = 6;

int Inves::removeTinyComponents(vector<int>& component)
{
	int tmp[psize];
	for(int i = 0; i < psize; i++) tmp[i] = source[i];

	int begin = 0, end = psize-1;

	// component must have at least 2 elements
	for(unsigned i = component.size() - 1; i > 0; i--){
		int start = component[i-1];
		int stop  = component[i];

		if(stop - start <= COMPONENT_SIZE_THRESHOLD){
			for(int j = start; j < stop; j++)
				source[begin++] = tmp[j];
			continue;
		}
		while(start != stop) source[end--] = tmp[--stop];
	}

	return begin;
}

void Inves::DFSOrder(int* vertices, unsigned v, bool* visited, unsigned subsize, vector<int>& order)
{
	visited[v] = true;
	order.push_back(vertices[v]);
	for(unsigned i = 0; i < subsize; i++)
		if(!visited[i] && x->elabel(vertices[v], vertices[i]))
			DFSOrder(vertices, i, visited, subsize, order);
}

void Inves::determineVertexOrder(int* vertices, unsigned subsize, vector<int>& order, vector<int>* component)
{
	order.reserve(subsize);

	bool visited[subsize];
	for(unsigned i = 0; i < subsize; i++) visited[i] = false;
	for(unsigned i = 0; i < subsize; i++){
		if(visited[i]) continue;
		if(component) component->push_back(order.size());
		DFSOrder(vertices, i, visited, subsize, order);
	}

	if(component) component->push_back(subsize);
}

/*
void Inves::frequencyVertexOrder(int* vertices, unsigned subsize, vector<int>* component)
{
	int max = numeric_limits<int>::min();
	unsigned v = 0;
	for(unsigned i = 0; i < subsize; i++){
		int lv = gx->vlabel(vertices[i]);
		int weight = gy->vsize() - gy->vfreq[lv - 1];
		for(unsigned j = 0; j < subsize(); j++){
			int le = gx->elabel(vertices[i], vertices[j]);
			if(le) weight += gy->esize() - gy->efreq[le - 1];
		if(weight > max) { max = weight; v = i; }
	} 

	orders.push_back(v);
	visited[v] = true;

	while(orders.size() < subsize){
		max = numeric_limits<int>::min();

		for(unsigned i = 0; i < vsize(); i++){
			if(visited[i]) continue;

			int lv = gx->vlabel(vertices[i]);
			int weight = gy->vsize() - gy.vfreq[lv - 1];

			for(unsigned j = 0; j < orders.size(); j++){
				int le = gx->elabel(vertices[i], vertices[order[j]]);
				if(le) weight += gy->esize() - gy.efreq[le - 1];
			}
			if(weight > max){max = weight; v = i; }
		}

		orders.push_back(v);
		visited[v] = true;
	}

//	connection??
}
*/

void Inves::alternativeVertexOrder(int* vertices, unsigned subsize, vector<int>* component)
{
	if(component) component->push_back(0);

	for(unsigned i = 1; i < subsize; i++){
		unsigned j = 0;
		for(; j < i; j++) if(x->elabel(vertices[j], vertices[i])) break;
		if(j < i) continue; // if current vertex i is connected to one of already ordered vertices 

		// else j == i
		bool connected = false;
		for(unsigned k = i+1; k < subsize; k++){
			unsigned j = 0;
			for(; j < i; j++)
				if(x->elabel(vertices[j], vertices[k])) break;

			if(j < i){
				int item = vertices[k];
				for(unsigned p = k; p > i; p--) vertices[p] = vertices[p-1];
				vertices[i] = item;
				connected = true;
				break;
			}
		}

		if(!connected && component) component->push_back(i); 
	}

	if(component) component->push_back(subsize);
}

// reorder vertices of a partition for efficient subgraph isomorphism
int Inves::reorderPartition()
{
	sort(source, source + psize, vertexCompare(x, y));

	vector<int> order, component;
//	alternativeVertexOrder(source, psize, &component);
	determineVertexOrder(source, psize, order, &component);
	for(int i = 0; i < psize; i++) source[i] = order[i];

	return removeTinyComponents(component);
}

bool Inves::rematch()
{
	return false;

	int tmp = source[fail_position];
	for(int i = fail_position; i > 0; i--) source[i] = source[i-1];
	source[0] = tmp;

	// subgraph isomorphism test must fail within the following partition size
	int partition_size = fail_position + 1;

	vector<int> order;
	determineVertexOrder(source, partition_size, order, NULL);
	for(int i = 0; i < partition_size; i++) source[i] = order[i];

	for(unsigned i = 0; i < y->vsize(); i++) F[i] = false;
	bridge_error = 0; fail_position = 0;
	return inducedSI(0);
}

bool Inves::incrementalPartitioning(int* vertices, int& offset)
{
	source = vertices + offset;
	psize = x->vsize() - offset;

	if(psize == 0) return true;

	// reorder source and remove tiny connected components
	int begin = reorderPartition(); 

	//collect tiny connected components
	for(int i = 0; i < begin; i++) tinycomponents.push_back(source[i]);
	offset += begin;

	source = vertices + offset;
	psize = x->vsize() - offset;

	for(unsigned i = 0; i < y->vsize(); i++) F[i] = false;
	bridge_error = 0; fail_position = 0; 

	bool res = inducedSI(0);

	// re-matching mismatching partition while the size of mismatch decreases
	if(fail_position < psize){
		int prev_fail = fail_position;
		do{
			prev_fail = fail_position;
			res = rematch();
		}while(fail_position < prev_fail);
	}

	return res;
}

bool Inves::inducedSI(int d)
{
	if(d == psize){ fail_position = psize; return true;}

	int berr_saved = bridge_error;
	int u = source[d];

	// for space-state tree at level > 0, findVertices can be inefficient
	vector<unsigned>& vertex_list = y->findVertices(x->vlabel(u));
	for(unsigned i = 0; i < vertex_list.size(); i++){
		bridge_error = berr_saved;

		int v = vertex_list[i];
		if(F[v] || !valid(u, v, d)) continue;

		F[v] = true;
		target[d] = v;
		if(inducedSI(d+1)) return true;
		F[v] = false;
	}

	bridge_error = berr_saved;
	if(d > fail_position) fail_position = d;

	return false;
}

bool Inves::valid(int u, int v, int d)
{
	for(int i = d - 1; i >= 0; i--)
		if(x->elabel(u, source[i]) != y->elabel(v, target[i])) return false;

 	// for space-state tree at level > 0, edgeLabelErrors do not count errors now
	bridge_error += x->edgeLabelErrors(u, v, *y);
	if(bridge_error > 1) return false;

	return true;
}

bool sizeCompare(const vector<int>& v1, const vector<int>& v2)
{
	return v1.size() < v2.size();
}

// reorder vertices of a graph for efficient GED computation
void Inves::reorderGraph(int* vertices, int offset, int tau)
{
	int np = tau+1;
	for(int i = 0; i < tau+1; i++)
		if(partition[i].size() == 0){ np = i; break; }

	sort(partition, partition + np, sizeCompare);

	int pos = 0;
	for(int p = 0; p < np; p++){
		sort(partition[p].begin(), partition[p].end(), vertexCompare(x, y));

		vector<int> order;
		vector<int>::iterator iter = partition[p].begin();
		determineVertexOrder(&(*iter), partition[p].size(), order, NULL);
		for(unsigned i = 0; i < partition[p].size(); i++) partition[p][i] = order[i];

		std::copy(partition[p].begin(), partition[p].end(), vertices+pos);
		pos += partition[p].size();
	}

	// DFS Ordering that goes through all mismatching partitions
	vector<int> order;
	determineVertexOrder(vertices, offset, order, NULL);
	// push back remaining vertices // TODO: should retain connection!!
	for(unsigned i = offset; i < x->vsize(); i++) order.push_back(vertices[i]);
	x->reorder(order);
}

unsigned Inves::incrementalLB(int tau)
{
	if(vertices_saved == NULL){
		vertices_saved = new int[x->vsize()];
		for(unsigned i = 0; i < x->vsize(); i++) vertices_saved[i] = i;
	}

	int* vertices = vertices_saved;
	int offset = offset_saved; // the start position of the last partition
	for(int p = p_saved; p < tau + 1; p++){
		if(incrementalPartitioning(vertices, offset)){
			tightLB = true;
			return p;
		}

		offset += fail_position + 1;
	}

	offset_saved = offset;
	p_saved = tau+1;
	return tau+1;
}

unsigned Inves::lowerBound(int tau)
{
	int size = x->vsize();

	int vertices[size]; // vertiecs of x
	for(int i = 0; i < size; i++) vertices[i] = i;

	vector<int> tmp[tau+1];
	this->partition = tmp;

	int offset = 0; // the start position of the last partition
	for(int p = 0; p < tau + 1; p++){
		if(incrementalPartitioning(vertices, offset)) return p;

		for(int i = offset; i < offset + fail_position + 1; i++)
			partition[p].push_back(vertices[i]);

		int pos = (p == 0 ? 0 : p-1);
		for(unsigned i = 0; i < tinycomponents.size(); i++)
			partition[pos].push_back(tinycomponents[i]);
		tinycomponents.clear();

		offset += fail_position + 1;
	}

	return tau+1;
}
