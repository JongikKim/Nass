#ifndef __MAPPING_H
#define __MAPPING_H

#include <iostream>
#include <cstring>
#include "coregraph.h"
#include "inves.h"

enum ESTIMATION {LF, BF, PF, NONE};
struct unmappedDistance
{
	unsigned short dist[3];
	ESTIMATION method;
	Inves* partition;

	unmappedDistance(){ 
		dist[LF] = dist[BF] = dist[PF] = 0;
		method = NONE;
		partition = NULL;
	}

	unsigned short distance(){
		unsigned short max = dist[LF];

		if(dist[BF] > max) max = dist[BF];
		if(dist[PF] > max) max = dist[PF];

		return max;
	}

	~unmappedDistance(){ delete partition; }
};

struct mapping
{
//	static unsigned bitmap_size;

	vertex_t* ordering; // vertex ordering
	unsigned short size; // size of mapping, which is a border between mapped and unmapped vertices

	unsigned short mdist; // mapped distance
	unsigned short bdist; // bridge distance
	unsigned short udist; // unmapped distance

	unmappedDistance* unmapped; 

	unsigned long long* bitmap; // bitmap for the mapping: mapped vertices = 1 and unmapped vertices = 0;

	mapping(unsigned sz, unsigned bitmap_size, bool init = false)
	{ 
		ordering = new vertex_t[sz]; 
		bitmap = new unsigned long long[bitmap_size];

		if(init){
			size = 0;
			for(unsigned i = 0; i < sz; i++) ordering[i] = i;
			memset(bitmap, 0, sizeof(unsigned long long)*bitmap_size);
		}

		mdist = bdist = udist = 0;
		unmapped = NULL;
	}
	~mapping()
	{
		delete [] ordering;
		delete [] bitmap; 
	}
	unsigned distance() { return mdist + bdist + udist; }

	void setbit(unsigned v){
		unsigned idx = v / 64;
		unsigned pos = v % 64;
		unsigned long long base = 1;

		bitmap[idx] |= (base << pos);
	}

	void print(){
		cout << "MDIST: " << mdist << ", BDIST: " << bdist << ", UDIST: " << udist << endl;
		for(unsigned i = 0; i < size; i++)
			cout << ordering[i] << " -> " << i << endl;
		cout << endl;
	}
};

#endif //__MAPPING_H
