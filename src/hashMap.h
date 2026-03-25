#ifndef HASHMAP_H
#define HASHMAP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// NewsRecord — stores all metadata for one article
struct NewsRecord {
    std::string uuid;
    std::string type;
    std::string site_url;
    int         domain_rank;
    std::string title_text_cleaned;   // title + " " + text (merged)
    float       spam_score;
    int         replies_count;
    int         participants_count;
    int         likes;
    int         comments;
    int         shares;
};

// HashNode — one node in the separate-chaining linked list
struct HashNode {
    std::string key;        // uuid
    NewsRecord  value;      // article metadata
    HashNode*   next;       // next node in chain

    HashNode(const std::string& k, const NewsRecord& v)
        : key(k), value(v), next(nullptr) {}
};

// HashMap — custom hash map with separate chaining
class HashMap {
public:
    HashMap(int initialCapacity = 1000);
    ~HashMap();

    // Core operations
    void        insert(const std::string& key, const NewsRecord& record);
    NewsRecord* get(const std::string& key);
    bool        contains(const std::string& key);
    int         size() const;

    // Pipeline operations
    void loadFromCSV(const std::string& filePath);
    void displaySummary();

private:
    HashNode**  buckets;        // array of linked-list heads
    int         bucketCount;    // current number of buckets
    int         elementCount;   // total elements stored
    float       maxLoadFactor;  // threshold for rehashing (0.75)

    // Internal helpers
    int         hashFunction(const std::string& key);
    void        rehash();
    std::vector<std::string> parseCSVLine(const std::string& line);
};

#endif