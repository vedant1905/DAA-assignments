#include <iostream>
#include <vector>
#include <queue>      // For priority_queue
#include <limits>     // For INT_MAX
#include <algorithm>  // For fill, find
#include <numeric>    // For iota

using namespace std;

const int INF = numeric_limits<int>::max();
int N; // Number of cities

// Node represents a state in the state space tree
struct Node {
    vector<vector<int>> mat; // Reduced cost matrix for this node
    vector<int> path;        // Path taken so far
    int cost;                // Cost of the path so far
    int lb;                  // Lower bound cost for this node
    int level;               // Number of cities visited (level in the tree)
    int curr_city;           // The current city this node is at
};

// Custom comparator for the min-priority queue
struct CompareNode {
    bool operator()(const Node& a, const Node& b) {
        return a.lb > b.lb;
    }
};

// Performs row and column reduction on the matrix
// Modifies the matrix in-place and returns the total reduction cost
int reduce_matrix(vector<vector<int>>& mat) {
    int reduction = 0;

    // 1. Row Reduction
    for (int i = 0; i < N; ++i) {
        int min_val = INF;
        for (int j = 0; j < N; ++j) {
            if (mat[i][j] < min_val) {
                min_val = mat[i][j];
            }
        }
        
        if (min_val != 0 && min_val != INF) {
            for (int j = 0; j < N; ++j) {
                if (mat[i][j] != INF) {
                    mat[i][j] -= min_val;
                }
            }
            reduction += min_val;
        }
    }

    // 2. Column Reduction
    for (int j = 0; j < N; ++j) {
        int min_val = INF;
        for (int i = 0; i < N; ++i) {
            if (mat[i][j] < min_val) {
                min_val = mat[i][j];
            }
        }
        
        if (min_val != 0 && min_val != INF) {
            for (int i = 0; i < N; ++i) {
                if (mat[i][j] != INF) {
                    mat[i][j] -= min_val;
                }
            }
            reduction += min_val;
        }
    }
    
    return reduction;
}

// Helper function to check if a city is already in the path
bool isVisited(const vector<int>& path, int city) {
    for (int p : path) {
        if (p == city) return true;
    }
    return false;
}

int main() {
    cout << "Enter the number of cities (N): ";
    cin >> N;
    
    vector<vector<int>> start_mat(N, vector<int>(N));
    cout << "Enter the cost matrix (use -1 or a large number for INF):\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cin >> start_mat[i][j];
            if (start_mat[i][j] == -1) {
                start_mat[i][j] = INF;
            }
        }
    }

    // Priority queue to store live nodes (best-first)
    priority_queue<Node, vector<Node>, CompareNode> pq;

    // --- Create the Root Node ---
    Node root;
    root.mat = start_mat;
    root.path.push_back(0); // Start at city 0
    root.cost = 0;
    root.level = 0;
    root.curr_city = 0;
    root.lb = reduce_matrix(root.mat); // Initial lower bound
    
    pq.push(root);

    int bestCost = INF;
    vector<int> bestPath;

    // --- Main LC Branch & Bound Loop ---
    while (!pq.empty()) {
        Node node = pq.top();
        pq.pop();

        // --- Pruning Step ---
        // If this node's LB is already worse than our best, prune it.
        if (node.lb >= bestCost) {
            continue;
        }

        // --- Goal Check ---
        // If we have visited N cities (level N-1)
        if (node.level == N - 1) {
            // This is a complete tour.
            // Add cost of returning to start city.
            if (node.mat[node.curr_city][0] != INF) {
                int final_cost = node.cost + node.mat[node.curr_city][0];
            
                if (final_cost < bestCost) {
                    bestCost = final_cost;
                    bestPath = node.path;
                    bestPath.push_back(0); // Add return to start
                }
            }
            // Since this is a leaf, we don't branch further.
            continue; 
        }

        // --- Branching Step ---
        // For each possible next city 'v'
        for (int v = 0; v < N; ++v) {
            // Check if city 'v' is not visited and there is a path
            if (!isVisited(node.path, v) && node.mat[node.curr_city][v] != INF) {
                
                // Create the child node for path u -> v
                Node child;
                child.mat = node.mat;
                child.path = node.path;
                child.path.push_back(v);
                child.level = node.level + 1;
                child.curr_city = v;

                int u = node.curr_city; // Parent city

                // Cost of path so far
                child.cost = node.cost + node.mat[u][v];

                // --- Prepare Child's Reduced Matrix ---
                // Set row 'u', col 'v', and 'v->start' to INF
                for (int j = 0; j < N; ++j) {
                    child.mat[u][j] = INF; // Set row u to INF
                }
                for (int i = 0; i < N; ++i) {
                    child.mat[i][v] = INF; // Set col v to INF
                }
                child.mat[v][0] = INF; // Set v -> start_city to INF

                // Get new reduction cost
                int reduction = reduce_matrix(child.mat);
                
                // --- Calculate Child's Lower Bound ---
                // Cost so far + LB of remaining problem
                child.lb = child.cost + reduction;

                // --- Push Child if Promising ---
                if (child.lb < bestCost) {
                    pq.push(child);
                }
            }
        }
    }

    // --- Final Output ---
    if (bestCost == INF) {
        cout << "No feasible tour found." << endl;
    } else {
        cout << "Minimum Cost: " << bestCost << endl;
        cout << "Optimal Path: ";
        for (size_t i = 0; i < bestPath.size(); ++i) {
            cout << bestPath[i];
            if (i < bestPath.size() - 1) cout << " -> ";
        }
        cout << endl;
    }

    return 0;
}
