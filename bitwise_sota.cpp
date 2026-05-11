#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <bitset>
#include <algorithm>
#include <chrono>
#include <sys/resource.h>

using namespace std;

// Allocate enough memory for the largest FIMI dataset (Accidents has ~340k transactions)
const int MAX_TRANSACTIONS = 400000; 

// The Linear Table: Maps an Item ID to a contiguous binary array representing transactions
map<int, bitset<MAX_TRANSACTIONS>> verticalData;

// Config
double MIN_SUP_RATIO = 0.90;
int min_sup_count = 0;
int totalTransactions = 0;

// Type alias: An itemset and its corresponding bitset
using Itemset = vector<int>;
struct Node {
    Itemset items;
    bitset<MAX_TRANSACTIONS> bitmap;
};

// Load Data into Vertical Bitmaps 
void loadDataset(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[FATAL] Missing dataset! File not found: " << filename << endl;
        exit(1);
    }

    string line;
    int tid = 0;
    while (getline(file, line) && tid < MAX_TRANSACTIONS) {
        stringstream ss(line);
        int item;
        while (ss >> item) {
            verticalData[item].set(tid);
        }
        tid++;
    }
    totalTransactions = tid;
    min_sup_count = totalTransactions * MIN_SUP_RATIO;
    
    cout << "[SYSTEM] Loaded " << totalTransactions << " transactions into Vertical Bitmaps." << endl;
    cout << "[SYSTEM] Min Support threshold: " << min_sup_count << " occurrences." << endl;
}

// Recursive Mining Function using Bitwise AND
void mineBitwise(vector<Node>& currentLevel, int k, int& total_found) {
    if (currentLevel.empty()) return;
    
    vector<Node> nextLevel;
    
    for (size_t i = 0; i < currentLevel.size(); ++i) {
        for (size_t j = i + 1; j < currentLevel.size(); ++j) {
            // Check if prefixes match
            bool prefixMatch = true;
            for (int p = 0; p < k - 2; ++p) { 
                // k - 2 because combining 1 item datasets count for 2 prefixes
                if (currentLevel[i].items[p] != currentLevel[j].items[p]) {
                    prefixMatch = false;
                    break;
                }
            }
            
            if (prefixMatch) {
                // Calculate support by ANDing the two bitsets together. 
                bitset<MAX_TRANSACTIONS> intersection = currentLevel[i].bitmap & currentLevel[j].bitmap;
                
                int support = intersection.count();
                if (support >= min_sup_count) {
                    Itemset newItemset = currentLevel[i].items;
                    newItemset.push_back(currentLevel[j].items.back()); // Append the differing item
                    
                    nextLevel.push_back({newItemset, intersection});
                    total_found++;
                }
            }
        }
    }
    
    if (!nextLevel.empty()) {
        cout << "[LOG] Found " << nextLevel.size() << " frequent " << k << "-itemsets using Bitwise AND." << endl;
        mineBitwise(nextLevel, k + 1, total_found);
    }
}

int main() {
    cout << "=== LINEAR TABLE BITWISE FIM (2022 SOTA) ===" << endl;
    
    // CHANGE THIS FILE TO MATCH DATASETS
    loadDataset("chess.dat"); 

    // Start algorithm timing
    auto start_time = chrono::high_resolution_clock::now();

    // Generate L1 (Frequent 1-itemsets)
    vector<Node> L1;
    for (auto const& [item, bitmap] : verticalData) {
        int support = bitmap.count();
        if (support >= min_sup_count) {
            L1.push_back({{item}, bitmap});
        }
    }
    
    int total_frequent_itemsets = L1.size();
    cout << "[LOG] Found " << L1.size() << " frequent 1-itemsets." << endl;

    // Start recursive bitwise mining
    mineBitwise(L1, 2, total_frequent_itemsets);

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> execution_time = end_time - start_time;

    cout << "\n=== RESULTS ===" << endl;
    cout << "Total Frequent Itemsets: " << total_frequent_itemsets << endl;
    cout << "Execution Time: " << execution_time.count() << " seconds." << endl;
    
    // --- RAM USAGE TRACKING ---
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    // ru_maxrss is in kilobytes on Linux/Mac. Divide by 1024 to get Megabytes.
    double peak_ram_mb = usage.ru_maxrss / 1024.0;
    cout << "Peak RAM Usage: " << peak_ram_mb << " MB" << endl;

    return 0;
}