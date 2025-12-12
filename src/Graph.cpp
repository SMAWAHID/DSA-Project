#include "../../include/graph/Graph.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <iostream>

// Added: Use standard namespace
using namespace std;

// Adds a connection between 'from' and 'to' in both directions
void Graph::addEdge(const string& from, const string& to, int weight) {
    adjList[from].push_back({to, weight});
    adjList[to].push_back({from, weight});
}

// THE BIG ALGORITHM: Dijkstra's Shortest Path
// pair<vector<string>, int> Graph::dijkstra(const string& start, const string& end) const {
//     // If we don't know these rooms, give up immediately.
//     if (adjList.find(start) == adjList.end() || adjList.find(end) == adjList.end()) {
//         return {{}, -1};
//     }

//     // Scorecard: How far is each room? Start with "Infinity" for everyone.
//     map<string, int> distances;
//     // Breadcrumbs: To remember the path, we store "Who sent me here?"
//     map<string, string> predecessors;

//     // Priority Queue: A magic to-do list that always keeps the CLOSEST room at the top.
//     priority_queue<pair<int, string>,
//                    vector<pair<int, string>>,
//                    greater<pair<int, string>>> pq;

//     for (const auto& pair : adjList) {
//         distances[pair.first] = numeric_limits<int>::max();
//     }

//     // Distance to self is 0.
//     distances[start] = 0;
//     pq.push({0, start});

//     while (!pq.empty()) {
//         // Get the closest room from the list
//         int current_dist = pq.top().first;
//         string u = pq.top().second;
//         pq.pop();

//         // If we found a faster way to this room already, skip this stale entry
//         if (current_dist > distances[u]) {
//             continue;
//         }

//         // If we reached the destination, stop!
//         if (u == end) break;

//         // Check all neighbors
//         for (const auto& edge : adjList.at(u)) {
//             string v = edge.first;
//             int weight = edge.second;

//             // If going through 'u' is faster than the old way to 'v'...
//             if (distances.at(u) + weight < distances.at(v)) {
//                 distances[v] = distances.at(u) + weight; // Update score
//                 predecessors[v] = u; // Drop a breadcrumb
//                 pq.push({distances[v], v}); // Add to to-do list
//             }
//         }
//     }

//     // If destination is still at Infinity distance, there is no path.
//     if (distances.at(end) == numeric_limits<int>::max()) {
//         return {{}, -1};
//     }

//     // Reconstruct the path by following breadcrumbs backwards from End to Start.
//     vector<string> path;
//     string current = end;
//     while (current != start) {
//         path.push_back(current);
//         if (predecessors.find(current) == predecessors.end()) {
//             return {{}, -1};
//         }
//         current = predecessors[current];
//     }
//     path.push_back(start);
//     reverse(path.begin(), path.end()); // Flip it so it goes Start -> End

//     return {path, distances.at(end)};
// }

// // Simple Breadth-First Search
// vector<string> Graph::bfs(const string& start, const string& end) const {
//     if (adjList.find(start) == adjList.end() || adjList.find(end) == adjList.end()) {
//         return {};
//     }

//     queue<string> q;
//     map<string, string> predecessors;
//     map<string, bool> visited;

//     q.push(start);
//     visited[start] = true;

//     bool found = false;
//     while (!q.empty()) {
//         string u = q.front();
//         q.pop();

//         if (u == end) {
//             found = true;
//             break;
//         }

//         for (const auto& edge : adjList.at(u)) {
//             string v = edge.first;
//             if (!visited[v]) {
//                 visited[v] = true;
//                 predecessors[v] = u;
//                 q.push(v);
//             }
//         }
//     }

//     if (!found) {
//         return {};
//     }

//     vector<string> path;
//     string current = end;
//     while (current != start) {
//         path.push_back(current);
//         current = predecessors[current];
//     }
//     path.push_back(start);
//     reverse(path.begin(), path.end());

//     return path;
// }

vector<string> Graph::getNodes() const {
    vector<string> nodes;
    for (const auto& pair : adjList) {
        nodes.push_back(pair.first);
    }
    sort(nodes.begin(), nodes.end());
    return nodes;
}

const map<string, vector<pair<string, int>>>& Graph::getGraphData() const {
    return adjList;
}
