#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>

struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;

    TrieNode();
};

class Trie {
public:
    Trie();
    void insert(std::string word);
    bool search(std::string word);
    int countKeywords(std::vector<std::string> words);

private:
    TrieNode* root;
};

#endif