#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<map>
#include<bits/stdc++.h>
#include<algorithm>
#include"params.h"
#include"dijkstra.h"
#include"deploy.h"
#include"multi_stage.h"
using namespace std;

vector<vector<struct LinkInfo>> graph;
vector<int> edge_nodes;
int nodes; // number of nodes to be present in the graph
vector<struct Node> nodeInfo;   // storing the information about the nodes in a global variable

int MAX_NODES = 100;

bool criteria(struct Request request1, struct Request request2)
{
	struct Resources resource1, resource2;

	resource1.cpu = 0;
	resource2.cpu = 0;

	for(int i=0;i<request1.NF.size();++i)
		resource1.cpu += request1.NF[i].second.cpu;

	for(int i=0;i<request2.NF.size();++i)
		resource2.cpu += request2.NF[i].second.cpu;

	return (float(request1.throughput/resource1.cpu) > float(request2.throughput/resource2.cpu));
}

float algo1(vector<struct Request> requests)
{
	vector<vector<struct LinkInfo>> local_graph(graph);
	vector<struct Node> local_nodes(nodeInfo);

	int satisfied = 0;
	for(auto &request:requests)
	{
		struct path_info selected_path_info = dijkstra(request, local_graph);  // add local_nodes here as a paramter if path selection is to be done taking node capability into considerartion too
		if(!selected_path_info.path.empty())
			satisfied += deployVNFS(request, selected_path_info.path, local_nodes, local_graph);
		cout<<"--------------------------------------path size-------"<<selected_path_info.path.size()<<"----------------------"<<endl;
	}
	cout<<"satisfied "<<satisfied<<" "<<requests.size()<<"  "<<endl;
	return float(1.0*satisfied)/requests.size();
}

float algo2(vector<struct Request> requests)
{
	vector<vector<struct LinkInfo>> local_graph(graph);
	vector<struct Node> local_nodes(nodeInfo);
	map<int, vector<int>> vnfNodes;   // for a vnf type, nodes that run it

	int satisfied = 0;
	for(auto &request:requests)
	{
		vector<pair<int, int>> selected_path = multi_stage(request, local_graph, vnfNodes, local_nodes);  // add local_nodes here as a paramter if path selection is to be done taking node capability into considerartion too
		if(!selected_path.empty())
			satisfied += deployVNFSwithInterference(request, selected_path, local_nodes, local_graph);
		cout<<"--------------------------------------path size-------"<<selected_path.size()<<"----------------------"<<endl;
	}
	cout<<"satisfied "<<satisfied<<" "<<requests.size()<<"  "<<endl;
	return float(1.0*satisfied)/requests.size();
}

void serveRequests(vector<struct Request> requests)
{

	sort(requests.begin(), requests.end(), criteria);
	//algo1
	float tat1 = algo1(requests);
	cout<<"TAT with algo1 is "<<tat1<<endl;

	//algo2
	float tat2 = algo2(requests);
	cout<<"TAT with algo2 is "<<tat2<<endl;
}

void processRequests()
{
	int onlyOnce=1;
	while(onlyOnce)
	{

		onlyOnce--;
		vector<struct Request> requests;
		// create random requests here
		for(int i=0;i<MAX_REQUESTS;++i)
		{
			// Generate and store request
			struct Request temp;
			int chain_length = /*rand()%*/CHAIN_LENGTH /*+ 1*/;
			temp.destination = rand()%nodes;
			// source!= destination
			do
			{
				temp.source = edge_nodes[rand()%edge_nodes.size()];
			} while(temp.source==temp.destination);
			float delay = (rand()%(1000*REQUEST_DELAY) + 0.1)/1000.0;
			temp.delay = delay;
			temp.throughput = REQUEST_THROUGHPUT[rand()%REQUEST_THROUGHPUT.size()];
			
			// Filling the request
			for(int j=0;j<chain_length;++j)
			{
				map<int, int> ids;
				int id = rand()%TYPES_AVAILABLE;
				struct Resources resources;
				resources.cpu = /*rand()%*/REQUEST_RESOURCES + 1;
				temp.NF.push_back(make_pair(id, resources));
			}
			requests.push_back(temp);
		}
	
		serveRequests(requests);
	}
}

int main()
{
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
				temp.resources.cpu = rand()%(EDGE_RESOURCES/2) + ((EDGE_RESOURCES-1)/2);
			else
				temp.resources.cpu = rand()%(CORE_RESOURCES/2) + ((CORE_RESOURCES-1)/2);

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
				int for_bandwidth = rand()%2;
				if(for_bandwidth==0)
					temp.bandwidth = 100;
				else
					temp.bandwidth = 1000;
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

				if(e1==1&&e2==1) // both are edge nodes
				{
					;
				}
				else if(e1==0&&e2==0)  // both are core nodes
				{
					;
				}
				else   // one of them is edge 
				{
					;
				}
				temp.node1 = node1;
				temp.node2 = node2;
				float delay = (rand()%100)/100.0;
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