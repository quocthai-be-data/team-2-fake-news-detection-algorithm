#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

class Preprocessor {
private:
    std::unordered_set<std::string> stopwords;
    
    // Các hàm bổ trợ (Helper functions) để chế độ private
    void loadStopwords();
    std::vector<std::string> parseCSVLine(const std::string& line);
    std::string cleanContent(std::string raw);
    std::string ensureNumeric(std::string val);

public:
    Preprocessor(); // Constructor
    void processCSV(std::string inputPath, std::string outputPath);
};

#endif
