#include "dataset.h"

DataSet* DataSet::instance = NULL;
map<string, int> vlabel_map;
map<string, int> elabel_map;

vector<string> vertex_label;
vector<string> edge_label;
