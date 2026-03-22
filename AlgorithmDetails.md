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

### Step 3 — Clean and merge text columns (title + text)

Merge `title` and `text` into one column: `title_text_cleaned`. Then apply the following 5 sub-steps in order:

```
[3.1] Merge title and text into one string:
      "BREAKING: Weiner With FBI!!!" + "Fox News reported this morning..."
      → "BREAKING: Weiner With FBI!!! Fox News reported this morning..."

[3.2] Convert to lowercase:
      → "breaking: weiner with fbi!!! fox news reported this morning..."

[3.3] Remove special characters  (! ? : , . " ( ) - ' [ ] / \):
      → "breaking weiner with fbi fox news reported this morning"

[3.4] Tokenize — split into individual words:
      → ["breaking", "weiner", "with", "fbi", "fox", "news", "reported", "this", "morning"]

[3.5] Remove stopwords — words with no meaningful signal:
      Stopword list: {"the", "a", "an", "is", "are", "was", "were", "be", "been",
                      "have", "has", "had", "do", "does", "did", "will", "would",
                      "could", "should", "this", "that", "these", "those", "with",
                      "from", "and", "or", "but", "in", "on", "at", "to", "for",
                      "of", "by", "as", "it", "its", "he", "she", "they", "we",
                      "you", "i", "not", "so", "if", "then", "than", "what", ...}

      → ["breaking", "weiner", "fbi", "fox", "news", "reported", "morning"]
```

---

### Output Format (`dataCleaned.csv`)

```
uuid, type, site_url, domain_rank, title_text_cleaned, spam_score,
replies_count, participants_count, likes, comments, shares
```

**Real examples from the dataset:**

```
c70e149f, bias, 100percentfedup.com, 25689,
breaking weiner cooperating fbi hillary email investigation fox news reported,
0.0, 0, 1, 0, 0, 0

8a35883f, fake, abcnews.com.co, 65078,
amish america commit vote donald trump mathematically guaranteeing presidential victory,
0.0, 0, 0, 0, 0, 0

98b54c30, conspiracy, 21stcenturywire.com, 0,
intl community financing protecting terrorists mother agnes vanessa beeley syria,
0.015, 0, 0, 0, 0, 0
```

### Summary

| | Details |
|---|---|
| **Read** | `dataTesting.csv` |
| **Keep** | 12 columns relevant to scoring |
| **Empty numeric cells** | Replace with `0` |
| **title + text** | Merge → lowercase → remove special chars → tokenize → remove stopwords → store as `title_text_cleaned` |
| **Write** | `dataCleaned.csv` |
| **Output consumed by** | Trie (reads `title_text_cleaned`), HashMap (reads all columns) |

---

## Module 2 — Trie

### Responsibility
Build a Trie loaded with suspicious keywords. For each article in `dataCleaned.csv`, scan its cleaned text and count how many suspicious keywords it contains. Provide `uuid → keyword_count` to the Scoring module.

### Input / Output

```
INPUT:  dataCleaned.csv — columns: uuid, title_text_cleaned
OUTPUT: map<string, int> — uuid → keyword_count
```

### Trie Structure

A Trie (prefix tree) stores strings character by character. Each path from the root to an end-marked node represents one complete word.

```
Trie containing: "breaking", "bias", "bomb"

root
 └── b
      ├── r → e → a → k → i → n → g  [END]  ✓ "breaking"
      ├── i → a → s  [END]                   ✓ "bias"
      └── o → m → b  [END]                   ✓ "bomb"
```

**Node definition:**
```cpp
struct TrieNode {
    TrieNode* children[26];   // one slot per lowercase letter
    bool isEndOfWord;

    TrieNode() {
        isEndOfWord = false;
        for (int i = 0; i < 26; i++) children[i] = nullptr;
    }
};
```

### Suspicious Keywords to Load

```cpp
// Emotionally charged
{"breaking", "bombshell", "shocking", "scandal", "alert", "urgent", "exclusive"}

// Conspiracy-related
{"conspiracy", "hoax", "coverup", "rigged", "corrupt", "exposed", "deepstate"}

// Unverified claim indicators
{"allegedly", "rumored", "unconfirmed", "leaked"}

// Polarizing language
{"crooked", "evil", "destroy", "invasion", "regime", "radical"}
```

### Operations to Implement

**insert(word)** — add one keyword to the Trie:
```
insert("breaking"):
  root → [b] → [r] → [e] → [a] → [k] → [i] → [n] → [g] → isEndOfWord = true
```

**search(word)** — check if a word exists in the Trie:
```
search("breaking") → true
search("weather")  → false
```

**countKeywords(words)** — count how many words in a list match the Trie:
```
words = ["breaking", "weiner", "fbi", "hillary", "exposed"]

  "breaking" → search() = true   (+1)
  "weiner"   → search() = false
  "fbi"      → search() = false
  "hillary"  → search() = false
  "exposed"  → search() = true   (+1)

→ return 2
```

### Workflow

```
1. On startup:
   Insert all suspicious keywords into the Trie.

2. For each article in dataCleaned.csv:
   - Read uuid and title_text_cleaned
   - Split title_text_cleaned into word list
   - keyword_count = countKeywords(word list)
   - Store: result_map[uuid] = keyword_count

3. Pass result_map to Scoring module.
```

### Time Complexity

| Operation | Complexity |
|-----------|-----------|
| Insert one word | O(m) |
| Search one word | O(m) |
| countKeywords for one article | O(n × m) |

*m = word length, n = number of words in article*

### Summary

| | Details |
|---|---|
| **Read** | `dataCleaned.csv` — columns `uuid`, `title_text_cleaned` |
| **Initialize** | Insert suspicious keyword list into Trie |
| **Process** | For each article, count keyword matches |
| **Output** | `map<string, int>` — uuid → keyword_count |
| **Pass to** | Scoring module |

---

## Module 3 — HashMap

### Responsibility
Read all 10 columns from `dataCleaned.csv` and store each article as a `NewsRecord` struct in a custom HashMap, keyed by `uuid`. The Scoring module will call `get(uuid)` to retrieve article metadata.

### Input / Output

```
INPUT:  dataCleaned.csv — all 12 columns
OUTPUT: HashMap<uuid, NewsRecord> — O(1) average lookup
```

### NewsRecord Struct

```cpp
struct NewsRecord {
    string uuid;
    string type;
    string site_url;
    int    domain_rank;
    string title_text_cleaned;
    float  spam_score;
    int    replies_count;
    int    participants_count;
    int    likes;
    int    comments;
    int    shares;
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

### Workflow

```
1. Initialize HashMap (starting bucket count: e.g., 1000)

2. For each row in dataCleaned.csv:
   - Parse all 12 columns into a NewsRecord
   - Call insert(uuid, record)
   - Check load_factor → rehash if needed

3. Expose get(uuid) to the Scoring module
```

### Summary

| | Details |
|---|---|
| **Read** | `dataCleaned.csv` — all 12 columns |
| **Key** | `uuid` (string) |
| **Value** | `NewsRecord` struct |
| **Collision handling** | Separate chaining (linked list per bucket) |
| **Rehash trigger** | load_factor > 0.75 |
| **API to expose** | `insert(uuid, record)`, `get(uuid) → NewsRecord` |
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
Column order:
uuid, type, site_url, domain_rank, title_text_cleaned, spam_score,
replies_count, participants_count, likes, comments, shares
```

### Trie → Scoring

```cpp
// Option A: look up one article at a time
int getKeywordCount(string uuid);

// Option B: return full map for Scoring to iterate
map<string, int> getAllKeywordCounts();
```

### HashMap → Scoring

```cpp
// Returns full metadata for a given article
NewsRecord get(string uuid);
```

### Scoring → Evaluation

Scoring writes `results.csv`. Evaluation reads it directly.

```
Column order:
uuid, predicted_label, final_score, actual_type
```

---

**Last Updated:** March 18, 2026
