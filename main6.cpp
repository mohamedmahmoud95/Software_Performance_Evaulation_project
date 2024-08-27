#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <iomanip>
#include <set>
#include <cstdlib>

#define LOG_CALL(graph) Logger log(__func__, graph)

using namespace std;
using namespace std::chrono;



struct PathInfo {
    long long totalTime = 0;
    int callCount = 0;
};

class CallGraph {
public:
    void addCall(const string &caller, const string &callee) {
        callGraph[caller].push_back(callee);
    }

    // Original printGraph function to output the call graph in text format
    void printGraph() const {
        ofstream graphFile("call_graph.txt");

        graphFile << "Call Graph Tree:\n";
        set<string> visited;  // Set to track visited nodes and prevent infinite recursion

        // Start with the top-level calls (those that are never called by others)
        for (const auto &entry : callGraph) {
            if (visited.find(entry.first) == visited.end()) {
                printGraphHelper(entry.first, visited, 0, graphFile);
            }
        }

        graphFile.close();
    }

    // Helper function for printGraph
    void printGraphHelper(const string &node, set<string> &visited, int depth, ofstream &graphFile) const {
        if (visited.find(node) != visited.end()) {
            return;  // Prevent infinite recursion in case of cycles
        }
        visited.insert(node);

        // Print the current node with indentation based on the depth in the call hierarchy
        graphFile << string(depth * 2, ' ') << node << endl;

        // Recursively print each child node
        auto it = callGraph.find(node);
        if (it != callGraph.end()) {
            for (const string &callee : it->second) {
                printGraphHelper(callee, visited, depth + 1, graphFile);
            }
        }
    }

    // New method to generate a DOT file for Graphviz visualization


    void generateDotFile(bool isDynamicCallTree, const string &dotFilename, const string &pngFilename) const {
        for (const auto &entry : callGraph) {
            cout << "Key: " << entry.first << ", Value: ";

            // Iterate through the vector stored as the value in the map
            for (const auto &str : entry.second) {
                cout << str << " ";
            }

            cout << endl;
        }
        ofstream dotFile(dotFilename);

        if (!dotFile) {
            cerr << "Error: Could not open the file " << dotFilename << " for writing." << endl;
            return;
        }

        dotFile << "digraph CallGraph {\n";
        dotFile << "    bgcolor=\"lightgray\";\n";
        dotFile << "    node [style=filled, color=lightblue, shape=oval, fontname=\"Arial\"];\n";
        dotFile << "    edge [fontname=\"Arial\", fontsize=10];\n";

        if (isDynamicCallTree) {
            for (const auto &entry : callGraph) {
                const string &caller = entry.first;

                for (const string &callee : entry.second) {
                    // Create multiple nodes with the same name for each call
                    static int instanceCounter = 1;  // Counter to differentiate nodes internally
                    string calleeNodeName = callee + to_string(instanceCounter++);

                    // Add the edge from the caller to this callee node
                    dotFile << "    \"" << caller << "\" -> \"" << calleeNodeName << "\";\n";

                    // Declare the callee node with the same label as the function name
                    dotFile << "    \"" << calleeNodeName << "\" [label=\"" << callee << "\"];\n";
                }
            }
        } else {
            map<pair<string, string>, int> callCounts;

            for (const auto &entry : callGraph) {
                for (const auto &callee : entry.second) {
                    callCounts[{entry.first, callee}]++;
                }
            }

            for (const auto &entry : callCounts) {
                dotFile << "    \"" << entry.first.first << "\" -> \"" << entry.first.second << "\" [label=\"" << entry.second << "\"];\n";
            }
        }

        dotFile << "}\n";
        dotFile.close();

        cout << "DOT file created successfully: " << dotFilename << endl;

        // Run Graphviz to create the PNG file
        string command = "dot -Tpng \"" + dotFilename + "\" -o \"" + pngFilename + "\"";
        int result = system(command.c_str());

        if (result == 0) {
            cout << "PNG file created successfully: " << pngFilename << endl;
        } else {
            cerr << "Error: Failed to generate PNG file. Please check if Graphviz is installed and accessible." << endl;
        }
    }






    // Function to log paths and their time and call counts
    void logPaths() const {
        ofstream pathFile("path_profiles.txt");

        pathFile << left << setw(60) << "Path"
                 << setw(20) << "Total Time (µs)"
                 << setw(15) << "Call Count" << endl;

        pathFile << string(95, '-') << endl;

        for (const auto &entry : pathProfiles) {
            pathFile << left << setw(60) << entry.first
                     << setw(20) << entry.second.totalTime
                     << setw(15) << entry.second.callCount << endl;
        }
        pathFile.close();
    }

    void updatePathProfile(const string &path, long long duration) {
        pathProfiles[path].totalTime += duration;
        pathProfiles[path].callCount += 1;
    }

private:
    map<string, vector<string>> callGraph;
    map<string, PathInfo> pathProfiles;
};

// Logger class to log function entry, exit, and timing
class Logger {
public:
    Logger(const string &functionName, CallGraph &graph)
            : funcName(functionName), callGraph(graph) {

        startTime = high_resolution_clock::now();
        logFile.open("event_log.txt", ios_base::app);
        logFile << "Entering " << funcName << "\n";

        if (!callStack.empty()) {
            callGraph.addCall(callStack.back(), funcName);
        }
        callStack.push_back(funcName);
    }

    ~Logger() {
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(endTime - startTime).count();

        logFile << "Exiting " << funcName << " (Execution Time: " << duration << " µs)\n";
        callStack.pop_back();

        // Construct the path string
        string path;
        for (const string &func : callStack) {
            path += func + " -> ";
        }
        path += funcName;

        // Update path profile with timing and call count
        callGraph.updatePathProfile(path, duration);

        logFile.close();
    }

private:
    string funcName;
    CallGraph &callGraph;
    ofstream logFile;
    static vector<string> callStack;
    high_resolution_clock::time_point startTime;
};

vector<string> Logger::callStack;

void functionD(CallGraph &graph);
void functionC(CallGraph &graph);
void functionA(CallGraph &graph);
void functionB(CallGraph &graph);
// Sample functions to demonstrate logging
void functionC(CallGraph &graph);

void functionA(CallGraph &graph) {
    LOG_CALL(graph);
    cout << "Inside functionA\n";
    functionC(graph);

}

void functionB(CallGraph &graph) {
    LOG_CALL(graph);
    cout << "Inside functionB\n";
    functionA(graph);
    functionC(graph);
    functionC(graph);

}

void functionC(CallGraph &graph) {
    LOG_CALL(graph);
    cout << "Inside functionC\n";
}

void functionD(CallGraph &graph) {
    LOG_CALL(graph);
    cout << "Inside functionD\n";
    functionA(graph);
    functionB(graph);
    functionA(graph);
    functionB(graph);
}

int main() {
    CallGraph callGraph;

    LOG_CALL(callGraph);

    functionD(callGraph);

    callGraph.printGraph();

    // Generate dynamic call graph
    callGraph.generateDotFile(true, "dynamic_call_graph.dot", "dynamic_call_graph.png");

    // Generate call context tree
    callGraph.generateDotFile(false, "call_context_tree.dot", "call_context_tree.png");
    // Iterate through the map

    callGraph.logPaths();

    return 0;
}
