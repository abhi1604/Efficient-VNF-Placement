#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 

using namespace std;


int deployVNFS(struct Request request, vector<int> path, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph)
{
	float delay = request.delay;
	int src = request.source;
	int dest = request.destination;
	int throughput = request.throughput;
	vector<pair<int, int>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;

	// if(delay<DELAY_SENSITIVE)  // means it is delay sensitive
	// {
	if(path.size()<NF.size())
		return 0;  // TODO: consolidate or find different path

	int curr=0;
	for(auto &nf:NF)
	{
		int type = nf.first;
		int resources = nf.second;

		while(true)
		{
			int node_id = path[curr];

			if(local_nodes[node_id].available_resources>resources)
			{
				deployed_path.push_back(node_id);
				break;
			}

			else  // cannot place this node here!!
			{
				curr++;
				if(curr==path.size())
					return 0;
			}
		}
	}
	// request placed successfully here!
	// update the local graph now
	for(int i=0; i<path.size()-1; ++i)
	{
		int node1 = path[i];
		int node2 = path[i+1];
		for(auto &edges: local_graph[node1])
		{
			if(edges.node2==node2)
				edges.available_bandwidth -= throughput;
		}
	}

	int counter=0;
	for(auto node_id:deployed_path)
	{
		int type = request.NF[counter].first;
		int resources = request.NF[counter].second;
		counter++;
		// deploy this nf here
		local_nodes[node_id].available_resources -= resources;

		struct VNF temp;
		temp.type = type;
		temp.resources = resources;
		local_nodes[node_id].existing_vnf.push_back(temp);
	}
	// }
	// else  // if not delay sensitive
	// {

	// }
	return 1;
}


int deployVNFSwithInterference(struct Request request, vector<pair<int, int>> path, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph)
{
	int throughput = request.throughput;
	vector<pair<int, int>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;

	vector<int> shareable_id;
	for(int i=0; i<path.size(); ++i)
	{
		if(path[i].second==1)
			shareable_id.push_back(i);
	}

	int node1 = shareable_id[0], node2 = shareable_id[1];
	int counter = 1; // not 0 because it is source
    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
    	int resources = request.NF[i].second;
		if(is_shareable(vnf_type))
		{
			counter++;
			node1 = node2;
			node2 = shareable_id[counter];
			deployed_path.push_back(node1);
		}
		else
		{
			int minInterference=INT_MAX;
			int minInterferenceNodeId = path[node1].first;
			// place between path[node1] and path[node2]
			for(int j=node1; j<=node2; ++j)
			{
				int path_node_id = path[j].first; 
				// compute interference of vnf_type with node with id path[j] here and update minInterference
				if(local_nodes[path_node_id].available_resources>resources) // consider only if the node has sufficient resources
				{
					float temp = interference_metric(local_nodes[path_node_id], request.NF[i]);
					if(minInterference > temp)
					{
						minInterference = temp;     // update the interference value
						minInterferenceNodeId = j;  // update the node id
					}		
				}
			}
			if(minInterference==INT_MAX) // cannot place this vnf anywhere in the path
				return 0;
			node1 = minInterferenceNodeId;
			// deploy this nf here
			deployed_path.push_back(minInterferenceNodeId);
		}
    }

	// request placed successfully here!
	// update the local graph now
	for(int i=0; i<path.size()-1; ++i)
	{
		int node1 = path[i].first;
		int node2 = path[i+1].first;
		for(auto &edges: local_graph[node1])  // search for the required edge
		{
			if(edges.node2==node2)
				edges.available_bandwidth -= throughput;
		}
	}

	counter=0;
	for(auto node:deployed_path)
	{
		int type = request.NF[counter].first;
		int resources = request.NF[counter].second;
		
		if(is_shareable(type))
		{
			for(auto &localvnf: local_nodes[node].existing_vnf)
			{
				if(localvnf.type == type)
					localvnf.resources -= resources;
			}
		}
		else
		{
			local_nodes[node].available_resources -= resources;
			struct VNF temp;
			temp.type = type;
			temp.resources = resources;
			local_nodes[node].existing_vnf.push_back(temp);
		}
		counter++;
	}

	return 1;
}