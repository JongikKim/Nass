#include <limits>
#include <cstring>
#include <cassert>

#include "graph.h"
#include "nassged.h"

#define SWAP(x, y, t) (t = x, x = y, y = t)

graph::graph(coregraph& source, vertex_t* order)
	: src(source), adjacent(source.adjacent) {
	begin = 0;
	end = src.vsize();
	border = 0;		

	ordering = order; //org_ordering;
	mapping_info = new bool[src.vsize()];
	reverse_order = new vertex_t[src.vsize()];

	vfreq = src.getVertexFrequencies();
	efreq = src.getEdgeFrequencies();
	vertex_efreq = src.getVertexEdgeFrequencies();

	vertex_bridges = NULL;
	nedges = src.esize();

	vlabel_index = src.getVertexLabelIndex();
	vlabel_index_mem = NULL;
}

vector<unsigned>& graph::findVertices(unsigned label)
{
	if(vlabel_index) return vlabel_index[label-1];

	if(vlabel_index_mem == NULL)
		vlabel_index_mem = new vector<unsigned>[vlabel_map.size()];
	else
		for(unsigned i = 0; i < vlabel_map.size(); i++)
			vlabel_index_mem[i].clear();

	vlabel_index = vlabel_index_mem;
	for(unsigned i = 0; i < vsize(); i++) vlabel_index[vlabel(i) - 1].push_back(i);

	return vlabel_index[label-1];
}

graph::~graph()
{
	delete [] mapping_info;
	delete [] reverse_order;
	delete [] vlabel_index_mem;
}

void graph::setState(unsigned border, LabelFreq* freq)
{
	assert(border <= src.vsize());
	this->border = border;

	vertex_bridges = freq->vertex_bridges;
	vertex_efreq = freq->vertex_efreq;
	vfreq = freq->vfreq;
	efreq = freq->efreq;
	vlabel_index = NULL;

	selectUnmappedPart(); // update begin and end !!
}

void graph::setState(mapping* m, LabelFreq* freq)
{
	border = m->size;
	ordering = m->ordering;

	vertex_bridges = freq->vertex_bridges;
	vertex_efreq = freq->vertex_efreq;
	vfreq = freq->vfreq;
	efreq = freq->efreq;
	vlabel_index = NULL;

	selectUnmappedPart(); // update begin and end !!
}

mapping* graph::generateNextState(int next_vertex, mapping* pm, unsigned bitmap_size)
{
	mapping* m = new mapping(src.vsize(), bitmap_size);
	memcpy(m->ordering, ordering, src.vsize() * sizeof(vertex_t));
	memcpy(m->bitmap, pm->bitmap, bitmap_size * sizeof(uint64));
	m->size = border + 1;

	next_vertex = next_vertex + border;
	m->setbit(m->ordering[next_vertex]);
	int tmp;
	SWAP(m->ordering[border], m->ordering[next_vertex], tmp);

	return m;
}

void graph::countBridgesIncremental(bool restore)
{
	selectMappedPart();

	int delta = restore ? -1 : 1;

	unsigned ivertex = O(end-1); // inserted vertex

	for(unsigned i = 0; i < adjacent[ivertex].size(); i++){
		short target = adjacent[ivertex][i];
		if(mapping_info[target]){
			int pos = reverse_order[target];
			vertex_bridges[pos][src(ivertex, target)-1] -= delta;
		}
	}

	if(restore){ selectUnmappedPart(); return; }

	// compute bridges for the vertex inserted into the mapped part
	memset(vertex_bridges[end-1], 0, sizeof(int) * elabel_map.size());
	for(unsigned i = 0; i < adjacent[ivertex].size(); i++){
		short target = adjacent[ivertex][i];
		if(!mapping_info[target])
			vertex_bridges[end-1][src(ivertex, target)-1] += 1;
	}

	selectUnmappedPart();
}

void graph::countLabelsIncremental(bool restore)
{
	int dvertex = O(-1); // deleted vertex: begin will be added

	int delta = restore ? -1 : 1;
	
	vfreq[src(dvertex)-1] -= delta;

	for(unsigned i = 0; i < adjacent[dvertex].size(); i++){
		short target = adjacent[dvertex][i];
		if(!mapping_info[target]){
			nedges -= delta;
			efreq[src(dvertex, target)-1] -= delta;
		}
	}
}

void graph::countBridgeFrequencies()
{
	selectMappedPart();

	for(unsigned v = begin; v < end; v++){
		memset(vertex_bridges[v], 0, sizeof(int) * elabel_map.size());
		for(unsigned i = 0; i < adjacent[O(v)].size(); i++){
			short target = adjacent[O(v)][i];
			if(!mapping_info[target])
				vertex_bridges[v][src(O(v), target) - 1]++;
		}
	}

	selectUnmappedPart();

	// for incremental counting
	for(unsigned i = 0; i < src.vsize(); i++) reverse_order[ordering[i]] = i;
}

void graph::countLabelFrequencies()
{
//	if(vsize() == 0){ vfreq = efreq = NULL; return; }

	memset(vfreq, 0, sizeof(int) * vlabel_map.size());
	memset(efreq, 0, sizeof(int) * elabel_map.size());

	for(unsigned i = 0; i < vsize(); i++) vfreq[vlabel(i)-1]++;

	for(unsigned i = 0; i < src.vsize(); i++){
		if(i < border) mapping_info[ordering[i]] = true;
		else mapping_info[ordering[i]] = false;
	}

	nedges = 0;
	for(unsigned i = 0; i < vsize(); i++)
		for(unsigned j = 0; j < adjacent[O(i)].size(); j++){
			short target = adjacent[O(i)][j];
			if(!mapping_info[target]){
				nedges++;
				efreq[src(O(i), target) - 1]++;
			}
		}

	nedges = nedges/2;
	for(unsigned i = 0; i < elabel_map.size(); i++) efreq[i] /= 2;
}

void graph::countVertexEdgeFrequencies()
{
	for(unsigned i = 0; i < src.vsize(); i++){
		if(i < border) mapping_info[ordering[i]] = true;
		else mapping_info[ordering[i]] = false;
	}

	for(unsigned v = 0; v < vsize(); v++){
		memset(vertex_efreq[v], 0, sizeof(unsigned)*(elabel_map.size()+1));
		for(unsigned i = 0; i < adjacent[O(v)].size(); i++){
			short target = adjacent[O(v)][i];
			if(!mapping_info[target]) vertex_efreq[v][src(O(v), target) - 1]++;
		}

		for(unsigned i = 0; i < elabel_map.size(); i++)
			vertex_efreq[v][elabel_map.size()] += vertex_efreq[v][i];
	}
}

void graph::reorder(vector<int>& order)
{
	for(unsigned i = 0; i < vsize(); i++)
		ordering[i + begin] = vertex_t(order[i] + begin);
}

#define WV(v)         (1.0 - (double)g.vfreq[vlabel(v) - 1]/(double)g.vsize())
#define WE(v1, v2)    (1.0 - (double)g.efreq[elabel(v1, v2) - 1]/(double)g.esize())
void graph::reorder(graph& g)
{
	if(border != 0 || g.border != 0){
		cerr << "ordering failed: mapped vertex pair exists" << endl;
		return;
	}

	for(unsigned i = 0; i < vsize(); i++) ordering[i] = i;
	for(unsigned i = 0; i < g.vsize(); i++) g.ordering[i] = i;

    double max = numeric_limits<double>::min();
    unsigned v = 0;

    for(unsigned i = 0; i < vsize(); i++){
        double weight = WV(i);
        for(unsigned j = 0; j < vsize(); j++)
            if(elabel(i, j)) weight += WE(i, j);
        if(weight > max){ max = weight; v = i; }
    }

    int tmp;
    SWAP(ordering[0], ordering[v], tmp);

    for(unsigned next = 1; next < vsize(); next++){
        max = numeric_limits<double>::min();

        for(unsigned i = next; i < vsize(); i++){
			double weight = 0.0;
            for(unsigned j = 0; j < next; j++)
                if(elabel(i, j)) weight += WE(i, j);
			if(weight != 0.0) weight += WV(i);
            if(weight > max){ max = weight; v = i; }
        }

		if(max == 0.0){ // disconnected here!!
        	for(unsigned i = next; i < vsize(); i++){
				double weight = WV(i);
				for(unsigned j = next; j < vsize(); j++)
					if(elabel(i, j)) weight += WE(i, j);
            	if(weight > max){ max = weight; v = i; }
			}
        }

        SWAP(ordering[next], ordering[v], tmp);
    }
}

/*
unsigned graph::sizeErrors(graph& rh)
{
	int vdiff = int(vsize()) - int(rh.vsize());
	int ediff = int(esize()) - int(rh.esize());

	if(vdiff < 0) vdiff = -1*vdiff;
	if(ediff < 0) ediff = -1*ediff;

	return unsigned(vdiff + ediff);
}
*/

unsigned graph::labelErrors(unsigned* arr_l, unsigned* arr_r, unsigned size)
{
	unsigned diff = 0;
	unsigned sizeA = 0, sizeB = 0, ins = 0;
	for(unsigned i = 0; i < size; i++){
		sizeA += arr_l[i];
		sizeB += arr_r[i];

		ins += (arr_l[i] < arr_r[i] ? arr_l[i] : arr_r[i]);
	}

	diff = (sizeA > sizeB ? sizeA : sizeB);
	diff = diff - ins;

	return diff;
}

// for deletion error only
unsigned graph::mappedErrors(int u)
{
	selectMappedPart();

	unsigned verr = 1;
	unsigned eerr = 0;

	for(int v = 0; v < u; v++)
		if(elabel(u, v)) eerr++;

	selectUnmappedPart();

	return verr + eerr;
}

unsigned graph::mappedErrors(int u, graph& rh)
{
	selectMappedPart(); rh.selectMappedPart();

	unsigned verr = 0;
	unsigned eerr = 0;

	if(vlabel(u) != rh.vlabel(u)) verr++;
	for(int v = 0; v < u; v++)
		if(elabel(u, v) != rh.elabel(u, v)) eerr++;

	selectUnmappedPart(); rh.selectUnmappedPart();

	return verr + eerr;
}

/*
unsigned graph::mappedErrors(graph& rh)
{
	selectMappedPart(); rh.selectMappedPart();

	unsigned err = 0;
	for(unsigned i = 0; i < vsize(); i++) err += mappedErrors(i, rh);

	selectUnmappedPart(); rh.selectUnmappedPart();

	return err;
}
*/

unsigned graph::bridgeErrors(graph& rh)
{
	if(vertex_bridges == NULL || rh.vertex_bridges == NULL) return 0;

	selectMappedPart(); rh.selectMappedPart();

	unsigned err = 0;
	for(unsigned i = 0; i < vsize(); i++)
		err += labelErrors(vertex_bridges[i], rh.vertex_bridges[i], elabel_map.size());

	selectUnmappedPart(); rh.selectUnmappedPart();

	return err;
}

unsigned graph::edgeLabelErrors(unsigned v1, unsigned v2, graph& rh)
{
	if(vertex_efreq == NULL || rh.vertex_efreq == NULL) return 0;
	return labelErrors(vertex_efreq[v1], rh.vertex_efreq[v2], elabel_map.size());
}

unsigned graph::unmappedErrors(graph& rh)
{
	unsigned verr = labelErrors(vfreq, rh.vfreq, vlabel_map.size());
	unsigned eerr = labelErrors(efreq, rh.efreq, elabel_map.size());

	return verr + eerr;
}

void graph::print()
{
	cout << "# vertices: " << vsize() << endl << endl;

	cout << "vertex order: ";
	for(unsigned i = 0; i < src.vsize(); i++)
		cout << ordering[i] << " ";
	cout << endl;

	for(unsigned i = 0; i < vsize(); i++){
		cout << vlabel(i) << " " << flush;
	}
	cout << endl << endl << flush;

	cout << "   " << flush;
	for(unsigned i = 0; i < vsize(); i++)
		cout << i % 10 << " " << flush;
	cout << endl;

	for(unsigned i = 0; i < vsize(); i++){
		cout << i % 10 << ": " << flush;
		for(unsigned j = 0; j < vsize(); j++)
			cout << elabel(i, j) << " " << flush;
		cout << endl << flush;
	}
}
