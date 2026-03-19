#include "preprocessing.h"
#include "Preprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Preprocessor::Preprocessor() {
    loadStopwords();
}

void Preprocessor::loadStopwords() {
    stopwords = {"a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "as", "at", 
            "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", "can", "could", 
            "did", "do", "does", "doing", "down", "during", "each", "few", "for", "from", "further", "had", "has", 
            "have", "having", "he", "her", "here", "hers", "herself", "him", "himself", "his", "how", "i", "if", 
            "in", "into", "is", "it", "its", "itself", "just", "me", "more", "most", "my", "myself", "no", "nor", 
            "not", "now", "of", "off", "on", "once", "only", "or", "other", "ought", "our", "ours", "ourselves", 
            "out", "over", "own", "same", "she", "should", "so", "some", "such", "than", "that", "the", "their", 
            "theirs", "them", "themselves", "then", "there", "these", "they", "this", "those", "through", "to", 
            "too", "under", "until", "up", "very", "was", "we", "were", "what", "when", "where", "which", "while", 
            "who", "whom", "why", "with", "would", "you", "your", "yours", "yourself", "yourselves"}; // Rút gọn cho ví dụ
}

std::vector<std::string> Preprocessor::parseCSVLine(const std::string& line) {
    std::vector<std::string> row;
        std::string cell;
        bool insideQuotes = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (c == '"') {
                // Xử lý dấu ngoặc kép "" (escape)
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
            if (std::isalnum(c)) cleaned += (char)std::tolower(c);
            else cleaned += ' ';
        }
        std::stringstream ss(cleaned);
        std::string word, result = "";
        while (ss >> word) {
            if (stopwords.find(word) == stopwords.end()) result += word + " ";
        }
        if (!result.empty()) result.pop_back();
        return result;}

std::string Preprocessor::ensureNumeric(std::string val) {
   if (val.empty() || val == " " || val == "\r" || val == "\n") return "0";
        val.erase(std::remove_if(val.begin(), val.end(), [](char c) { 
            return !std::isdigit(c) && c != '.'; 
        }), val.end());
        return val.empty() ? "0" : val;
}

void Preprocessor::processCSV(std::string inputPath, std::string outputPath) {
     std::ifstream fin(inputPath);
        std::ofstream fout(outputPath);

        if (!fin.is_open()) {
            std::cerr << "Loi: Khong tim thay file " << inputPath << std::endl;
            return;
        }

        std::string line;
        std::string fullRecord = "";
        bool recordInsideQuotes = false;

        // Ghi Header cho file output
        fout << "uuid,type,site_url,domain_rank,title_clean,text_clean,spam_score,replies_count,participants_count,likes,comments,shares\n";

        // Bỏ qua dòng tiêu đề của file input
        std::getline(fin, line);

        // --- MÃ NGUỒN 2: LOGIC GHÉP DÒNG (Xử lý xuống dòng trong ngoặc kép) ---
        while (std::getline(fin, line)) {
            if (line.empty() && !recordInsideQuotes) continue;

            fullRecord += line;

            // Kiểm tra xem dòng hiện tại có chứa dấu ngoặc kép chưa đóng không
            for (char c : line) {
                if (c == '"') recordInsideQuotes = !recordInsideQuotes;
            }

            // Nếu dấu ngoặc chưa đóng, tiếp tục đọc dòng tiếp theo và nối vào
            if (recordInsideQuotes) {
                fullRecord += " "; // Thêm khoảng trắng thay cho dấu xuống dòng
                continue;
            }

            // Đã có một Record hoàn chỉnh, tiến hành parse
            std::vector<std::string> row = parseCSVLine(fullRecord);
            fullRecord.clear(); // Reset cho record tiếp theo

            if (row.size() < 20) continue; // Chỉ xử lý nếu đủ cấu trúc cột

            // Trích xuất và clean (Chỉ số cột dựa trên dataset của bạn)
            fout << row[0] << ","                         // uuid
                 << row[19] << ","                        // type
                 << row[8] << ","                         // site_url
                 << ensureNumeric(row[10]) << ","         // domain_rank
                 << "\"" << cleanContent(row[4]) << "\"," // title
                 << "\"" << cleanContent(row[5]) << "\"," // text
                 << ensureNumeric(row[12]) << ","         // spam_score
                 << ensureNumeric(row[14]) << ","         // replies
                 << ensureNumeric(row[15]) << ","         // participants
                 << ensureNumeric(row[16]) << ","         // likes
                 << ensureNumeric(row[17]) << ","         // comments
                 << ensureNumeric(row[18]) << "\n";       // shares
        }

        fin.close();
        fout.close();
        std::cout << "SUCCESS: File da duoc lam sach tai " << outputPath << std::endl;
}
