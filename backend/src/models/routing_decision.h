#ifndef WFM_MODELS_ROUTING_DECISION_H
#define WFM_MODELS_ROUTING_DECISION_H

#include <string>

namespace wfm {

// Enum class for routing decisions (Enum.3: prefer enum class)
enum class RoutingDecision {
    STP,           // Straight-Through Processing (auto-resolve)
    HumanQueue     // Route to human operator queue
};

// Domain model: Decision result from confidence evaluation
struct Decision {
    std::string ticket_id;
    double confidence_score;
    RoutingDecision routing;

    // Constructor with default values
    explicit Decision(
        std::string id = "",
        double score = 0.0,
        RoutingDecision route = RoutingDecision::HumanQueue
    ) : ticket_id(std::move(id)),
        confidence_score(score),
        routing(route) {}
};

} // namespace wfm

#endif // WFM_MODELS_ROUTING_DECISION_H
