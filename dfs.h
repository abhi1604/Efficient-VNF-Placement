#include<bits/stdc++.h> 
using namespace std; 
  

float max_delay = FLT_MIN;
vector<pair<int, int>> max_path;

// visited[] array to make nodes visited 
// src is starting node for DFS traversal 
// prev_delay is sum of delays till current node 
// max_delay is pointer which stores the maximum delay 
// value after DFS traversal 
void DFS(vector<vector<struct LinkInfo>> graph, int node, vector <bool> &visited, vector<pair<int,int>> &cur_path, struct Request request, float cur_path_delay)
{

    if(request.destination==node)
    {
        if(max_delay < cur_path_delay && cur_path_delay<=request.delay)   
        {
            max_delay = cur_path_delay;
            max_path = cur_path;
        }
        return;
    }

    float new_path_delay;

    for(auto x: graph[node])
    {
        if(!visited[x.node2])
        {
            visited[x.node2] = 1;
            if(x.node2==request.destination)
                cur_path.push_back(make_pair(x.node2, -2));
            else
                cur_path.push_back(make_pair(x.node2, -1));

            new_path_delay = cur_path_delay + x.delay;

            DFS(graph, x.node2, visited, cur_path, request, new_path_delay);

            visited[x.node2] = 0;

            cur_path.pop_back();
        }
    }
    return ;
} 

struct path_info longestPath(struct Request request, vector<vector<struct LinkInfo>> graph, map<int, vector<int>> vnfNodes, vector<struct Node> local_nodes, string algo_name)
{
    // request has source, destination, NF{vector<pair<int, struct Resources>>}, throughput, delay 
    int V = graph.size();// Get the number of vertices in graph 
    int src = request.source;
    int dest = request.destination;
    int throughput = request.throughput;
    float delay = request.delay;

    // initialize visited array with 0 
    vector< bool > visited(V, false); 

    vector<pair<int, int>> complete_path;
    vector<pair<int, int>> temp_path;

    temp_path.push_back(make_pair(src, -2));
    visited[src]=1;

    // Call DFS for src vertex i 
    DFS(graph, src, visited, temp_path, request, 0); 

    complete_path = max_path;

//     cout<<"complete_path: "<<request.source<<" "<<request.destination<<endl;
//     for(auto i:max_path)
//     {
//         cout<<i.first<<" "<<i.second<<endl;
//     }
// cout<<"---------------------------------------------\n";

    if(max_delay>delay || max_delay == FLT_MIN)
    {
        struct path_info selected_path_info;
        vector<pair<int, int>> temp;
        selected_path_info.path_with_type = temp;
        return selected_path_info;
    }

    struct path_info selected_path_info;
    selected_path_info.delay = max_delay;
    selected_path_info.path_with_type = complete_path;

    return selected_path_info;
}