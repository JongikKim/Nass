#ifndef __MAPPING_INDEX__
#define __MAPPING_INDEX__

#include <list>
#include <map>

#include "mapping.h"

typedef unsigned long long uint64;

struct mappingCompare
{
	unsigned bitmap_size;

	mappingCompare(unsigned bsize) : bitmap_size (bsize){}
	bool operator () (const uint64* m1, const uint64* m2) const{
		for(unsigned i = 0; i < bitmap_size; i++)
			if(m1[i] != m2[i]) return m1[i] < m2[i];

		return false;
	}
}; 


typedef	map<uint64*, unmappedDistance*, mappingCompare> maptree;
typedef maptree::iterator map_iter;

#endif
