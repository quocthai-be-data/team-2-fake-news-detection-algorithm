#include "preprocessing.h"
#include "trie.h"
#include "hashMap.h"
#include "scoring.h"
using namespace std;

int main(){
    Preprocessor p;
    p.processCSV("data/dataTesting.csv", "data/dataCleaned.csv");
    
    Trie detector;
    detector.loadSuspiciousKeywords("data/SuspiciousKeywords.csv");
    detector.processFileAndDisplay("data/dataCleaned.csv");
    
    return 0;
}
