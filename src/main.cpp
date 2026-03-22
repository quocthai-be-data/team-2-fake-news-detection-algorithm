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
    Trie detector;
    detector.loadSuspiciousKeywords("data/SuspiciousKeywords.csv");
    detector.processFileAndDisplay("data/dataCleaned.csv");
    system("pause");
    return 0;
}
