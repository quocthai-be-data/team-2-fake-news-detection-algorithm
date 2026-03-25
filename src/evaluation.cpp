#include "evaluation.h"

// ============================================================
// Constructor
// ============================================================
Evaluator::Evaluator() {}

// ============================================================
// isActualFake — map multi-class labels to binary
//   {"fake", "bs", "conspiracy", "bias"} → FAKE
//   anything else → REAL
// ============================================================
bool Evaluator::isActualFake(const std::string& type) {
    return (type == "fake" || type == "bs" ||
            type == "conspiracy" || type == "bias");
}

// ============================================================
// evaluate — Module 5: Evaluation
//   Compare predicted_label vs actual_type for each article
//   Build confusion matrix and compute 4 metrics
// ============================================================
EvaluationReport Evaluator::evaluate(const std::vector<ScoringResult>& results) {
    EvaluationReport report;
    report.total = results.size();
    report.TP = 0;
    report.TN = 0;
    report.FP = 0;
    report.FN = 0;

    for (size_t i = 0; i < results.size(); ++i) {
        bool actualFake    = isActualFake(results[i].actual_type);
        bool predictedFake = (results[i].predicted_label == "FAKE");

        if (actualFake && predictedFake)        report.TP++;   // Correctly caught fake
        else if (!actualFake && !predictedFake) report.TN++;   // Correctly allowed real
        else if (!actualFake && predictedFake)  report.FP++;   // False alarm
        else if (actualFake && !predictedFake)  report.FN++;   // Missed fake
    }

    // Accuracy = (TP + TN) / total
    report.accuracy = (report.total > 0) ?
        (float)(report.TP + report.TN) / report.total : 0.0f;

    // Precision = TP / (TP + FP)
    report.precision = (report.TP + report.FP > 0) ?
        (float)report.TP / (report.TP + report.FP) : 0.0f;

    // Recall = TP / (TP + FN)
    report.recall = (report.TP + report.FN > 0) ?
        (float)report.TP / (report.TP + report.FN) : 0.0f;

    // F1 = 2 * (Precision * Recall) / (Precision + Recall)
    report.f1_score = (report.precision + report.recall > 0) ?
        2.0f * (report.precision * report.recall) / (report.precision + report.recall) : 0.0f;

    return report;
}

// ============================================================
// displayReport — print evaluation results to console
// ============================================================
void Evaluator::displayReport(const EvaluationReport& report) {
    std::cout << "| " << std::endl;
    std::cout << " --- Evaluation Report ---" << std::endl;
    std::cout << "| " << std::endl;
    std::cout << "  Total articles evaluated: " << report.total << std::endl;
    std::cout << "| " << std::endl;
    std::cout << "  Confusion Matrix:" << std::endl;
    std::cout << "    True Positive  (TP): " << report.TP << std::endl;
    std::cout << "    True Negative  (TN): " << report.TN << std::endl;
    std::cout << "    False Positive (FP): " << report.FP << std::endl;
    std::cout << "    False Negative (FN): " << report.FN << std::endl;
    std::cout << "| " << std::endl;
    std::cout << "  Metrics:" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "    Accuracy  : " << report.accuracy  << std::endl;
    std::cout << "    Precision : " << report.precision  << std::endl;
    std::cout << "    Recall    : " << report.recall     << std::endl;
    std::cout << "    F1 Score  : " << report.f1_score   << std::endl;
    std::cout << "| " << std::endl;
    std::cout << " --- End of Evaluation Report ---" << std::endl;
}

// ============================================================
// writeReport — save evaluation_report.txt
// ============================================================
void Evaluator::writeReport(const std::string& outputPath,
                             const EvaluationReport& report) {
    std::ofstream fout(outputPath);
    if (!fout.is_open()) {
        std::cerr << "[Error] Cannot write to: " << outputPath << std::endl;
        return;
    }

    fout << "===== Evaluation Report =====" << std::endl;
    fout << "Total articles evaluated: " << report.total << std::endl;
    fout << std::endl;
    fout << "Confusion Matrix:" << std::endl;
    fout << "  True Positive  (TP): " << report.TP << std::endl;
    fout << "  True Negative  (TN): " << report.TN << std::endl;
    fout << "  False Positive (FP): " << report.FP << std::endl;
    fout << "  False Negative (FN): " << report.FN << std::endl;
    fout << std::endl;
    fout << "Metrics:" << std::endl;
    fout << std::fixed << std::setprecision(3);
    fout << "  Accuracy  : " << report.accuracy  << std::endl;
    fout << "  Precision : " << report.precision  << std::endl;
    fout << "  Recall    : " << report.recall     << std::endl;
    fout << "  F1 Score  : " << report.f1_score   << std::endl;
    fout << "=============================" << std::endl;

    fout.close();
    std::cout << " --- Evaluation report saved to: " << outputPath << " ---" << std::endl;
}
