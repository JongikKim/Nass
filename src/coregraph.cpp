#include <map>
#include <sstream>
#include <cstring>

#include "coregraph.h"
#include "graph.h"

coregraph::coregraph(unsigned n)
{
	num_vertices = n;	
	num_edges = 0;

	vlabels = new int[num_vertices];
	for(unsigned i = 0; i < num_vertices; i++) vlabels[i] = 0;

	elabels = new int*[num_vertices];
	for(unsigned i = 0; i < num_vertices; i++) elabels[i] = new int[num_vertices];

	for(unsigned i = 0; i < num_vertices; i++)
		for(unsigned j = 0; j < num_vertices; j++)  elabels[i][j] = 0;

    vfreq = efreq = NULL;

    vertex_efreq = new unsigned*[num_vertices];
    for(unsigned i = 0; i < num_vertices; i++) vertex_efreq[i] = NULL;

	adjacent = NULL;
	vlabel_index = NULL;
}

coregraph::~coregraph()
{
	delete [] vlabels;
	for(unsigned i = 0; i < num_vertices; i++) delete [] elabels[i];
	delete [] elabels;

    delete [] vfreq;
    delete [] efreq;

    for(unsigned i = 0; i < num_vertices; i++) delete [] vertex_efreq[i];
    delete [] vertex_efreq;

	delete [] adjacent;
	delete [] vlabel_index;
}

void coregraph::setVertexLabel(unsigned v, string label)
{
	map<string,int>::iterator iter = vlabel_map.find(label);

	int numeric_label = (int)vlabel_map.size()+1; // numeric_label cannot be zero
	if(iter == vlabel_map.end()) vlabel_map.insert(pair<string,int>(label, numeric_label));
	else numeric_label = iter->second;

	vlabels[v] = numeric_label;
}

void coregraph::setEdgeLabel(unsigned v1, unsigned v2, string label)
{
	map<string,int>::iterator iter = elabel_map.find(label);

	int numeric_label = (int)elabel_map.size()+1; // numeric_label cannot be zero!!! zero means no edge
	if(iter == elabel_map.end()) elabel_map.insert(pair<string,int>(label, numeric_label));
	else numeric_label = iter->second;

	num_edges++;
	elabels[v1][v2] = elabels[v2][v1] = numeric_label;
}

void coregraph::setEdgeBond(unsigned v1, unsigned v2, int bond)
{
	// bond cannot be zero
	elabels[v1][v2] = elabels[v2][v1] = bond;
	num_edges++;

	// unnecessary (for counting total distinct edge labels)
	stringstream label;
	label << bond;
	map<string,int>::iterator iter = elabel_map.find(label.str());

	int numeric_label = (int)elabel_map.size()+1;
	if(iter == elabel_map.end()) elabel_map.insert(pair<string,int>(label.str(), numeric_label));
}

void coregraph::labelRescan(unsigned sz_v, unsigned sz_e)
{
	if(sz_v != vlabel_map.size()){
		unsigned* tmp = new unsigned[vlabel_map.size()];
		memset(tmp, 0, sizeof(unsigned) * vlabel_map.size());
		memcpy(tmp, vfreq, sizeof(unsigned) * sz_v);
		delete [] vfreq; vfreq = tmp;

		vector<unsigned>* vtmp = new vector<unsigned>[vlabel_map.size()];
		for(unsigned i = 0; i < sz_v; i++)
			std::swap(vtmp[i], vlabel_index[i]);
		delete [] vlabel_index; vlabel_index = vtmp;
	}

	if(sz_e != elabel_map.size()){
		unsigned* tmp = new unsigned[elabel_map.size()];
		memset(tmp, 0, sizeof(unsigned) * elabel_map.size());
		memcpy(tmp, efreq, sizeof(unsigned) * sz_e);

		delete [] efreq; efreq = tmp;

		for(unsigned i = 0; i < vsize(); i++){
			unsigned* etmp = new unsigned[elabel_map.size() + 1];
			memset(etmp, 0, sizeof(unsigned) * (elabel_map.size() + 1));
			memcpy(etmp, vertex_efreq[i], sizeof(unsigned) * sz_e);
			etmp[elabel_map.size()] = vertex_efreq[i][sz_e];

			delete [] vertex_efreq[i]; vertex_efreq[i] = etmp;
		}
	}
}

void coregraph::labelScan()
{
	vfreq = new unsigned[vlabel_map.size()];
	efreq = new unsigned[elabel_map.size()];

	vlabel_index = new vector<unsigned>[vlabel_map.size()];

	memset(vfreq, 0, sizeof(unsigned) * vlabel_map.size());
	memset(efreq, 0, sizeof(unsigned) * elabel_map.size());

	for(unsigned i = 0; i < vsize(); i++){
		vfreq[vlabel(i)-1]++;
		vlabel_index[vlabel(i)-1].push_back(i);
	}

	for(unsigned i = 0; i < vsize(); i++){
		for(unsigned j = i + 1; j < vsize(); j++)
			if(elabel(i, j)) efreq[elabel(i, j)-1]++; 

		vertex_efreq[i] = new unsigned[elabel_map.size() + 1];
		memset(vertex_efreq[i], 0, sizeof(unsigned) * (elabel_map.size() + 1));
		for(unsigned j = 0; j < vsize(); j++)
			if(elabel(i, j)) vertex_efreq[i][elabel(i, j)-1]++;

        // calculate vertex degree and save it in vertex_size[i][elabel_map.size()]
        for(unsigned j = 0; j < elabel_map.size(); j++)
			vertex_efreq[i][elabel_map.size()] += vertex_efreq[i][j];		
	}

	adjacent = new vector<short>[vsize()];
	for(int u = 0; u < (int)vsize(); u++)
		for(int v = 0; v < (int)vsize(); v++)
			if(elabel(u, v)) adjacent[u].push_back(v);
}

unsigned coregraph::sizeFilter(coregraph* rh)
{
    int vdiff = int(vsize()) - int(rh->vsize());
    int ediff = int(esize()) - int(rh->esize());

    if(vdiff < 0) vdiff = -1*vdiff;
    if(ediff < 0) ediff = -1*ediff;

    return unsigned(vdiff + ediff);
}

unsigned coregraph::labelFilter(coregraph* rh)
{
    unsigned verr = graph::labelErrors(vfreq, rh->vfreq, vlabel_map.size());
    unsigned eerr = graph::labelErrors(efreq, rh->efreq, elabel_map.size());

    return verr + eerr;
}

void coregraph::output(ostream& out, int number){
	out << "t # " << number << endl;
	for(unsigned i = 0; i < num_vertices; i++)
		out << "v " << i << " " << vertex_label[vlabels[i]] << endl;

	for(unsigned i = 0; i < num_vertices; i++)
		for(unsigned j = i+1; j < num_vertices; j++)
			if(elabels[i][j])
				out << "e " << i << " " << j << " " << edge_label[elabels[i][j]] << endl;
}
