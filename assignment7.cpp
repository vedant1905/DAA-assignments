#include <iostream>     // For cin, cout, ios::sync_with_stdio, cin.tie
#include <vector>       // For vector
#include <string>       // For string, getline
#include <unordered_map>  // For unordered_map
#include <unordered_set>  // For unordered_set
#include <algorithm>    // For sort, max, unique
#include <numeric>      // For iota
#include <iomanip>      // For setw (if needed, though not in final output)

using namespace std;

int main() {
    // Optimize C++ stream I/O for speed
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // --- 1. Read Courses ---
    int numCourses;
    if (!(cin >> numCourses)) return 0; 

    vector<string> courseNames(numCourses);
    unordered_map<string, int> courseIndex;
    for (int i = 0; i < numCourses; i++) {
        cin >> ws; // Consume trailing newline
        getline(cin, courseNames[i]);
        courseIndex[courseNames[i]] = i;
    }

    // --- 2. Build Conflict Graph ---
    int numStudents;
    cin >> numStudents;
    
    vector<int> studentsIn(numCourses, 0); // Count students per course
    vector<vector<int>> adj(numCourses);   // Adjacency list for conflicts
    
    for (int si = 0; si < numStudents; si++) {
        string sid;
        int k; // Number of courses for this student
        cin >> ws;
        cin >> sid >> k;
        
        vector<int> enrolled;
        for (int j = 0; j < k; j++) {
            string cname;
            cin >> ws;
            getline(cin, cname);
            if (courseIndex.find(cname) == courseIndex.end()) continue;
            
            int idx = courseIndex[cname];
            enrolled.push_back(idx);
            studentsIn[idx]++;
        }

        // Add edges between all pairs of courses this student is enrolled in
        sort(enrolled.begin(), enrolled.end());
        enrolled.erase(unique(enrolled.begin(), enrolled.end()), enrolled.end());

        for (size_t a = 0; a < enrolled.size(); a++) {
            for (size_t b = a + 1; b < enrolled.size(); b++) {
                int u = enrolled[a], v = enrolled[b];
                adj[u].push_back(v);
                adj[v].push_back(u);
            }
        }
    }

    // Clean up adjacency lists (remove duplicate edges)
    vector<int> degree(numCourses);
    for (int i = 0; i < numCourses; i++) {
        sort(adj[i].begin(), adj[i].end());
        adj[i].erase(unique(adj[i].begin(), adj[i].end()), adj[i].end());
        degree[i] = adj[i].size();
    }

    // --- 3. Graph Coloring (Welsh-Powell Heuristic) ---
    // Create an 'order' vector [0, 1, 2, ..., C-1]
    vector<int> order(numCourses);
    iota(order.begin(), order.end(), 0); 

    // Sort 'order' vector based on degree (descending)
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (degree[a] != degree[b]) return degree[a] > degree[b];
        return a < b;
    });

    vector<int> color(numCourses, -1); // -1 = uncolored
    int maxColor = 0;
    
    for (int idx : order) {
        vector<char> used(numCourses + 1, false); // Tracks used colors by neighbors
        
        for (int v : adj[idx]) {
            if (color[v] != -1) {
                used[color[v]] = true;
            }
        }

        // Find the smallest available color (1-indexed)
        int c = 1;
        while (used[c]) c++;
        
        color[idx] = c;
        maxColor = max(maxColor, c); 
    }

    // --- 4. Room Allocation ---
    int numRooms;
    cin >> numRooms; 
    
    vector<pair<string, int>> rooms(numRooms);
    for (int i = 0; i < numRooms; i++) {
        string rname; 
        int cap;
        cin >> ws;
        getline(cin, rname);
        cin >> cap;
        rooms[i] = {rname, cap};
    }

    // Get order of rooms, sorted by capacity (ascending)
    vector<int> roomOrder(numRooms);
    iota(roomOrder.begin(), roomOrder.end(), 0); 
    sort(roomOrder.begin(), roomOrder.end(), [&](int a, int b) {
        if (rooms[a].second != rooms[b].second) {
            return rooms[a].second < rooms[b].second; 
        }
        return a < b;
    });

    // Tracks which room (by index) is used in which slot
    vector<unordered_set<int>> roomOccupied(maxColor + 1);
    vector<string> assignedRoom(numCourses, "Unassigned");

    // Assign rooms for each time slot
    for (int t = 1; t <= maxColor; t++) { 
        vector<int> exams;
        for (int i = 0; i < numCourses; i++) {
            if (color[i] == t) exams.push_back(i); 
        }

        // Sort exams in this slot by student count (descending)
        sort(exams.begin(), exams.end(), [&](int a, int b) {
            if (studentsIn[a] != studentsIn[b]) {
                return studentsIn[a] > studentsIn[b];
            }
            return a < b;
        });

        // Assign rooms using best-fit
        for (int e : exams) {
            for (int ri : roomOrder) {
                // If room fits and is not occupied in this slot
                if (rooms[ri].second >= studentsIn[e] && 
                    roomOccupied[t].count(ri) == 0) { 
                    
                    assignedRoom[e] = rooms[ri].first;
                    roomOccupied[t].insert(ri);
                    break;
                }
            }
        }
    }

    // --- 5. Display Final Timetable ---
    cout << "Course\tTimeSlot\tRoom\n";
    for (int i = 0; i < numCourses; i++) {
        cout << courseNames[i] << "\tSlot " << color[i] << "\t" << assignedRoom[i] << "\n";
    }

    return 0;
}
