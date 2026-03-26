#ifndef WFM_MODELS_TICKET_H
#define WFM_MODELS_TICKET_H

#include <string>

namespace wfm {

// Domain model: Ticket
// Represents an incoming ticket for triage
class Ticket {
public:
    // Constructor following C++ Core Guidelines (C.46: explicit single-argument)
    explicit Ticket(
        std::string id,
        double confidence,
        std::string type,
        std::string raw_data
    );

    // Const accessors (Con.2: make member functions const by default)
    const std::string& id() const noexcept;
    double confidence_score() const noexcept;
    const std::string& ticket_type() const noexcept;
    const std::string& raw_data() const noexcept;

private:
    std::string id_;
    double confidence_score_;
    std::string ticket_type_;
    std::string raw_data_;
};

} // namespace wfm

#endif // WFM_MODELS_TICKET_H
