#include "scoring.h"

// ============================================================
// Constructor
// ============================================================
Scorer::Scorer(float threshold) {
    classificationThreshold = threshold;
}

// ============================================================
// source_score — based on domain_rank
//   domain_rank == 0       → 0.3 (unknown)
//   domain_rank <= 10000   → 0.9 (highly reputable)
//   domain_rank <= 50000   → 0.6 (moderate)
//   domain_rank >  50000   → 0.2 (low credibility)
// ============================================================
float Scorer::computeSourceScore(int domain_rank) {
    if (domain_rank == 0)       return 0.3f;
    if (domain_rank <= 10000)   return 0.9f;
    if (domain_rank <= 50000)   return 0.6f;
    return 0.2f;
}

// ============================================================
// content_score — from spam_score + keyword_count
//   base_score      = 1.0 - spam_score
//   keyword_penalty = keyword_count * 0.1
//   content_score   = max(0.0, base_score - keyword_penalty)
// ============================================================
float Scorer::computeContentScore(float spam_score, int keyword_count) {
    float base_score      = 1.0f - spam_score;
    float keyword_penalty = keyword_count * 0.1f;
    float score = base_score - keyword_penalty;
    if (score < 0.0f) score = 0.0f;
    return score;
}

// ============================================================
// engagement_score — from social metrics
//   total = likes + comments + shares + replies + participants
//   if total == 0  → 0.5 (neutral)
//   if total >  0  → min(1.0, total / 100.0)
// ============================================================
float Scorer::computeEngagementScore(int likes, int comments, int shares,
                                     int replies_count, int participants_count) {
    int total = likes + comments + shares + replies_count + participants_count;
    if (total == 0) return 0.5f;
    float score = total / 100.0f;
    if (score > 1.0f) score = 1.0f;
    return score;
}

// ============================================================
// scoreArticles — Module 4: Scoring
//   For each article in keywordCounts:
//     1. Get metadata from HashMap via get(uuid)
//     2. Compute 3 sub-scores
//     3. Final = 0.4*source + 0.4*content + 0.2*engagement
//     4. Classify: >= threshold → REAL, < threshold → FAKE
// ============================================================
std::vector<ScoringResult> Scorer::scoreArticles(
    HashMap& hashmap,
    std::map<std::string, int>& keywordCounts
) {
    std::cout << " --- Start Scoring Articles ---" << std::endl;
    std::cout << "| " << std::endl;
    std::cout << " --- Computing credibility score for each article ---" << std::endl;
    std::cout << "| " << std::endl;

    // Table header
    std::cout << std::left
              << std::setw(42) << "UUID" << " | "
              << std::setw(8)  << "Source" << " | "
              << std::setw(8)  << "Content" << " | "
              << std::setw(8)  << "Engage" << " | "
              << std::setw(8)  << "Final" << " | "
              << "Label"
              << std::endl;
    std::cout << "------------------------------------------------------------"
              << "------------------------------------------------------------" << std::endl;

    std::vector<ScoringResult> results;

    for (auto it = keywordCounts.begin(); it != keywordCounts.end(); ++it) {
        std::string uuid = it->first;
        int keyword_count = it->second;

        // Lookup metadata from HashMap
        NewsRecord* rec = hashmap.get(uuid);
        if (rec == nullptr) {
            std::cout << "[WARNING] UUID not found in HashMap: " << uuid << std::endl;
            continue;
        }

        // Compute sub-scores
        float source_score     = computeSourceScore(rec->domain_rank);
        float content_score    = computeContentScore(rec->spam_score, keyword_count);
        float engagement_score = computeEngagementScore(
            rec->likes, rec->comments, rec->shares,
            rec->replies_count, rec->participants_count
        );

        // Final score = 0.4*source + 0.4*content + 0.2*engagement
        float final_score = (0.4f * source_score) +
                            (0.4f * content_score) +
                            (0.2f * engagement_score);

        // Classification
        std::string predicted_label = (final_score >= classificationThreshold) ? "REAL" : "FAKE";

        // Store result
        ScoringResult sr;
        sr.uuid             = uuid;
        sr.predicted_label  = predicted_label;
        sr.final_score      = final_score;
        sr.actual_type      = rec->type;
        results.push_back(sr);

        // Display row
        std::cout << std::left << std::fixed << std::setprecision(3)
                  << std::setw(42) << uuid << " | "
                  << std::setw(8)  << source_score << " | "
                  << std::setw(8)  << content_score << " | "
                  << std::setw(8)  << engagement_score << " | "
                  << std::setw(8)  << final_score << " | "
                  << predicted_label
                  << std::endl;
    }

    std::cout << "| " << std::endl;
    std::cout << " --- Scoring complete: " << results.size() << " articles scored ---" << std::endl;

    return results;
}

// ============================================================
// writeResultsCSV — write results.csv
//   Format: uuid, predicted_label, final_score, actual_type
// ============================================================
void Scorer::writeResultsCSV(const std::string& outputPath,
                              const std::vector<ScoringResult>& results) {
    std::ofstream fout(outputPath);
    if (!fout.is_open()) {
        std::cerr << "[Error] Cannot write to: " << outputPath << std::endl;
        return;
    }

    fout << "uuid,predicted_label,final_score,actual_type" << std::endl;

    for (size_t i = 0; i < results.size(); ++i) {
        fout << results[i].uuid << ","
             << results[i].predicted_label << ","
             << std::fixed << std::setprecision(3) << results[i].final_score << ","
             << results[i].actual_type
             << std::endl;
    }

    fout.close();
    std::cout << "| " << std::endl;
    std::cout << " --- Results saved to: " << outputPath << " ---" << std::endl;
}
