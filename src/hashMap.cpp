#include "hashMap.h"

// Constructor — allocate bucket array
HashMap::HashMap(int initialCapacity) {
    bucketCount   = initialCapacity;
    elementCount  = 0;
    maxLoadFactor = 0.75f;
    buckets       = new HashNode*[bucketCount];
    for (int i = 0; i < bucketCount; ++i) {
        buckets[i] = nullptr;
    }
}

// Destructor — free all nodes and bucket array
HashMap::~HashMap() {
    for (int i = 0; i < bucketCount; ++i) {
        HashNode* curr = buckets[i];
        while (curr != nullptr) {
            HashNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
    delete[] buckets;
}

// Hash function — djb2 algorithm
// Converts a uuid string into a bucket index
int HashMap::hashFunction(const std::string& key) {
    unsigned long hash = 5381;
    for (char c : key) {
        hash = ((hash << 5) + hash) + c;   // hash * 33 + c
    }
    return static_cast<int>(hash % bucketCount);
}

// Insert — add a NewsRecord keyed by uuid
// If key already exists, update the value
void HashMap::insert(const std::string& key, const NewsRecord& record) {
    // Check load factor before inserting
    float loadFactor = static_cast<float>(elementCount + 1) / bucketCount;
    if (loadFactor > maxLoadFactor) {
        rehash();
    }

    int index = hashFunction(key);
    HashNode* curr = buckets[index];

    // Check if key already exists — update if so
    while (curr != nullptr) {
        if (curr->key == key) {
            curr->value = record;
            return;
        }
        curr = curr->next;
    }

    // Key not found — insert at head of chain
    HashNode* newNode = new HashNode(key, record);
    newNode->next = buckets[index];
    buckets[index] = newNode;
    elementCount++;
}

// Get — retrieve a NewsRecord pointer by uuid
// Returns nullptr if key not found
NewsRecord* HashMap::get(const std::string& key) {
    int index = hashFunction(key);
    HashNode* curr = buckets[index];

    while (curr != nullptr) {
        if (curr->key == key) {
            return &(curr->value);
        }
        curr = curr->next;
    }
    return nullptr;
}

// Contains — check if a uuid exists in the map
bool HashMap::contains(const std::string& key) {
    return get(key) != nullptr;
}

// Size — return total number of stored elements
int HashMap::size() const {
    return elementCount;
}

// Rehash — double the bucket count and re-insert all elements
// Triggered when load_factor > 0.75
void HashMap::rehash() {
    int oldBucketCount = bucketCount;
    HashNode** oldBuckets = buckets;

    bucketCount = oldBucketCount * 2;
    buckets = new HashNode*[bucketCount];
    for (int i = 0; i < bucketCount; ++i) {
        buckets[i] = nullptr;
    }
    elementCount = 0;

    // Re-insert all elements from old buckets
    for (int i = 0; i < oldBucketCount; ++i) {
        HashNode* curr = oldBuckets[i];
        while (curr != nullptr) {
            insert(curr->key, curr->value);
            HashNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
    delete[] oldBuckets;
}

// parseCSVLine — handles quoted fields correctly
// Replicates the same CSV parsing logic as preprocessing.cpp
std::vector<std::string> HashMap::parseCSVLine(const std::string& line) {
    std::vector<std::string> row;
    std::string cell;
    bool insideQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (insideQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                cell += '"';
                i++;
            } else {
                insideQuotes = !insideQuotes;
            }
        } else if (c == ',' && !insideQuotes) {
            row.push_back(cell);
            cell.clear();
        } else {
            cell += c;
        }
    }
    row.push_back(cell);
    return row;
}

// loadFromCSV — read preprocessed.csv (dataCleaned.csv)
//   and insert each row as a NewsRecord into the HashMap
//
// Expected CSV columns (from preprocessing.cpp output):
//   0: uuid
//   1: type
//   2: site_url
//   3: domain_rank        (int)
//   4: title              (cleaned text)
//   5: text               (cleaned text)
//   6: spam_score         (float)
//   7: replies_count      (int)
//   8: participants_count (int)
//   9: likes              (int)
//  10: comments           (int)
//  11: shares             (int)

void HashMap::loadFromCSV(const std::string& filePath) {
    std::cout << " --- Loading articles into HashMap from: " << filePath << " ---" << std::endl;
    std::cout << "| " << std::endl;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Error] Cannot open file: " << filePath << std::endl;
        return;
    }

    std::string line;

    // Read and display header
    if (!std::getline(file, line)) {
        std::cout << "[WARNING] The file is empty!" << std::endl;
        return;
    }
    std::cout << "Read Header: " << line << std::endl;

    // Display table header
    std::cout << std::left
              << std::setw(42) << "UUID" << " | "
              << std::setw(14) << "Type" << " | "
              << std::setw(30) << "Site URL" << " | "
              << std::setw(12) << "Domain Rank" << " | "
              << std::setw(10) << "Spam Score"
              << std::endl;
    std::cout << "--------------------------------------------------------------------------------------------------------------" << std::endl;

    int rowCount = 0;
    std::string fullRecord = "";
    bool recordInsideQuotes = false;

    while (std::getline(file, line)) {
        if (line.empty() && !recordInsideQuotes) continue;

        fullRecord += line;

        // Track quotes to handle multi-line fields
        for (char c : line) {
            if (c == '"') recordInsideQuotes = !recordInsideQuotes;
        }

        if (recordInsideQuotes) {
            fullRecord += " ";
            continue;
        }

        std::vector<std::string> cols = parseCSVLine(fullRecord);
        fullRecord.clear();

        if (cols.size() < 12) {
            std::cout << "[FORMAT ERROR] Row " << (rowCount + 1)
                      << ": expected >= 12 columns, got " << cols.size() << std::endl;
            continue;
        }

        rowCount++;

        // Build NewsRecord
        NewsRecord record;
        record.uuid                = cols[0];
        record.type                = cols[1];
        record.site_url            = cols[2];
        record.domain_rank         = std::atoi(cols[3].c_str());
        record.title_text_cleaned  = cols[4] + " " + cols[5];   // merge title + text
        record.spam_score          = std::atof(cols[6].c_str());
        record.replies_count       = std::atoi(cols[7].c_str());
        record.participants_count  = std::atoi(cols[8].c_str());
        record.likes               = std::atoi(cols[9].c_str());
        record.comments            = std::atoi(cols[10].c_str());
        record.shares              = std::atoi(cols[11].c_str());

        // Insert into HashMap
        insert(record.uuid, record);

        // Display row in formatted table
        std::cout << std::left
                  << std::setw(42) << record.uuid << " | "
                  << std::setw(14) << record.type << " | "
                  << std::setw(30) << record.site_url << " | "
                  << std::setw(12) << record.domain_rank << " | "
                  << record.spam_score
                  << std::endl;
    }

    file.close();

    if (rowCount == 0) {
        std::cout << "--- [Announcement] The file has only header ---" << std::endl;
    }

    std::cout << "| " << std::endl;
    std::cout << " --- HashMap loaded: " << rowCount << " articles stored ---" << std::endl;
}

// displaySummary — show HashMap statistics
void HashMap::displaySummary() {
    std::cout << "| " << std::endl;
    std::cout << " --- HashMap Summary ---" << std::endl;
    std::cout << "  Total articles : " << elementCount << std::endl;
    std::cout << "  Bucket count   : " << bucketCount << std::endl;

    float loadFactor = static_cast<float>(elementCount) / bucketCount;
    std::cout << "  Load factor    : " << std::fixed << std::setprecision(3) << loadFactor << std::endl;

    // Count non-empty buckets and max chain length
    int nonEmpty = 0;
    int maxChain = 0;
    for (int i = 0; i < bucketCount; ++i) {
        int chainLen = 0;
        HashNode* curr = buckets[i];
        while (curr != nullptr) {
            chainLen++;
            curr = curr->next;
        }
        if (chainLen > 0) nonEmpty++;
        if (chainLen > maxChain) maxChain = chainLen;
    }

    std::cout << "  Non-empty buckets : " << nonEmpty << std::endl;
    std::cout << "  Max chain length  : " << maxChain << std::endl;
    std::cout << " --- End of HashMap Summary ---" << std::endl;
}
