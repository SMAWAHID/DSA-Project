#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <map>
#include <utility>

// This class handles the math of the map.
class Graph {
public:
    // Connects two rooms (nodes) with a specific distance (weight).
    void addEdge(const std::string& from, const std::string& to, int weight);

    // The "GPS" function. Finds the fastest path from Start to End.
    std::pair<std::vector<std::string>, int> dijkstra(const std::string& start, const std::string& end) const;

    // A simpler search (Breadth-First Search).
    std::vector<std::string> bfs(const std::string& start, const std::string& end) const;

    // Returns a list of every single room name we know about.
    std::vector<std::string> getNodes() const;

    // Lets the GUI see the raw connection data to draw lines on screen.
    const std::map<std::string, std::vector<std::pair<std::string, int>>>& getGraphData() const;

private:
    // The "Adjacency List". The core memory of the map.
    std::map<std::string, std::vector<std::pair<std::string, int>>> adjList;
};

#endif // GRAPH_H
