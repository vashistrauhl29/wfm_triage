#ifndef WFM_CORE_ROUTER_CONFIDENCE_EVALUATOR_H
#define WFM_CORE_ROUTER_CONFIDENCE_EVALUATOR_H

#include "models/ticket.h"
#include "models/routing_decision.h"

namespace wfm::router {

// ConfidenceEvaluator: Evaluates ML confidence scores against threshold
// Following C++ Core Guidelines:
// - C.2: Use class if invariant exists (threshold invariant)
// - C.9: Minimize exposure of members
// - F.16: Pass cheaply-copied types by value, others by const&
class ConfidenceEvaluator {
public:
    // C.46: Declare single-argument constructors explicit
    explicit ConfidenceEvaluator(double threshold);

    // Con.2: Make member functions const by default
    // F.16: Pass Ticket by const& (expensive to copy)
    Decision evaluate(const Ticket& ticket) const;

    // Accessor for threshold (const-correct)
    double threshold() const noexcept;

private:
    const double threshold_;  // Con.4: const for values that don't change

    // Helper: Validate confidence score
    void validate_confidence(double confidence) const;
};

} // namespace wfm::router

#endif // WFM_CORE_ROUTER_CONFIDENCE_EVALUATOR_H
