#include<bits/stdc++.h> 
using namespace std; 
  
// visited[] array to make nodes visited 
// src is starting node for DFS traversal 
// prev_delay is sum of delays till current node 
// total_delay is pointer which stores the maximum delay 
// value after DFS traversal 
vector<pair<int, int>> DFS(vector<vector<struct LinkInfo>> graph, int node, float prev_delay, float *total_delay, vector <bool> &visited, vector<pair<int,int>> path, struct Request request)
{
    vector<pair<int, int>> temp_path(path);

    pair<int, int> temp1;

    temp1.first = node;

    if(request.source==node||request.destination==node)
        temp1.second = -2; // to indicate, src, dest
    else
        temp1.second = -1;

    temp_path.push_back(temp1);

    // Mark the node node visited 
    visited[node] = 1; 

    if(request.destination==node)
        return temp_path;

    // curr_delay is for delay from node to its adjacent nodes 
    float curr_delay = 0; 

    // Adjacent is struct type which stores link info between src and adjacent nodes  
    struct LinkInfo adjacent; 

    // Traverse all adjacent nodes
    for (int i=0; i<graph[node].size(); i++) 
    {
        // Adjacent node 
        adjacent = graph[node][i]; 

        // If node is not visited 
        vector<pair<int, int>> return_path;
        if (!visited[adjacent.node2]) 
        { 
            // Total delay from current node to its adjacent 
            curr_delay = prev_delay + adjacent.delay; 

            // Call DFS for adjacent node
            return_path = DFS(graph, adjacent.node2, curr_delay, total_delay,  visited, temp_path, request); 
        } 
    
        // If total delay till now greater than previous delay then update it 
        if ((*total_delay) < curr_delay && return_path.size()!=0 && curr_delay<request.delay)
        { 
            if(return_path[return_path.size()-1].first==request.destination)
            {
                temp_path = return_path;
                *total_delay = curr_delay; 
            }
        }
        // make curr_delay = 0 for next adjacent node
        curr_delay = 0; 
    }
    return temp_path;
} 

struct path_info longestPath(struct Request request, vector<vector<struct LinkInfo>> graph, map<int, vector<int>> vnfNodes, vector<struct Node> local_nodes, string algo_name)
{
    // request has source, destination, NF{vector<pair<int, struct Resources>>}, throughput, delay 
    int V = graph.size();// Get the number of vertices in graph 
    int src = request.source;
    int dest = request.destination;
    int throughput = request.throughput;
    float delay = request.delay;

    float total_delay = FLT_MIN; 

    // initialize visited array with 0 
    vector< bool > visited(V, false); 

    vector<pair<int, int>> complete_path;
    vector<pair<int, int>> temp_path;

    // Call DFS for src vertex i 
    complete_path = DFS(graph, src, 0, &total_delay, visited, temp_path, request); 

    if(total_delay>delay || total_delay == FLT_MIN)
    {
        struct path_info selected_path_info;
        vector<pair<int, int>> temp;
        selected_path_info.path_with_type = temp;
        return selected_path_info;
    }

    if(complete_path.size()!=0 && complete_path[complete_path.size()-1].first!=dest)
    {
        struct path_info selected_path_info;
        vector<pair<int, int>> temp;
        selected_path_info.path_with_type = temp;
        return selected_path_info;   
    }

    struct path_info selected_path_info;
    selected_path_info.delay = total_delay;
    selected_path_info.path_with_type = complete_path;

    return selected_path_info;
}