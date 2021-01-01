#ifndef __DATASET_H
#define __DATASET_H

#include "coregraph.h"
#include "datafile.h"

#include <vector>
#include <map>
#include <iostream>

using namespace std;

class DataSet
{
private:
	vector<coregraph*> data;
    unsigned max_vertices;

private:
	static DataSet* instance;

private:
	DataSet(){}
	~DataSet()
	{
		// delete data graphs
		for(unsigned i = 0; i < data.size(); i++) delete data[i];
	}

public:
	static DataSet* getInstance()
	{
		if(instance == NULL) instance = new DataSet();
		return instance;
	}

	static void finishUp() { delete instance; }

	void buildDataSet(DataFile& file, bool verbose = false)
	{
		unsigned long vsize_total = 0;
		unsigned long esize_total = 0;

		unsigned e_min = 100;
		unsigned v_min = 100;

		unsigned e_max = 0;
		unsigned v_max = 0;

		cout << "Reading dataset .. " << flush;

		coregraph* g = NULL;
		for(int id = 0; (g=file.getNextGraph()) != NULL; id++){
			data.push_back(g);

			vsize_total += g->vsize();
			esize_total += g->esize();

			if(g->vsize() > v_max) v_max = g->vsize();
			if(g->vsize() < v_min) v_min = g->vsize();
			if(g->esize() > e_max) e_max = g->esize();
			if(g->esize() < e_min) e_min = g->esize();
		}

		cout << "Done (" << data.size() << " graphs)" << endl << endl;
       // cout << "Building statistics (label multisets and frequencies) .. ";
        for(unsigned i = 0; i < data.size(); i++) data[i]->labelScan();
       // cout << " Done" << endl << endl << flush;

		// to restore original label strings for printing result graph
		vertex_label.resize(vlabel_map.size()+1);
		map<string, int>::iterator iter;
		for(iter = vlabel_map.begin(); iter != vlabel_map.end(); iter++)
			vertex_label[iter->second] = iter->first;

		edge_label.resize(elabel_map.size()+1);
		for(iter = elabel_map.begin(); iter != elabel_map.end(); iter++)
			edge_label[iter->second] = iter->first;

        max_vertices = v_max;

		if(verbose){
			double vavg = (double)vsize_total/data.size();
			double eavg = (double)esize_total/data.size();

			double vdev = 0, edev = 0;	
			for(unsigned i = 0; i < data.size(); i++){
				double var = (data[i]->vsize() - vavg);
				vdev += (var * var);
				var = (data[i]->esize() - eavg);
				edev += (var * var);
			}

			vdev /= data.size();
			edev /= data.size();


			cout << "vdev: " << vdev << endl;
			cout << "edev: " << edev << endl;

			cout << "v_max: " << v_max << " v_min: " << v_min << endl;
			cout << "e_max: " << e_max << " e_min: " << e_min << endl;

			cout << "vlabel_map size: " << vlabel_map.size() << endl;
			cout << "elabel_map size: " << elabel_map.size() << endl;
		}
	}

	unsigned numGraphs() { return data.size(); }
    unsigned vmax() { return max_vertices; }
	coregraph* graphAt(int i){ return data[i]; }
};

#endif
