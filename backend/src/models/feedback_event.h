#ifndef WFM_MODELS_FEEDBACK_EVENT_H
#define WFM_MODELS_FEEDBACK_EVENT_H

#include <string>
#include <cstdint>

namespace wfm {

// Disagreement category enumeration
enum class DisagreementCategory {
    PolicyChange,      // Policy or guideline has changed
    EdgeCase,          // Unseen edge case not covered by training
    ModelError,        // Clear model prediction error
    AmbiguousCase,     // Case with unclear correct action
    ContextMissing,    // Insufficient context for decision
    UserError,         // Operator made an error (not model issue)
    Other              // Other category
};

// Convert DisagreementCategory to string
inline std::string disagreement_category_to_string(DisagreementCategory category) {
    switch (category) {
        case DisagreementCategory::PolicyChange: return "policy_change";
        case DisagreementCategory::EdgeCase: return "edge_case";
        case DisagreementCategory::ModelError: return "model_error";
        case DisagreementCategory::AmbiguousCase: return "ambiguous_case";
        case DisagreementCategory::ContextMissing: return "context_missing";
        case DisagreementCategory::UserError: return "user_error";
        case DisagreementCategory::Other: return "other";
        default: return "unknown";
    }
}

// Convert string to DisagreementCategory
inline DisagreementCategory string_to_disagreement_category(const std::string& str) {
    if (str == "policy_change") return DisagreementCategory::PolicyChange;
    if (str == "edge_case") return DisagreementCategory::EdgeCase;
    if (str == "model_error") return DisagreementCategory::ModelError;
    if (str == "ambiguous_case") return DisagreementCategory::AmbiguousCase;
    if (str == "context_missing") return DisagreementCategory::ContextMissing;
    if (str == "user_error") return DisagreementCategory::UserError;
    if (str == "other") return DisagreementCategory::Other;
    return DisagreementCategory::Other;
}

// Domain model: Feedback event from operator override
struct FeedbackEvent {
    std::string id;                          // Unique event identifier
    std::string ticket_id;                   // Associated ticket ID
    std::string operator_id;                 // Operator who provided feedback
    std::string recommended_action;          // AI-recommended action
    std::string actual_action;               // Actual action taken by operator
    DisagreementCategory category;           // Disagreement category
    std::string disagreement_notes;          // Additional notes from operator
    double confidence_score;                 // Original AI confidence score
    std::string created_at;                  // Timestamp

    // ES.20: Always initialize
    FeedbackEvent()
        : id(),
          ticket_id(),
          operator_id(),
          recommended_action(),
          actual_action(),
          category(DisagreementCategory::Other),
          disagreement_notes(),
          confidence_score(0.0),
          created_at() {}

    // Constructor with essential fields
    FeedbackEvent(
        std::string event_id,
        std::string tid,
        std::string oid,
        std::string recommended,
        std::string actual,
        DisagreementCategory cat
    ) : id(std::move(event_id)),
        ticket_id(std::move(tid)),
        operator_id(std::move(oid)),
        recommended_action(std::move(recommended)),
        actual_action(std::move(actual)),
        category(cat),
        disagreement_notes(),
        confidence_score(0.0),
        created_at() {}

    // Check if this represents an actual disagreement
    bool is_disagreement() const noexcept {
        return recommended_action != actual_action &&
               !recommended_action.empty() &&
               !actual_action.empty();
    }

    // Check if this is high-value feedback (not user error)
    bool is_high_value_feedback() const noexcept {
        return is_disagreement() && category != DisagreementCategory::UserError;
    }
};

} // namespace wfm

#endif // WFM_MODELS_FEEDBACK_EVENT_H
