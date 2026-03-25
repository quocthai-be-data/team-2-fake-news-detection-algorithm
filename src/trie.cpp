#include "trie.h"

Trie::Trie() {
    root = new TrieNode();
}
Trie::~Trie() {
    delete root;
}

int getIndex(char c) {
    if (c == ' ') return 26;
    char low = std::tolower(static_cast<unsigned char>(c));
    if (low >= 'a' && low <= 'z') return low - 'a';
    return -1;
}

void Trie::insert(const std::string& word) {
    TrieNode* curr = root;
    for (char c : word) {
        int index = getIndex(c);
        if (index == -1) continue;
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

void Trie::loadSuspiciousKeywords(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            if (line.back() == '\r') line.pop_back();
            insert(line);
        }
    }
    file.close();
}

// ==== parseCSVLine — xu ly quoted fields, nhu hashMap.cpp ====
std::vector<std::string> Trie::parseCSVLine(const std::string& line) {
    std::vector<std::string> row;
    std::string cell;
    bool insideQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            // dau "" ben trong field → giu lai 1 dau "
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

std::map<std::string, int> Trie::processFileAndDisplay(const std::string& filePath) {
    std::cout << " --- Start checking file: " << filePath << " ---" << std::endl;
    std::cout << "| \n";
    std::cout << " --- The scoring for each article based on the number of suspicious words that match the list ---" << std::endl;
    std::cout << "| \n";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Error] Don't open the file." << std::endl;
        return {};
    }

    std::string line;

    if (!std::getline(file, line)) {
        std::cout << "[WARNING] The file is empty!" << std::endl;
        return {};
    }
    std::cout << "Read Header: " << line << std::endl;

    std::cout << std::left << std::setw(42) << "UUID" << " | " << "Keyword Count" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    int rowCount = 0;
    std::map<std::string, int> result;

    std::string fullRecord = "";
    bool recordInsideQuotes = false;

    while (std::getline(file, line)) {
        if (line.empty() && !recordInsideQuotes) continue;

        fullRecord += line;

        // dem so dau " de biet con dang trong quoted field hay khong
        for (char c : line) {
            if (c == '"') recordInsideQuotes = !recordInsideQuotes;
        }

        if (recordInsideQuotes) {
            fullRecord += " ";
            continue;
        }

        std::vector<std::string> cols = parseCSVLine(fullRecord);
        fullRecord.clear();

        if (cols.size() < 6) {
            std::cout << "[FORMAT ERROR] Line " << (rowCount + 1)
                      << ": expected >= 6 columns, got " << cols.size() << std::endl;
            continue;
        }

        rowCount++;

        std::string uuid    = cols[0];  // da duoc strip dau " boi parseCSVLine
        std::string content = cols[4] + " " + cols[5];  // title + text

        // tokenize content thanh danh sach tu
        std::vector<std::string> words;
        std::stringstream textStream(content);
        std::string w;
        while (textStream >> w) words.push_back(w);

        // greedy longest-match
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
                i = bestMatchLength;  // skip cac tu da consumed
            }
        }

        result[uuid] = count;
        std::cout << std::left << std::setw(42) << uuid << " | " << count << std::endl;
    }

    if (rowCount == 0) {
        std::cout << "--- [Announcement] The file has only header ---" << std::endl;
    }

    file.close();
    std::cout << "--- End of processing ---" << std::endl;
    return result;
}