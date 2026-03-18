# Fake News Detection Algorithm 🚨

A C++ implementation of a fake news detection system using **Trie (Prefix Tree)**, **Hash Map**, and a **Scoring Algorithm** to efficiently identify and filter out misleading news articles from a dataset.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Dataset](#dataset)
- [Installation](#installation)
- [Usage](#usage)
- [Algorithm Overview](#algorithm-overview)
- [Performance](#performance)
- [Contributors](#contributors)

---

## 🎯 Overview

This project implements a pipeline that reads raw news articles from a CSV file, processes the content and metadata, and classifies each article as **REAL** or **FAKE** using a composite credibility score.

**Key Approach:**
- **Preprocessing** — cleans raw CSV data and normalizes text for analysis
- **Trie** — efficiently searches article content for suspicious keywords
- **Hash Map** — stores article metadata for fast O(1) lookup by uuid
- **Scoring** — combines source reputation, content quality, and engagement signals into a final score

---

## ✨ Features

- ✅ **Efficient Data Structures** — Trie for keyword lookup, Hash Map for O(1) metadata retrieval
- ✅ **Multi-factor Scoring** — credibility assessed from source, content, and engagement
- ✅ **Batch Processing** — handles large datasets efficiently
- ✅ **CSV Input/Output** — easy integration with data pipelines
- ✅ **Modular Design** — each component is independently developed and testable

---

## 📁 Project Structure

```
team-2-fake-news-detection-algorithm/
├── data/
│   ├── dataTesting.csv          # Raw input dataset (20 columns)
│   └── preprocessed.csv         # Cleaned dataset output (10 columns)
├── src/
│   ├── preprocessing.cpp / .h   # Module 1: Data cleaning & normalization
│   ├── trie.cpp / .h            # Module 2: Keyword search via Trie
│   ├── hashMap.cpp / .h         # Module 3: Metadata storage via Hash Map
│   ├── scoring.cpp / .h         # Module 4: Credibility scoring & classification
│   └── main.cpp                 # Main driver — runs the full pipeline
├── output/
│   └── results.csv              # Final classification results
├── AlgorithmDetails.md          # In-depth technical documentation for the team
└── README.md                    # This file
```

---

## 📊 Dataset

**Input:** `dataTesting.csv` — 20 columns of raw news article data

**Key columns used:**

| Column | Description |
|--------|-------------|
| `uuid` | Unique article identifier |
| `type` | Ground truth label: `bias`, `fake`, `bs`, `conspiracy` |
| `title` + `text` | Article headline and full content |
| `site_url`, `domain_rank` | Source credibility signals |
| `spam_score` | Spam likelihood (0.0 → 1.0) |
| `likes`, `comments`, `shares`, `replies_count`, `participants_count` | Engagement metrics |

**Original Dataset:** `fakeNews.csv` — larger dataset available in repository history.

---

## 🛠 Installation

### Prerequisites
- **C++ Compiler**: GCC, Clang, or MSVC (C++11 or higher)
- **OS**: Windows, macOS, or Linux

### Clone Repository
```bash
git clone https://github.com/quocthai-be-data/team-2-fake-news-detection-algorithm.git
cd team-2-fake-news-detection-algorithm
```

### Compile
```bash
# Using g++
g++ -std=c++11 -o main.exe src/main.cpp src/preprocessing.cpp src/hashMap.cpp src/trie.cpp src/scoring.cpp

# Using MSVC
cl /std:c++latest src/main.cpp src/preprocessing.cpp src/hashMap.cpp src/trie.cpp src/scoring.cpp /Fe:main.exe
```

---

## 🚀 Usage

```bash
# Basic usage
./main.exe data/dataTesting.csv

# With output file
./main.exe data/dataTesting.csv output/results.csv

# With custom classification threshold
./main.exe data/dataTesting.csv output/results.csv --threshold 0.7
```

### Output Format
```csv
uuid, predicted_label, final_score, actual_type
c70e149f..., FAKE, 0.562, bias
8a35883f..., FAKE, 0.489, fake
```

---

## 🧠 Algorithm Overview

The system runs four stages in sequence:

```
dataTesting.csv
      |
[ PREPROCESSING ]  →  Clean text, normalize numeric columns
      |
      +------------------+
      |                  |
  [ TRIE ]          [ HASHMAP ]       ← Run in parallel
  Count suspicious   Store metadata
  keywords           by uuid
      |                  |
      +------------------+
      |
  [ SCORING ]  →  Final Score = 0.4×source + 0.4×content + 0.2×engagement
      |
  results.csv  →  Score ≥ 0.7 = REAL,  Score < 0.7 = FAKE
```

For full implementation details — including data structures, formulas, step-by-step logic, and code-level interfaces — see [AlgorithmDetails.md](./AlgorithmDetails.md).

---

## 📈 Performance

| Module | Estimated Time | Memory |
|--------|---------------|--------|
| Preprocessing | ~2–5s | O(n) |
| Trie Build | ~1–2s | O(m) |
| HashMap Load | < 1s | O(n) |
| Scoring | ~3–8s | O(n) |
| **Total Pipeline** | **~10–15s** | **O(n + m)** |

*n = number of articles, m = total characters in all text*

---

## 👥 Contributors

| Name | Role | GitHub |
|------|------|--------|
| Team 2 | Fake News Detection | [@quocthai-be-data](https://github.com/quocthai-be-data) |

---

## 📝 License

This project is licensed under the **MIT License**. Free to use, modify, and distribute for educational and research purposes.

---

**Last Updated:** March 18, 2026
**Status:** 🟡 In Development
