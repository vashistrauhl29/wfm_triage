#ifndef WFM_CORE_ROUTER_ROUTING_ENGINE_H
#define WFM_CORE_ROUTER_ROUTING_ENGINE_H

#include <memory>
#include <vector>
#include "core/router/confidence_evaluator.h"
#include "models/ticket.h"
#include "models/routing_decision.h"

namespace wfm::router {

// Statistics for routing operations
struct RoutingStatistics {
    size_t total_processed;
    size_t stp_count;
    size_t human_queue_count;
    double stp_rate;

    RoutingStatistics()
        : total_processed(0),
          stp_count(0),
          human_queue_count(0),
          stp_rate(0.0) {}
};

// RoutingEngine: Routes tickets to STP or human queue based on evaluation
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - R.21: Prefer unique_ptr over shared_ptr unless sharing ownership
// - C.41: Constructor should create fully initialized object
class RoutingEngine {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit RoutingEngine(std::unique_ptr<ConfidenceEvaluator> evaluator);

    // F.16: Pass Ticket by const& (expensive to copy)
    void route(const Ticket& ticket);

    // Const accessors (Con.2)
    const std::vector<Decision>& get_stp_queue() const noexcept;
    const std::vector<Decision>& get_human_queue() const noexcept;
    double get_stp_rate() const noexcept;
    RoutingStatistics get_statistics() const noexcept;

    // Mutators
    void clear_queues() noexcept;

private:
    std::unique_ptr<ConfidenceEvaluator> evaluator_;  // R.20: unique_ptr for ownership
    std::vector<Decision> stp_queue_;
    std::vector<Decision> human_queue_;

    // Cached statistics
    size_t total_processed_{0};
    size_t stp_count_{0};

    // Helper: Update statistics after routing
    void update_statistics(RoutingDecision decision) noexcept;
};

} // namespace wfm::router

#endif // WFM_CORE_ROUTER_ROUTING_ENGINE_H
