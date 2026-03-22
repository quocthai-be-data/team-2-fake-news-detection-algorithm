#include "trie.h"

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

// int Trie::countKeywords(std::vector<std::string>& words) {
//     int count = 0;
//     for (const std::string& word : words) {
//         if (search(word)) {
//             count++;
//         }
//     }
//     return count; // Function none using
// }

void Trie::loadSuspiciousKeywords(const std::string& filename) {
    std::ifstream file(filename);
    if(!file.is_open()) return;
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
    std::cout << " --- Start checking file: " << filePath << " ---" << std::endl;
    std::cout << "| \n";
    std::cout << " --- The scoring for each article based on the number of suspicious words that match the list ---" << std::endl;
    std::cout << "| \n";
    
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
        std::vector<std::string> cols;
        std::stringstream ss(line);
        std::string col;
        while (std::getline(ss, col, ',')) {
            cols.push_back(col);
        }
        if (cols.size() < 6) {
            std::cout << "[FORMAT ERROR] Line " << rowCount
                      << ": expected >= 6 columns, got " << cols.size() << std::endl;
            continue;
        }
        std::string uuid = cols[0];
        std::string content = cols[4] + " " + cols[5]; // Chỉ lấy title + text
        // split content to words
        std::vector<std::string> words;
        std::stringstream textStream(content);
        std::string w;
        while (textStream >> w) words.push_back(w);
        int count = 0;
        for (size_t i = 0; i < words.size(); ++i) {
            int bestMatchLength = -1;
            std::string currentPhrase = "";
            
            for (size_t j = i; j < words.size() && j < i + 10; ++j) {
                if (currentPhrase.empty()) currentPhrase = words[j];
                else currentPhrase += " " + words[j];
                if (search(currentPhrase)) {
                    bestMatchLength = j;
                }
            }
            if (bestMatchLength != -1) {
                count++;
                i = bestMatchLength;
            }
        }
        std::cout << std::left << std::setw(42) << uuid << " | " << count << std::endl;
    }

    if (rowCount == 0) {
        std::cout << "--- [Announcement] The file has only header ---" << std::endl;
    }
    
    file.close();
    std::cout << "--- End of processing ---" << std::endl;
}