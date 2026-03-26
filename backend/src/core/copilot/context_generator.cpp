// ContextGenerator implementation
// Following C++ Core Guidelines and TDD principles

#include "core/copilot/context_generator.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace wfm::copilot {

// I.11: Take ownership by unique_ptr (following RAII)
ContextGenerator::ContextGenerator(std::unique_ptr<RAGService> rag_service)
    : rag_service_(std::move(rag_service)) {
    if (!rag_service_) {
        throw std::invalid_argument("RAGService cannot be null");
    }
}

// Generate enriched context for operator view
OperatorContext ContextGenerator::generate_context(const Ticket& ticket) const {
    validate_ticket(ticket);

    // Create context with ticket
    OperatorContext context(ticket);

    // Retrieve relevant SOPs from RAG service
    context.relevant_sops = rag_service_->get_relevant_sops_for_ticket(
        ticket.ticket_type(),
        ticket.raw_data(),
        5  // Top-5 most relevant SOPs (increased to ensure multiple results)
    );

    // If we got fewer than 2 SOPs, try to get additional generic ones
    if (context.relevant_sops.size() < 2) {
        const auto generic_sops = rag_service_->search_similar_sops("generic", 3);
        for (const auto& sop : generic_sops) {
            // Add if not already present
            bool is_duplicate = false;
            for (const auto& existing : context.relevant_sops) {
                if (existing.document.id == sop.document.id) {
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate && context.relevant_sops.size() < 5) {
                context.relevant_sops.push_back(sop);
            }
        }
    }

    // Generate three-bullet summary
    context.three_bullet_summary = generate_three_bullet_summary(
        ticket,
        context.relevant_sops
    );

    // Generate action recommendation
    context.action_recommendation = generate_action_recommendation(
        ticket,
        context.relevant_sops
    );

    // Set confidence override threshold (could be adjusted based on ticket complexity)
    context.confidence_override_threshold = ticket.confidence_score();

    return context;
}

// Generate three-bullet summary from ticket data and SOPs
std::vector<std::string> ContextGenerator::generate_three_bullet_summary(
    const Ticket& ticket,
    const std::vector<SOPSearchResult>& relevant_sops
) const {
    std::vector<std::string> bullets;
    bullets.reserve(3);

    // Extract key anomalies from ticket
    const auto anomalies = extract_key_anomalies(ticket);

    // Bullet 1: Ticket overview with type and confidence
    std::ostringstream bullet1;
    bullet1 << "Ticket " << ticket.ticket_type()
            << " flagged with "
            << static_cast<int>(ticket.confidence_score() * 100)
            << "% confidence";
    bullets.push_back(format_bullet_point(bullet1.str()));

    // Bullet 2: Key anomalies or ticket details
    if (!anomalies.empty()) {
        std::ostringstream bullet2;
        bullet2 << "Key anomalies: ";
        for (size_t i = 0; i < std::min(size_t(2), anomalies.size()); ++i) {
            if (i > 0) bullet2 << ", ";
            bullet2 << anomalies[i];
        }
        bullets.push_back(format_bullet_point(bullet2.str()));
    } else {
        bullets.push_back(format_bullet_point("Review ticket data for policy compliance"));
    }

    // Bullet 3: SOP-based guidance or action
    if (!relevant_sops.empty() && !relevant_sops[0].document.summary.empty()) {
        std::ostringstream bullet3;
        bullet3 << "Recommended action: " << relevant_sops[0].document.summary;
        bullets.push_back(format_bullet_point(bullet3.str()));
    } else {
        bullets.push_back(format_bullet_point("Follow standard operating procedure for ticket type"));
    }

    // Ensure exactly 3 bullets (defensive programming)
    while (bullets.size() < 3) {
        bullets.push_back(format_bullet_point("Additional context pending"));
    }
    if (bullets.size() > 3) {
        bullets.resize(3);
    }

    return bullets;
}

// Generate action recommendation based on ticket and SOPs
std::string ContextGenerator::generate_action_recommendation(
    const Ticket& ticket,
    const std::vector<SOPSearchResult>& relevant_sops
) const {
    std::ostringstream recommendation;

    // Base recommendation on ticket type and confidence
    if (ticket.confidence_score() >= 0.95) {
        recommendation << "High confidence " << ticket.ticket_type()
                      << " - proceed with standard resolution. ";
    } else if (ticket.confidence_score() >= 0.80) {
        recommendation << "Moderate confidence " << ticket.ticket_type()
                      << " - review carefully before resolution. ";
    } else {
        recommendation << "Low confidence " << ticket.ticket_type()
                      << " - escalate for additional review. ";
    }

    // Add SOP-based guidance
    if (!relevant_sops.empty()) {
        const auto& top_sop = relevant_sops[0].document;
        if (!top_sop.content.empty()) {
            // Extract first sentence or key phrase from SOP content
            const auto first_period = top_sop.content.find('.');
            if (first_period != std::string::npos && first_period < 100) {
                recommendation << top_sop.content.substr(0, first_period + 1);
            } else {
                recommendation << "Refer to " << top_sop.title << " for guidance.";
            }
        }
    }

    return recommendation.str();
}

// Validate that context meets quality requirements
bool ContextGenerator::validate_context(const OperatorContext& context) const noexcept {
    // Check if summary has exactly 3 non-empty bullets
    if (!context.has_valid_summary()) {
        return false;
    }

    // Additional validation could be added here
    // For now, valid summary is the primary requirement
    return true;
}

// Helper: Extract key anomalies from ticket data
std::vector<std::string> ContextGenerator::extract_key_anomalies(const Ticket& ticket) const {
    std::vector<std::string> anomalies;

    const std::string& raw_data = ticket.raw_data();

    // Simple extraction based on common keywords in raw data
    // In production, this would use NLP/ML for proper extraction

    if (raw_data.find("safety") != std::string::npos) {
        anomalies.push_back("safety concern detected");
    }
    if (raw_data.find("document") != std::string::npos) {
        anomalies.push_back("document issue identified");
    }
    if (raw_data.find("issue") != std::string::npos) {
        anomalies.push_back("reported issue");
    }
    if (raw_data.find("concern") != std::string::npos) {
        anomalies.push_back("flagged concern");
    }
    if (raw_data.find("user") != std::string::npos || raw_data.find("user_id") != std::string::npos) {
        anomalies.push_back("user-reported");
    }

    // If no specific anomalies found, use ticket type as anomaly
    if (anomalies.empty()) {
        anomalies.push_back(ticket.ticket_type() + " detected");
    }

    return anomalies;
}

// Helper: Format bullet point with max length constraint
std::string ContextGenerator::format_bullet_point(
    const std::string& text,
    const size_t max_length
) const {
    if (text.empty()) {
        return "No information available";
    }

    // Trim whitespace
    std::string trimmed = text;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    // Truncate if too long
    if (trimmed.length() > max_length) {
        trimmed = trimmed.substr(0, max_length - 3) + "...";
    }

    return trimmed;
}

// Helper: Validate ticket has required fields
void ContextGenerator::validate_ticket(const Ticket& ticket) const {
    if (ticket.id().empty()) {
        throw std::invalid_argument("Ticket ID cannot be empty");
    }
    if (ticket.ticket_type().empty()) {
        throw std::invalid_argument("Ticket type cannot be empty");
    }
    if (ticket.confidence_score() < 0.0 || ticket.confidence_score() > 1.0) {
        throw std::invalid_argument("Ticket confidence score must be between 0.0 and 1.0");
    }
}

} // namespace wfm::copilot
