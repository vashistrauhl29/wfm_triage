#ifndef WFM_CORE_COPILOT_RAG_SERVICE_H
#define WFM_CORE_COPILOT_RAG_SERVICE_H

#include <string>
#include <vector>
#include <optional>
#include "models/sop_document.h"

namespace wfm::copilot {

// RAGService: Retrieval-Augmented Generation service for SOP knowledge base
// Following C++ Core Guidelines:
// - C.2: Use class if invariant exists
// - Con.2: Make member functions const by default
class RAGService {
public:
    // Default constructor
    RAGService() = default;

    // Virtual destructor for potential inheritance
    virtual ~RAGService() = default;

    // Retrieve SOP document by exact ticket type match
    // Con.2: Const member function
    virtual std::optional<SOPDocument> retrieve_sop_by_type(
        const std::string& ticket_type
    ) const = 0;

    // Search for similar SOPs using vector similarity
    // Returns top-K results ordered by relevance score (highest first)
    virtual std::vector<SOPSearchResult> search_similar_sops(
        const std::string& query,
        size_t top_k = 5
    ) const = 0;

    // Get relevant SOPs for a specific ticket
    // Combines type-based and content-based retrieval
    virtual std::vector<SOPSearchResult> get_relevant_sops_for_ticket(
        const std::string& ticket_type,
        const std::string& ticket_content,
        size_t top_k = 3
    ) const = 0;

    // Check if SOP exists for given ticket type
    virtual bool has_sop_for_type(const std::string& ticket_type) const noexcept = 0;

    // Get all available ticket types with SOPs
    virtual std::vector<std::string> get_available_ticket_types() const = 0;
};

} // namespace wfm::copilot

#endif // WFM_CORE_COPILOT_RAG_SERVICE_H
