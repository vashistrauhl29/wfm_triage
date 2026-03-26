#ifndef WFM_MODELS_GOLDEN_DATASET_ENTRY_H
#define WFM_MODELS_GOLDEN_DATASET_ENTRY_H

#include <string>
#include <cstdint>

namespace wfm {

// Domain model: Golden dataset entry for RLHF training
// Represents high-quality training examples from operator feedback
struct GoldenDatasetEntry {
    std::string id;                      // Unique entry identifier
    std::string ticket_data;             // Full ticket data (JSON)
    std::string operator_action;         // Ground truth action from operator
    double confidence_score;             // Original AI confidence score
    int quality_score;                   // Quality rating 1-5 (5 = highest quality)
    bool exported;                       // Whether exported to training pipeline
    std::string feedback_event_id;       // Reference to originating feedback event
    std::string created_at;              // Timestamp

    // ES.20: Always initialize
    GoldenDatasetEntry()
        : id(),
          ticket_data(),
          operator_action(),
          confidence_score(0.0),
          quality_score(0),
          exported(false),
          feedback_event_id(),
          created_at() {}

    // Constructor with essential fields
    GoldenDatasetEntry(
        std::string entry_id,
        std::string ticket_json,
        std::string action,
        double confidence,
        int quality
    ) : id(std::move(entry_id)),
        ticket_data(std::move(ticket_json)),
        operator_action(std::move(action)),
        confidence_score(confidence),
        quality_score(quality),
        exported(false),
        feedback_event_id(),
        created_at() {}

    // Validate quality score is in range [1, 5]
    bool has_valid_quality_score() const noexcept {
        return quality_score >= 1 && quality_score <= 5;
    }

    // Check if entry is ready for training (high quality, not yet exported)
    bool is_ready_for_training() const noexcept {
        return has_valid_quality_score() &&
               quality_score >= 3 &&  // Only use quality >= 3
               !exported &&
               !ticket_data.empty() &&
               !operator_action.empty();
    }

    // Check if this is high-quality training data
    bool is_high_quality() const noexcept {
        return quality_score >= 4;
    }
};

// Statistics about golden dataset
struct GoldenDatasetStats {
    size_t total_entries;           // Total entries in dataset
    size_t exported_entries;        // Number already exported
    size_t ready_for_training;      // Number ready for training
    size_t high_quality_entries;    // Number with quality >= 4
    double avg_quality_score;       // Average quality score

    GoldenDatasetStats()
        : total_entries(0),
          exported_entries(0),
          ready_for_training(0),
          high_quality_entries(0),
          avg_quality_score(0.0) {}
};

} // namespace wfm

#endif // WFM_MODELS_GOLDEN_DATASET_ENTRY_H
