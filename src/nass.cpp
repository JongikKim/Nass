#include <sstream>

#include "nass.h"
#include "nassged.h"

//statistics
long long cands_final;
long long push_count;

int nass::threshold = 0;

nass::nass(const char* filename, bool load_index)
{
	this->indexfile = filename;

	this->use_index = false;
	this->nassIndex = NULL;
	this->sampled = NULL;
	this->index_max_threshold = 0;

	if(load_index && indexfile && loadNassIndex())
		use_index = true;

	clearStats();
}

nass::~nass()
{
	delete [] nassIndex;
	delete [] sampled;
}


void nass::clearStats()
{
	cands_final = 0;
	push_count = 0;
	res_vec.clear();
}

void nass::run(Workload& workload)
{
	clearStats();

	timer = new loop_timer("Nass", "", workload.size(), NULL, 1); 
	timer->start();

	NassGED::initialize();
	for(unsigned i = 0; i < workload.size(); i++){
		//unsigned res_begin = res_vec.size(); // TO ACCESS iTH QUERY RESULTS

		search(workload[i]);

		/******** ACCESS QUERY RESULTS AS FOLLOWS *********
		cout << "Result for the " << i << "th query" << endl;
		DataSet* dataset = DataSet::getInstance();
		for(unsigned k = res_begin; k < res_vec.size(); k++){
			coregraph* g = dataset->graphAt(res_vec[k]);
			g->output(cout, k - res_begin);	
		}
		****************************************************/

		timer->next();
	}
	NassGED::finalize();

	timer->stop();
	long long elapsed = timer->end(true);
	cout << "===================== Results =====================" << endl;
	cout << "GED threshold: " << threshold << endl;
	cout << "# queries: " << workload.size() << endl;
	cout << "# total candidates: " << cands_final << endl;
	cout << "# total mappings: " << push_count << endl;
	cout << "# total results: " << res_vec.size() << endl;
	cout << "Processing time: ";
    fprintf(stdout, "%lld.%03lld seconds\n", elapsed/1000000, (elapsed % 1000000)/1000); 
	cout << "===================================================" << endl;

	delete timer;
}

bool gtid(const pair<int, int>& c1, const pair<int, int>& c2)
{
	return c1.first < c2.first;
}

bool gtlb(const pair<int, int>& c1, const pair<int, int>& c2)
{
	return c1.second < c2.second;
}

unsigned nass::regenCandidates(vector<pair<int, int> >& candidates, unsigned pos, int res, int distance)
{
	int max_threshold = index_max_threshold;
	if(!sampled[res]) max_threshold -= 1;

	res_vec.push_back(res);
	if(distance + threshold > max_threshold) return pos;

	std::sort(candidates.begin() + pos, candidates.end(), gtid);

	vector<pair<int,int> > tmp;
	unsigned ptr1 = pos, ptr2 = 0;

	while(ptr1 < candidates.size() && ptr2 < nassIndex[res].size()){
		int gdist = nassIndex[res][ptr2].second;
		bool inexact = (gdist < 0);
		if(gdist == INEXACT_ZERO) gdist = 0;
		if(inexact) gdist = gdist * -1;

		if(gdist > threshold + distance) ptr2++;
		else if(nassIndex[res][ptr2].first < candidates[ptr1].first) ptr2++; 
		else if(nassIndex[res][ptr2].first > candidates[ptr1].first) ptr1++;
		else{
			if(gdist + distance <= threshold && !inexact)
				res_vec.push_back(nassIndex[res][ptr2].first);
			else tmp.push_back(candidates[ptr1]);

			ptr1++; ptr2++;
		}
	}
	std::swap(candidates, tmp);

	std::sort(candidates.begin(), candidates.end(), gtlb);

	return 0;
}

void nass::basicFilter(coregraph* x, vector<pair<int, int> >& candidates, int threshold, int begin)
{
	DataSet* dataset = DataSet::getInstance();

	for(unsigned i = begin; i < dataset->numGraphs(); i++){
		coregraph* y = dataset->graphAt(i);
		if(x == y) continue; // skip the same graph
		if(x->sizeFilter(y) > unsigned(threshold)) continue;

		int lb = x->labelFilter(y); //gx.unmappedErrors(gy);
		if(lb <= threshold)
			candidates.push_back(pair<int, int>(i, lb));
	}
}

#include <unistd.h>
void nass::search(coregraph* x)
{
	DataSet* dataset = DataSet::getInstance();

	vector<pair<int, int> > candidates;
	basicFilter(x, candidates, threshold);

	if(use_index) std::sort(candidates.begin(), candidates.end(), gtlb);

	unsigned i = 0;
	while(i < candidates.size()){
		coregraph* y = dataset->graphAt(candidates[i].first);

		NassGED* ged = new NassGED(x, y, threshold, timer);
		int distance = threshold + 1;
		distance = ged->computeGED();
		delete ged;
		
		if(distance <= threshold){
			if(use_index) i = regenCandidates(candidates, i+1, candidates[i].first, distance);
			else res_vec.push_back(candidates[i++].first);
		}
		else i++;
	}
}
