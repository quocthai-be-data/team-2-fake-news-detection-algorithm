#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <fstream>

struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;

    TrieNode() {
        isEndOfWord = false;
        for (int i = 0; i < 26; i++) children[i] = nullptr;
    }
};

class Trie {
public:
    Trie();
    void insert(std::string word);
    bool search(std::string word);
    void loadSuspiciousKeywords(std::string filename);
    void processFileAndDisplay(std::string filePath);

private:
    TrieNode* root;
    int countKeywords(std::vector<std::string> words);
};

#endif