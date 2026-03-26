// GoldenDatasetManager implementation
// Following C++ Core Guidelines and TDD principles

#include "core/rlhf/golden_dataset_manager.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace wfm::rlhf {

// I.11: Take ownership by unique_ptr (following RAII)
GoldenDatasetManager::GoldenDatasetManager(std::unique_ptr<FeedbackCollector> collector)
    : collector_(std::move(collector)), dataset_() {
    if (!collector_) {
        throw std::invalid_argument("FeedbackCollector cannot be null");
    }
}

// Create golden dataset entry from feedback event
GoldenDatasetEntry GoldenDatasetManager::create_entry_from_feedback(
    const FeedbackEvent& feedback,
    const std::string& ticket_data_json
) const {
    // Calculate quality score automatically
    const int quality = calculate_quality_score(feedback);

    return create_entry_with_quality(feedback, ticket_data_json, quality);
}

// Create golden dataset entry with manual quality score
GoldenDatasetEntry GoldenDatasetManager::create_entry_with_quality(
    const FeedbackEvent& feedback,
    const std::string& ticket_data_json,
    const int quality_score
) const {
    if (quality_score < 1 || quality_score > 5) {
        throw std::invalid_argument("Quality score must be between 1 and 5");
    }

    GoldenDatasetEntry entry(
        generate_entry_id(),
        ticket_data_json,
        feedback.actual_action,
        feedback.confidence_score,
        quality_score
    );

    entry.feedback_event_id = feedback.id;
    entry.exported = false;
    entry.created_at = feedback.created_at;

    return entry;
}

// Calculate quality score for feedback event
// Returns score 1-5 based on disagreement category and other factors
int GoldenDatasetManager::calculate_quality_score(const FeedbackEvent& feedback) const noexcept {
    // Quality scoring heuristic based on disagreement category
    switch (feedback.category) {
        case DisagreementCategory::EdgeCase:
            // Unseen edge cases are highest value for training
            return 5;

        case DisagreementCategory::PolicyChange:
            // Policy changes are high value (new ground truth)
            return 5;

        case DisagreementCategory::ModelError:
            // Clear model errors are good training signal
            return 4;

        case DisagreementCategory::AmbiguousCase:
            // Ambiguous cases have moderate value
            return 3;

        case DisagreementCategory::ContextMissing:
            // Context missing indicates need for better feature extraction
            return 3;

        case DisagreementCategory::UserError:
            // User errors are low quality - not useful for training
            return 1;

        case DisagreementCategory::Other:
        default:
            // Default moderate quality
            return 3;
    }
}

// Add entry to golden dataset
void GoldenDatasetManager::add_to_dataset(const GoldenDatasetEntry& entry) const {
    if (!validate_entry(entry)) {
        throw std::invalid_argument("Invalid golden dataset entry");
    }

    dataset_.push_back(entry);
}

// Get all entries ready for training (quality >= 3, not exported)
std::vector<GoldenDatasetEntry> GoldenDatasetManager::get_ready_for_training() const {
    std::vector<GoldenDatasetEntry> ready;

    for (const auto& entry : dataset_) {
        if (entry.is_ready_for_training()) {
            ready.push_back(entry);
        }
    }

    return ready;
}

// Get high-quality entries (quality >= 4)
std::vector<GoldenDatasetEntry> GoldenDatasetManager::get_high_quality_entries() const {
    std::vector<GoldenDatasetEntry> high_quality;

    for (const auto& entry : dataset_) {
        if (entry.is_high_quality()) {
            high_quality.push_back(entry);
        }
    }

    return high_quality;
}

// Mark entries as exported
void GoldenDatasetManager::mark_as_exported(const std::vector<std::string>& entry_ids) const {
    for (auto& entry : dataset_) {
        for (const auto& id : entry_ids) {
            if (entry.id == id) {
                entry.exported = true;
                break;
            }
        }
    }
}

// Export dataset to JSON format for training pipeline
std::string GoldenDatasetManager::export_to_json(
    const std::vector<GoldenDatasetEntry>& entries
) const {
    std::ostringstream json;

    json << "{\n";
    json << "  \"dataset\": [\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];

        json << "    {\n";
        json << "      \"id\": \"" << entry.id << "\",\n";
        json << "      \"ticket_data\": " << entry.ticket_data << ",\n";
        json << "      \"operator_action\": \"" << entry.operator_action << "\",\n";
        json << "      \"confidence_score\": " << std::fixed << std::setprecision(4)
             << entry.confidence_score << ",\n";
        json << "      \"quality_score\": " << entry.quality_score << ",\n";
        json << "      \"feedback_event_id\": \"" << entry.feedback_event_id << "\"\n";
        json << "    }";

        if (i < entries.size() - 1) {
            json << ",";
        }
        json << "\n";
    }

    json << "  ],\n";
    json << "  \"metadata\": {\n";
    json << "    \"total_entries\": " << entries.size() << ",\n";
    json << "    \"exported_at\": \"" << "2024-01-01T00:00:00Z" << "\"\n";
    json << "  }\n";
    json << "}";

    return json.str();
}

// Get dataset statistics
GoldenDatasetStats GoldenDatasetManager::get_dataset_stats() const {
    GoldenDatasetStats stats;

    stats.total_entries = dataset_.size();

    if (stats.total_entries == 0) {
        return stats;
    }

    double total_quality = 0.0;

    for (const auto& entry : dataset_) {
        if (entry.exported) {
            ++stats.exported_entries;
        }

        if (entry.is_ready_for_training()) {
            ++stats.ready_for_training;
        }

        if (entry.is_high_quality()) {
            ++stats.high_quality_entries;
        }

        total_quality += entry.quality_score;
    }

    stats.avg_quality_score = total_quality / static_cast<double>(stats.total_entries);

    return stats;
}

// Validate entry meets quality requirements
bool GoldenDatasetManager::validate_entry(const GoldenDatasetEntry& entry) const noexcept {
    // Check required fields
    if (entry.id.empty()) {
        return false;
    }

    if (entry.ticket_data.empty()) {
        return false;
    }

    if (entry.operator_action.empty()) {
        return false;
    }

    // Check quality score range
    if (!entry.has_valid_quality_score()) {
        return false;
    }

    // Validate ticket data JSON (basic check)
    if (!validate_ticket_json(entry.ticket_data)) {
        return false;
    }

    return true;
}

// Filter entries by minimum quality score
std::vector<GoldenDatasetEntry> GoldenDatasetManager::filter_by_quality(
    const std::vector<GoldenDatasetEntry>& entries,
    const int min_quality
) const {
    std::vector<GoldenDatasetEntry> filtered;

    for (const auto& entry : entries) {
        if (entry.quality_score >= min_quality) {
            filtered.push_back(entry);
        }
    }

    return filtered;
}

// Helper: Validate ticket data JSON
bool GoldenDatasetManager::validate_ticket_json(const std::string& json) const noexcept {
    // Basic JSON validation - check for braces
    if (json.empty()) {
        return false;
    }

    // Must start and end with braces (simplistic check)
    const auto first_brace = json.find('{');
    const auto last_brace = json.rfind('}');

    return first_brace != std::string::npos && last_brace != std::string::npos;
}

// Helper: Generate unique entry ID
std::string GoldenDatasetManager::generate_entry_id() const {
    // Simple counter-based ID generation
    // In production, would use UUID or database auto-increment
    static size_t counter = 0;
    return "entry_" + std::to_string(++counter);
}

} // namespace wfm::rlhf
