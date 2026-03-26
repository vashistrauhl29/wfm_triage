#ifndef WFM_CORE_COPILOT_CONTEXT_GENERATOR_H
#define WFM_CORE_COPILOT_CONTEXT_GENERATOR_H

#include <memory>
#include "models/ticket.h"
#include "models/operator_context.h"
#include "core/copilot/rag_service.h"

namespace wfm::copilot {

// ContextGenerator: Generates enriched operator context with RAG-based summaries
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - R.21: Prefer unique_ptr over shared_ptr unless sharing ownership
// - Con.2: Make member functions const by default
class ContextGenerator {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit ContextGenerator(std::unique_ptr<RAGService> rag_service);

    // Generate enriched context for operator view
    // Con.2: Const member function
    OperatorContext generate_context(const Ticket& ticket) const;

    // Generate three-bullet summary from ticket data and SOPs
    // Returns exactly 3 bullet points
    std::vector<std::string> generate_three_bullet_summary(
        const Ticket& ticket,
        const std::vector<SOPSearchResult>& relevant_sops
    ) const;

    // Generate action recommendation based on ticket and SOPs
    std::string generate_action_recommendation(
        const Ticket& ticket,
        const std::vector<SOPSearchResult>& relevant_sops
    ) const;

    // Validate that context meets quality requirements
    bool validate_context(const OperatorContext& context) const noexcept;

private:
    std::unique_ptr<RAGService> rag_service_;  // R.20: unique_ptr for ownership

    // Helper: Extract key anomalies from ticket data
    std::vector<std::string> extract_key_anomalies(const Ticket& ticket) const;

    // Helper: Format bullet point with max length constraint
    std::string format_bullet_point(const std::string& text, size_t max_length = 100) const;

    // Helper: Validate ticket has required fields
    void validate_ticket(const Ticket& ticket) const;
};

} // namespace wfm::copilot

#endif // WFM_CORE_COPILOT_CONTEXT_GENERATOR_H
