#include "preprocessing.h"
Preprocessor::Preprocessor() {
    loadStopwords();
}

void Preprocessor::loadStopwords() {
    stopwords = {
        "a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "as", "at", 
        "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", "can", "could", 
        "did", "do", "does", "doing", "down", "during", "each", "few", "for", "from", "further", "had", "has", 
        "have", "having", "he", "her", "here", "hers", "herself", "him", "himself", "his", "how", "i", "if", 
        "in", "into", "is", "it", "its", "itself", "just", "me", "more", "most", "my", "myself", "no", "nor", 
        "not", "now", "of", "off", "on", "once", "only", "or", "other", "ought", "our", "ours", "ourselves", 
        "out", "over", "own", "same", "she", "should", "so", "some", "such", "than", "that", "the", "their", 
        "theirs", "them", "themselves", "then", "there", "these", "they", "this", "those", "through", "to", 
        "too", "under", "until", "up", "very", "was", "we", "were", "what", "when", "where", "which", "while", 
        "who", "whom", "why", "with", "would", "you", "your", "yours", "yourself", "yourselves"
    };
}

std::vector<std::string> Preprocessor::parseCSVLine(const std::string& line) {
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
std::string Preprocessor::cleanContent(std::string raw) {
    std::string cleaned = "";
    for (char c : raw) {
        if (std::isalnum(c) || c == '\'') {
            cleaned += (char)std::tolower(c);
        } else {
            cleaned += ' ';
        }
    }
    
    std::stringstream ss(cleaned);
    std::string word, result = "";
    while (ss >> word) {
        if (stopwords.find(word) == stopwords.end()) {
            result += word + " ";
        }
    }
    
    if (!result.empty()) result.pop_back();
    return result;
}
std::string Preprocessor::ensureNumeric(std::string val) {
    size_t first = val.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "0";
    size_t last = val.find_last_not_of(" \t\r\n");
    std::string trimmed = val.substr(first, (last - first + 1));

    std::string filtered = "";
    for (size_t i = 0; i < trimmed.length(); ++i) {
        if (i == 0 && trimmed[i] == '-') {
            filtered += '-';
        } else if (std::isdigit(trimmed[i]) || trimmed[i] == '.') {
            filtered += trimmed[i];
        }
    }

    if (filtered.empty() || filtered == "-") return "0";
    return filtered;
}

void Preprocessor::processCSV(std::string inputPath, std::string outputPath) {
    std::ifstream fin(inputPath);
    std::ofstream fout(outputPath);

    if (!fin.is_open()) {
        std::cerr << "Error: Cannot open file " << inputPath << std::endl;
        return;
    }

    std::string line;
    std::string fullRecord = "";
    bool recordInsideQuotes = false;
    fout << "uuid,type,site_url,domain_rank,title,text,spam_score,replies_count,participants_count,likes,comments,shares\n";
    std::getline(fin, line);

    while (std::getline(fin, line)) {
        if (line.empty() && !recordInsideQuotes) continue;

        fullRecord += line;
        for (char c : line) {
            if (c == '"') recordInsideQuotes = !recordInsideQuotes;
        }

        if (recordInsideQuotes) {
            fullRecord += " "; 
            continue;
        }

        std::vector<std::string> row = parseCSVLine(fullRecord);
        fullRecord.clear();

        if (row.size() < 20) continue;
        fout << "\"" << row[0] << "\","                         
             << "\"" << row[19] << "\","                        
             << "\"" << row[8] << "\","                         
             << "\"" << ensureNumeric(row[10]) << "\","         
             << "\"" << cleanContent(row[4]) << "\","          
             << "\"" << cleanContent(row[5]) << "\","           
             << "\"" << ensureNumeric(row[12]) << "\","         
             << "\"" << ensureNumeric(row[14]) << "\","         
             << "\"" << ensureNumeric(row[15]) << "\","         
             << "\"" << ensureNumeric(row[16]) << "\","         
             << "\"" << ensureNumeric(row[17]) << "\","         
             << "\"" << ensureNumeric(row[18]) << "\"\n";       
    }

    fin.close();
    fout.close();
    std::cout << "SUCCESS: Cleaned file saved at " << outputPath << std::endl;
}
