#include<vector>
using namespace std;

int CORE_NODE = 0;
int MAX_REQUESTS = 100;
int EDGE_NODE = 1;
int CHAIN_LENGTH = 4;
int TYPES_AVAILABLE = 5;
int EDGE_RESOURCES = 10; 
int CORE_RESOURCES = 50; 
vector<int> REQUEST_THROUGHPUT={1,2,5,10};
int REQUEST_DELAY = 1;
int REQUEST_RESOURCES = 2;

int REQUESTS_PER_UNIT = 10;

float DELAY_SENSITIVE = 0.5;

struct Resources
{
	int cpu;
	int mem;
	int IO;
};

struct VNF
{
	int type;
	bool shareable;
	struct Resources resources;
};

struct Node
{
	int id;					
	struct Resources available_resources;		// current resources
	struct Resources resources;					// total resources available here
	int node_type;                // edge or cloud
	vector<struct VNF> existing_vnf; // already deployed VNFs in this node
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
	int source;
	int destination;
	vector<pair<int, struct Resources>> NF;  // the type of VNF required and how many resources it should have
	int throughput;   // end-end
	float delay;     // end-end
};


bool is_shareable(int id)
{
	if(id==0||id==1||id==2)
		return true;
	else
		return false;
}

struct path_info
{
	float delay;
	vector<int> path;
};

#include<vector>
using namespace std;

float interference_metric(struct Node node, pair<int, struct Resources> NF)
{
	vector<struct VNF> existing_vnf = node.existing_vnf;

	int type = NF.first;
	struct Resources resources = NF.second;

	int total_required_cpu_resources = 0;
	for(auto vnf: existing_vnf)
	{
		total_required_cpu_resources += vnf.resources.cpu;
	}
	total_required_cpu_resources += resources.cpu;

	float interference = 1.0*total_required_cpu_resources/node.resources.cpu;
	return interference;
}

bool is_available(struct Resources r1, struct Resources r2)
{
	if(r1.cpu>=r2.cpu&&r1.mem>=r2.mem&&r1.IO>=r2.IO)
		return 1;
	else
		return 0;
}

void consume_resources(struct Resources *r1, struct Resources r2)
{
	r1->cpu-=r2.cpu;
	r1->mem-=r2.mem;
	r1->IO-=r2.IO;
}