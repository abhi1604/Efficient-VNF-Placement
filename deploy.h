#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
using namespace std;

int deployVNFSforSPH(struct Request request, struct path_info selected_path_info, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph)
{
	vector<int> path = selected_path_info.path;
	float current_delay = selected_path_info.delay; 
	float delay = request.delay;
	int src = request.source;
	int dest = request.destination;
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;

	int curr=0;
	for(auto vnf:NF)
	{
		int type = vnf.first;
		struct Resources resources = vnf.second;

		while(true)
		{
			int node_id = path[curr];

			if(is_available(local_nodes[node_id].available_resources, resources))
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
		for(auto &edges: local_graph[node2])
		{
			if(edges.node1==node1)
				edges.available_bandwidth -= throughput;
		}
	}

	int counter=0;

	VNFS_FOR_SPH+=deployed_path.size();
	for(auto node_id:deployed_path)
	{
		int type = request.NF[counter].first;
		struct Resources resources = request.NF[counter].second;
		counter++;
		// deploy this vnf here
		struct Resources new_vnf_resources;
		new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
		consume_resources(&local_nodes[node_id].available_resources, new_vnf_resources);

		struct VNF temp;
		temp.type = type;
		temp.resources = new_vnf_resources;
		temp.available_resources = new_vnf_resources;
		consume_resources(&temp.available_resources, resources);
		local_nodes[node_id].existing_vnf.push_back(make_pair(temp, request));
	}

	return 1;
}

int deployVNFSwithInterference(struct Request request, vector<pair<int, int>> path, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, vector<int>> &vnfNodes)
{
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;
	map<int, int> shareable_vnf_deployed;

	vector<int> shareable_id;
	for(int i=0; i<path.size(); ++i)
	{
		if(path[i].second!=-1)
			shareable_id.push_back(i);
	}

	int node1 = shareable_id[0], node2 = shareable_id[1];
	int counter = 1; // not 0, because it is the source
    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
    	struct Resources resources = request.NF[i].second;
		if(is_shareable(vnf_type) && path[node1].second==vnf_type)
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
				if(is_available(local_nodes[path_node_id].available_resources, resources)) // consider only if the node has sufficient resources
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
			// deploy this vnf here

			if(is_shareable(vnf_type))
			{
				shareable_vnf_deployed[vnf_type] = minInterferenceNodeId;
			}
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
		for(auto &edges: local_graph[node2])  // search for the required edge
		{
			if(edges.node1==node1)
				edges.available_bandwidth -= throughput;
		}
	}

	VNFS_FOR_ALGO+=deployed_path.size();
	counter=0;
	for(auto node:deployed_path)
	{
		int type = request.NF[counter].first;
		struct Resources resources = request.NF[counter].second;
		
		if(is_shareable(type) && shareable_vnf_deployed.count(type)>0)
		{
			for(auto &localvnf: local_nodes[node].existing_vnf)
			{
				if(localvnf.first.type == type && is_available(localvnf.first.available_resources, resources))
				{
					consume_resources(&localvnf.first.available_resources, resources);
					break;
				}
			}
		}

		else
		{
			if(is_shareable(type))
				vnfNodes[type].push_back(shareable_vnf_deployed[type]);
			struct Resources new_vnf_resources;
			new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
			consume_resources(&local_nodes[node].available_resources, new_vnf_resources);
			struct VNF temp;
			temp.type = type;
			temp.resources = new_vnf_resources;
			temp.available_resources = new_vnf_resources;
			consume_resources(&temp.available_resources, resources);
			local_nodes[node].existing_vnf.push_back(make_pair(temp, request));
		}
		counter++;
	}

	return 1;
}

bool criteria1(pair<int, struct Node> n1, pair<int, struct Node> n2)
{
	int resources1 = n1.second.resources.cpu;
	int resources2 = n2.second.resources.cpu;
	int available_resources1 = n1.second.available_resources.cpu;
	int available_resources2 = n2.second.available_resources.cpu;

	return(float(available_resources2*1.0/resources2) < float(available_resources1*1.0/resources1));
}

int deployVNFSforGUS(struct Request request, vector<int> path, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph)
{
	float delay = request.delay;
	int src = request.source;
	int dest = request.destination;
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	vector<pair<int, struct Node>> most_loaded;  // will store index of path, node id

	vector<float> path_delays;  // it stores the path delays between consecutive nodes in the path

	// copy local nodes into a temp vector
	vector<struct Node> temp_nodes(local_nodes);

	for(int i=0;i<path.size()-1;++i)
	{
		int node1 = path[i];
		int node2 = path[i+1];

		for(auto link: local_graph[node1])
		{
			if(link.node2==node2)
			{
				path_delays.push_back(link.delay);
				break;
			}	
		}
	}

	// will store index of path, node id
	for(int i=0;i<path.size(); ++i)
	{
		most_loaded.push_back(make_pair(i, temp_nodes[path[i]]));
	}

	sort(most_loaded.begin(), most_loaded.end(), criteria1);

	vector<pair<int, int>> deployed_path; // will store index of the path, vnf type deployed

	for(auto vnf: NF)
	{
		int type = vnf.first;
		struct Resources resources = vnf.second;
		int is_deployed=0;
		for(auto &node: most_loaded) 
		{
			if(is_available(node.second.available_resources, resources))
			{
				deployed_path.push_back(make_pair(node.first, type));
				is_deployed = 1;
				consume_resources(&node.second.available_resources, resources);
				break;
			}
		}
		if(is_deployed==0)
			return 0; // this VNF cannot be deployed anywhere in the path
	}

	// check if the deployed path taken can satisfy the delay requirement

	float total_delay=0;

	for(int i=0; i<deployed_path.size()-1; ++i)
	{
		int index_of_path_node1 = deployed_path[i].first;
		int index_of_path_node2 = deployed_path[i+1].first;

		if(index_of_path_node1 != index_of_path_node2)
		{
			if(index_of_path_node2<index_of_path_node1)
			{
				int temp = index_of_path_node2;
				index_of_path_node2=index_of_path_node1;
				index_of_path_node1 = temp;
			}
			for(int j=index_of_path_node1;j<index_of_path_node2;++j)
				total_delay+=path_delays[j];
		}
	}

	if(total_delay>delay)
		return 0;

	VNFS_FOR_GUS+=deployed_path.size();

	// request placed successfully here, consume resources in the path now
	// in deployed path, path[index] will be the node where .second(vnf_type) is deployed
	for(int i=0;i<deployed_path.size();++i)
	{
		int index_of_path_node = deployed_path[i].first;
		int type = deployed_path[i].second;
		int node_id = path[index_of_path_node];
		struct Resources resources = NF[i].second;
		
		struct Resources new_vnf_resources;
		new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
		consume_resources(&local_nodes[node_id].available_resources, new_vnf_resources);

		struct VNF temp;
		temp.type = type;
		temp.resources = new_vnf_resources;
		temp.available_resources = new_vnf_resources;
		consume_resources(&temp.available_resources, resources);
		local_nodes[node_id].existing_vnf.push_back(make_pair(temp, request));
	}

	// update the local graph now
	for(int i=0; i<deployed_path.size()-1; ++i)
	{
		int i1=deployed_path[i].first;
		int i2=deployed_path[i+1].first;

		for(int j=i1;j<i2;++j)
		{
			int node1 = path[j];
			int node2 = path[j+1];
			for(auto &edges: local_graph[node1])
			{
				if(edges.node2==node2)
					edges.available_bandwidth -= throughput;
			}
			for(auto &edges: local_graph[node2])
			{
				if(edges.node1==node1)
					edges.available_bandwidth -= throughput;
			}
		}
	}

	return 1;
}