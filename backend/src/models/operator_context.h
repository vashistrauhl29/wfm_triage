#ifndef WFM_MODELS_OPERATOR_CONTEXT_H
#define WFM_MODELS_OPERATOR_CONTEXT_H

#include <string>
#include <vector>
#include "ticket.h"
#include "sop_document.h"

namespace wfm {

// Domain model: Operator context for copilot interface
// Provides enriched ticket information with RAG-enhanced context
struct OperatorContext {
    Ticket ticket;                                // Original ticket data
    std::vector<std::string> three_bullet_summary;  // Exactly 3 bullet points
    std::vector<SOPSearchResult> relevant_sops;   // Top-K relevant SOP documents
    std::string action_recommendation;            // AI-generated action recommendation
    double confidence_override_threshold;         // Suggested threshold for this ticket

    // ES.20: Always initialize - Constructor with ticket (required)
    explicit OperatorContext(Ticket t)
        : ticket(std::move(t)),
          three_bullet_summary(),
          relevant_sops(),
          action_recommendation(),
          confidence_override_threshold(0.0) {}

    // Validate that summary has exactly 3 bullets
    bool has_valid_summary() const noexcept {
        return three_bullet_summary.size() == 3 &&
               !three_bullet_summary[0].empty() &&
               !three_bullet_summary[1].empty() &&
               !three_bullet_summary[2].empty();
    }
};

} // namespace wfm

#endif // WFM_MODELS_OPERATOR_CONTEXT_H
