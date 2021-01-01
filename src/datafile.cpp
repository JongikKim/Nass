#include "datafile.h"
#include <iostream>
#include <cstdlib>

#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_LINE 4096
SyntheticFile::SyntheticFile(char* filename)
{
	synthetic.open(filename);
	if(!synthetic.is_open()){
		cerr << "Failed to open file: " << filename << endl << flush;
		exit(1);
	}

	// eat the first line
	char line[MAX_LINE];
	synthetic.getline(line, MAX_LINE);
}

SyntheticFile::~SyntheticFile()
{
	if(synthetic.is_open()) synthetic.close();
}

void SyntheticFile::close()
{
	if(synthetic.is_open()) synthetic.close();
}

coregraph* SyntheticFile::getNextGraph()
{
	if(!synthetic.good()) return NULL;

	coregraph* graph = parseSynthetic();
	return graph;
}

coregraph* SyntheticFile::parseSynthetic()
{
	char line[MAX_LINE];

	vector<string> vertices;
	synthetic.getline(line, MAX_LINE); 
	while(line[0] == 'v'){
		int i = 2;
		while(line[i++] != ' ');

		vertices.push_back(&line[i]);
		synthetic.getline(line, MAX_LINE); 
	}

	vector<string> elabels;
	vector<pair<int,int> > edges;
	while(line[0] == 'e'){
		int i = 2;
		int j = i;
		while(line[j] != ' ') j++;
		line[j] = '\0';

		int v1 = atoi(&line[i]);

		i = j+1;
		while(line[j] != ' ') j++;
		line[j] = '\0';

		int v2 = atoi(&line[i]);

		i = j+1;
		elabels.push_back(&line[i]);
		edges.push_back(pair<int,int>(v1, v2));
		synthetic.getline(line, MAX_LINE); 
	}

	unsigned vertexSize = vertices.size();
	unsigned edgeSize = edges.size();

	coregraph* g = new coregraph(vertexSize);
	for(unsigned i = 0; i < vertexSize; i++)
		g->setVertexLabel(i, vertices[i]);

	for(unsigned i = 0; i < edgeSize; i++){
		unsigned vertex1 = unsigned(edges[i].first);
		unsigned vertex2 = unsigned(edges[i].second);

		if(vertex1 > vertexSize || vertex2 > vertexSize){ delete g; return NULL; }
		g->setEdgeLabel(vertex1, vertex2, elabels[i]);
	}

	return g;
}

MolFile::MolFile(char* filename)
{
	mol.open(filename);
	if(!mol.is_open()){
		cerr << "Failed to open file: " << filename << endl << flush;
		exit(1);
	}
}

MolFile::~MolFile()
{
	if(mol.is_open()) mol.close();
}

void MolFile::close()
{
	if(mol.is_open()) mol.close();
}

coregraph* MolFile::getNextGraph()
{
	if(!mol.good()) return NULL;

	coregraph* graph = parseMol();

	string line;
	while(getline(mol, line)){
		if(!mol.good()) break;
		if(line == "$$$$") break;
	}

	return graph;
}

string MolFile::trim(string str)
{
	size_t first = str.find_first_not_of(' ');
	size_t last = str.find_last_not_of(' ');

	return str.substr(first, last-first+1);
}

coregraph* MolFile::parseMol()
{
	string line;
	// skip first three lines
	getline(mol, line); 
	if(!mol.good()) return NULL;
	getline(mol, line); getline(mol, line);

	getline(mol, line);
	unsigned vertexSize = unsigned(stoi(line.substr(0, 3))); // stoi trims string
	unsigned edgeSize = unsigned(stoi(line.substr(3, 3))); // stoi trims string

	coregraph* g = new coregraph(unsigned(vertexSize));
	for(unsigned i = 0; i < vertexSize; i++){
		getline(mol, line);
		g->setVertexLabel(i, trim(line.substr(31, 3)));
	}

	for(unsigned i = 0; i < edgeSize; i++){
		getline(mol, line);
		unsigned vertex1 = unsigned(stoi(line.substr(0, 3))); // stoi trims string
		unsigned vertex2 = unsigned(stoi(line.substr(3, 3))); // stoi trims string
		int numeric_edge = stoi(line.substr(6, 3));

		if(vertex1 > vertexSize || vertex2 > vertexSize){ delete g; return NULL; }
		g->setEdgeBond(vertex1 - 1, vertex2 - 1, numeric_edge);
	}

	return g;
}
