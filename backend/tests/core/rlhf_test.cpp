// Unit tests for RLHF Capture Gate module
// Following C++ Core Guidelines and Google Test best practices
// TDD Approach: Tests written before implementation

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

// Forward declarations - implementation files to be created
#include "core/rlhf/feedback_collector.h"
#include "core/rlhf/golden_dataset_manager.h"
#include "models/feedback_event.h"
#include "models/golden_dataset_entry.h"
#include "models/ticket.h"

namespace wfm::rlhf {

// ============================================================================
// Mock FeedbackCollector for Testing
// ============================================================================

class MockFeedbackCollector : public FeedbackCollector {
public:
    MockFeedbackCollector() : event_counter_(0), total_events_(0) {}

    FeedbackEvent capture_override(
        const std::string& ticket_id,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes
    ) const override {
        FeedbackEvent event(
            "feedback_" + std::to_string(++event_counter_),
            ticket_id,
            operator_id,
            recommended_action,
            actual_action,
            category
        );
        event.disagreement_notes = notes;
        event.confidence_score = 0.85;  // Default confidence
        event.created_at = "2024-01-01T00:00:00Z";

        feedback_events_.push_back(event);
        ++total_events_;

        return event;
    }

    FeedbackEvent capture_override_with_context(
        const Ticket& ticket,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes
    ) const override {
        auto event = capture_override(
            ticket.id(),
            operator_id,
            recommended_action,
            actual_action,
            category,
            notes
        );
        event.confidence_score = ticket.confidence_score();
        // Update in storage
        for (auto& e : feedback_events_) {
            if (e.id == event.id) {
                e.confidence_score = event.confidence_score;
                break;
            }
        }
        return event;
    }

    std::vector<FeedbackEvent> get_feedback_for_ticket(
        const std::string& ticket_id
    ) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& event : feedback_events_) {
            if (event.ticket_id == ticket_id) {
                result.push_back(event);
            }
        }
        return result;
    }

    std::vector<FeedbackEvent> get_feedback_by_category(
        DisagreementCategory category
    ) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& event : feedback_events_) {
            if (event.category == category) {
                result.push_back(event);
            }
        }
        return result;
    }

    std::vector<FeedbackEvent> get_feedback_by_operator(
        const std::string& operator_id
    ) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& event : feedback_events_) {
            if (event.operator_id == operator_id) {
                result.push_back(event);
            }
        }
        return result;
    }

    size_t count_disagreements() const noexcept override {
        size_t count = 0;
        for (const auto& event : feedback_events_) {
            if (event.is_disagreement()) {
                ++count;
            }
        }
        return count;
    }

    size_t count_disagreements_by_category(
        DisagreementCategory category
    ) const noexcept override {
        size_t count = 0;
        for (const auto& event : feedback_events_) {
            if (event.is_disagreement() && event.category == category) {
                ++count;
            }
        }
        return count;
    }

    double calculate_disagreement_rate() const noexcept override {
        if (total_events_ == 0) return 0.0;
        return static_cast<double>(count_disagreements()) / static_cast<double>(total_events_);
    }

    bool validate_feedback(const FeedbackEvent& event) const noexcept override {
        return !event.ticket_id.empty() &&
               !event.operator_id.empty() &&
               !event.actual_action.empty();
    }

private:
    mutable size_t event_counter_;
    mutable size_t total_events_;
    mutable std::vector<FeedbackEvent> feedback_events_;
};

// ============================================================================
// FeedbackCollector Test Suite
// ============================================================================

class FeedbackCollectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        collector_ = std::make_unique<MockFeedbackCollector>();
    }

    void TearDown() override {
        collector_.reset();
    }

    std::unique_ptr<FeedbackCollector> collector_;
};

// Test: Capture basic override event
TEST_F(FeedbackCollectorTest, CapturesBasicOverride) {
    // Arrange
    const std::string ticket_id = "ticket_001";
    const std::string operator_id = "operator_123";
    const std::string recommended = "approve";
    const std::string actual = "reject";
    const auto category = DisagreementCategory::PolicyChange;

    // Act
    const auto event = collector_->capture_override(
        ticket_id, operator_id, recommended, actual, category
    );

    // Assert
    EXPECT_FALSE(event.id.empty());
    EXPECT_EQ(event.ticket_id, ticket_id);
    EXPECT_EQ(event.operator_id, operator_id);
    EXPECT_EQ(event.recommended_action, recommended);
    EXPECT_EQ(event.actual_action, actual);
    EXPECT_EQ(event.category, DisagreementCategory::PolicyChange);
}

// Test: Capture override with notes
TEST_F(FeedbackCollectorTest, CapturesOverrideWithNotes) {
    // Arrange
    const std::string notes = "Policy updated on 2024-01-01, new guidelines apply";

    // Act
    const auto event = collector_->capture_override(
        "ticket_001", "op_001", "approve", "reject",
        DisagreementCategory::PolicyChange, notes
    );

    // Assert
    EXPECT_EQ(event.disagreement_notes, notes);
}

// Test: Capture override with ticket context
TEST_F(FeedbackCollectorTest, CapturesOverrideWithContext) {
    // Arrange
    const Ticket ticket("ticket_001", 0.92, "safety_flag", "{}");

    // Act
    const auto event = collector_->capture_override_with_context(
        ticket, "op_001", "approve", "reject",
        DisagreementCategory::EdgeCase
    );

    // Assert
    EXPECT_EQ(event.ticket_id, ticket.id());
    EXPECT_DOUBLE_EQ(event.confidence_score, ticket.confidence_score());
}

// Test: Get feedback for specific ticket
TEST_F(FeedbackCollectorTest, GetsFeedbackForTicket) {
    // Arrange
    const std::string ticket_id = "ticket_001";
    collector_->capture_override(ticket_id, "op_001", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override(ticket_id, "op_002", "approve", "escalate",
                                 DisagreementCategory::EdgeCase);
    collector_->capture_override("ticket_002", "op_001", "approve", "reject",
                                 DisagreementCategory::ModelError);

    // Act
    const auto feedback = collector_->get_feedback_for_ticket(ticket_id);

    // Assert
    EXPECT_EQ(feedback.size(), 2);
    for (const auto& event : feedback) {
        EXPECT_EQ(event.ticket_id, ticket_id);
    }
}

// Test: Get feedback by category
TEST_F(FeedbackCollectorTest, GetsFeedbackByCategory) {
    // Arrange
    collector_->capture_override("t1", "op1", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t2", "op2", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t3", "op3", "approve", "reject",
                                 DisagreementCategory::EdgeCase);

    // Act
    const auto policy_changes = collector_->get_feedback_by_category(
        DisagreementCategory::PolicyChange
    );

    // Assert
    EXPECT_EQ(policy_changes.size(), 2);
    for (const auto& event : policy_changes) {
        EXPECT_EQ(event.category, DisagreementCategory::PolicyChange);
    }
}

// Test: Get feedback by operator
TEST_F(FeedbackCollectorTest, GetsFeedbackByOperator) {
    // Arrange
    const std::string operator_id = "operator_123";
    collector_->capture_override("t1", operator_id, "approve", "reject",
                                 DisagreementCategory::EdgeCase);
    collector_->capture_override("t2", operator_id, "approve", "escalate",
                                 DisagreementCategory::ModelError);
    collector_->capture_override("t3", "other_op", "approve", "reject",
                                 DisagreementCategory::PolicyChange);

    // Act
    const auto operator_feedback = collector_->get_feedback_by_operator(operator_id);

    // Assert
    EXPECT_EQ(operator_feedback.size(), 2);
    for (const auto& event : operator_feedback) {
        EXPECT_EQ(event.operator_id, operator_id);
    }
}

// Test: Count disagreements
TEST_F(FeedbackCollectorTest, CountsDisagreements) {
    // Arrange - create disagreements
    collector_->capture_override("t1", "op1", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t2", "op2", "approve", "escalate",
                                 DisagreementCategory::EdgeCase);

    // Act
    const auto count = collector_->count_disagreements();

    // Assert
    EXPECT_EQ(count, 2);
}

// Test: Count disagreements by category
TEST_F(FeedbackCollectorTest, CountsDisagreementsByCategory) {
    // Arrange
    collector_->capture_override("t1", "op1", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t2", "op2", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t3", "op3", "approve", "reject",
                                 DisagreementCategory::EdgeCase);

    // Act
    const auto policy_count = collector_->count_disagreements_by_category(
        DisagreementCategory::PolicyChange
    );
    const auto edge_count = collector_->count_disagreements_by_category(
        DisagreementCategory::EdgeCase
    );

    // Assert
    EXPECT_EQ(policy_count, 2);
    EXPECT_EQ(edge_count, 1);
}

// Test: Calculate disagreement rate
TEST_F(FeedbackCollectorTest, CalculatesDisagreementRate) {
    // Arrange - 3 disagreements out of 3 total
    collector_->capture_override("t1", "op1", "approve", "reject",
                                 DisagreementCategory::PolicyChange);
    collector_->capture_override("t2", "op2", "approve", "escalate",
                                 DisagreementCategory::EdgeCase);
    collector_->capture_override("t3", "op3", "approve", "reject",
                                 DisagreementCategory::ModelError);

    // Act
    const auto rate = collector_->calculate_disagreement_rate();

    // Assert
    EXPECT_DOUBLE_EQ(rate, 1.0);  // 100% disagreement rate
}

// Test: Validate valid feedback event
TEST_F(FeedbackCollectorTest, ValidatesValidFeedback) {
    // Arrange
    const auto event = collector_->capture_override(
        "ticket_001", "op_001", "approve", "reject",
        DisagreementCategory::PolicyChange
    );

    // Act
    const bool is_valid = collector_->validate_feedback(event);

    // Assert
    EXPECT_TRUE(is_valid);
}

// Test: Invalidate feedback with empty ticket ID
TEST_F(FeedbackCollectorTest, InvalidatesFeedbackWithEmptyTicketId) {
    // Arrange
    FeedbackEvent invalid_event("evt_001", "", "op_001", "approve", "reject",
                                DisagreementCategory::PolicyChange);

    // Act
    const bool is_valid = collector_->validate_feedback(invalid_event);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Invalidate feedback with empty operator ID
TEST_F(FeedbackCollectorTest, InvalidatesFeedbackWithEmptyOperatorId) {
    // Arrange
    FeedbackEvent invalid_event("evt_001", "ticket_001", "", "approve", "reject",
                                DisagreementCategory::PolicyChange);

    // Act
    const bool is_valid = collector_->validate_feedback(invalid_event);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Disagreement category conversion to string
TEST_F(FeedbackCollectorTest, ConvertsCategoryToString) {
    // Assert
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::PolicyChange), "policy_change");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::EdgeCase), "edge_case");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::ModelError), "model_error");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::AmbiguousCase), "ambiguous_case");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::ContextMissing), "context_missing");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::UserError), "user_error");
    EXPECT_EQ(disagreement_category_to_string(DisagreementCategory::Other), "other");
}

// Test: String to disagreement category conversion
TEST_F(FeedbackCollectorTest, ConvertsStringToCategory) {
    // Assert
    EXPECT_EQ(string_to_disagreement_category("policy_change"), DisagreementCategory::PolicyChange);
    EXPECT_EQ(string_to_disagreement_category("edge_case"), DisagreementCategory::EdgeCase);
    EXPECT_EQ(string_to_disagreement_category("model_error"), DisagreementCategory::ModelError);
    EXPECT_EQ(string_to_disagreement_category("ambiguous_case"), DisagreementCategory::AmbiguousCase);
    EXPECT_EQ(string_to_disagreement_category("context_missing"), DisagreementCategory::ContextMissing);
    EXPECT_EQ(string_to_disagreement_category("user_error"), DisagreementCategory::UserError);
    EXPECT_EQ(string_to_disagreement_category("unknown"), DisagreementCategory::Other);
}

// Test: FeedbackEvent is_disagreement method
TEST_F(FeedbackCollectorTest, DetectsDisagreement) {
    // Arrange
    FeedbackEvent disagreement("evt_001", "t1", "op1", "approve", "reject",
                              DisagreementCategory::PolicyChange);
    FeedbackEvent agreement("evt_002", "t2", "op2", "approve", "approve",
                           DisagreementCategory::Other);

    // Assert
    EXPECT_TRUE(disagreement.is_disagreement());
    EXPECT_FALSE(agreement.is_disagreement());
}

// Test: FeedbackEvent is_high_value_feedback method
TEST_F(FeedbackCollectorTest, DetectsHighValueFeedback) {
    // Arrange
    FeedbackEvent high_value("evt_001", "t1", "op1", "approve", "reject",
                            DisagreementCategory::EdgeCase);
    FeedbackEvent user_error("evt_002", "t2", "op2", "approve", "reject",
                            DisagreementCategory::UserError);

    // Assert
    EXPECT_TRUE(high_value.is_high_value_feedback());
    EXPECT_FALSE(user_error.is_high_value_feedback());  // User errors not high value
}

// Test: Handle multiple categories
TEST_F(FeedbackCollectorTest, HandlesMultipleCategories) {
    // Arrange - Create feedback in each category
    const std::vector<DisagreementCategory> categories = {
        DisagreementCategory::PolicyChange,
        DisagreementCategory::EdgeCase,
        DisagreementCategory::ModelError,
        DisagreementCategory::AmbiguousCase,
        DisagreementCategory::ContextMissing,
        DisagreementCategory::UserError,
        DisagreementCategory::Other
    };

    // Act - Capture one feedback for each category
    for (const auto& category : categories) {
        collector_->capture_override("t1", "op1", "approve", "reject", category);
    }

    // Assert - Should have feedback for each category
    for (const auto& category : categories) {
        const auto feedback = collector_->get_feedback_by_category(category);
        EXPECT_EQ(feedback.size(), 1);
    }
}

// Test: Zero disagreement rate when no events
TEST_F(FeedbackCollectorTest, ZeroDisagreementRateWhenEmpty) {
    // Act
    const auto rate = collector_->calculate_disagreement_rate();

    // Assert
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// Test: Const-correctness
TEST_F(FeedbackCollectorTest, IsConstCorrect) {
    // Arrange
    const auto& const_collector = *collector_;

    // Act - should compile without issues
    const auto count = const_collector.count_disagreements();
    const auto rate = const_collector.calculate_disagreement_rate();

    // Assert
    EXPECT_EQ(count, 0);
    EXPECT_DOUBLE_EQ(rate, 0.0);
}

// ============================================================================
// GoldenDatasetManager Test Suite
// ============================================================================

class GoldenDatasetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto mock_collector = std::make_unique<MockFeedbackCollector>();
        collector_ptr_ = mock_collector.get();  // Keep raw pointer for test access
        manager_ = std::make_unique<GoldenDatasetManager>(std::move(mock_collector));
    }

    void TearDown() override {
        manager_.reset();
    }

    // Helper to create test feedback event
    FeedbackEvent create_test_feedback(DisagreementCategory category) const {
        return FeedbackEvent(
            "feedback_001",
            "ticket_001",
            "operator_123",
            "approve",
            "reject",
            category
        );
    }

    std::unique_ptr<GoldenDatasetManager> manager_;
    MockFeedbackCollector* collector_ptr_;  // Non-owning pointer for tests
};

// Test: Create entry from feedback event
TEST_F(GoldenDatasetManagerTest, CreatesEntryFromFeedback) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
    const std::string ticket_json = R"({"id": "ticket_001", "type": "safety_flag"})";

    // Act
    const auto entry = manager_->create_entry_from_feedback(feedback, ticket_json);

    // Assert
    EXPECT_FALSE(entry.id.empty());
    EXPECT_EQ(entry.ticket_data, ticket_json);
    EXPECT_EQ(entry.operator_action, feedback.actual_action);
    EXPECT_TRUE(entry.has_valid_quality_score());
}

// Test: Create entry with manual quality score
TEST_F(GoldenDatasetManagerTest, CreatesEntryWithQualityScore) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::PolicyChange);
    const std::string ticket_json = R"({})";
    const int quality_score = 5;

    // Act
    const auto entry = manager_->create_entry_with_quality(feedback, ticket_json, quality_score);

    // Assert
    EXPECT_EQ(entry.quality_score, 5);
    EXPECT_TRUE(entry.is_high_quality());
}

// Test: Calculate quality score for edge case
TEST_F(GoldenDatasetManagerTest, CalculatesQualityScoreForEdgeCase) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);

    // Act
    const int quality = manager_->calculate_quality_score(feedback);

    // Assert
    EXPECT_GE(quality, 4);  // Edge cases are high quality
    EXPECT_LE(quality, 5);
}

// Test: Calculate quality score for policy change
TEST_F(GoldenDatasetManagerTest, CalculatesQualityScoreForPolicyChange) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::PolicyChange);

    // Act
    const int quality = manager_->calculate_quality_score(feedback);

    // Assert
    EXPECT_GE(quality, 4);  // Policy changes are high quality
    EXPECT_LE(quality, 5);
}

// Test: Calculate quality score for model error
TEST_F(GoldenDatasetManagerTest, CalculatesQualityScoreForModelError) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::ModelError);

    // Act
    const int quality = manager_->calculate_quality_score(feedback);

    // Assert
    EXPECT_GE(quality, 3);  // Model errors are good quality
    EXPECT_LE(quality, 5);
}

// Test: Calculate quality score for user error (low quality)
TEST_F(GoldenDatasetManagerTest, CalculatesLowQualityScoreForUserError) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::UserError);

    // Act
    const int quality = manager_->calculate_quality_score(feedback);

    // Assert
    EXPECT_LE(quality, 2);  // User errors are low quality
}

// Test: Add entry to dataset
TEST_F(GoldenDatasetManagerTest, AddsEntryToDataset) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
    const auto entry = manager_->create_entry_from_feedback(feedback, "{}");

    // Act
    manager_->add_to_dataset(entry);
    const auto stats = manager_->get_dataset_stats();

    // Assert
    EXPECT_EQ(stats.total_entries, 1);
}

// Test: Get entries ready for training
TEST_F(GoldenDatasetManagerTest, GetsEntriesReadyForTraining) {
    // Arrange - Add high quality entry (not exported)
    const auto feedback1 = create_test_feedback(DisagreementCategory::EdgeCase);
    const auto entry1 = manager_->create_entry_from_feedback(feedback1, "{}");
    manager_->add_to_dataset(entry1);

    // Add low quality entry (should be filtered)
    const auto feedback2 = create_test_feedback(DisagreementCategory::UserError);
    const auto entry2 = manager_->create_entry_from_feedback(feedback2, "{}");
    manager_->add_to_dataset(entry2);

    // Act
    const auto ready = manager_->get_ready_for_training();

    // Assert - Only high quality (>= 3) entries
    EXPECT_GT(ready.size(), 0);
    for (const auto& entry : ready) {
        EXPECT_GE(entry.quality_score, 3);
        EXPECT_FALSE(entry.exported);
    }
}

// Test: Get high quality entries (quality >= 4)
TEST_F(GoldenDatasetManagerTest, GetsHighQualityEntries) {
    // Arrange - Add mix of quality levels
    const auto feedback1 = create_test_feedback(DisagreementCategory::EdgeCase);  // High quality
    manager_->add_to_dataset(manager_->create_entry_from_feedback(feedback1, "{}"));

    const auto feedback2 = create_test_feedback(DisagreementCategory::AmbiguousCase);  // Medium quality
    manager_->add_to_dataset(manager_->create_entry_from_feedback(feedback2, "{}"));

    // Act
    const auto high_quality = manager_->get_high_quality_entries();

    // Assert
    for (const auto& entry : high_quality) {
        EXPECT_GE(entry.quality_score, 4);
        EXPECT_TRUE(entry.is_high_quality());
    }
}

// Test: Mark entries as exported
TEST_F(GoldenDatasetManagerTest, MarksEntriesAsExported) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
    const auto entry = manager_->create_entry_from_feedback(feedback, "{}");
    manager_->add_to_dataset(entry);

    // Act
    manager_->mark_as_exported({entry.id});
    const auto stats = manager_->get_dataset_stats();

    // Assert
    EXPECT_EQ(stats.exported_entries, 1);
}

// Test: Export dataset to JSON
TEST_F(GoldenDatasetManagerTest, ExportsDatasetToJson) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
    const auto entry = manager_->create_entry_from_feedback(feedback, "{}");
    std::vector<GoldenDatasetEntry> entries = {entry};

    // Act
    const auto json = manager_->export_to_json(entries);

    // Assert
    EXPECT_FALSE(json.empty());
    EXPECT_GT(json.length(), 10);  // Should contain actual JSON data
}

// Test: Get dataset statistics
TEST_F(GoldenDatasetManagerTest, GetsDatasetStatistics) {
    // Arrange - Add multiple entries
    for (int i = 0; i < 5; ++i) {
        const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
        manager_->add_to_dataset(manager_->create_entry_from_feedback(feedback, "{}"));
    }

    // Act
    const auto stats = manager_->get_dataset_stats();

    // Assert
    EXPECT_EQ(stats.total_entries, 5);
    EXPECT_GT(stats.avg_quality_score, 0.0);
}

// Test: Validate valid entry
TEST_F(GoldenDatasetManagerTest, ValidatesValidEntry) {
    // Arrange
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);
    const auto entry = manager_->create_entry_from_feedback(feedback, "{}");

    // Act
    const bool is_valid = manager_->validate_entry(entry);

    // Assert
    EXPECT_TRUE(is_valid);
}

// Test: Invalidate entry with invalid quality score
TEST_F(GoldenDatasetManagerTest, InvalidatesEntryWithInvalidQuality) {
    // Arrange
    GoldenDatasetEntry invalid_entry("entry_001", "{}", "approve", 0.9, 0);  // Quality = 0

    // Act
    const bool is_valid = manager_->validate_entry(invalid_entry);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Invalidate entry with quality score too high
TEST_F(GoldenDatasetManagerTest, InvalidatesEntryWithQualityTooHigh) {
    // Arrange
    GoldenDatasetEntry invalid_entry("entry_001", "{}", "approve", 0.9, 6);  // Quality = 6 (> 5)

    // Act
    const bool is_valid = manager_->validate_entry(invalid_entry);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Filter entries by minimum quality
TEST_F(GoldenDatasetManagerTest, FiltersEntriesByQuality) {
    // Arrange
    std::vector<GoldenDatasetEntry> entries;
    entries.emplace_back("e1", "{}", "approve", 0.9, 5);
    entries.emplace_back("e2", "{}", "reject", 0.8, 3);
    entries.emplace_back("e3", "{}", "escalate", 0.7, 2);

    // Act
    const auto high_quality = manager_->filter_by_quality(entries, 4);

    // Assert
    EXPECT_EQ(high_quality.size(), 1);
    EXPECT_EQ(high_quality[0].quality_score, 5);
}

// Test: Entry is_ready_for_training method
TEST_F(GoldenDatasetManagerTest, ChecksEntryReadyForTraining) {
    // Arrange
    GoldenDatasetEntry ready("e1", "{\"data\": true}", "approve", 0.9, 4);
    ready.exported = false;

    GoldenDatasetEntry already_exported("e2", "{}", "approve", 0.9, 4);
    already_exported.exported = true;

    GoldenDatasetEntry low_quality("e3", "{}", "approve", 0.9, 2);

    // Assert
    EXPECT_TRUE(ready.is_ready_for_training());
    EXPECT_FALSE(already_exported.is_ready_for_training());
    EXPECT_FALSE(low_quality.is_ready_for_training());
}

// Test: Entry has_valid_quality_score method
TEST_F(GoldenDatasetManagerTest, ValidatesQualityScoreRange) {
    // Arrange
    GoldenDatasetEntry valid("e1", "{}", "approve", 0.9, 3);
    GoldenDatasetEntry too_low("e2", "{}", "approve", 0.9, 0);
    GoldenDatasetEntry too_high("e3", "{}", "approve", 0.9, 6);

    // Assert
    EXPECT_TRUE(valid.has_valid_quality_score());
    EXPECT_FALSE(too_low.has_valid_quality_score());
    EXPECT_FALSE(too_high.has_valid_quality_score());
}

// Test: Dataset stats with mixed quality
TEST_F(GoldenDatasetManagerTest, CalculatesStatsWithMixedQuality) {
    // Arrange
    manager_->add_to_dataset(GoldenDatasetEntry("e1", "{}", "approve", 0.9, 5));
    manager_->add_to_dataset(GoldenDatasetEntry("e2", "{}", "reject", 0.8, 4));
    manager_->add_to_dataset(GoldenDatasetEntry("e3", "{}", "escalate", 0.7, 3));

    // Act
    const auto stats = manager_->get_dataset_stats();

    // Assert
    EXPECT_EQ(stats.total_entries, 3);
    EXPECT_EQ(stats.high_quality_entries, 2);  // Quality >= 4
    EXPECT_DOUBLE_EQ(stats.avg_quality_score, 4.0);  // (5+4+3)/3
}

// Test: RAII principles with unique_ptr
TEST_F(GoldenDatasetManagerTest, MaintainsRaiiPrinciples) {
    // Arrange
    auto collector = std::make_unique<MockFeedbackCollector>();

    // Act - ownership transferred
    auto manager = std::make_unique<GoldenDatasetManager>(std::move(collector));

    // Assert - collector is now null (ownership transferred)
    EXPECT_EQ(collector, nullptr);
    EXPECT_NE(manager, nullptr);
}

// Test: Handle empty dataset
TEST_F(GoldenDatasetManagerTest, HandlesEmptyDataset) {
    // Act
    const auto stats = manager_->get_dataset_stats();
    const auto ready = manager_->get_ready_for_training();
    const auto high_quality = manager_->get_high_quality_entries();

    // Assert
    EXPECT_EQ(stats.total_entries, 0);
    EXPECT_TRUE(ready.empty());
    EXPECT_TRUE(high_quality.empty());
}

// Test: Quality score calculation consistency
TEST_F(GoldenDatasetManagerTest, CalculatesConsistentQualityScores) {
    // Arrange - Same feedback should get same quality score
    const auto feedback = create_test_feedback(DisagreementCategory::EdgeCase);

    // Act
    const int quality1 = manager_->calculate_quality_score(feedback);
    const int quality2 = manager_->calculate_quality_score(feedback);

    // Assert
    EXPECT_EQ(quality1, quality2);
}

} // namespace wfm::rlhf

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
