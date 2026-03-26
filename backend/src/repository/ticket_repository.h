#ifndef WFM_REPOSITORY_TICKET_REPOSITORY_H
#define WFM_REPOSITORY_TICKET_REPOSITORY_H

#include <string>
#include <vector>
#include <optional>
#include "models/ticket.h"

namespace wfm::repository {

// TicketRepository: Abstract interface for ticket data access
// Following C++ Core Guidelines:
// - C.120: Use class hierarchy to represent concepts with inherent hierarchical structure
class TicketRepository {
public:
    // Default constructor
    TicketRepository() = default;

    // Virtual destructor for polymorphism
    virtual ~TicketRepository() = default;

    // Find ticket by ID
    virtual std::optional<Ticket> find_by_id(const std::string& ticket_id) const = 0;

    // Save ticket (insert or update)
    virtual void save(const Ticket& ticket) = 0;

    // Get all tickets
    virtual std::vector<Ticket> find_all() const = 0;

    // Delete ticket by ID
    virtual void remove(const std::string& ticket_id) = 0;

    // Count total tickets
    virtual size_t count() const noexcept = 0;
};

} // namespace wfm::repository

#endif // WFM_REPOSITORY_TICKET_REPOSITORY_H
