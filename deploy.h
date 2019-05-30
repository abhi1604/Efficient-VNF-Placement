#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
using namespace std;

struct end_result deployVNFSforSPH(struct Request request, struct path_info selected_path_info, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, vector<int>> &vnfNodes, map<int, struct Request> &map_request)
{
	struct end_result results;
	results.is_satisfied=0;

	vector<pair<int, int>> path = selected_path_info.path_with_type;
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;     
	map<int, int> shareable_vnf_deployed;
	float current_delay = selected_path_info.delay; 
	float delay = request.delay;
	vector<float> throughput_interference;
	vector<int> shareable_id;

	for(int i=0; i<path.size(); ++i)
	{
		if(path[i].second!=-1)
			shareable_id.push_back(i);
	}

	int path_node1_id = shareable_id[1], path_node2_id = shareable_id[1];  // because 0 is the source and the shareable types start from 1
	int counter = 1; // not 0, because it is the source
    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
    	struct Resources resources = request.NF[i].second;
		if(is_shareable(vnf_type) && path[path_node1_id].second==vnf_type)
		{
			if(is_violating(local_nodes[path[path_node1_id].first], request.NF[i], map_request))  // if current request violates SLA of already deployed rquests, reject this
			{
				return results;
			}
			float interference = interference_metric(local_nodes[path[path_node1_id].first], request.NF[i]);
			throughput_interference.push_back(interference);
			current_delay += interference*delay_for_vnf_type(vnf_type);
			deployed_path.push_back(path_node1_id);
			counter++;
			path_node1_id = path_node2_id;
			path_node2_id = shareable_id[counter];
		}
		else
		{
			// place between path[node1] and path[node2]
			int is_deployed = 0;
			for(int j=path_node1_id; j<=path_node2_id; ++j)
			{
				int node_id = path[j].first; 
				if(is_available(local_nodes[node_id].available_resources, resources)) // consider only if the node has sufficient resources
				{
					// compute interference of vnf_type with node with id path[j] here and update minInterference
					float temp = interference_metric(local_nodes[node_id], request.NF[i]);
					current_delay += temp*delay_for_vnf_type(vnf_type);
					if(is_violating(local_nodes[node_id], request.NF[i], map_request))  // if current request violates SLA of already deployed rquests, reject this
						return results;
					throughput_interference.push_back(temp);
					deployed_path.push_back(j);
					path_node1_id = j;
					is_deployed=1;	
					break;

				}
			}
			if(is_deployed==0) // this vnf cannot be deployed anywhere in the path
				return results;
			// deploy this vnf here
			if(is_shareable(vnf_type))
			{
				shareable_vnf_deployed[vnf_type] = path[path_node1_id].first;
			}
		}
		if(current_delay>delay)
			return results;
    }

	// request placed successfully here!
	// update the local graph now
	float throughput_interference_till_now=1;
	for(int i=0; i<deployed_path.size()-1; ++i)
	{
		int i1=deployed_path[i];
		int i2=deployed_path[i+1];
		throughput_interference_till_now *= throughput_interference[i];
		for(int j=i1;j<i2;++j)
		{
			int node1 = path[j].first;
			int node2 = path[j+1].first;
			for(auto &edges: local_graph[node1])
			{
				if(edges.node2==node2)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
			for(auto &edges: local_graph[node2])
			{
				if(edges.node1==node1)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
		}
	}

	VNFS_FOR_SPH+=deployed_path.size();
	counter=0;
	for(auto node:deployed_path)
	{
		int type = request.NF[counter].first;
		struct Resources resources = request.NF[counter].second;
		
		if(is_shareable(type) && shareable_vnf_deployed.count(type)>0)
		{
			for(auto &localvnf: local_nodes[path[node].first].existing_vnf)
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
			// push the node running the shareable vnf type
			if(is_shareable(type))  
				vnfNodes[type].push_back(shareable_vnf_deployed[type]);
			struct Resources new_vnf_resources;
			new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
			consume_resources(&local_nodes[path[node].first].available_resources, new_vnf_resources);
			struct VNF temp;
			temp.type = type;
			temp.resources = new_vnf_resources;
			temp.available_resources = new_vnf_resources;
			consume_resources(&temp.available_resources, resources);
			local_nodes[path[node].first].existing_vnf.push_back(make_pair(temp, request.request_id));
		}
		map_request[request.request_id].current_delay=current_delay;
		map_request[request.request_id].nodes.push_back(path[node].first);
		counter++;
	}

	results.throughput = throughput*throughput_interference_till_now;
	results.is_satisfied=1;

	return results;
}

struct end_result deployVNFSwithInterference(struct Request request, struct path_info selected_path_info, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, vector<int>> &vnfNodes, map<int, struct Request> &map_request)
{
	struct end_result results;
	results.is_satisfied=0;

	vector<pair<int, int>> path = selected_path_info.path_with_type;
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	vector<int> deployed_path;
	map<int, int> shareable_vnf_deployed;
	float current_delay = selected_path_info.delay; 
	float delay = request.delay;
	vector<float> throughput_interference;
	vector<int> skipnode;
	vector<int> shareable_id;

	for(int i=0; i<path.size(); ++i)
	{
		if(path[i].second!=-1)
			shareable_id.push_back(i);
	}

	int path_node1_id = shareable_id[1], path_node2_id = shareable_id[1];  // because 0 is the source and the shareable types start from 1
	int counter = 1; // not 0, because it is the source
    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
    	struct Resources resources = request.NF[i].second;
		if(is_shareable(vnf_type) && path[path_node1_id].second==vnf_type)
		{
			if(is_violating(local_nodes[path[path_node1_id].first], request.NF[i], map_request))  // if current request violates SLA of already deployed rquests, reject this
			{
				goto here;  // if the node hosting a shareable vnf is violating the SLA of past request, try other nodes
				return results;
			}
			float interference = interference_metric(local_nodes[path[path_node1_id].first], request.NF[i]);
			throughput_interference.push_back(interference);
			current_delay += interference*delay_for_vnf_type(vnf_type);
			deployed_path.push_back(path_node1_id);
			counter++;
			path_node1_id = path_node2_id;
			path_node2_id = shareable_id[counter];
		}
		else
		{
			skipnode.clear();
			here:
			float minInterference=FLT_MAX;
			int minInterferenceNodeId = path_node1_id;
			// place between path[node1] and path[node2]
			for(int j=path_node1_id; j<=path_node2_id; ++j)
			{
				if(find(skipnode.begin(), skipnode.end(), j) == skipnode.end())
				{
					int node_id = path[j].first; 
					if(is_available(local_nodes[node_id].available_resources, resources)) // consider only if the node has sufficient resources
					{
						// compute interference of vnf_type with node with id path[j] here and update minInterference
						float temp = interference_metric(local_nodes[node_id], request.NF[i]);
						if(minInterference > temp)
						{
							minInterference = temp;     // update the interference value
							minInterferenceNodeId = j;  // update the node id
						}
					}
				}
			}
			if(minInterference==FLT_MAX) // cannot place this vnf anywhere in the path
				return results;

			// deploy this vnf here
			if(is_shareable(vnf_type))
			{
				shareable_vnf_deployed[vnf_type] = path[minInterferenceNodeId].first;
			}
			if(is_violating(local_nodes[path[minInterferenceNodeId].first], request.NF[i], map_request)) // if current request violates SLA of already deployed rquests, skip this node and try others
			{
				skipnode.push_back(minInterferenceNodeId);  // ignoring this node for the next iteration
				goto here;
				return results;
			}
			path_node1_id = minInterferenceNodeId;
			float interference = interference_metric(local_nodes[path[minInterferenceNodeId].first], request.NF[i]);
			throughput_interference.push_back(interference);
			current_delay += interference*delay_for_vnf_type(vnf_type);
			deployed_path.push_back(minInterferenceNodeId);
		}
		if(current_delay>delay)
			return results;
    }

	// request placed successfully here!
	// update the local graph now
	float throughput_interference_till_now=1;
	for(int i=0; i<deployed_path.size()-1; ++i)
	{
		int i1=deployed_path[i];
		int i2=deployed_path[i+1];
		throughput_interference_till_now *= throughput_interference[i];
		for(int j=i1;j<i2;++j)
		{
			int node1 = path[j].first;
			int node2 = path[j+1].first;
			for(auto &edges: local_graph[node1])
			{
				if(edges.node2==node2)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
			for(auto &edges: local_graph[node2])
			{
				if(edges.node1==node1)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
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
			for(auto &localvnf: local_nodes[path[node].first].existing_vnf)
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
			// push the node running the shareable vnf type
			if(is_shareable(type))  
				vnfNodes[type].push_back(shareable_vnf_deployed[type]);
			struct Resources new_vnf_resources;
			new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
			consume_resources(&local_nodes[path[node].first].available_resources, new_vnf_resources);
			struct VNF temp;
			temp.type = type;
			temp.resources = new_vnf_resources;
			temp.available_resources = new_vnf_resources;
			consume_resources(&temp.available_resources, resources);
			local_nodes[path[node].first].existing_vnf.push_back(make_pair(temp, request.request_id));
		}
		map_request[request.request_id].current_delay=current_delay;
		map_request[request.request_id].nodes.push_back(path[node].first);
		counter++;
	}

	results.throughput = throughput*throughput_interference_till_now;
	results.is_satisfied=1;
	return results;
}

bool most_loaded_criteria(pair<int, struct Node> n1, pair<int, struct Node> n2)
{
	int resources1 = n1.second.resources.cpu;
	int resources2 = n2.second.resources.cpu;
	int available_resources1 = n1.second.available_resources.cpu;
	int available_resources2 = n2.second.available_resources.cpu;
	return(float(available_resources2*1.0/resources2) < float(available_resources1*1.0/resources1));
}

struct end_result deployVNFSforGUS(struct Request request, struct path_info selected_path_info, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, vector<int>> &vnfNodes, map<int, struct Request> &map_request)
{

	struct end_result results;
	results.is_satisfied=0;

	vector<pair<int, int>> path = selected_path_info.path_with_type;
	int throughput = request.throughput;
	vector<pair<int, struct Resources>> NF = request.NF;  // type of NF, resources it should have
	map<int, int> shareable_vnf_deployed;
	float current_delay = 0; 
	float delay = request.delay;
	vector<float> throughput_interference;
	vector<int> shareable_id;


	vector<pair<int, struct Node>> most_loaded;  // will store index of path, node id


	vector<float> path_delays;  // it stores the path delays between consecutive nodes in the path

	for(int i=0; i<path.size(); ++i)
	{
		if(path[i].second!=-1)
			shareable_id.push_back(i);
	}

		// copy local nodes into a temp vector
	vector<struct Node> temp_nodes(local_nodes);

	for(int i=0;i<path.size()-1;++i)
	{
		int node1 = path[i].first;
		int node2 = path[i+1].first;

		for(auto link: local_graph[node1])
		{
			if(link.node2==node2)
			{
				path_delays.push_back(link.delay);
				break;
			}
		}
	}

	vector<pair<int, int>> deployed_path; // will store index of the path, vnf type deployed

	// will store index of path, node id
	for(int i=0;i<path.size(); ++i)
	{
		most_loaded.push_back(make_pair(i, temp_nodes[path[i].first]));
	}
	
	sort(most_loaded.begin(), most_loaded.end(), most_loaded_criteria);

	int path_node1_id = shareable_id[1];  // because 0 is the source and the shareable types start from 1
	int counter = 1; // not 0, because it is the source

    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
    	struct Resources resources = request.NF[i].second;
		if(is_shareable(vnf_type) && path[path_node1_id].second==vnf_type)
		{
			if(is_violating(local_nodes[path[path_node1_id].first], request.NF[i], map_request))  // if current request violates SLA of already deployed rquests, reject this
			{
				cout<<"From 1\n\n";
				return results;
			}
			
			float interference = interference_metric(local_nodes[path[path_node1_id].first], request.NF[i]);
			throughput_interference.push_back(interference);
			current_delay += interference*delay_for_vnf_type(vnf_type);
			deployed_path.push_back(make_pair(path_node1_id, vnf_type));
			counter++;
			path_node1_id = shareable_id[counter];
		}
		else
		{
			int is_deployed=0;
			for(auto &node: most_loaded) 
			{
				if(is_available(node.second.available_resources, resources))
				{
					if(is_violating(local_nodes[node.second.id], request.NF[i], map_request))  // if current request violates SLA of already deployed rquests, reject this
					{
						cout<<"From 2\n\n";
						return results;
					}
					
					float interference = interference_metric(local_nodes[node.second.id], request.NF[i]);
					throughput_interference.push_back(interference);
					current_delay += (1+interference)*delay_for_vnf_type(vnf_type)*1.0;  // 1+ to consider both vnf delay and interference delay
					deployed_path.push_back(make_pair(node.first, vnf_type));
					is_deployed = 1;
					consume_resources(&node.second.available_resources, resources);
					break;
				}
			}

			if(is_deployed==0)
			{
				// cout<<"From 3\n\n";	
				return results; // this VNF cannot be deployed anywhere in the path
			}
			// deploy this vnf here
			if(is_shareable(vnf_type))
			{
				shareable_vnf_deployed[vnf_type] = path[path_node1_id].first;
			}
		}
		if(current_delay>delay)
		{
			cout<<"From 4\n\n";
			return results;
		}
    }

	// check if the deployed path taken can satisfy the delay requirement

	float total_delay=current_delay;

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
	{
		cout<<"From 5\n\n";

		return results;
	}

	// request placed successfully here!
	// update the local graph now
	float throughput_interference_till_now=1;
	for(int i=0; i<deployed_path.size()-1; ++i)
	{
		int i1=deployed_path[i].first;
		int i2=deployed_path[i+1].first;
		throughput_interference_till_now *= throughput_interference[i];
		for(int j=i1;j<i2;++j)
		{
			int node1 = path[j].first;
			int node2 = path[j+1].first;
			for(auto &edges: local_graph[node1])
			{
				if(edges.node2==node2)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
			for(auto &edges: local_graph[node2])
			{
				if(edges.node1==node1)
					edges.available_bandwidth -= throughput*throughput_interference_till_now;
			}
		}
	}

	VNFS_FOR_GUS+=deployed_path.size();
	counter=0;
	for(auto node:deployed_path)
	{
		int type = request.NF[counter].first;
		struct Resources resources = request.NF[counter].second;
		
		if(is_shareable(type) && shareable_vnf_deployed.count(type)>0)
		{
			for(auto &localvnf: local_nodes[path[node.first].first].existing_vnf)
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
			// push the node running the shareable vnf type
			if(is_shareable(type))  
				vnfNodes[type].push_back(shareable_vnf_deployed[type]);
			struct Resources new_vnf_resources;
			new_vnf_resources.cpu = ((VNF_MAX_RESOURCES - VNF_MIN_RESOURCES) * ((float)rand() / RAND_MAX)) + VNF_MIN_RESOURCES;
			consume_resources(&local_nodes[path[node.first].first].available_resources, new_vnf_resources);
			struct VNF temp;
			temp.type = type;
			temp.resources = new_vnf_resources;
			temp.available_resources = new_vnf_resources;
			consume_resources(&temp.available_resources, resources);
			local_nodes[path[node.first].first].existing_vnf.push_back(make_pair(temp, request.request_id));
		}
		map_request[request.request_id].current_delay=total_delay;
		map_request[request.request_id].nodes.push_back(path[node.first].first);
		counter++;
	}

	results.is_satisfied=1;
	results.throughput = throughput*throughput_interference_till_now;
	return results;
}