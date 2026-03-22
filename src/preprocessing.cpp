#include "preprocessing.h"

Preprocessor::Preprocessor() {
    loadStopwords();
}

void Preprocessor::loadStopwords() {
    stopwords = {
        "a", "about", "above", "after", "again", "against", "all", "am", "an", "and",
        "any", "are", "as", "at", "be", "because", "been", "before", "being", "below",
        "between", "both", "but", "by", "can", "could", "did", "do", "does", "doing",
        "down", "during", "each", "few", "for", "from", "further", "had", "has", "have",
        "having", "he", "her", "here", "hers", "herself", "him", "himself", "his", "how",
        "i", "if", "in", "into", "is", "it", "its", "itself", "just", "me", "more",
        "most", "my", "myself", "no", "nor", "not", "now", "of", "off", "on", "once",
        "only", "or", "other", "ought", "our", "ours", "ourselves", "out", "over", "own",
        "same", "she", "should", "so", "some", "such", "than", "that", "the", "their",
        "theirs", "them", "themselves", "then", "there", "these", "they", "this", "those",
        "through", "to", "too", "under", "until", "up", "very", "was", "we", "were",
        "what", "when", "where", "which", "while", "who", "whom", "why", "with", "would",
        "you", "your", "yours", "yourself", "yourselves"
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

// Giu lai dau ' (apostrophe) de bao toan nghia phu dinh (don't, won't, isn't)
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

// FIX 1: Xu ly so am, chi cho phep 1 dau cham, trim whitespace
std::string Preprocessor::ensureNumeric(std::string val) {
    size_t start = val.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "0";

    size_t end = val.find_last_not_of(" \t\r\n");
    val = val.substr(start, end - start + 1);

    std::string result = "";
    bool hasDecimal = false;

    for (size_t i = 0; i < val.size(); ++i) {
        char c = val[i];
        if (c == '-' && i == 0 && result.empty()) {
            result += c;
        } else if (c == '.' && !hasDecimal) {
            result += c;
            hasDecimal = true;
        } else if (std::isdigit(c)) {
            result += c;
        }
    }

    if (result.empty() || result == "-" || result == ".") return "0";
    return result;
}

// FIX 2: Escape field cho output CSV — wrap trong "" va escape dau " ben trong
std::string Preprocessor::escapeCSV(const std::string& field) {
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

void Preprocessor::processCSV(std::string inputPath, std::string outputPath) {
    std::ifstream fin(inputPath);
    std::ofstream fout(outputPath);

    if (!fin.is_open()) {
        std::cerr << "Error: Cannot open file " << inputPath << std::endl;
        return;
    }

    // FIX 4: Check output file
    if (!fout.is_open()) {
        std::cerr << "Error: Cannot write to " << outputPath << std::endl;
        fin.close();
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

        // CSV header goc (da verify):
        // 0:uuid, 1:ord_in_thread, 2:author, 3:published, 4:title, 5:text,
        // 6:language, 7:crawled, 8:site_url, 9:country, 10:domain_rank,
        // 11:thread_title, 12:spam_score, 13:main_img_url, 14:replies_count,
        // 15:participants_count, 16:likes, 17:comments, 18:shares, 19:type

        // FIX 3: escapeCSV cho text, ghi truc tiep cho so
        fout << escapeCSV(row[0]) << ","                  // uuid (text)
             << escapeCSV(row[19]) << ","                 // type (text)
             << escapeCSV(row[8]) << ","                  // site_url (text)
             << ensureNumeric(row[10]) << ","             // domain_rank (so)
             << escapeCSV(cleanContent(row[4])) << ","    // title (text, cleaned)
             << escapeCSV(cleanContent(row[5])) << ","    // text (text, cleaned)
             << ensureNumeric(row[12]) << ","             // spam_score (so)
             << ensureNumeric(row[14]) << ","             // replies_count (so)
             << ensureNumeric(row[15]) << ","             // participants_count (so)
             << ensureNumeric(row[16]) << ","             // likes (so)
             << ensureNumeric(row[17]) << ","             // comments (so)
             << ensureNumeric(row[18]) << "\n";           // shares (so)
    }

    fin.close();
    fout.close();
    std::cout << "SUCCESS: Cleaned file saved at " << outputPath << std::endl;
}