#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <algorithm>

enum NodeType {
    SOURCE = 0,
    JUNCTION = 1,
    HOUSE = 2
};

struct Node {
    int id;
    NodeType type;
    std::string name;
    int households;
};

class Graph {
private:
    std::map<int, Node> nodes;
    std::map<int, std::vector<int>> adjList;

    // Helper DFS for finding bridges (Bonus Module)
    void dfsBridge(int u, int p, int& timer, 
                   std::map<int, int>& tin, std::map<int, int>& low, 
                   std::set<int>& visited, std::vector<std::pair<int, int>>& bridges) const {
        visited.insert(u);
        tin[u] = low[u] = timer++;

        for (int v : adjList.at(u)) {
            if (v == p) continue;
            if (visited.count(v)) {
                low[u] = std::min(low[u], tin[v]);
            } else {
                dfsBridge(v, u, timer, tin, low, visited, bridges);
                low[u] = std::min(low[u], low[v]);
                if (low[v] > tin[u]) {
                    bridges.push_back({u, v});
                }
            }
        }
    }

public:
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string line;
        bool readingNodes = false;
        bool readingEdges = false;

        nodes.clear();
        adjList.clear();

        while (std::getline(file, line)) {
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
                    adjList[u].push_back(v);
                    adjList[v].push_back(u);
                }
            }
        }
        file.close();
        return true;
    }

    // Module 2: BFS Impact Analysis O(V + E)
    void analyzeImpact(int closedU, int closedV, std::vector<int>& affectedHousesOut, int& totalHouseholdsOut) const {
        affectedHousesOut.clear();
        totalHouseholdsOut = 0;

        int sourceId = -1;
        for (const auto& pair : nodes) {
            if (pair.second.type == SOURCE) {
                sourceId = pair.first;
                break;
            }
        }

        if (sourceId == -1) return;

        std::set<int> visited;
        std::queue<int> q;

        q.push(sourceId);
        visited.insert(sourceId);

        while (!q.empty()) {
            int curr = q.front();
            q.pop();

            for (int neighbor : adjList.at(curr)) {
                if ((curr == closedU && neighbor == closedV) || (curr == closedV && neighbor == closedU)) {
                    continue; // Skip closed pipe
                }

                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }

        for (const auto& pair : nodes) {
            int nodeId = pair.first;
            const Node& node = pair.second;

            if (node.type == HOUSE) {
                if (visited.find(nodeId) == visited.end()) {
                    affectedHousesOut.push_back(nodeId);
                    totalHouseholdsOut += node.households;
                }
            }
        }
    }

    // Module 3 & 4: Best Valve to Close & Redundancy Check O(K * (V + E))
    void findBestValve(const std::vector<std::pair<int, int>>& candidates) const {
        if (candidates.empty()) {
            std::cout << "No candidate valves provided.\n";
            return;
        }

        int bestU = -1, bestV = -1;
        int minAffectedHouseholds = 999999;
        bool foundValid = false;

        std::cout << "\n=== EVALUATING CANDIDATE VALVES NEAR LEAK ===\n";
        for (const auto& pipe : candidates) {
            int u = pipe.first;
            int v = pipe.second;

            std::vector<int> affectedHouses;
            int totalHouseholds = 0;

            analyzeImpact(u, v, affectedHouses, totalHouseholds);

            std::cout << "Pipe (" << u << " - " << v << "): ";
            if (totalHouseholds == 0) {
                std::cout << "[REDUNDANT] Loop detected! Alternate path exists (0 affected).\n";
            } else {
                std::cout << "[BRANCH] Cuts off " << affectedHouses.size() << " houses (" << totalHouseholds << " families).\n";
            }

            if (totalHouseholds < minAffectedHouseholds) {
                minAffectedHouseholds = totalHouseholds;
                bestU = u;
                bestV = v;
                foundValid = true;
            }
        }

        if (foundValid) {
            std::cout << "\nRECOMMENDATION: Close pipe (" << bestU << " - " << bestV 
                      << ") as it minimizes disruption (" << minAffectedHouseholds << " households affected).\n";
        }
    }

    // Module 5: Export Graphviz .dot file
    void exportGraphviz(const std::string& filename, int closedU, int closedV, const std::vector<int>& affectedHouses) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create dot file " << filename << std::endl;
            return;
        }

        file << "graph WaterNetwork {\n";
        file << "    node [fontname=\"Arial\", style=filled];\n";

        for (const auto& pair : nodes) {
            int id = pair.first;
            const Node& node = pair.second;
            std::string color = "white";

            if (node.type == SOURCE) color = "lightblue";
            else if (node.type == HOUSE) {
                if (std::find(affectedHouses.begin(), affectedHouses.end(), id) != affectedHouses.end()) {
                    color = "lightgray"; // Affected / isolated
                } else {
                    color = "lightgreen"; // Normal / supplied
                }
            } else if (node.type == JUNCTION) {
                color = "lightyellow";
            }

            file << "    " << id << " [label=\"" << node.name << "\\n(" << (node.type==HOUSE ? std::to_string(node.households) + " hhold" : "J/T") << ")\", fillcolor=\"" << color << "\"];\n";
        }

        // Output edges
        std::set<std::pair<int, int>> drawnEdges;
        for (const auto& pair : adjList) {
            int u = pair.first;
            for (int v : pair.second) {
                if (u < v) { // prevent duplicate undirected edges
                    std::string color = "black";
                    std::string style = "solid";
                    std::string constraint = "";

                    if ((u == closedU && v == closedV) || (u == closedV && v == closedU)) {
                        color = "red";
                        style = "dashed";
                        constraint = " [color=red, style=dashed, penwidth=3.0]";
                    }

                    file << "    " << u << " -- " << v << constraint << ";\n";
                }
            }
        }

        file << "}\n";
        file.close();
        std::cout << "Successfully exported Graphviz file to " << filename << "\n";
    }

    // BONUS Module 7: Find All Network Bridges in O(V + E) using Tarjan's Low-Link DFS
    void findBridges() const {
        int timer = 0;
        std::map<int, int> tin, low;
        std::set<int> visited;
        std::vector<std::pair<int, int>> bridges;

        for (const auto& pair : nodes) {
            int u = pair.first;
            if (!visited.count(u)) {
                dfsBridge(u, -1, timer, tin, low, visited, bridges);
            }
        }

        std::cout << "\n=== BONUS MODULE: CRITICAL SYSTEM BRIDGES O(V+E) ===\n";
        std::cout << "Total Critical Bridges Found: " << bridges.size() << "\n";
        for (const auto& b : bridges) {
            std::cout << "Bridge Pipe (" << b.first << " - " << b.second << ") -> Failure splits the network!\n";
        }
    }

    void printSummary() const {
        std::cout << "Loaded Network with " << nodes.size() << " nodes.\n";
    }
};

#endif
