#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <chrono>
#include <sys/resource.h>

using namespace std;

// The database: A list of transactions, where each transaction is a set of item IDs
vector<set<int>> transactions;

// Config
double MIN_SUP_RATIO = 0.70;
int min_sup_count = 0;

// Type alias for readability
using Itemset = set<int>;

// Loading the FIMI Dataset (Horizontal Format)
void loadDataset(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[ERROR] Could not open " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        int item;
        set<int> transaction;
        while (ss >> item) {
            transaction.insert(item);
        }
        if (!transaction.empty()) {
            transactions.push_back(transaction);
        }
    }
    
    min_sup_count = transactions.size() * MIN_SUP_RATIO;
    cout << "[SYSTEM] Loaded " << transactions.size() << " transactions." << endl;
    cout << "[SYSTEM] Min Support Count set to: " << min_sup_count << endl;
}

// Generate initial L1 (Frequent 1-itemsets)
map<Itemset, int> getFrequent1Itemsets() {
    map<int, int> itemCounts;
    for (const auto& t : transactions) {
        for (int item : t) {
            itemCounts[item]++;
        }
    }

    map<Itemset, int> L1;
    for (const auto& pair : itemCounts) {
        if (pair.second >= min_sup_count) {
            L1[{pair.first}] = pair.second; // Itemset, Support Count
        }
    }
    return L1;
}

// Generate Candidate k-itemsets (Ck) from L(k-1)
vector<Itemset> generateCandidates(const map<Itemset, int>& L_prev, int k) {
    vector<Itemset> candidates;
    vector<Itemset> prev_itemsets;
    for (const auto& pair : L_prev) {
        prev_itemsets.push_back(pair.first); // Make identical to L_prev Itemsets
    }

    for (size_t i = 0; i < prev_itemsets.size(); ++i) {
        for (size_t j = i + 1; j < prev_itemsets.size(); ++j) {
            Itemset c = prev_itemsets[i];
            c.insert(prev_itemsets[j].begin(), prev_itemsets[j].end());
            
            // Only keep if the union results in exactly size k
            if (c.size() == k) {
                candidates.push_back(c);
            }
        }
    }
    
    // Remove duplicates
    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());
    
    return candidates;
}

// Filter Candidates to get Lk
map<Itemset, int> filterCandidates(const vector<Itemset>& candidates) {
    map<Itemset, int> Lk;
    vector<int> counts(candidates.size(), 0);

    // Scan the entire database to count support 
    for (const auto& t : transactions) {
        for (size_t i = 0; i < candidates.size(); ++i) {
            // Check if candidate is a subset of the transaction
            if (includes(t.begin(), t.end(), candidates[i].begin(), candidates[i].end())) {
                counts[i]++; // increase its support count
            }
        }
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        if (counts[i] >= min_sup_count) {
            Lk[candidates[i]] = counts[i];
        }
    }
    return Lk;
}

int main() {
    cout << "=== APRIORI BASELINE (CLASSIC) ===" << endl;
    
    // CHANGE THIS FILE TO MATCH DATASETS
    loadDataset("chess.dat"); 

    // Algorithm timing start
    auto start_time = chrono::high_resolution_clock::now();

    // Run Apriori
    int total_frequent_itemsets = 0;
    map<Itemset, int> L = getFrequent1Itemsets();
    int k = 2;

    while (!L.empty()) {
        cout << "[LOG] Found " << L.size() << " frequent " << k - 1 << "-itemsets." << endl;
        total_frequent_itemsets += L.size();
        
        vector<Itemset> Ck = generateCandidates(L, k);
        if (Ck.empty()) break;
        
        L = filterCandidates(Ck);
        k++;
    }

    // Stop timing
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