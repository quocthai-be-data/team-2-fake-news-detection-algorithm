#include "preprocessing.h"
#include "trie.h"
#include "hashMap.h"
#include "scoring.h"
using namespace std;

int main(){
    Preprocessor p;
    p.processCSV("data/dataTesting.csv", "dataCleaned.csv");
    
    Trie detector;
    detector.loadSuspiciousKeywords("SuspiciousKeywords.csv");
    detector.processFileAndDisplay("dataCleaned.csv");
    
    return 0;
}
