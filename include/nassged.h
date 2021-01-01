#ifndef __NASS_GED_H
#define __NASS_GED_H

#include "graph.h"
#include "mapping.h"
#include "dataset.h"
#include "queue.h"
#include "labelfreq.h"
#include "maptree.h"

#include "looptimer.h"
#include "nass.h"

#include <cstring>

#define INEXACT_ZERO -3000

class NassGED
{
private:
	graph* gx; //to graph - larger sized graph
	graph* gy; //from graph - smaller sized graph

	unsigned full_mapping_size; // size of a full mapping
	unsigned threshold; // NassGED threshold

	int wid; // current worker thread id

private:
	unsigned bitmap_size;

	maptree* mtree;
	void saveUnmappedOf(mapping* m);

private:
	static LabelFreq* freqx;
	static LabelFreq* freqy;

public:
	static bool interrupted;
	static int victim;

	static bool* wait;

public:
	static vertex_t* ordering;
	static vertex_t** orderingx;
	static vertex_t** orderingy;

public:
	static void initialize();
	static void finalize();

private:
	unsigned calculateDistance(mapping* m = NULL, mapping* pm = NULL);
	void expandState(mapping* pm, PriorityQueue& queue);
	bool complete(mapping* m) { return m->size == full_mapping_size; }

private:
	int branchFilter(bool debug = false);
	int compactBranchFilter(bool debug = false);

private:
	loop_timer* timer;

private:
	int victimSelection(mapping* m, PriorityQueue& queue);
	bool applyFilter(mapping* m);
public:
	NassGED(coregraph* x, coregraph* y, unsigned threshold, loop_timer* timer, int wid = 0);
	~NassGED();
	int computeGED();//unsigned gid = 0, unsigned* cur_gid = NULL);
};

#endif// __NASS_GED_H
