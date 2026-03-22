#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

struct TrieNode {
    TrieNode* children[27];
    bool isEndOfWord;

    TrieNode() {
        isEndOfWord = false;
        for (int i = 0; i < 27; i++) children[i] = nullptr;
    }

    ~TrieNode() {
        for (int i = 0; i < 27; i++) {
            if (children[i] != nullptr) {
                delete children[i];
            }
        }
    }
};

class Trie {
public:
    Trie();
    ~Trie();
    void insert(const std::string& word);
    bool search(const std::string& word);
    void loadSuspiciousKeywords(const std::string& filename);
    void processFileAndDisplay(const std::string& filePath);

private:
    TrieNode* root;
    int countKeywords(std::vector<std::string>& words);
};

#endif