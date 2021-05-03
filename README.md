# Nass

This code accompaines the following paper:

Jongik Kim, "Boosting Graph Similarity Search through Pre-computation", SIGMOD 2021 (a preliminary version is available online at [arxiv:2004.01124](http://arxiv.org/abs/2004.01124)).

Sample data files and index files are included in the "data" directory. 
Please uncompress the data and index files before using them (e.g., gzip -d *.gz).
The included AIDS and PubChem indices have been constructed with thresholds 9 and 8, respectively.
The code has been complied and tested in Ubuntu and MacOS.

# How to build binaries (both nass and nass-index)
> make

# How to run (search)
> nass GED_threshold data_file [index_file]

index_file is optional. If index_file is not specified, it directly verify each candidate graph through GED computation (after applying the label filtering).

For example, the following command runs Nass on the AIDS dataset for the threshold 3 using the index AIDS.idx

> nass 3 data/AIDS data/AIDS.idx

Nass randomly selects 100 queries from the dataset and performs graph similarity searches for the specified threshold. Please note that for each query, Nass excludes the query graph in the dataset while processing similarity search (refer to the following if statement in nass::search()). 

if(x == y) continue; (in Line 135 of src/nass.cpp)

Hence, the query graph itself will not be contained in the results of the query. To include the query graph in the results, you can simply comment out the line above. If an index is used in this case, however, Nass will be extremely fast. This is because Nass immediately identifies all results as soon as it meets the query graph (please try it to see how fast Nass will be).

Nass also supports a custom workload from a file. To use a custom workload file, please see the comment inlined in src/searchmain/main.cpp, and refer to Workload::readWorkload(char* filename) function in include/workload.h.

Nass does not output or save result graphs in a file. The results are saved in nass::res_vec. To output query results, please take a look at the comments inlined in nass::run() in src/nass.cpp (by simply uncommenting them, you can output query results).

# How to construct an index
> nass-index index_threshold data_file index_file [options]

nass-index has the following options.<br>
-M memory limit in MB (default 1000, i.e., 1GB)<br>
-p number of threads (default 8)<br>
--coordinator (to play a role of a coordinator for distributed index building, see below)<br>
-s sampling_rate (experimental, see the description at the end) <br>

For example, the following command constructs an index of the AIDS dataset for threshold 5 with 8 threads and 2GB memory limit, and save the index into the file AIDS.2GB.idx.
> nass-index 5 data/AIDS AIDS.2GB.idx -p 8 -M 2000

We include indices AIDS.idx and CHEM.idx in this project. The index file AIDS.idx (CHEM.idx) is dependent on the datafile AIDS (CHEM). If the datafile is edited, the index is not valid any more.

# Distributed index construction
Nass also supports distributed index building. For distributed index building, the command for a client is as follows.

> nass-index datafile address_of_coordinator

Do not specify any option for a client node. A client node receives necessary options from the coordinator. 

For example, the following command construct an index of the AIDS dataset for threshold 5 using 3 distributed nodes (24 thresads in total), and save the index into the file AIDS.2GB.idx in the coordinator. We assume the ip address of the coordinator is 123.456.789.111
> nass-index 5 data/AIDS AIDS.2GB.idx -p 8 -M 2000 --coordinator (coordinator) <br>
> nass-index data/AIDS 123.456.789.111 (client 1) <br>
> nass-index data/AIDS 123.456.789.111 (client 2)

A client can participate in the index construction at any time. Our implementation is not fault-tolerant. Once a client has participated, therefore, it should not be terminated until the index construction is completed. Otherwise, the coordinator will wait forever for the results of a graph assigned to the terminated client.

# Sampling for indexing (experimental)
We have an experimental sampling option in building an index. Our sampling option is not for constructing a sparse index. Given a threshold index_threshold for an index, we build index entries with index_threshold only for the sampled graphs. For remaining graphs, we construct index entries with index_threshold - 1. We assume that a graph having similar graphs is more likely to be queried. Based on the assumption, we sample graphs having similar graphs within a very low GED threshold.

To use sampling, we first build an index with a very low threshold. Nass utilizes the index to identify and sample graphs having similar graphs. For example, the following commands construct an index with 30% sampling rate.

> nass-index 2 data/AIDS AIDS.sample.idx <br>
><br>
> nass-index 6 data/AIDS AIDS.sample.idx -s 30 -p 8 -M 2000 --coordinator <br>
> nass-index data/AIDS 123.456.789.111

The first command makes an index for sampling data graphs, which should be run very fast. Then the second command uses the index for sampling and construct an index with 30% sampling rate. Note that the index constructed from the second command overwrites AIDS.sample.idx. The thrid command launches a client for distributed index building. For a client node, we don't need any options except the data file and the address of the coordinator.

