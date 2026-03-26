#include "core/router/routing_engine.h"
#include <algorithm>

namespace wfm::router {

// Constructor
// I.11: Transfer ownership via unique_ptr
// C.41: Constructor creates fully initialized object
RoutingEngine::RoutingEngine(std::unique_ptr<ConfidenceEvaluator> evaluator)
    : evaluator_(std::move(evaluator)),  // R.20: Use unique_ptr for ownership
      stp_queue_(),                       // ES.20: Always initialize
      human_queue_(),
      total_processed_(0),
      stp_count_(0)
{
    // R.1: RAII - evaluator_ will be automatically destroyed
}

// Route a ticket based on confidence evaluation
// F.16: Pass Ticket by const& (expensive to copy)
void RoutingEngine::route(const Ticket& ticket) {
    // ES.25: Declare const unless modification is intended
    const Decision decision = evaluator_->evaluate(ticket);

    // Route based on decision
    if (decision.routing == RoutingDecision::STP) {
        stp_queue_.push_back(decision);
    } else {
        human_queue_.push_back(decision);
    }

    // Update statistics
    update_statistics(decision.routing);
}

// Get STP queue
// Con.2: Const member function
// F.20: Return by const reference (avoid copy)
const std::vector<Decision>& RoutingEngine::get_stp_queue() const noexcept {
    return stp_queue_;
}

// Get human queue
// Con.2: Const member function
const std::vector<Decision>& RoutingEngine::get_human_queue() const noexcept {
    return human_queue_;
}

// Calculate STP rate
// Con.2: Const member function
double RoutingEngine::get_stp_rate() const noexcept {
    // ES.46: Avoid narrowing conversions
    if (total_processed_ == 0) {
        return 0.0;
    }

    // Calculate rate as double to avoid integer division
    return static_cast<double>(stp_count_) / static_cast<double>(total_processed_);
}

// Get routing statistics
// Con.2: Const member function
RoutingStatistics RoutingEngine::get_statistics() const noexcept {
    RoutingStatistics stats;
    stats.total_processed = total_processed_;
    stats.stp_count = stp_count_;
    stats.human_queue_count = total_processed_ - stp_count_;
    stats.stp_rate = get_stp_rate();

    return stats;  // F.20: Return by value (RVO will optimize)
}

// Clear all queues
void RoutingEngine::clear_queues() noexcept {
    stp_queue_.clear();
    human_queue_.clear();
    total_processed_ = 0;
    stp_count_ = 0;
}

// Private helper: Update statistics after routing
void RoutingEngine::update_statistics(RoutingDecision decision) noexcept {
    ++total_processed_;

    if (decision == RoutingDecision::STP) {
        ++stp_count_;
    }
}

} // namespace wfm::router
