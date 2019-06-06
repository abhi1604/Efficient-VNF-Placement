#include<vector>
using namespace std;

int MAX_REQUESTS = 50;
int EDGE_NODE = 0;
int CORE_NODE = 1;
int CHAIN_LENGTH = 3;
int TYPES_AVAILABLE = 4;

int CPU_TYPE = 0;
int IO_TYPE = 1;
int MEM_TYPE = 2;

// edge node resources
int EDGE_MIN_RESOURCES = 20;
int EDGE_MAX_RESOURCES = 40; 

// core node resources
int CORE_MIN_RESOURCES = 50;
int CORE_MAX_RESOURCES = 200; 

// request resources for a SFC
int REQUEST_MIN_THROUGHPUT = 80;
int REQUEST_MAX_THROUGHPUT = 100;

// Request delay
int REQUEST_MIN_DELAY = 80;
int REQUEST_MAX_DELAY = 100;

// VNF resources
int VNF_MIN_RESOURCES = 5;
int VNF_MAX_RESOURCES = 10;

// Request resources
int REQUEST_MIN_RESOURCES = 1;
int REQUEST_MAX_RESOURCES = 5;

// edge node -to- edge node delay
int EDGE_EDGE_MIN_DELAY = 1;
int EDGE_EDGE_MAX_DELAY = 5;

// edge node -to- core node delay
int EDGE_CORE_MIN_DELAY = 5;
int EDGE_CORE_MAX_DELAY = 10;

// core node -to- core node delay
int CORE_CORE_MIN_DELAY = 10;
int CORE_CORE_MAX_DELAY = 15;

// float DELAY_SENSITIVE = 0.5; 

// interference
float INTERFERENCE_CPU_MEM = 0.5;
float INTERFERENCE_CPU_CPU = 1;
float INTERFERENCE_CPU_IO = 0.3;
float INTERFERENCE_MEM_MEM = 1;
float INTERFERENCE_MEM_CPU = 0.5;
float INTERFERENCE_MEM_IO = 0.3;
float INTERFERENCE_IO_MEM = 0.3;
float INTERFERENCE_IO_CPU = 0.3;
float INTERFERENCE_IO_IO = 1;

// VNF delays
float delay_for_vnf_type(int type)
{
	if(type==0)    // type 1
		return 5.5;  // in ms
	else if(type==1)  // type 2 
		return 4.5;  // in ms
	else if(type==2) // type 3
		return 2.5;  // 2.5 ms
	else if(type==3)  // type 4
		return 3.5;   // 3.5ms
}
// temp for stats purpose
int VNFS_FOR_SPH=0;
int VNFS_FOR_ALGO=0;
int VNFS_FOR_GUS=0;
int VNFS_FOR_AIA=0;

struct Resources
{
	int cpu;
	// int mem;  // not needed as of now, can be useful in the future
	// int IO;
};

struct VNF
{
	int type;
	bool shareable;
	struct Resources resources;
	struct Resources available_resources;
};

struct Node
{
	int id;					
	struct Resources available_resources;		// current resources
	struct Resources resources;					// total resources available here
	int node_type;                // edge or cloud
	vector<pair<struct VNF, int>> existing_vnf; // already deployed VNFs in this node
	// vector<struct Request> existing_requests; // requests already being served by this Node
};

struct LinkInfo
{
	int node1;
	int node2;
	int available_bandwidth;
	int bandwidth;
	float delay;
};

struct Request
{
	int request_id;
	int source;
	int destination;
	vector<pair<int, struct Resources>> NF;  // the type of VNF required and how many resources it should have
	int throughput;   // end-end
	float delay;     // end-end (required)
	float current_delay;  // actual delay with which the request is served
	vector<int> nodes;    // nodes the VNFs are deployed on
};

// to set which VNFs are shareable
bool is_shareable(int id)
{
	if(id==0||id==1)
		return false;
	else
		return false;
}

struct path_info
{
	float delay;
	vector<int> path;
	vector<pair<int, int>> path_with_type; // for multi stage, for knowing shareability info 
};

int typeofvnf(int type)
{
	if(type==0||type==3)
		return CPU_TYPE;
	else if(type==1)
		return IO_TYPE;
	else if(type==2)
		return MEM_TYPE;
}

struct end_result
{
	int is_satisfied;
	float throughput;
};

float interference_metric(struct Node node, pair<int, struct Resources> NF)
{
	vector<pair<struct VNF, int>> existing_vnf = node.existing_vnf;

	float interference_with_IO_type = 0;
	float interference_with_cpu_type = 0;
	float interference_with_mem_type = 0;

	int type = NF.first;
	struct Resources resources = NF.second;

	int total_required_cpu_resources = 0;
	int with_mem = 0, with_cpu = 0, with_io = 0;
	for(auto vnf: existing_vnf)
	{
		int vnf_resources = vnf.first.resources.cpu;
		if(typeofvnf(vnf.first.type)==CPU_TYPE)
			with_cpu += vnf_resources;
		else if(typeofvnf(vnf.first.type)==MEM_TYPE)
			with_mem += vnf_resources;
		else if(typeofvnf(vnf.first.type)==IO_TYPE)
			with_io += vnf_resources;

		total_required_cpu_resources += vnf_resources;
	}

	total_required_cpu_resources += resources.cpu;
	if(typeofvnf(type) == CPU_TYPE)
		with_cpu += resources.cpu;
	else if(typeofvnf(type)==MEM_TYPE)
		with_mem += resources.cpu;
	else if(typeofvnf(type)==IO_TYPE)
		with_io += resources.cpu;

	interference_with_mem_type = (1.0*with_mem);
	interference_with_cpu_type = (1.0*with_cpu);
	interference_with_IO_type = (1.0*with_io);

	float interference;
	if(typeofvnf(type) == CPU_TYPE)
	{
		interference = INTERFERENCE_CPU_MEM*interference_with_mem_type + INTERFERENCE_CPU_CPU*interference_with_cpu_type + INTERFERENCE_CPU_IO*interference_with_IO_type;
	}
	else if(typeofvnf(type) == MEM_TYPE)
	{
		interference = INTERFERENCE_MEM_MEM*interference_with_mem_type + INTERFERENCE_MEM_CPU*interference_with_cpu_type + INTERFERENCE_MEM_IO*interference_with_IO_type;
	}
	else if(typeofvnf(type) == IO_TYPE)
	{
		interference = INTERFERENCE_IO_MEM*interference_with_mem_type + INTERFERENCE_IO_CPU*interference_with_cpu_type + INTERFERENCE_IO_IO*interference_with_IO_type;
	}

	interference = interference*1.0/node.resources.cpu;
	return interference;
}

float interference_metric_AIA(struct Node node, pair<int, struct Resources> NF)
{
	vector<pair<struct VNF, int>> existing_vnf = node.existing_vnf;

	float interference_with_IO_type = 0;
	float interference_with_cpu_type = 0;
	float interference_with_mem_type = 0;

	int type = NF.first;
	struct Resources resources = NF.second;

	int total_required_cpu_resources = 0;
	int with_mem = 0, with_cpu = 0, with_io = 0;
	for(auto vnf: existing_vnf)
	{
		int vnf_resources = vnf.first.resources.cpu;
		if(typeofvnf(vnf.first.type)==CPU_TYPE)
			with_cpu += vnf_resources;
		else if(typeofvnf(vnf.first.type)==MEM_TYPE)
			with_mem += vnf_resources;
		else if(typeofvnf(vnf.first.type)==IO_TYPE)
			with_io += vnf_resources;

		total_required_cpu_resources += vnf_resources;
	}

	total_required_cpu_resources += resources.cpu;
	if(typeofvnf(type) == CPU_TYPE)
		with_cpu += resources.cpu;
	else if(typeofvnf(type)==MEM_TYPE)
		with_mem += resources.cpu;
	else if(typeofvnf(type)==IO_TYPE)
		with_io += resources.cpu;

	interference_with_mem_type = (1.0*with_mem);
	interference_with_cpu_type = (1.0*with_cpu);
	interference_with_IO_type = (1.0*with_io);

	float interference;
	if(typeofvnf(type) == CPU_TYPE)
	{
		interference = 1*interference_with_mem_type + 1*interference_with_cpu_type + 1*interference_with_IO_type;
	}
	else if(typeofvnf(type) == MEM_TYPE)
	{
		interference = 1*interference_with_mem_type + 1*interference_with_cpu_type + 1*interference_with_IO_type;
	}
	else if(typeofvnf(type) == IO_TYPE)
	{
		interference = 1*interference_with_mem_type + 1*interference_with_cpu_type + 1*interference_with_IO_type;
	}

	interference = interference*1.0/node.resources.cpu;
	return interference;
}

float compute_vnf_delay(struct Request req)
{
	float total_vnf_delay = 0;

	for(auto vnf: req.NF)
	{
		total_vnf_delay += delay_for_vnf_type(vnf.first);
	}

	return total_vnf_delay;
}

bool is_available(struct Resources r1, struct Resources r2)
{
	if(r1.cpu>=r2.cpu/*&&r1.mem>=r2.mem&&r1.IO>=r2.IO*/)
		return 1;
	else
		return 0;
}

void consume_resources(struct Resources *r1, struct Resources r2)
{
	r1->cpu-=r2.cpu;
	// r1->mem-=r2.mem;
	// r1->IO-=r2.IO;
}

void stats(vector<struct Node> nodes)
{
	int total=0;

	for(auto node:nodes)
		if(node.existing_vnf.size()>0)
			total++;

	cout<<"Total nodes activated is "<<total<<endl;
}

void remove_request(int request_id, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, struct Request> &map_request)
{
	int counter=0;
	for(auto node: map_request[request_id].nodes)
	{
		pair<int, struct Resources> vnf = map_request[request_id].NF[counter];
		// struct Node l_node = local_nodes[node];
		int remove_temp_counter=0;
		for(auto &e_vnf: local_nodes[node].existing_vnf)
		{
			if(e_vnf.first.type == vnf.first && e_vnf.first.resources.cpu == vnf.second.cpu) // remove this vnf from the node
			{
				local_nodes[node].existing_vnf.erase(local_nodes[node].existing_vnf.begin()+remove_temp_counter);
				local_nodes[node].available_resources.cpu += vnf.second.cpu;  // this nodes gets its resources back
				break;
			}
			remove_temp_counter++;
		}
		counter++;
	}

	// request removed successfully here!
	// update the local graph now
	for(int i=0; i<map_request[request_id].nodes.size()-1; ++i)
	{
		int node1 = map_request[request_id].nodes[i];
		int node2 = map_request[request_id].nodes[i+1];
		for(auto &edges: local_graph[node1])  // search for the required edge
		{
			if(edges.node2==node2)
				edges.available_bandwidth += map_request[request_id].throughput;
		}
		for(auto &edges: local_graph[node2])  // search for the required edge
		{
			if(edges.node1==node1)
				edges.available_bandwidth += map_request[request_id].throughput;
		}
	}
	// done
}

int remove_violated_helper(pair<int, struct Resources> vnf, int node_id, vector<vector<struct LinkInfo>> &local_graph, vector<struct Node> &local_nodes, map<int, struct Request> &map_request)
{
	int total_removed = 0;
	float incremental_interference = vnf.second.cpu*1.0/local_nodes[node_id].resources.cpu;

	for(auto &existing_req: local_nodes[node_id].existing_vnf)
	{
		struct VNF vnf_temp = existing_req.first;
		int req_temp_id = existing_req.second;
		float new_delay = delay_for_vnf_type(vnf_temp.type)*incremental_interference + map_request[req_temp_id].current_delay;
		if( new_delay > map_request[req_temp_id].delay)  // check whether the new request is violating any request already passing throught this node
		{
			remove_request(req_temp_id, local_nodes, local_graph, map_request);
			total_removed++;
		}
		else // update the existing requests delay here otherwise
		{
			map_request[req_temp_id].current_delay = new_delay;
		}
	}
	return total_removed;
}

int remove_violated(struct Request request, vector<struct Node> &local_nodes, vector<vector<struct LinkInfo>> &local_graph, map<int, struct Request> &map_request)
{
	vector<int> nodes = map_request[request.request_id].nodes;

	int total_removed = 0;
	int counter = 0;
	for(auto node_id: nodes)
	{
		total_removed += remove_violated_helper(request.NF[counter++], node_id, local_graph, local_nodes, map_request);
	}

	return total_removed;
}

bool is_violating(struct Node node, pair<int, struct Resources> vnf, map<int, struct Request> &map_request)
{
	for(auto vnf_request: node.existing_vnf)
	{
		struct VNF vnf_temp = vnf_request.first;
		int request_id = vnf_request.second;
		float current_delay = map_request[request_id].current_delay;
		float interference=0; 
		float with_mem=0, with_cpu=0, with_io=0;

		if(typeofvnf(vnf_temp.type)==CPU_TYPE)
		{
			with_cpu = vnf.second.cpu;
		}
		else if(typeofvnf(vnf_temp.type)==MEM_TYPE)
		{
			with_mem = vnf.second.cpu;
		}
		else if(typeofvnf(vnf_temp.type)==IO_TYPE)
		{
			with_io = vnf.second.cpu;
		}

		if(typeofvnf(vnf.first)==CPU_TYPE)
		{
			interference += INTERFERENCE_CPU_CPU*with_cpu + INTERFERENCE_CPU_MEM*with_mem + INTERFERENCE_CPU_IO*with_io;
		}
		else if(typeofvnf(vnf.first)==MEM_TYPE)
		{
			interference += INTERFERENCE_MEM_CPU*with_cpu + INTERFERENCE_MEM_MEM*with_mem + INTERFERENCE_MEM_IO*with_io;
		}
		else if(typeofvnf(vnf.first)==IO_TYPE)
		{
			interference += INTERFERENCE_IO_CPU*with_cpu + INTERFERENCE_IO_MEM*with_mem + INTERFERENCE_IO_IO*with_io;
		}

		interference /= node.resources.cpu;

		if(interference*delay_for_vnf_type(vnf_temp.type)+current_delay>map_request[request_id].delay)
			return true;
	}
	return false;
}