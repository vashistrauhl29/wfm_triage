#ifndef WFM_MODELS_COST_SIMULATION_H
#define WFM_MODELS_COST_SIMULATION_H

#include <cstddef>

namespace wfm {

// Domain model: Result of cost simulation for a given threshold
struct CostSimulationResult {
    double threshold;                      // AI confidence threshold (0.0-1.0)
    double projected_stp_rate;             // Projected straight-through processing rate
    double projected_cost_per_resolution;  // Cost per ticket resolution (dollars)
    size_t total_tickets;                  // Total number of tickets in simulation

    // ES.20: Always initialize
    CostSimulationResult()
        : threshold(0.0),
          projected_stp_rate(0.0),
          projected_cost_per_resolution(0.0),
          total_tickets(0) {}
};

// Domain model: Monthly cost projection
struct MonthlyCostProjection {
    double total_cost;           // Total monthly cost (dollars)
    size_t total_tickets;        // Total tickets processed in month
    double avg_cost_per_ticket;  // Average cost per ticket
    size_t stp_tickets;          // Tickets processed via STP
    size_t human_tickets;        // Tickets requiring human review

    MonthlyCostProjection()
        : total_cost(0.0),
          total_tickets(0),
          avg_cost_per_ticket(0.0),
          stp_tickets(0),
          human_tickets(0) {}
};

// Domain model: Comparison between two threshold strategies
struct StrategyComparison {
    double cost_difference;      // Cost difference (dollars)
    double stp_rate_difference;  // STP rate difference (percentage points)
    CostSimulationResult strategy_a;
    CostSimulationResult strategy_b;

    StrategyComparison()
        : cost_difference(0.0),
          stp_rate_difference(0.0) {}
};

} // namespace wfm

#endif // WFM_MODELS_COST_SIMULATION_H
