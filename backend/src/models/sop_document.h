#ifndef WFM_MODELS_SOP_DOCUMENT_H
#define WFM_MODELS_SOP_DOCUMENT_H

#include <string>
#include <vector>
#include <cstdint>

namespace wfm {

// Domain model: Standard Operating Procedure document
// Used for RAG-based knowledge retrieval in operator copilot
struct SOPDocument {
    std::string id;                          // Unique SOP identifier
    std::string ticket_type;                 // Associated ticket type (e.g., "safety_flag")
    std::string title;                       // SOP title/heading
    std::string content;                     // Full SOP text content
    std::string summary;                     // Brief summary (for quick reference)
    std::vector<double> embedding;           // Vector embedding for similarity search (1536 dims)
    int64_t version;                         // SOP version number
    std::string updated_at;                  // Last update timestamp

    // ES.20: Always initialize
    SOPDocument()
        : id(),
          ticket_type(),
          title(),
          content(),
          summary(),
          embedding(),
          version(1),
          updated_at() {}

    // Constructor with essential fields
    SOPDocument(
        std::string sop_id,
        std::string type,
        std::string sop_title,
        std::string sop_content
    ) : id(std::move(sop_id)),
        ticket_type(std::move(type)),
        title(std::move(sop_title)),
        content(std::move(sop_content)),
        summary(),
        embedding(),
        version(1),
        updated_at() {}
};

// Search result with relevance score
struct SOPSearchResult {
    SOPDocument document;
    double relevance_score;  // Cosine similarity score [0.0, 1.0]

    SOPSearchResult()
        : document(), relevance_score(0.0) {}

    SOPSearchResult(SOPDocument doc, double score)
        : document(std::move(doc)), relevance_score(score) {}
};

} // namespace wfm

#endif // WFM_MODELS_SOP_DOCUMENT_H
