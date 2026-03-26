#include "models/ticket.h"

namespace wfm {

// Constructor implementation
// C.41: Constructor creates fully initialized object
Ticket::Ticket(
    std::string id,
    double confidence,
    std::string type,
    std::string raw_data
) : id_(std::move(id)),           // ES.56: Use std::move for efficiency
    confidence_score_(confidence),
    ticket_type_(std::move(type)),
    raw_data_(std::move(raw_data))
{
    // Constructor body intentionally empty - initialization in member initializer list
}

// Const accessors following Con.2
const std::string& Ticket::id() const noexcept {
    return id_;
}

double Ticket::confidence_score() const noexcept {
    return confidence_score_;
}

const std::string& Ticket::ticket_type() const noexcept {
    return ticket_type_;
}

const std::string& Ticket::raw_data() const noexcept {
    return raw_data_;
}

} // namespace wfm
