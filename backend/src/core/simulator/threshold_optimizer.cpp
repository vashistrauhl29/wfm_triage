// ThresholdOptimizer implementation
// Following C++ Core Guidelines and TDD principles

#include "core/simulator/threshold_optimizer.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <limits>

namespace wfm::simulator {

// I.11: Take ownership by unique_ptr (following RAII)
ThresholdOptimizer::ThresholdOptimizer(std::unique_ptr<CostCalculator> calculator)
    : calculator_(std::move(calculator)) {
    if (!calculator_) {
        throw std::invalid_argument("CostCalculator cannot be null");
    }
}

// Simulate a single threshold scenario
CostSimulationResult ThresholdOptimizer::simulate_threshold(
    const double threshold,
    const size_t total_tickets,
    const double projected_stp_rate
) const {
    validate_threshold(threshold);

    // Calculate cost using the cost calculator
    const double cost_per_resolution = calculator_->calculate_cost_per_resolution(
        projected_stp_rate,
        total_tickets
    );

    // Build result
    CostSimulationResult result;
    result.threshold = threshold;
    result.projected_stp_rate = projected_stp_rate;
    result.projected_cost_per_resolution = cost_per_resolution;
    result.total_tickets = total_tickets;

    return result;
}

// Simulate multiple threshold scenarios
std::vector<CostSimulationResult> ThresholdOptimizer::simulate_scenarios(
    const std::vector<double>& thresholds,
    const size_t total_tickets,
    const std::vector<double>& projected_stp_rates
) const {
    if (thresholds.size() != projected_stp_rates.size()) {
        throw std::invalid_argument(
            "Thresholds and STP rates must have the same size"
        );
    }

    std::vector<CostSimulationResult> results;
    results.reserve(thresholds.size());

    for (size_t i = 0; i < thresholds.size(); ++i) {
        results.push_back(simulate_threshold(
            thresholds[i],
            total_tickets,
            projected_stp_rates[i]
        ));
    }

    return results;
}

// Find optimal threshold within constraints
CostSimulationResult ThresholdOptimizer::find_optimal_threshold(
    const double min_threshold,
    const double max_threshold,
    const double max_acceptable_cost,
    const size_t total_tickets
) const {
    validate_threshold(min_threshold);
    validate_threshold(max_threshold);

    if (min_threshold > max_threshold) {
        throw std::invalid_argument("min_threshold cannot be greater than max_threshold");
    }

    // Simple optimization: try different thresholds and find one within cost constraint
    // In production, this would use more sophisticated optimization algorithms
    const double step = 0.01;
    double best_threshold = min_threshold;
    double best_stp_rate = estimate_stp_rate_for_threshold(min_threshold);

    for (double threshold = min_threshold; threshold <= max_threshold; threshold += step) {
        const double stp_rate = estimate_stp_rate_for_threshold(threshold);
        const double cost = calculator_->calculate_cost_per_resolution(stp_rate, total_tickets);

        // Find threshold with highest STP rate that meets cost constraint
        if (cost <= max_acceptable_cost && stp_rate > best_stp_rate) {
            best_threshold = threshold;
            best_stp_rate = stp_rate;
        }
    }

    return simulate_threshold(best_threshold, total_tickets, best_stp_rate);
}

// Calculate break-even STP rate (where blended cost equals human-only cost)
double ThresholdOptimizer::calculate_break_even_stp_rate() const noexcept {
    const auto& params = calculator_->get_parameters();

    // Human-only cost
    const double human_cost = (static_cast<double>(params.avg_handling_time_seconds) / 3600.0)
                             * params.avg_human_rate;

    // Break-even: (stp_rate × api_cost) + ((1 - stp_rate) × human_cost) = human_cost
    // Solving for stp_rate: stp_rate × api_cost + human_cost - stp_rate × human_cost = human_cost
    // stp_rate × (api_cost - human_cost) = 0
    // This means break-even is when using API doesn't save money

    // More practically, break-even is when cost reduction starts
    // Since API is almost always cheaper, break-even is very high
    // Return a conservative estimate
    if (params.api_cost_per_call >= human_cost) {
        return 0.0;  // No benefit from API if it costs more than human
    }

    // Break-even when savings start to matter (arbitrary threshold)
    return 0.1;  // 10% STP rate as break-even point
}

// Find minimum cost scenario from simulation results
CostSimulationResult ThresholdOptimizer::find_minimum_cost_scenario(
    const std::vector<CostSimulationResult>& results
) const {
    if (results.empty()) {
        throw std::invalid_argument("Cannot find minimum cost from empty results");
    }

    // Find result with minimum cost per resolution
    const auto min_it = std::min_element(
        results.begin(),
        results.end(),
        [](const CostSimulationResult& a, const CostSimulationResult& b) {
            return a.projected_cost_per_resolution < b.projected_cost_per_resolution;
        }
    );

    return *min_it;
}

// Simulate with capacity constraints
CostSimulationResult ThresholdOptimizer::simulate_with_capacity_constraint(
    const double threshold,
    const size_t total_tickets,
    const double projected_stp_rate,
    const size_t max_human_capacity
) const {
    validate_threshold(threshold);

    // Calculate human tickets required
    const size_t human_tickets = static_cast<size_t>(
        static_cast<double>(total_tickets) * (1.0 - projected_stp_rate)
    );

    // Check if within capacity
    if (human_tickets > max_human_capacity) {
        // Would exceed capacity - adjust STP rate to fit capacity
        const double adjusted_stp_rate = 1.0 - (static_cast<double>(max_human_capacity) /
                                                 static_cast<double>(total_tickets));
        return simulate_threshold(threshold, total_tickets, adjusted_stp_rate);
    }

    return simulate_threshold(threshold, total_tickets, projected_stp_rate);
}

// Project monthly costs
MonthlyCostProjection ThresholdOptimizer::project_monthly_cost(
    const double threshold,
    const size_t daily_tickets,
    const double projected_stp_rate,
    const size_t days_per_month
) const {
    validate_threshold(threshold);

    const size_t total_monthly_tickets = daily_tickets * days_per_month;
    const size_t stp_tickets = static_cast<size_t>(
        static_cast<double>(total_monthly_tickets) * projected_stp_rate
    );
    const size_t human_tickets = total_monthly_tickets - stp_tickets;

    const double total_cost = calculator_->calculate_total_cost(
        projected_stp_rate,
        total_monthly_tickets
    );
    const double avg_cost = total_cost / static_cast<double>(total_monthly_tickets);

    MonthlyCostProjection projection;
    projection.total_cost = total_cost;
    projection.total_tickets = total_monthly_tickets;
    projection.avg_cost_per_ticket = avg_cost;
    projection.stp_tickets = stp_tickets;
    projection.human_tickets = human_tickets;

    return projection;
}

// Compare two threshold strategies
StrategyComparison ThresholdOptimizer::compare_strategies(
    const CostSimulationResult& strategy_a,
    const CostSimulationResult& strategy_b
) const noexcept {
    StrategyComparison comparison;
    comparison.strategy_a = strategy_a;
    comparison.strategy_b = strategy_b;

    // Calculate differences (A - B)
    comparison.cost_difference = strategy_a.projected_cost_per_resolution -
                                 strategy_b.projected_cost_per_resolution;
    comparison.stp_rate_difference = strategy_a.projected_stp_rate -
                                     strategy_b.projected_stp_rate;

    return comparison;
}

// Helper: Validate threshold range
void ThresholdOptimizer::validate_threshold(const double threshold) const {
    if (threshold < 0.0 || threshold > 1.0) {
        throw std::invalid_argument(
            "Threshold must be between 0.0 and 1.0, got: " + std::to_string(threshold)
        );
    }
}

// Helper: Estimate STP rate based on threshold (simplified model)
// In production, this would use historical data and ML models
double ThresholdOptimizer::estimate_stp_rate_for_threshold(const double threshold) const noexcept {
    // Simplified inverse relationship: higher threshold = lower STP rate
    // Linear approximation for demonstration
    // threshold 0.85 -> ~90% STP
    // threshold 0.95 -> ~75% STP
    // threshold 0.99 -> ~45% STP

    // Linear interpolation
    if (threshold <= 0.85) {
        return 0.90;
    } else if (threshold >= 0.99) {
        return 0.45;
    } else {
        // Linear interpolation between 0.85 and 0.99
        const double range = 0.99 - 0.85;
        const double position = (threshold - 0.85) / range;
        return 0.90 - (position * (0.90 - 0.45));
    }
}

} // namespace wfm::simulator
