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
Trie::~Trie() {
    delete root; 
}

int getIndex(char c) {
    if (c == ' ') return 26; // Dấu cách dùng index 26
    char low = std::tolower(static_cast<unsigned char>(c));
    if (low >= 'a' && low <= 'z') return low - 'a';
    return -1; // Ký tự không hợp lệ
}

void Trie::insert(const std::string& word) {
    TrieNode* curr = root;
    for (char c : word) {
        int index = getIndex(c);
        if (index == -1) continue; // Bỏ qua ký tự đặc biệt như @, #, $

        if (curr->children[index] == nullptr) {
            curr->children[index] = new TrieNode();
        }
        curr = curr->children[index];
    }
    curr->isEndOfWord = true;
}

bool Trie::search(const std::string& word) {
    TrieNode* curr = root;
    for (char c : word) {
        int index = getIndex(c);
        if (index == -1) continue; 

        if (curr->children[index] == nullptr) {
            return false;
        }
        curr = curr->children[index];
    }
    return curr != nullptr && curr->isEndOfWord;
}

int Trie::countKeywords(std::vector<std::string>& words) {
    int count = 0;
    for (const std::string& word : words) {
        if (search(word)) {
            count++;
        }
    }
    return count;
}

void Trie::loadSuspiciousKeywords(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            // Xóa ký tự \r nếu file được tạo từ Windows
            if (line.back() == '\r') line.pop_back();
            insert(line); 
        }
    }
    file.close();
}

void Trie::processFileAndDisplay(const std::string& filePath) {
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
            // Tách tất cả các từ trong dòng vào một vector để dễ duyệt theo chỉ số
            std::vector<std::string> words;
            std::stringstream textStream(cleaned_text);
            std::string w;
            while (textStream >> w) words.push_back(w);

            int count = 0;
            // Duyệt qua từng từ trong câu
            for (size_t i = 0; i < words.size(); ++i) {
                int bestMatchLength = -1;
                std::string currentPhrase = "";
                
                // Từ vị trí i, thử ghép thêm các từ tiếp theo (i+1, i+2...) 
                // để xem có tạo thành cụm từ nào có trong Trie không
                // Giới hạn j để tránh cụm từ quá dài (ví dụ tối đa 10 từ)
                for (size_t j = i; j < words.size() && j < i + 10; ++j) {
                    if (currentPhrase.empty()) currentPhrase = words[j];
                    else currentPhrase += " " + words[j];

                    if (search(currentPhrase)) {
                        // Em đang muốn một từ chỉ thuộc về một cụm từ duy nhất (không trùng lặp)
                        bestMatchLength = j;
                    }
                }

                if (bestMatchLength != -1) {
                    count++;
                    i = bestMatchLength; // NHẢY CÁCH: Bỏ qua các từ con đã nằm trong cụm vừa khớp
                }
            }
            std::cout << std::left << std::setw(42) << uuid << " | " << count << std::endl;
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