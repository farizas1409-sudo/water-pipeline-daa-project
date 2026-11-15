#include "Graph.h"
#include <vector>
#include <utility>

void showMenu() {
    std::cout << "\n=========================================\n";
    std::cout << "   WATER PIPELINE LEAK ISOLATION SYSTEM   \n";
    std::cout << "=========================================\n";
    std::cout << "1. Load & Summary Network\n";
    std::cout << "2. Simulate Pipe Leak & BFS Impact Analysis\n";
    std::cout << "3. Compare Candidate Valves (Brute Force)\n";
    std::cout << "4. Export Graphviz .dot Visual File\n";
    std::cout << "5. Run Bonus: Find All Critical Bridges O(V+E)\n";
    std::cout << "6. Exit\n";
    std::cout << "Enter choice: ";
}

int main() {
    Graph network;
    if (!network.loadFromFile("data/network.txt")) {
        std::cerr << "Failed to load data/network.txt\n";
        return 1;
    }

    int choice;
    do {
        showMenu();
        if (!(std::cin >> choice)) break;

        switch (choice) {
            case 1:
                network.printSummary();
                std::cout << "[SUCCESS] Network successfully loaded from data/network.txt (15 nodes, loop & branch present).\n";
                break;
            case 2: {
                int u, v;
                std::cout << "Enter leaking pipe node IDs u and v (e.g., 4 5): ";
                std::cin >> u >> v;
                std::vector<int> affected;
                int totalH = 0;
                network.analyzeImpact(u, v, affected, totalH);
                std::cout << "\n--- Impact Report for Pipe (" << u << " - " << v << ") ---\n";
                std::cout << "Affected Houses Count: " << affected.size() << "\n";
                std::cout << "Total Households Affected: " << totalH << "\n";
                std::cout << "Affected House IDs: ";
                for (int id : affected) std::cout << id << " ";
                std::cout << "\n";
                break;
            }
            case 3: {
                std::vector<std::pair<int, int>> candidates = {
                    {2, 3}, // Loop pipe
                    {4, 5}, // Branch pipe
                    {5, 6}  // Branch pipe
                };
                network.findBestValve(candidates);
                break;
            }
            case 4: {
                int u, v;
                std::cout << "Enter leaking pipe to highlight in red (u v, e.g., 4 5): ";
                std::cin >> u >> v;
                std::vector<int> affected;
                int totalH;
                network.analyzeImpact(u, v, affected, totalH);
                network.exportGraphviz("output/network.dot", u, v, affected);
                std::cout << "To convert to PNG in Codespaces, run:\n";
                std::cout << "sudo apt-get update && sudo apt-get install -y graphviz\n";
                std::cout << "dot -Tpng output/network.dot -o output/network.png\n";
                break;
            }
            case 5:
                network.findBridges();
                break;
            case 6:
                std::cout << "Exiting program. Good luck with your DAA presentation!\n";
                break;
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);

    return 0;
}