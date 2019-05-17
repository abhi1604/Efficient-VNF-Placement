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
int INTERFERENCE_CPU_MEM = 0.5;
int INTERFERENCE_CPU_CPU = 1;
int INTERFERENCE_CPU_IO = 0.3;
int INTERFERENCE_MEM_MEM = 1;
int INTERFERENCE_MEM_CPU = 0.5;
int INTERFERENCE_MEM_IO = 0.3;
int INTERFERENCE_IO_MEM = 0.3;
int INTERFERENCE_IO_CPU = 0.3;
int INTERFERENCE_IO_IO = 1;


// temp for stats purpose

int VNFS_FOR_SPH=0;
int VNFS_FOR_ALGO=0;
int VNFS_FOR_GUS=0;

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
	if(id==0||id==1)
		return true;
	else
		return false;
}

struct path_info
{
	float delay;
	vector<int> path;
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

float interference_metric(struct Node node, pair<int, struct Resources> NF)
{
	vector<struct VNF> existing_vnf = node.existing_vnf;

	float interference_with_IO_type = 0;
	float interference_with_cpu_type = 0;
	float interference_with_mem_type = 0;

	int type = NF.first;
	struct Resources resources = NF.second;

	int total_required_cpu_resources = 0;
	int with_mem = 0, with_cpu = 0, with_io = 0;
	for(auto vnf: existing_vnf)
	{
		int vnf_resources = vnf.resources.cpu;
		if(typeofvnf(vnf.type)==CPU_TYPE)
			with_cpu += vnf_resources;
		else if(typeofvnf(vnf.type)==MEM_TYPE)
			with_mem += vnf_resources;
		else if(typeofvnf(vnf.type)==IO_TYPE)
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