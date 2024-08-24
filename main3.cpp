#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <iomanip> // For formatting output

using namespace std;
using namespace std::chrono;

// Struct to store path information
struct PathInfo {
    long long totalTime = 0; // Total time spent on this path
    int callCount = 0;       // Number of times this path was taken
};

// CallGraph class to store function calls and their relationships
class CallGraph {
public:
    void addCall(const string &caller, const string &callee) {
        callGraph[caller].push_back(callee);
    }

    void printGraph() const {
        ofstream graphFile("call_graph.txt");
        graphFile << "Call Graph:\n";
        for (const auto &entry : callGraph) {
            graphFile << entry.first << " calls: ";
            for (const auto &callee : entry.second) {
                graphFile << callee << " ";
            }
            graphFile << endl;
        }
        graphFile.close();
    }

    // Function to log paths and their time and call counts
    void logPaths() const {
        ofstream pathFile("path_profiles.txt");

        pathFile << left << setw(60) << "Path"
                 << setw(20) << "Total Time (µs)"
                 << setw(15) << "Call Count" << endl;

        pathFile << string(95, '-') << endl; // Horizontal separator

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

// Macro to simplify logging
#define LOG_CALL(graph) Logger log(__FUNCTION__, graph)

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
}

int main() {
    CallGraph callGraph;

    LOG_CALL(callGraph);

    functionD(callGraph);
    functionB(callGraph);

    callGraph.printGraph();
    callGraph.logPaths();

    return 0;
}



