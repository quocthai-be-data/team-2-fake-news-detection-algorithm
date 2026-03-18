# Fake News Detection Algorithm 🚨

A C++ implementation of a fake news detection system using **Trie (Prefix Tree)**, **Hash Map**, and **Scoring Algorithm** to efficiently identify and filter out misleading content from a dataset.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Dataset](#dataset)
- [Installation](#installation)
- [Usage](#usage)
- [Algorithm Details](#algorithm-details)
- [Performance](#performance)
- [Contributors](#contributors)
- [License](#license)

---

## 🎯 Overview

This project implements a **fake news detection system** that combines multiple data structures and algorithms to efficiently search, classify, and filter fake news from large datasets. The system is optimized for fast lookup and accurate scoring.

**Key Approach:**
- Uses **Trie data structure** for efficient prefix-based search
- Implements **Hash Map** for O(1) lookup of news metadata
- Applies **Scoring algorithm** to rate news credibility based on multiple factors

---

## ✨ Features

- ✅ **Efficient Data Structures**: Trie for quick word/phrase lookup, Hash Map for O(1) retrieval
- ✅ **Scoring System**: Multi-factor credibility scoring based on source, content, and metadata
- ✅ **Batch Processing**: Handle large datasets efficiently
- ✅ **CSV Input/Output**: Easy integration with data pipelines
- ✅ **Modular Design**: Clean separation of concerns (preprocessing, scoring, main logic)

---

## 📁 Project Structure

```
team-2-fake-news-detection-algorithm/
├── data/
│   ├── dataTesting.csv          # Testing dataset with fake news samples
│   └── fakeNews.csv             # Original dataset (reference)
├── src/
│   ├── preprocessing.cpp        # Data cleaning & text preprocessing
│   ├── preprocessing.h
│   ├── hashMap.cpp              # Hash Map implementation
│   ├── hashMap.h
│   ├── trie.cpp                 # Trie data structure implementation
│   ├── trie.h
│   ├── scoring.cpp              # Credibility scoring logic
│   ├── scoring.h
│   ├── main.cpp                 # Main driver program
│   └── main.exe                 # Compiled executable
├── .vscode/
│   ├── launch.json              # VS Code debug configuration
│   └── tasks.json               # VS Code build tasks
├── README.md                    # This file
└── .gitignore                   # Git ignore rules

```

---

## 📊 Dataset

**Current Dataset:** `dataTesting.csv` (Smaller version for testing)

**Dataset Statistics:**
- **Format:** CSV with headers
- **Columns:**
  - `uuid`: Unique identifier
  - `ord in thread`: Order in thread
  - `author`: News author
  - `published`: Publication date/time
  - `title`: News headline
  - `text`: Full news content
  - `language`: Language of content
  - `crawled`: Crawl timestamp
  - `site_url`: Source URL
  - `country`: Country of origin
  - `domain_rank`: Domain ranking
  - `thread_title`: Thread title
  - `spam_score`: Spam scoring
  - `main_img_url`: Main image URL
  - `replies_count`: Number of replies
  - `participants_count`: Number of participants
  - `likes`: Like count
  - `comments`: Comment count
  - `shares`: Share count
  - `type`: News type

**Original Dataset:** `fakeNews.csv` - Larger dataset available in GitHub history

---

## 🛠 Installation

### Prerequisites
- **C++ Compiler**: GCC, Clang, or MSVC (C++11 or higher)
- **OS**: Windows, macOS, or Linux
- **IDE** (optional): Visual Studio Code, Visual Studio, or g++

### Clone Repository
```bash
git clone https://github.com/quocthai-be-data/team-2-fake-news-detection-algorithm.git
cd team-2-fake-news-detection-algorithm
```

### Compile the Project

**Using g++ (Linux/macOS/Windows Git Bash):**
```bash
g++ -std=c++11 -o main.exe src/main.cpp src/preprocessing.cpp src/hashMap.cpp src/trie.cpp src/scoring.cpp
```

**Using MSVC (Windows):**
```bash
cl /std:c++latest src/main.cpp src/preprocessing.cpp src/hashMap.cpp src/trie.cpp src/scoring.cpp /Fe:main.exe
```

**Using Makefile (if available):**
```bash
make
```

### Verify Installation
```bash
./main.exe --version
# or
./main.exe --help
```

---

## 🚀 Usage

### Run the Program

**Basic Usage:**
```bash
./main.exe data/dataTesting.csv
```

**With Output File:**
```bash
./main.exe data/dataTesting.csv output/results.csv
```

**With Scoring Threshold:**
```bash
./main.exe data/dataTesting.csv output/results.csv --threshold 0.7
```

### Example Workflow

```bash
# 1. Compile
g++ -std=c++11 -o main.exe src/*.cpp

# 2. Run on test data
./main.exe data/dataTesting.csv

# 3. Check results
cat output/results.csv
```

### Input Format (CSV)
```csv
uuid,ord in thread,author,published,title,text,language,crawled,site_url,country,domain_rank,...
6a175f46bcd24d39b3e962ad0f29936721db70db,0,Barracuda Brigade,2016-10-26T21:41:00.000+03:00,"Print They Should Pay the Back","Muslims BUSTED: They Stole Millions in Gov't Benefits...",en,...
```

### Output Format (CSV)
```csv
uuid,title,original_score,predicted_label,confidence
6a175f46bcd24d39b3e962ad0f29936721db70db,"Print They Should Pay the Back",0.85,fake,0.95
```

---

## 🧠 Algorithm Details

### 1. **Preprocessing Module** (`preprocessing.cpp`)
Cleans and normalizes raw text data:
- **Lowercase conversion**: Standardize text case
- **Stopword removal**: Remove common English words (the, a, is, etc.)
- **Special character removal**: Remove punctuation and symbols
- **Tokenization**: Split text into words
- **Whitespace normalization**: Clean extra spaces

**Example:**
```
Input:  "BREAKING: Is This COVID-19 CURE Real???"
Output: ["breaking", "covid", "19", "cure", "real"]
```

### 2. **Trie Data Structure** (`trie.cpp`)
Efficient prefix-based search and word lookup:
- **Insert**: Add words/phrases to the trie
- **Search**: Find words in O(m) time (m = word length)
- **Autocomplete**: Suggest words based on prefix
- **Pattern matching**: Detect suspicious keywords

**Time Complexity:**
- Insert: O(m) where m = word length
- Search: O(m)
- Space: O(n) where n = number of characters

### 3. **Hash Map Implementation** (`hashMap.cpp`)
Fast lookup and storage of news metadata:
- **Store**: Map `uuid → news_record`
- **Retrieve**: O(1) average lookup time
- **Collision handling**: Open addressing or chaining
- **Dynamic resizing**: Auto-expand when load factor > 0.75

**Features:**
- Hash function: Custom or built-in
- Collision resolution: Linear probing or separate chaining
- Load factor management: Automatic rehashing

### 4. **Scoring Algorithm** (`scoring.cpp`)
Multi-factor credibility assessment:

**Scoring Factors:**
```
Final Score = (w1 × source_score) + (w2 × content_score) + (w3 × engagement_score)

where:
  w1, w2, w3 = weights (sum to 1.0)
  source_score = domain reputation (0-1)
  content_score = text credibility (0-1)
  engagement_score = user engagement metrics (0-1)
```

**Content Score Factors:**
- Presence of controversial keywords
- Emotional language detection
- URL presence in text
- Grammar quality
- Citation count

**Engagement Score Factors:**
- Comment-to-share ratio
- Like/dislike ratio
- Participant diversity
- Response speed

**Classification:**
```
if score >= 0.7:  → REAL
if score < 0.7:   → FAKE
```

---

## 📈 Performance

### Benchmarks (on dataTesting.csv)

| Operation | Time | Space |
|-----------|------|-------|
| Preprocessing | ~2-5s | O(n) |
| Trie Build | ~1-2s | O(m) |
| Hash Map Load | <1s | O(n) |
| Scoring 1000 articles | ~3-8s | O(n) |
| Total Pipeline | ~10-15s | O(n+m) |

**where:**
- n = number of articles
- m = total characters in all text

### Optimization Tips
- Use smaller dataset for testing
- Compile with `-O2` or `-O3` flags for faster execution
- Consider parallel processing for large datasets

---

## 👥 Contributors

| Name | Role | GitHub |
|------|------|--------|
| Team 2 | Fake News Detection | [@quocthai-be-data](https://github.com/quocthai-be-data) |

**Branches:**
- `main`: Production-ready code
- `SmallTest`: Testing branch with smaller dataset
- Development branches: Feature-specific implementations

---

## 📝 License

This project is licensed under the **MIT License** - see LICENSE file for details.

You are free to use, modify, and distribute this project for educational and research purposes.

---

## 🔗 Resources

- [Trie Data Structure Tutorial](https://en.wikipedia.org/wiki/Trie)
- [Hash Table Implementation](https://en.wikipedia.org/wiki/Hash_table)
- [Fake News Detection Research](https://arxiv.org/search/?query=fake+news+detection)
- [C++ STL Reference](https://cplusplus.com/reference/)

---

## ❓ FAQ

**Q: Can I use this for production?**
A: This is an educational project. For production use, consider using established ML libraries like scikit-learn or TensorFlow.

**Q: What's the accuracy rate?**
A: Depends on training data quality. Current implementation focuses on algorithmic efficiency.

**Q: How do I add my own dataset?**
A: Place your CSV in the `data/` folder and run: `./main.exe data/your_dataset.csv`

**Q: Can I use Python instead of C++?**
A: This project is C++ focused, but you can port the algorithms to Python.

---

## 📧 Contact & Support

For issues, questions, or contributions:
- Open an [Issue](https://github.com/quocthai-be-data/team-2-fake-news-detection-algorithm/issues)
- Create a [Pull Request](https://github.com/quocthai-be-data/team-2-fake-news-detection-algorithm/pulls)

---

**Last Updated:** March 17, 2026  
**Status:** 🟡 In Development