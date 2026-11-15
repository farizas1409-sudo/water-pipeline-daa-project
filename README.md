# Water Pipeline Leak Isolation System
B.Tech 2nd Year DAA Final Project

## Overview
This project models municipal water distribution networks as mathematical graphs. Using Graph Theory and Breadth-First Search (BFS), the system isolates leaking pipes, computes affected households, evaluates optimal valve closures, and detects network redundancy loops.

## Compilation & Execution
```bash
# Compile
g++ -std=c++17 src/main.cpp -o pipeline

# Run interactive menu
./pipeline
```bash
   g++ -std=c++17 src/main.cpp -o pipeline
./pipeline
# Install graphviz tools
sudo apt-get update && sudo apt-get install -y graphviz

# Convert the generated dot file to png (after running option 4 in your program)
dot -Tpng output/network.dot -o output/network.png
cat << 'EOF' > src/Graph.h
#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

// Node types in our water network
enum NodeType {
    SOURCE = 0,
    JUNCTION = 1,
    HOUSE = 2
};

// Represents a single location in the network (Tank, Junction, or House)
struct Node {
    int id;
    NodeType type;
    std::string name;
    int households; // Number of households (0 for source and junctions)
};

class Graph {
private:
    std::map<int, Node> nodes;                  // Stores all nodes by their ID
    std::map<int, std::vector<int>> adjList;    // Adjacency list: neighbor connections

public:
    // Load network data from a text file
    // Time Complexity: O(V + E) where V = nodes, E = edges
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string line;
        bool readingNodes = false;
        bool readingEdges = false;

        while (std::getline(file, line)) {
            // Skip empty lines or comment lines starting with '#'
            if (line.empty() || line[0] == '#') continue;

            if (line == "NODES") {
                readingNodes = true;
                readingEdges = false;
                continue;
            } else if (line == "EDGES") {
                readingNodes = false;
                readingEdges = true;
                continue;
            }

            std::stringstream ss(line);
            if (readingNodes) {
                int id, typeInt, households;
                std::string name;
                if (ss >> id >> typeInt >> name >> households) {
                    Node n;
                    n.id = id;
                    n.type = static_cast<NodeType>(typeInt);
                    n.name = name;
                    n.households = households;
                    nodes[id] = n;
                }
            } else if (readingEdges) {
                int u, v;
                if (ss >> u >> v) {
                    // Undirected graph: add connection both ways
                    adjList[u].push_back(v);
                    adjList[v].push_back(u);
                }
            }
        }
        file.close();
        return true;
    }

    // Print summary of loaded network
    void printNetworkSummary() const {
        std::cout << "--- Water Network Summary ---\n";
        std::cout << "Total Nodes: " << nodes.size() << "\n";
        std::cout << "Connected Pipes (Edges): \n";
        for (const auto& pair : adjList) {
            std::cout << "Node " << pair.first << " (" << nodes.at(pair.first).name << ") connected to: ";
            for (int neighbor : pair.second) {
                std::cout << neighbor << " ";
            }
            std::cout << "\n";
        }
    }
};

#endif
