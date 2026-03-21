#include "trie.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <map>
#include <string>

Trie::Trie() {
    root = new TrieNode();
}

void Trie::insert(std::string word) {
    TrieNode* curr = root;
    for (char c : word) {
        // Chuyển về chữ thường để đồng bộ (giả sử dữ liệu đã clean)
        int index = std::tolower(c) - 'a';
        if (index < 0 || index >= 26) continue; // Bỏ qua ký tự không hợp lệ

        if (curr->children[index] == nullptr) {
            curr->children[index] = new TrieNode();
        }
        curr = curr->children[index];
    }
    curr->isEndOfWord = true;
}

bool Trie::search(std::string word) {
    TrieNode* curr = root;
    for (char c : word) {
        int index = std::tolower(c) - 'a';
        if (index < 0 || index >= 26) return false;

        if (curr->children[index] == nullptr) {
            return false;
        }
        curr = curr->children[index];
    }
    return curr != nullptr && curr->isEndOfWord;
}

int Trie::countKeywords(std::vector<std::string> words) {
    int count = 0;
    for (const std::string& word : words) {
        if (search(word)) {
            count++;
        }
    }
    return count;
}

void Trie::loadSuspiciousKeywords(std::string filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "[ERROR] Don't find the keywords: " << filename << std::endl;
        return;
    }

    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line.find("//") != std::string::npos) continue;
        for (char &c : line) {
            if (c == '{' || c == '}' || c == '"' || c == ',') {
                c = ' ';
            }
        }

        std::stringstream ss(line);
        std::string word;
        while (ss >> word) {
            if (!word.empty()) {
                insert(word);
                count++;
            }
        }
    }
    
    std::cout << "[Trie] Done " << count << " from: " << filename << std::endl;
    file.close();
}

void Trie::processFileAndDisplay(std::string filePath) {
    std::cout << "\n--- Start checking file: " << filePath << " ---" << std::endl;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Error] Don't open the file." << std::endl;
        return;
    }

    std::string line;

    if (!std::getline(file, line)) {
        std::cout << "[WARNING] The file is empty!" << std::endl;
        return;
    }
    std::cout << "Read Header: " << line << std::endl;

    std::cout << std::left << std::setw(42) << "UUID" << " | " << "Keyword Count" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    int rowCount = 0;
    while (std::getline(file, line)) {
        rowCount++;
        std::stringstream ss(line);
        std::string uuid, cleaned_text;

        if (std::getline(ss, uuid, ',') && std::getline(ss, cleaned_text)) {
            std::stringstream textStream(cleaned_text);
            std::string word;
            int count = 0;
            while (textStream >> word) {
                if (search(word)) count++;
            }
            std::cout << std::left << std::setw(20) << uuid << " | " << count << std::endl;
        } else {
            std::cout << "[FORMAT EROOR] Line " << rowCount << " Incorrect format 'uuid,text': " << line << std::endl;
        }
    }

    if (rowCount == 0) {
        std::cout << "[Announcement] The file has only header" << std::endl;
    }
    
    file.close();
    std::cout << "--- End of processing ---" << std::endl;
}