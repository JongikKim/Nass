#include <cstdio>

#include "nassged.h"
#include "graph.h"

struct branchCompare
{
private:
	graph* g;

public:
	branchCompare(graph* g) : g(g){}
	bool operator()(const int& v1, const int& v2)
	{
		if(g->vlabel(v1) == g->vlabel(v2)){
			unsigned* efreq1 = g->vertex_efreq[v1];
			unsigned* efreq2 = g->vertex_efreq[v2];
			for(unsigned i = 0; i < elabel_map.size(); i++){
				if(efreq1[i] == efreq2[i]) continue;
				return efreq1[i] > efreq2[i];
			}
		}

		return g->vlabel(v1) < g->vlabel(v2);
	}
};

int cmpFreq(unsigned* efreq1, unsigned* efreq2)
{
	for(unsigned i = 0; i < elabel_map.size(); i++)
		if(efreq1[i] != efreq2[i]){
			if(efreq1[i] > efreq2[i]) return -1;
			else return 1;
		}

	return 0;
}

int NassGED::compactBranchFilter(bool debug)
{
	int bx[gx->vsize()]; // branch for x
	int by[gy->vsize()]; // branch for y

	for(int i = 0; i < (int)gx->vsize(); i++) bx[i] = i;
	for(int i = 0; i < (int)gy->vsize(); i++) by[i] = i;

	if(debug){
		gx->print(); gy->print();
	}

	std::sort(bx, bx + gx->vsize(), branchCompare(gx));
	std::sort(by, by + gy->vsize(), branchCompare(gy));

	if(debug){
		cout << "X sorted: ";
		for(unsigned i = 0; i < gx->vsize(); i++)
			cout << bx[i] << " ";
		cout << endl;

		cout << "Y sorted: ";
		for(unsigned i = 0; i < gy->vsize(); i++)
			cout << by[i] << " ";
		cout << endl;
	}

	const int REMOVED = -1;
	unsigned ptr1 = 0, ptr2 = 0;
	while(ptr1 < gx->vsize() && ptr2 < gy->vsize()){
		//if(debug) cout << bx[ptr1] << " vs. " << by[ptr2] << endl;

		if(gx->vlabel(bx[ptr1]) == gy->vlabel(by[ptr2])){
			int compare = cmpFreq(gx->vertex_efreq[bx[ptr1]], gy->vertex_efreq[by[ptr2]]);

			if(debug){
				cout << bx[ptr1] << " vs. " << by[ptr2] << endl;
				cout << "compare: " << compare << endl;

				for(int a = 0; a < 3; a++){
					cout << gx->vertex_efreq[bx[ptr1]][a] << " vs. " << gy->vertex_efreq[by[ptr2]][a] << endl;
				}

				cout << endl << endl;
			}
			if(compare < 0) ptr1++;
			else if(compare > 0) ptr2++;
			else{ bx[ptr1++] = REMOVED; by[ptr2++] = REMOVED; }
		}
		else if(gx->vlabel(bx[ptr1]) < gy->vlabel(by[ptr2])) ptr1++;
		else ptr2++;
	}

	if(debug){
		cout << "X removed: ";
		for(unsigned i = 0; i < gx->vsize(); i++)
			cout << bx[i] << " ";
		cout << endl;

		cout << "Y removed: ";
		for(unsigned i = 0; i < gy->vsize(); i++)
			cout << by[i] << " ";
		cout << endl;
	}

	ptr1 = ptr2 = 0;

	int distance = 0;
	while(ptr1 < gx->vsize() && ptr2 < gy->vsize()){
		if(bx[ptr1] == REMOVED){ ptr1++; continue; }
		if(by[ptr2] == REMOVED){ ptr2++; continue; }

		if(gx->vlabel(bx[ptr1]) == gy->vlabel(by[ptr2])){ 
			distance += 5; 
			bx[ptr1++] = REMOVED;
			by[ptr2++] = REMOVED;
		}
		else if(gx->vlabel(bx[ptr1]) < gy->vlabel(by[ptr2])) ptr1++;
		else ptr2++;
	}

	for(unsigned i = 0; i < gx->vsize(); i++)
		if(bx[i] != REMOVED) distance += 10;

	if(gx->vsize() < gy->vsize())
		distance += (gy->vsize() - gx->vsize()) * 10;

	return distance;
}
