#ifndef WFM_CORE_RLHF_GOLDEN_DATASET_MANAGER_H
#define WFM_CORE_RLHF_GOLDEN_DATASET_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include "models/golden_dataset_entry.h"
#include "models/feedback_event.h"
#include "core/rlhf/feedback_collector.h"

namespace wfm::rlhf {

// GoldenDatasetManager: Manages high-quality training data from operator feedback
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - Con.2: Make member functions const by default
class GoldenDatasetManager {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit GoldenDatasetManager(std::unique_ptr<FeedbackCollector> collector);

    // Create golden dataset entry from feedback event
    // Automatically assigns quality score based on heuristics
    GoldenDatasetEntry create_entry_from_feedback(
        const FeedbackEvent& feedback,
        const std::string& ticket_data_json
    ) const;

    // Create golden dataset entry with manual quality score
    GoldenDatasetEntry create_entry_with_quality(
        const FeedbackEvent& feedback,
        const std::string& ticket_data_json,
        int quality_score
    ) const;

    // Calculate quality score for feedback event
    // Returns score 1-5 based on disagreement category and other factors
    int calculate_quality_score(const FeedbackEvent& feedback) const noexcept;

    // Add entry to golden dataset
    void add_to_dataset(const GoldenDatasetEntry& entry) const;

    // Get all entries ready for training (quality >= 3, not exported)
    std::vector<GoldenDatasetEntry> get_ready_for_training() const;

    // Get high-quality entries (quality >= 4)
    std::vector<GoldenDatasetEntry> get_high_quality_entries() const;

    // Mark entries as exported
    void mark_as_exported(const std::vector<std::string>& entry_ids) const;

    // Export dataset to JSON format for training pipeline
    std::string export_to_json(const std::vector<GoldenDatasetEntry>& entries) const;

    // Get dataset statistics
    GoldenDatasetStats get_dataset_stats() const;

    // Validate entry meets quality requirements
    bool validate_entry(const GoldenDatasetEntry& entry) const noexcept;

    // Filter entries by minimum quality score
    std::vector<GoldenDatasetEntry> filter_by_quality(
        const std::vector<GoldenDatasetEntry>& entries,
        int min_quality
    ) const;

private:
    std::unique_ptr<FeedbackCollector> collector_;  // R.20: unique_ptr for ownership

    // In-memory storage for testing (production would use database)
    mutable std::vector<GoldenDatasetEntry> dataset_;

    // Helper: Validate ticket data JSON
    bool validate_ticket_json(const std::string& json) const noexcept;

    // Helper: Generate unique entry ID
    std::string generate_entry_id() const;
};

} // namespace wfm::rlhf

#endif // WFM_CORE_RLHF_GOLDEN_DATASET_MANAGER_H
