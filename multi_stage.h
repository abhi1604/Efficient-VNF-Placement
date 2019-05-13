#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 

using namespace std;

vector<pair<int, int>> multi_stage(struct Request request, vector<vector<struct LinkInfo>> graph, map<int, vector<int>> &vnfNodes)
{

    // request has source, destination, NF{vector<pair<int, int>>}, throughput, delay 
    int V = graph.size();// Get the number of vertices in graph 
    float dist[V];      // dist values used to pick minimum weight edge in cut 
    int src = request.source;
    int dest = request.destination;
    int throughput = request.throughput;
    float delay = request.delay;

    // create a shareable nodes vector to know if that node is already deployed in the graph
    vector<int> shareable_nodes;
    for(int i=0; i<request.NF.size(); ++i)
    {
    	int vnf_type = request.NF[i].first; // vnf type of request
		if(is_shareable(vnf_type))
		{
			if(!vnfNodes[vnf_type].empty())
			{
				shareable_nodes.push_back(vnf_type);
			}
		}
    }

    vector<vector<int>> matrix;

    matrix.resize(shareable_nodes.size()+2);
    matrix[0].push_back(src);
    matrix[matrix.size()-1].push_back(dest);

    int counter=0;
    for(int i=1; i<matrix.size()-1; ++i)
    {
    	matrix[i] = vnfNodes[shareable_nodes[counter++]];  // matrix has info about where all the shareable vnf is deployed
    }

    vector<vector<float>> d(graph.size(), vector<float>(matrix.size(), INT_MAX));
    vector<vector<int>> pt(graph.size(), vector<int>(matrix.size(), INT_MAX)); // INT_MAX means we have not selected a node from that stage yet

    for(int i=0; i<graph.size(); ++i)
        d[i][0] = 0.0;
    
    for(int i=1; i<matrix.size(); ++i)
    {
        for (auto m:matrix[i])
        {
            for(auto n:matrix[i-1])
            {
                // create temp_request here
                struct Request temp_request;
                temp_request.source = m;
                temp_request.destination = n;
                temp_request.throughput = request.throughput;
                temp_request.delay = request.delay;                
                struct path_info temp = dijkstra(request, graph);
                float current_delay = temp.delay;

                if(d[n][i-1] + current_delay < d[m][i])
                {
                    d[m][i] = d[n][i-1] + current_delay;
                    pt[m][i] = n;
                }
            }
        }
    }

    float lat_curr = d[dest][matrix.size()-1];

    // cannot provision this request if delay is more than requirement
    if(lat_curr > request.delay)
    {
        vector<pair<int, int>> temp;
        return temp;
    }

    vector<int> p_nodes; // a path containing only shareable nodes including src and dest
    p_nodes.resize(matrix.size());
    p_nodes[0] = src;
    p_nodes[matrix.size()-1] = dest;

    for(int i=matrix.size()-2; i>=1; --i)
        p_nodes[i]=pt[p_nodes[i+1]][i];
    
    vector<pair<int, int>> complete_path;  // complete path
    for(int i=0; i<matrix.size()-1; ++i)
    {
        struct Request temp_request;
        temp_request.source = p_nodes[i];
        temp_request.destination = p_nodes[i+1];
        temp_request.delay = request.delay;
        temp_request.throughput = request.throughput;
        struct path_info temp = dijkstra(request, graph);   
        for(int j=0; j<temp.path.size()-1; ++j)
        {
            pair<int, int> temp1;
            temp1.first = temp.path[j];
            if(j==0)
                temp1.second = 1;  // shareable vnf deployed here
            else
                temp1.second = 0;  // no shareable vnf deployed here
            complete_path.push_back(temp1);
        }
    }
    pair<int, int> temp1;
    temp1.first = dest;
    temp1.second = 1;
    complete_path.push_back(temp1);
    return complete_path;
}