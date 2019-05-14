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