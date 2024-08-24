#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

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

private:
    map<string, vector<string>> callGraph;
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
        logFile.close();

        // Store path profile
        ofstream pathFile("path_profiles.txt", ios_base::app);
        for (const string &func : callStack) {
            pathFile << func << " -> ";
        }
        pathFile << funcName << " (Execution Time: " << duration << " µs)\n";
        pathFile.close();
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

    return 0;
}

