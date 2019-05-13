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

struct VNF
{
	int type;
	bool shareable;
	int resources;
};

struct Node
{
	int id;					
	int available_resources;		// current resources
	int resources;					// total resources available here
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
	vector<pair<int, int>> NF;  // the type of VNF required and how many resources it should have
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

int interference_metric(struct Node node, pair<int, int> NF)
{
	vector<struct VNF> existing_vnf = node.existing_vnf;

	int type = NF.first;
	int resources = NF.second;
	return 1;
}