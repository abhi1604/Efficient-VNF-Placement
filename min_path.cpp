#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<map>
#include<bits/stdc++.h>
#include<algorithm>
#include<chrono>
#include<time.h> 
#include"params.h"
#include"dijkstra.h"
#include"deploy.h"
#include"multi_stage.h"

using namespace std;
using namespace std::chrono;


vector<vector<struct LinkInfo>> graph;
vector<int> edge_nodes;
int nodes; // number of nodes to be present in the graph
vector<struct Node> nodeInfo;   // storing the information about the nodes in a global variable

bool criteria(struct Request request1, struct Request request2)
{
	struct Resources resource1, resource2;

	resource1.cpu = 0;
	resource2.cpu = 0;

	for(int i=0;i<request1.NF.size();++i)
		resource1.cpu += request1.NF[i].second.cpu;

	for(int i=0;i<request2.NF.size();++i)
		resource2.cpu += request2.NF[i].second.cpu;

	return (float(request2.throughput*1.0/resource2.cpu) > float(request1.throughput*1.0/resource1.cpu));
}

float algo1(vector<struct Request> requests)  // SPH
{
	vector<vector<struct LinkInfo>> local_graph(graph);
	vector<struct Node> local_nodes(nodeInfo);

	int total_throughput=0;
	int satisfied = 0;
	for(auto &request:requests)
	{
		struct path_info selected_path_info = dijkstra(request, local_graph);  // add local_nodes here as a paramter if path selection is to be done taking node capability into considerartion too
		// cout<<"--------------------------------------path size-------"<<selected_path_info.path.size()<<"----------------------"<<endl;
		if(!selected_path_info.path.empty())
		{
			int temp_satisfied;
			temp_satisfied = deployVNFS(request, selected_path_info.path, local_nodes, local_graph);
			if(temp_satisfied==1)
			{
				total_throughput += request.throughput;
				satisfied++;
			}
		}
	}

	cout<<"satisfied for SPH "<<satisfied<<" "<<requests.size()<<"  "<<endl;
	cout<<"Total throughput "<<total_throughput<<endl;
	cout<<"Total VNFs placed with SPH is "<<VNFS_FOR_SPH<<endl;
	stats(local_nodes);
	return float(1.0*satisfied)/requests.size();
}

float algo2(vector<struct Request> requests)
{
	vector<vector<struct LinkInfo>> local_graph(graph);
	vector<struct Node> local_nodes(nodeInfo);
	map<int, vector<int>> vnfNodes;   // for a vnf type, nodes that run it

	int total_throughput=0;
	int satisfied = 0;
	for(auto &request:requests)
	{
		vector<pair<int, int>> selected_path = multi_stage(request, local_graph, vnfNodes, local_nodes);  // add local_nodes here as a paramter if path selection is to be done taking node capability into considerartion too
		// cout<<"I am here!\n";
		if(!selected_path.empty())
		{
			int temp_satisfied;
			temp_satisfied = deployVNFSwithInterference(request, selected_path, local_nodes, local_graph, vnfNodes);
			if(temp_satisfied==1)
			{
				total_throughput += request.throughput;
				satisfied++;
			}
		}
		// cout<<"--------------------------------------path size-------"<<selected_path.size()<<"----------------------"<<endl;
	}
	cout<<"satisfied for algo "<<satisfied<<" "<<requests.size()<<"  "<<endl;
	cout<<"Total throughput "<<total_throughput<<endl;
	cout<<"Total VNFs placed with Algo is "<<VNFS_FOR_ALGO<<endl;
	stats(local_nodes);
	return float(1.0*satisfied)/requests.size();
}


float algo3(vector<struct Request> requests)  // SPH
{
	vector<vector<struct LinkInfo>> local_graph(graph);
	vector<struct Node> local_nodes(nodeInfo);

	int total_throughput=0;
	int satisfied = 0;
	for(auto &request:requests)
	{
		struct path_info selected_path_info = dijkstra(request, local_graph);  // add local_nodes here as a paramter if path selection is to be done taking node capability into considerartion too
		// cout<<"--------------------------------------path size-------"<<selected_path_info.path.size()<<"----------------------"<<endl;
		if(!selected_path_info.path.empty())
		{
			int temp_satisfied;
			temp_satisfied = deployVNFSforGUS(request, selected_path_info.path, local_nodes, local_graph);
			if(temp_satisfied==1)
			{
				total_throughput += request.throughput;
				satisfied++;
			}
		}
	}

	cout<<"satisfied for GUS "<<satisfied<<" "<<requests.size()<<"  "<<endl;
	cout<<"Total throughput "<<total_throughput<<endl;
	cout<<"Total VNFs placed with GUS is "<<VNFS_FOR_GUS<<endl;
	stats(local_nodes);
	return float(1.0*satisfied)/requests.size();
}

void serveRequests(vector<struct Request> requests)
{
 
	// start the time here for SPH
	auto start = high_resolution_clock::now(); 
	
	//algo1
	float tat1 = algo1(requests);
	cout<<"TAT with SPH is "<<tat1<<endl;

	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<milliseconds>(stop - start); 
	cout<<"Time taken for SPH is "<<duration.count()<<" ms\n";

	start = high_resolution_clock::now(); 
	sort(requests.begin(), requests.end(), criteria);
	// start time for cutom algo
	//algo2
	float tat2 = algo2(requests);
	cout<<"TAT with algo is "<<tat2<<endl;

	stop = high_resolution_clock::now(); 
	duration = duration_cast<milliseconds>(stop - start); 
	cout<<"Time taken for algo is "<<duration.count()<<" ms\n";


	// start time for gus
	start = high_resolution_clock::now(); 

	// gus
	float tat3 = algo3(requests);
	cout<<"TAT with GUS is "<<tat3<<endl;

	stop = high_resolution_clock::now(); 
	duration = duration_cast<milliseconds>(stop - start); 
	cout<<"Time taken for GUS is "<<duration.count()<<" ms\n";
}

int MAX_REQUESTS_FROM_FILE;

void processRequests()
{
	int onlyOnce=1;
	while(onlyOnce)
	{
		onlyOnce--;
		vector<struct Request> requests;
		// create random requests here
		for(int i=0;i<MAX_REQUESTS_FROM_FILE;++i)
		{
			// Generate and store request
			struct Request temp;
			int chain_length = /*rand()%*/CHAIN_LENGTH /*+ 1*/;
			temp.destination = rand()%nodes;
			// ensure that source!= destination
			do
			{
				temp.source = edge_nodes[rand()%edge_nodes.size()];
			} while(temp.source==temp.destination);
			float delay = ((REQUEST_MAX_DELAY - REQUEST_MIN_DELAY) * ((float)rand() / RAND_MAX)) + REQUEST_MIN_DELAY;
			temp.delay = delay;
			// temp.throughput = REQUEST_THROUGHPUT[rand()%REQUEST_THROUGHPUT.size()];
			temp.throughput = ((REQUEST_MAX_THROUGHPUT - REQUEST_MIN_THROUGHPUT) * ((float)rand() / RAND_MAX)) + REQUEST_MIN_THROUGHPUT;
			
			// Filling the request
			for(int j=0;j<chain_length;++j)
			{
				map<int, int> ids;
				int id = rand()%TYPES_AVAILABLE;
				struct Resources resources;
				resources.cpu = ((REQUEST_MAX_RESOURCES - REQUEST_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + REQUEST_MIN_RESOURCES;
				temp.NF.push_back(make_pair(id, resources));
			}
			requests.push_back(temp);
		}
	
		serveRequests(requests);
	}
}

int main(int argc, char *argv[])
{
	srand(time(0));
	MAX_REQUESTS_FROM_FILE = atoi(argv[1]);
	ifstream in("input.txt");
	if(in.is_open())
	{
		// Take Information about the nodes here
		in>>nodes;
		nodeInfo.resize(nodes);
		for(int i=0;i<nodes;++i)
		{
			int id, type;
			in>>id>>type;
			struct Node temp;
			temp.id = id;
			temp.node_type = type;
			if(type==EDGE_NODE)
			{
				temp.resources.cpu = EDGE_MIN_RESOURCES + rand() % (( EDGE_MAX_RESOURCES + 1 ) - EDGE_MIN_RESOURCES);
				temp.available_resources.cpu = temp.resources.cpu;
			}
				// temp.resources.cpu = rand()%(EDGE_RESOURCES/2) + ((EDGE_RESOURCES)/2) + 1;
			else
			{
				temp.resources.cpu = CORE_MIN_RESOURCES + rand() % (( CORE_MAX_RESOURCES + 1 ) - CORE_MIN_RESOURCES);
				temp.available_resources.cpu = temp.resources.cpu;
			}
			if(type==EDGE_NODE)
				edge_nodes.push_back(id);

			nodeInfo[id] = temp;
		}
		
		graph.resize(nodes);

		// Take Link Information here
		for(int i=0; i<nodes; ++i)
		{
			int edges, node1, node2, bandwidth, delay;
			in>>node1>>edges;
			for(int j=0;j<edges; ++j)
			{
				in>>node2;
				struct LinkInfo temp;
				int for_bandwidth = 1/*rand()%2*/;
				if(for_bandwidth==0)
				{
					temp.available_bandwidth = 100;
					temp.bandwidth = 100;
				}
				else
				{
					temp.bandwidth = 1000;
					temp.available_bandwidth =1000;
				}
				int e1=0, e2=0; // initally mark both node1, node2 as non edge
				// make more delay between edge and core
				if (find(edge_nodes.begin(), edge_nodes.end(), node1)!=edge_nodes.end())
				{
					e1=1;
				}
				if (find(edge_nodes.begin(), edge_nodes.end(), node2)!=edge_nodes.end())
				{
					e2=1;
				}
				float delay;
				if(e1==1&&e2==1) // both are edge nodes
				{
					delay = ((EDGE_EDGE_MAX_DELAY - EDGE_EDGE_MIN_DELAY) * ((float)rand() / RAND_MAX)) + EDGE_EDGE_MIN_DELAY; // within [1-5]  ms				
				}
				else if(e1==0&&e2==0)  // both are core nodes
				{
					delay = ((CORE_CORE_MAX_DELAY - CORE_CORE_MIN_DELAY) * ((float)rand() / RAND_MAX)) + CORE_CORE_MIN_DELAY; // within [10-15]  ms				
				}
				else   // one of them is edge 
				{
					delay = ((EDGE_CORE_MAX_DELAY - EDGE_CORE_MIN_DELAY) * ((float)rand() / RAND_MAX)) + EDGE_CORE_MIN_DELAY; // within [5-10]  ms				
				}
				temp.node1 = node1;
				temp.node2 = node2;
				temp.delay = delay;
				graph[node1].push_back(temp);
			}
		}
		processRequests();
	}

	else
	{
		cout<<"Could not read the Info about the nodes!\n";
	}

	in.close();
	return 0;
}