# Algorithm Details

Technical reference for all four modules of the Fake News Detection pipeline.
This document is intended for team members to understand how each module works, what it expects as input, what it must produce as output, and how it connects to other modules.

---

## Table of Contents

- [Overall Pipeline](#overall-pipeline)
- [Module 1 — Preprocessing](#module-1--preprocessing)
- [Module 2 — Trie](#module-2--trie)
- [Module 3 — HashMap](#module-3--hashmap)
- [Module 4 — Scoring](#module-4--scoring)
- [Module 5 — Evaluation](#module-5--evaluation)
- [Module Interfaces](#module-interfaces)

---

## Overall Pipeline

```
dataTesting.csv  (20 columns, raw data)
        |
[ MODULE 1: PREPROCESSING ]
  - Keep 12 relevant columns
  - Clean and normalize data
        |
dataCleaned.csv  (12 columns, cleaned)
        |
  +-----+-----+
  |           |
[ MODULE 2 ] [ MODULE 3 ]       ← independent, run in parallel
[   TRIE   ] [  HASHMAP  ]
  Read         Read all
  title_text   12 columns
  Count        Store metadata
  keywords     by uuid
  |           |
  +-----+-----+
        |
[ MODULE 4: SCORING ]
  Combine Trie + HashMap results
  Compute final score → Classify REAL / FAKE
        |
[ MODULE 5: EVALUATION ]
  Compare predicted_label vs actual type
  Compute accuracy, precision, recall, F1
        |
  results.csv  +  evaluation_report.txt
```

> **Module 2 (Trie) and Module 3 (HashMap) are independent.** They both read from `dataCleaned.csv` but do not depend on each other. They can be developed and tested in parallel. Module 4 must wait for both to be ready.

---

## Module 1 — Preprocessing

### Responsibility
Read the raw input file `dataTesting.csv` (20 columns), discard irrelevant columns, clean the remaining data, and write a normalized `dataCleaned.csv` (12 columns) for downstream modules.

### Input / Output

```
INPUT:  dataTesting.csv   — 20 columns, raw
OUTPUT: dataCleaned.csv  — 12 columns, cleaned
```

### Step 1 — Keep only the 12 required columns

From the original 20 columns, retain only:

| Column | Used To Compute |
|--------|----------------|
| `uuid` | Article identifier — key for HashMap |
| `type` | Ground truth label — used in Evaluation |
| `title` | Text content → Trie |
| `text` | Text content → Trie |
| `site_url` | source_score |
| `domain_rank` | source_score |
| `spam_score` | content_score |
| `replies_count` | engagement_score |
| `participants_count` | engagement_score |
| `likes` | engagement_score |
| `comments` | engagement_score |
| `shares` | engagement_score |

**Dropped columns:** `ord_in_thread`, `author`, `published`, `language`, `crawled`, `country`, `thread_title`, `main_img_url`, and others — not used in any scoring formula.

---

### Step 2 — Handle missing values in numeric columns

Some rows in the dataset have empty cells in numeric columns. Replace all empty values with `0` so the Scoring module can compute without errors.

```
domain_rank        : ""  →  0
spam_score         : ""  →  0.0
replies_count      : ""  →  0
participants_count : ""  →  0
likes              : ""  →  0
comments           : ""  →  0
shares             : ""  →  0
```

---

### Step 3 — Clean text columns (title and text separately)

Apply the following cleaning steps to both `title` and `text` independently:

```
[3.1] Convert to lowercase:
      "BREAKING: Weiner With FBI!!!" → "breaking: weiner with fbi!!!"

[3.2] Replace non-alphanumeric characters with space (keep apostrophe '):
      → "breaking  weiner with fbi   "

[3.3] Tokenize — split into individual words:
      → ["breaking", "weiner", "with", "fbi"]

[3.4] Remove stopwords — words with no meaningful signal:
      Stopword list: {"the", "a", "an", "is", "are", "was", "were", "be", "been",
                      "have", "has", "had", "do", "does", "did", "will", "would",
                      "could", "should", "this", "that", "these", "those", "with",
                      "from", "and", "or", "but", "in", "on", "at", "to", "for",
                      "of", "by", "as", "it", "its", "he", "she", "they", "we",
                      "you", "i", "not", "so", "if", "then", "than", "what", ...}

      → ["breaking", "weiner", "fbi"]

[3.5] Join back into cleaned string:
      → "breaking weiner fbi"
```

> **Note:** Title and text are stored as **separate columns** (not merged) in the output.
> Numeric columns are passed through `ensureNumeric()` (trim whitespace, handle negatives, enforce single decimal point).
> Text columns are wrapped with `escapeCSV()` (double-quoted, internal quotes escaped as `""`).

---

### Output Format (`dataCleaned.csv`)

```
uuid, type, site_url, domain_rank, title, text, spam_score,
replies_count, participants_count, likes, comments, shares
```

**12 columns total — title and text are separate, cleaned columns.**

**Real examples from the dataset:**

```csv
"c70e149f...","bias","100percentfedup.com",25689.0,"breaking weiner cooperating fbi hillary email investigation","red state fox news sunday reported morning...",0.0,0,1,0,0,0
```

### Summary

| | Details |
|---|---|
| **Read** | `dataTesting.csv` (20 columns) |
| **Keep** | 12 columns relevant to scoring |
| **Empty numeric cells** | Replace with `0` via `ensureNumeric()` |
| **title** | Clean separately: lowercase → remove special chars → remove stopwords |
| **text** | Clean separately: same pipeline as title |
| **Text output** | Wrapped in `escapeCSV()` (double-quoted) |
| **Write** | `dataCleaned.csv` (12 columns) |
| **Output consumed by** | Trie (reads `title` + `text`), HashMap (reads all 12 columns) |

---

## Module 2 — Trie

### Responsibility
Build a Trie loaded with suspicious keywords (single words and multi-word phrases). For each article in `dataCleaned.csv`, scan its `title` + `text` and count how many suspicious keywords it contains using greedy longest-match. Return `uuid → keyword_count` as `std::map` to the Scoring module.

### Input / Output

```
INPUT:  data/SuspiciousKeywords.csv — one keyword/phrase per line (417 entries)
        data/dataCleaned.csv       — columns: uuid (col 0), title (col 4), text (col 5)
OUTPUT: std::map<string, int>      — uuid → keyword_count
```

### Trie Structure

A Trie (prefix tree) stores strings character by character. Each path from the root to an end-marked node represents one complete word or phrase.

**Node definition (supports multi-word via space at index 26):**
```cpp
struct TrieNode {
    TrieNode* children[27];   // 0-25 = a-z, 26 = space (for multi-word phrases)
    bool isEndOfWord;

    TrieNode() {
        isEndOfWord = false;
        for (int i = 0; i < 27; i++) children[i] = nullptr;
    }
    ~TrieNode() {
        for (int i = 0; i < 27; i++) {
            if (children[i] != nullptr) delete children[i];
        }
    }
};
```

**Character mapping via `getIndex()`:**
```cpp
int getIndex(char c) {
    if (c == ' ') return 26;                              // space → index 26
    char low = std::tolower(static_cast<unsigned char>(c));
    if (low >= 'a' && low <= 'z') return low - 'a';       // a-z → 0-25
    return -1;                                             // skip invalid chars
}
```

### Suspicious Keywords

Loaded from `data/SuspiciousKeywords.csv` — 417 entries, one per line.
Includes single words (`breaking`, `urgent`) and multi-word phrases (`money laundering`, `work from home`).

### Operations Implemented

**insert(word)** — add one keyword/phrase to the Trie (skips invalid chars via `getIndex`).

**search(word)** — check if a word/phrase exists in the Trie (same `getIndex` logic, `continue` on invalid chars).

**processFileAndDisplay(filePath)** — main processing function:
```
1. Parse each CSV row into columns (split by ',')
2. Extract uuid = cols[0], content = cols[4] + " " + cols[5]  (title + text)
3. Tokenize content into word list
4. Greedy multi-word matching:
   For each word[i], try building phrases: word[i], word[i]+" "+word[i+1], ...
   up to 10 words ahead. Keep the longest match found.
   If matched, skip consumed words to avoid double-counting.
5. Store result[uuid] = count
6. Return std::map<string, int>
```

### Time Complexity

| Operation | Complexity |
|-----------|-----------|
| Insert one keyword | O(m) |
| Search one keyword | O(m) |
| Process one article (greedy match) | O(n × k × m) |

*m = avg keyword length, n = words in article, k = max phrase length (10)*

### Summary

| | Details |
|---|---|
| **Read** | `dataCleaned.csv` — columns `uuid` (0), `title` (4), `text` (5) |
| **Keywords** | `SuspiciousKeywords.csv` — 417 entries, single + multi-word |
| **Matching** | Greedy longest-match, skips consumed words |
| **Output** | `std::map<string, int>` — uuid → keyword_count |
| **Pass to** | Scoring module |

---

## Module 3 — HashMap

### Responsibility
Read all 12 columns from `dataCleaned.csv` and store each article as a `NewsRecord` struct in a **custom-built HashMap** (not `std::unordered_map`), keyed by `uuid`. The Scoring module will call `get(uuid)` to retrieve article metadata in O(1) average time.

### Input / Output

```
INPUT:  data/dataCleaned.csv — 12 columns, text fields escaped with escapeCSV()
OUTPUT: HashMap<uuid, NewsRecord> — O(1) average lookup
```

### NewsRecord Struct

```cpp
struct NewsRecord {
    std::string uuid;                // col 0
    std::string type;                // col 1  — ground truth: bias, fake, bs, conspiracy
    std::string site_url;            // col 2
    double      domain_rank;         // col 3  → source_score
    std::string title;               // col 4  — cleaned text
    std::string text;                // col 5  — cleaned text
    double      spam_score;          // col 6  → content_score
    int         replies_count;       // col 7  ┐
    int         participants_count;  // col 8  │
    int         likes;               // col 9  ├→ engagement_score
    int         comments;            // col 10 │
    int         shares;              // col 11 ┘
};
```

### How the HashMap Works

**Hash function:**
```
Compute an integer index from the uuid string.
index = hash(uuid) % bucket_count
```

**Insert:**
```
hash("c70e149f...") = 42  →  bucket[42] = NewsRecord{...}
hash("8a35883f...") = 17  →  bucket[17] = NewsRecord{...}
hash("98b54c30...") = 42  →  COLLISION  →  handle via separate chaining
```

**Collision handling — separate chaining:**
```
bucket[42]:  [c70e149f | record] → [98b54c30 | record] → null
```
Each bucket holds a linked list. On collision, append the new record to the list.

**Lookup:**
```
get("c70e149f..."):
  index = hash("c70e149f...") = 42
  traverse bucket[42] until uuid matches
  → return NewsRecord
```

**Rehashing when load factor exceeds 0.75:**
```
load_factor = total elements / bucket_count

If load_factor > 0.75:
  new_bucket_count = bucket_count × 2
  re-insert all existing records into new buckets
```

### CSV Parsing Note

Since `dataCleaned.csv` uses `escapeCSV()` to wrap text fields in `"..."` (with internal `"` escaped as `""`), the HashMap's CSV parser must handle **quoted fields** — similar to `parseCSVLine()` in `preprocessing.cpp`. Simple split-by-comma will not work correctly for text columns.

### Workflow

```
1. Initialize HashMap (starting bucket count: e.g., 1000)

2. For each row in dataCleaned.csv:
   - Parse row using quote-aware CSV parser (handle "..." fields)
   - Build NewsRecord from 12 columns
   - Call insert(uuid, record)
   - Check load_factor → rehash if needed

3. Expose get(uuid) → NewsRecord* to the Scoring module
```

### Summary

| | Details |
|---|---|
| **Read** | `dataCleaned.csv` — all 12 columns |
| **CSV parsing** | Quote-aware parser (handles `escapeCSV()` output) |
| **Key** | `uuid` (string) |
| **Value** | `NewsRecord` struct (12 fields) |
| **Collision handling** | Separate chaining (linked list per bucket) |
| **Rehash trigger** | load_factor > 0.75 |
| **API to expose** | `insert(uuid, record)`, `get(uuid) → NewsRecord*` |
| **Pass to** | Scoring module |

---

## Module 4 — Scoring

### Responsibility
For each article, retrieve its metadata from HashMap and keyword count from Trie, compute a composite credibility score using three sub-scores, classify the article as REAL or FAKE, and write results to `results.csv`.

### Input / Output

```
INPUT:  - Trie:    map<string, int>  — uuid → keyword_count
        - HashMap: get(uuid)         → NewsRecord
OUTPUT: results.csv — uuid, predicted_label, final_score, actual_type
```

### Scoring Formula

```
Final Score = (0.4 × source_score) + (0.4 × content_score) + (0.2 × engagement_score)
```

---

### source_score — from `domain_rank`

```
domain_rank == 0        →  0.3   (unknown or missing source)
domain_rank <= 10,000   →  0.9   (highly reputable domain)
domain_rank <= 50,000   →  0.6   (moderately reputable)
domain_rank >  50,000   →  0.2   (low credibility domain)
```

---

### content_score — from `spam_score` + `keyword_count`

```
base_score      = 1.0 - spam_score
keyword_penalty = keyword_count × 0.1
content_score   = max(0.0, base_score - keyword_penalty)
```

Example:
```
spam_score = 0.2,  keyword_count = 3
base_score      = 1.0 - 0.2 = 0.8
keyword_penalty = 3 × 0.1   = 0.3
content_score   = max(0.0, 0.8 - 0.3) = 0.5
```

---

### engagement_score — from social metrics

```
total = likes + comments + shares + replies_count + participants_count

if total == 0  →  engagement_score = 0.5         (no data, neutral)
if total  > 0  →  engagement_score = min(1.0, total / 100.0)
```

Example:
```
likes=50, comments=30, shares=10, replies=5, participants=20
total = 115
engagement_score = min(1.0, 115 / 100.0) = 1.0
```

---

### Classification

```
Final Score >= 0.7  →  predicted_label = "REAL"
Final Score <  0.7  →  predicted_label = "FAKE"
```

---

### Full Example

```
Article: c70e149f  (actual type: bias)

  domain_rank   = 25689  →  source_score     = 0.6
  spam_score    = 0.0
  keyword_count = 2      →  content_score    = max(0, 1.0 - 0.0 - 0.2) = 0.8
  total_engage  = 1      →  engagement_score = min(1.0, 1/100) = 0.01

Final Score = (0.4 × 0.6) + (0.4 × 0.8) + (0.2 × 0.01)
            = 0.24 + 0.32 + 0.002
            = 0.562  →  FAKE
```

### Output Format (`results.csv`)

```
uuid, predicted_label, final_score, actual_type
c70e149f..., FAKE, 0.562, bias
8a35883f..., FAKE, 0.489, fake
98b54c30..., FAKE, 0.341, conspiracy
```

### Summary

| | Details |
|---|---|
| **Receive from Trie** | `map<string, int>` — uuid → keyword_count |
| **Receive from HashMap** | `get(uuid)` → NewsRecord |
| **Compute** | source_score, content_score, engagement_score |
| **Formula** | `0.4 × source + 0.4 × content + 0.2 × engagement` |
| **Threshold** | >= 0.7 → REAL, < 0.7 → FAKE |
| **Write** | `results.csv` |

---

## Module 5 — Evaluation

### Responsibility
Compare each article's `predicted_label` against its ground truth `actual_type` from the dataset. Compute standard classification metrics to measure how well the system performs.

### Input / Output

```
INPUT:  results.csv — predicted_label, actual_type per article
OUTPUT: evaluation_report.txt — accuracy, precision, recall, F1
```

### Label Mapping

The dataset uses multi-class labels (`bias`, `fake`, `bs`, `conspiracy`). For evaluation, map them to binary:

```
actual_type in {"fake", "bs", "conspiracy", "bias"}  →  FAKE
(anything not FAKE)                                  →  REAL
```

### Confusion Matrix

```
                    Predicted FAKE    Predicted REAL
Actual FAKE     |   True Positive  |  False Negative  |
Actual REAL     |   False Positive |  True Negative   |
```

### Metrics

```
Accuracy  = (TP + TN) / (TP + TN + FP + FN)

Precision = TP / (TP + FP)
  → of all articles predicted FAKE, how many are actually FAKE?

Recall    = TP / (TP + FN)
  → of all actually FAKE articles, how many did we catch?

F1 Score  = 2 × (Precision × Recall) / (Precision + Recall)
  → harmonic mean — balances Precision and Recall
```

### Example

```
Total articles: 100
TP = 60,  TN = 20,  FP = 10,  FN = 10

Accuracy  = (60 + 20) / 100        = 0.80
Precision = 60 / (60 + 10)         = 0.857
Recall    = 60 / (60 + 10)         = 0.857
F1 Score  = 2 × (0.857 × 0.857) / (0.857 + 0.857)  = 0.857
```

### Output Format (`evaluation_report.txt`)

```
===== Evaluation Report =====
Total articles evaluated: 100

Confusion Matrix:
  True Positive  (TP): 60
  True Negative  (TN): 20
  False Positive (FP): 10
  False Negative (FN): 10

Metrics:
  Accuracy  : 0.800
  Precision : 0.857
  Recall    : 0.857
  F1 Score  : 0.857
=============================
```

### Summary

| | Details |
|---|---|
| **Read** | `results.csv` |
| **Compare** | `predicted_label` vs `actual_type` |
| **Compute** | Accuracy, Precision, Recall, F1 |
| **Write** | `evaluation_report.txt` |

---

## Module Interfaces

> Exact contracts between modules so each developer knows what to produce and what to expect.

### Preprocessing → Trie & HashMap

Both modules read from `dataCleaned.csv` independently.

```
Column order (12 columns):
uuid, type, site_url, domain_rank, title, text, spam_score,
replies_count, participants_count, likes, comments, shares

Note: text fields are wrapped in "..." via escapeCSV().
```

### Trie → Scoring

```cpp
// Returns full map: uuid → keyword_count
std::map<std::string, int> processFileAndDisplay(const std::string& filePath);
```

### HashMap → Scoring

```cpp
// Returns pointer to article metadata (nullptr if not found)
NewsRecord* get(const std::string& uuid);
```

### Scoring → Evaluation

Scoring writes `results.csv`. Evaluation reads it directly.

```
Column order:
uuid, predicted_label, final_score, actual_type
```

---

**Last Updated:** March 22, 2026
