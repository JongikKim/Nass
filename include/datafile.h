#ifndef __GRAPHFILE_H
#define __GRAPHFILE_H

#include "coregraph.h"
#include <string>
#include <fstream>
#include <vector>

using namespace std;

class DataFile
{
public:
	virtual ~DataFile(){}
	virtual coregraph* getNextGraph() = 0;
	virtual void close(){}
};

// for reading synthetic graphs generated from GraphGen
// we use this file format as our default format
class SyntheticFile : public DataFile
{
private:
	ifstream synthetic;

public:
	SyntheticFile(char* filename);
	~SyntheticFile();
	
	void close();

public:
	coregraph* getNextGraph();

private:
	coregraph* parseSynthetic();
};

class MolFile : public DataFile
{
private:
	ifstream mol;

public:
	MolFile(char* filename);
	~MolFile();
	void close();

public:
	coregraph* getNextGraph();

private:
	string trim(string str);
	coregraph* parseMol();
};
#endif //__GRAPHFILE_H
