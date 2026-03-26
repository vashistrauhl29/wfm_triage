#ifndef WFM_CORE_SIMULATOR_THRESHOLD_OPTIMIZER_H
#define WFM_CORE_SIMULATOR_THRESHOLD_OPTIMIZER_H

#include <memory>
#include <vector>
#include "core/simulator/cost_calculator.h"
#include "models/cost_simulation.h"
#include "models/cost_parameters.h"

namespace wfm::simulator {

// ThresholdOptimizer: Simulates different threshold scenarios and finds optimal thresholds
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - R.21: Prefer unique_ptr over shared_ptr unless sharing ownership
class ThresholdOptimizer {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit ThresholdOptimizer(std::unique_ptr<CostCalculator> calculator);

    // Simulate a single threshold scenario
    // Con.2: Const member function
    CostSimulationResult simulate_threshold(
        double threshold,
        size_t total_tickets,
        double projected_stp_rate
    ) const;

    // Simulate multiple threshold scenarios
    // F.20: Return by value (RVO will optimize)
    std::vector<CostSimulationResult> simulate_scenarios(
        const std::vector<double>& thresholds,
        size_t total_tickets,
        const std::vector<double>& projected_stp_rates
    ) const;

    // Find optimal threshold within constraints
    CostSimulationResult find_optimal_threshold(
        double min_threshold,
        double max_threshold,
        double max_acceptable_cost,
        size_t total_tickets
    ) const;

    // Calculate break-even STP rate (where blended cost equals human-only cost)
    double calculate_break_even_stp_rate() const noexcept;

    // Find minimum cost scenario from simulation results
    CostSimulationResult find_minimum_cost_scenario(
        const std::vector<CostSimulationResult>& results
    ) const;

    // Simulate with capacity constraints
    CostSimulationResult simulate_with_capacity_constraint(
        double threshold,
        size_t total_tickets,
        double projected_stp_rate,
        size_t max_human_capacity
    ) const;

    // Project monthly costs
    MonthlyCostProjection project_monthly_cost(
        double threshold,
        size_t daily_tickets,
        double projected_stp_rate,
        size_t days_per_month
    ) const;

    // Compare two threshold strategies
    StrategyComparison compare_strategies(
        const CostSimulationResult& strategy_a,
        const CostSimulationResult& strategy_b
    ) const noexcept;

private:
    std::unique_ptr<CostCalculator> calculator_;  // R.20: unique_ptr for ownership

    // Helper: Validate threshold range
    void validate_threshold(double threshold) const;

    // Helper: Calculate projected STP rate based on threshold (simplified model)
    // In production, this would use historical data and ML models
    double estimate_stp_rate_for_threshold(double threshold) const noexcept;
};

} // namespace wfm::simulator

#endif // WFM_CORE_SIMULATOR_THRESHOLD_OPTIMIZER_H
