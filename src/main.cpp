#include "preprocessing.h"
#include "trie.h"
#include "hashMap.h"
#include "scoring.h"

int main(){
    // Preprocessor p;
    // p.processCSV("data/dataTesting.csv", "data/dataCleaned.csv"); doan nay khong can nua vi da tao file clean roi
    std::cout << " --- Data has been cleaned successfully ---- \n";
    std::cout << "| \n";
    std::cout << "| \n";

    //Trie iterates and score point for each articles by its numbers of suspicious words that is duplicated with word in the list
    Trie detector;
    detector.loadSuspiciousKeywords("data/SuspiciousKeywords.csv");
    std::map<std::string, int> keywordCounts = detector.processFileAndDisplay("data/dataCleaned.csv");
    for (auto i = keywordCounts.begin(); i != keywordCounts.end(); ++i){
        std::cout << i->first << " | " << i->second << std::endl;
    }
    system("pause");
    return 0;
}
