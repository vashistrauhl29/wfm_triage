#include "core/router/confidence_evaluator.h"
#include <stdexcept>
#include <sstream>

namespace wfm::router {

// Constructor with validation
// C.46: Single-argument constructor is explicit
// E.2: Throw exception to signal failure
ConfidenceEvaluator::ConfidenceEvaluator(double threshold)
    : threshold_(threshold)
{
    // Validate threshold is in valid range [0.0, 1.0]
    if (threshold < 0.0 || threshold > 1.0) {
        std::ostringstream msg;
        msg << "Invalid threshold: " << threshold
            << ". Threshold must be in range [0.0, 1.0]";
        throw std::invalid_argument(msg.str());
    }
}

// Main evaluation method
// Con.2: Member function is const
// F.16: Pass Ticket by const& (expensive to copy)
Decision ConfidenceEvaluator::evaluate(const Ticket& ticket) const {
    // E.6: Use RAII - no manual resource management needed
    const double confidence = ticket.confidence_score();

    // Validate confidence score
    validate_confidence(confidence);

    // ES.25: Declare object const unless modification is intended
    const RoutingDecision routing =
        (confidence >= threshold_) ? RoutingDecision::STP
                                    : RoutingDecision::HumanQueue;

    // F.20: Prefer return values to output parameters
    return Decision(ticket.id(), confidence, routing);
}

// Accessor for threshold
// Con.2: Const member function
double ConfidenceEvaluator::threshold() const noexcept {
    return threshold_;
}

// Private helper: Validate confidence score
// E.2: Throw exception for invalid input
void ConfidenceEvaluator::validate_confidence(double confidence) const {
    if (confidence < 0.0 || confidence > 1.0) {
        std::ostringstream msg;
        msg << "Invalid confidence score: " << confidence
            << ". Confidence must be in range [0.0, 1.0]";
        throw std::invalid_argument(msg.str());
    }
}

} // namespace wfm::router
