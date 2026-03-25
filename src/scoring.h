#ifndef SCORING_H
#define SCORING_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "hashMap.h"

// ============================================================
// ScoringResult — stores the scoring output for one article
// ============================================================
struct ScoringResult {
    std::string uuid;
    std::string predicted_label;    // "REAL" or "FAKE"
    float       final_score;
    std::string actual_type;        // ground truth from dataset
};

// ============================================================
// Scorer — computes credibility scores and classifies articles
// ============================================================
class Scorer {
public:
    Scorer(float threshold = 0.7f);

    // Score all articles using data from HashMap + Trie keyword counts
    std::vector<ScoringResult> scoreArticles(
        HashMap& hashmap,
        std::map<std::string, int>& keywordCounts
    );

    // Write results to CSV file
    void writeResultsCSV(const std::string& outputPath,
                         const std::vector<ScoringResult>& results);

private:
    float classificationThreshold;  // default 0.7

    // Sub-score calculations
    float computeSourceScore(int domain_rank);
    float computeContentScore(float spam_score, int keyword_count);
    float computeEngagementScore(int likes, int comments, int shares,
                                 int replies_count, int participants_count);
};

#endif
