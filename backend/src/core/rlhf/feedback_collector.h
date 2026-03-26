#ifndef WFM_CORE_RLHF_FEEDBACK_COLLECTOR_H
#define WFM_CORE_RLHF_FEEDBACK_COLLECTOR_H

#include <string>
#include <vector>
#include "models/feedback_event.h"
#include "models/ticket.h"

namespace wfm::rlhf {

// FeedbackCollector: Captures operator override events for RLHF
// Following C++ Core Guidelines:
// - Con.2: Make member functions const by default
// - C.9: Minimize exposure of members
class FeedbackCollector {
public:
    // Default constructor
    FeedbackCollector() = default;

    // Virtual destructor for potential inheritance
    virtual ~FeedbackCollector() = default;

    // Capture feedback event when operator overrides AI recommendation
    // Returns the created feedback event with assigned ID
    virtual FeedbackEvent capture_override(
        const std::string& ticket_id,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes = ""
    ) const = 0;

    // Capture feedback with full ticket context
    virtual FeedbackEvent capture_override_with_context(
        const Ticket& ticket,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes = ""
    ) const = 0;

    // Get all feedback events for a specific ticket
    virtual std::vector<FeedbackEvent> get_feedback_for_ticket(
        const std::string& ticket_id
    ) const = 0;

    // Get feedback events by category
    virtual std::vector<FeedbackEvent> get_feedback_by_category(
        DisagreementCategory category
    ) const = 0;

    // Get feedback events by operator
    virtual std::vector<FeedbackEvent> get_feedback_by_operator(
        const std::string& operator_id
    ) const = 0;

    // Get disagreement statistics
    virtual size_t count_disagreements() const noexcept = 0;

    // Get disagreements by category
    virtual size_t count_disagreements_by_category(
        DisagreementCategory category
    ) const noexcept = 0;

    // Calculate disagreement rate (disagreements / total feedback events)
    virtual double calculate_disagreement_rate() const noexcept = 0;

    // Validate feedback event
    virtual bool validate_feedback(const FeedbackEvent& event) const noexcept = 0;
};

} // namespace wfm::rlhf

#endif // WFM_CORE_RLHF_FEEDBACK_COLLECTOR_H
