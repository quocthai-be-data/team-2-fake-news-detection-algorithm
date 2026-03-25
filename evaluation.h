#ifndef EVALUATION_H
#define EVALUATION_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "scoring.h"

// ============================================================
// EvaluationReport — stores confusion matrix + metrics
// ============================================================
struct EvaluationReport {
    int   total;
    int   TP;       // True Positive  (actual FAKE, predicted FAKE)
    int   TN;       // True Negative  (actual REAL, predicted REAL)
    int   FP;       // False Positive (actual REAL, predicted FAKE)
    int   FN;       // False Negative (actual FAKE, predicted REAL)
    float accuracy;
    float precision;
    float recall;
    float f1_score;
};

// ============================================================
// Evaluator — compares predicted labels vs ground truth
// ============================================================
class Evaluator {
public:
    Evaluator();

    // Compute confusion matrix + metrics from scoring results
    EvaluationReport evaluate(const std::vector<ScoringResult>& results);

    // Display evaluation report to console
    void displayReport(const EvaluationReport& report);

    // Write evaluation report to text file
    void writeReport(const std::string& outputPath,
                     const EvaluationReport& report);

private:
    // Check if actual_type is a "FAKE" category
    bool isActualFake(const std::string& type);
};

#endif
