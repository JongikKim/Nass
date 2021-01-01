#ifndef __QUEUE_H
#define __QUEUE_H

#include <deque>
#include <algorithm>

#include "nass.h"

class PriorityQueue
{
private:
	deque<mapping*> q;

	struct MLComparator
	{
		bool operator() (mapping* m1, mapping* m2)
		{
			int d1 = m1->distance();
			int d2 = m2->distance();

			if(d1 == d2) return m1->size <= m2->size;
			return d1 > d2;
		}
	};

public:
	PriorityQueue()
	{
		push_count = 0;
	}

	~PriorityQueue()
	{ 
		clear(); 
	}

	void push(mapping* m){
		q.push_back(m);

		if(q.size() == 1) make_heap(q.begin(), q.end(), MLComparator());
		else push_heap(q.begin(), q.end(), MLComparator());

		push_count++;
	}

	mapping* pop(){
		mapping* m = NULL;
		if(!q.empty()){
			pop_heap(q.begin(), q.end(), MLComparator());
			m = q.back(); q.pop_back();
		}
		return m;
	}

	bool empty(){ return q.empty(); }

	void clear(int wid = -1){
		for(unsigned i = 0; i < q.size(); i++) delete q[i];
		deque<mapping*> tmp;
		swap(q, tmp);

		if(wid != -1) PriorityQueue::total_size[wid] = 0;
	}

	unsigned size(){ return q.size(); }

	unsigned long long push_count;
	static long long* total_size;

	void printQueue(){
		for(int i = 0; i < NTHREADS; i++)
			cout << "T#" << i << " " << total_size[i] << ", ";
		cout << endl << flush;
	}

	void shrink_to_fit(){ q.shrink_to_fit(); }
};

#endif// __QUEUE_H
