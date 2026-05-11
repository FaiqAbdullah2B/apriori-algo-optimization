# High-Performance Frequent Itemset Mining
# Apriori vs. Bitwise SOTA

![C++](https://img.shields.io/badge/C++-17%2B-blue.svg)

## Overview
This repository contains the source code, experimental setup, and analytical reports for the empirical comparison of classical Frequent Itemset Mining (FIM) algorithms against contemporary hardware-accelerated methodologies. 

Specifically, this project benchmarks the classical **Apriori Algorithm (1994)** against a modern **Linear Table Bitwise FIM Algorithm (2023)**. By transitioning from pointer-based tree structures to contiguous memory layouts and utilizing native CPU bitwise operations, this implementation achieves up to a **1084x speedup** on dense benchmark datasets.

## Key Optimizations (The SOTA Approach)
To bypass the traditional bottlenecks of candidate generation and iterative database scanning, our modern implementation utilizes two primary optimizations:
1. **Memory Layout Transformation (Linear Tables):** Horizontal transactional data is transposed into vertical, contiguous binary arrays (bitsets). This flattens the data structure, eliminating pointer-chasing and maximizing CPU L1/L2 cache hit rates.
2. **Hardware-Accelerated Support Counting:** Subset-checking is reduced to an $O(1)$ intersection calculation using native 64-bit hardware instructions (`__builtin_popcountll`). The algorithm processes up to 64 transactions per CPU clock cycle via Bitwise `AND` logic.

## 📊 Benchmarks & Datasets
Algorithms were evaluated against standard dense datasets from the FIMI (Frequent Itemset Mining Institute) repository. 

| Dataset | Characteristics | Threshold | Apriori Time | Bitwise SOTA Time | Speedup |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **accidents.dat** | 340k trans. (Dense) | 0.60 | 34.69 s | **0.03 s** | **1084x** |
| **connect.dat** | 43-item uniform (Dense) | 0.95 | 71.02 s | **0.09 s** | **728x** |
| **chess.dat** | 3k trans. (Dense) | 0.75 | 38.27 s | **0.87 s** | **43x** |
| **chess.dat** | *Stress Test* | 0.60 | 2970.00 s | **18.50 s** | **160x** |

*Note: The chess.dat stress test at a 0.60 threshold empirically demonstrates the catastrophic $O(2^d)$ failure of Apriori's candidate generation, while the Bitwise implementation scales gracefully.*

## ⚙️ Getting Started

### Prerequisites
* A modern C++ compiler (GCC, Clang, or MSVC) supporting C++17 or higher.
* FIMI Datasets (`chess.dat`, `connect.dat`, `accidents.dat`) placed in the root directory.

### Compilation
For maximum performance, ensure you compile with the `-O3` optimization flag to enable hardware-level bitwise registries.


### Compile the Apriori Baseline
g++ -O3 apriori.cpp -o apriori

### Compile the Linear Table Bitwise SOTA
g++ -O3 bitwise_sota.cpp -o bitwise_sota

### Execution

```
./apriori
./bitwise_sota
```
