#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>

#include "nassged.h"
#include "inves.h"
#include "datafile.h"
#include "dataset.h"
#include "nass.h"
//#include "inves.h"

using namespace std;

void exit_with_usage()
{
	cerr << "usage: nass-index threshold data_file index_file [options]" << endl;
	cerr << "usage[client]: nass-index data_file server_addr" << endl;
	exit(1);
}

int main(int argc, char* argv[])
{
	if(argc < 3) exit_with_usage();

	bool distributed = false;
	for(unsigned i = 0; argv[1][i] != '\0'; i++)
		if(argv[1][i] >= '0' && argv[1][i] <= '9') continue;
		else{ distributed = true; break; }

	if(distributed && argc != 3) exit_with_usage();
	if(!distributed && argc < 4) exit_with_usage();

	char* server_address = NULL;
	char* data_file = NULL;
	char* index_file = NULL;
	if(distributed){ data_file = argv[1]; server_address = argv[2]; }
	else{ nass::threshold = atoi(argv[1]); data_file = argv[2]; index_file = argv[3]; }

	int sampling_rate = 100;
	for(int i = 4; i < argc; i += 2){
		if(strcmp(argv[i], "-M") == 0)
			MEMLIMIT = atoi(argv[i+1]);
		else if(strcmp(argv[i], "-p") == 0)
			NTHREADS = atoi(argv[i+1]);
		else if(strcmp(argv[i], "-s") == 0)
			sampling_rate = atoi(argv[i+1]);
		else if(strcmp(argv[i], "--coordinator") == 0){
			distributed = true;
			i -= 1;
		}
		else{
			cerr << "Unknown option: " << argv[i] << endl;
			cerr << "usage: nass-index threshold data_file index_file [options]" << endl;
			exit(1);
		}
	}

	DataFile* file = new SyntheticFile(data_file);
	DataSet* dataset = DataSet::getInstance();
	dataset->buildDataSet(*file);
	file->close();
	delete file;

	nass builder(index_file, false);
	builder.buildNassIndex(distributed, server_address, sampling_rate);

	// all graph (wrapper) objects should be destroyed before calling finishUp()
	DataSet::finishUp();

	return 0;
}
