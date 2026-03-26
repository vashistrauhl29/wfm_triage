// EVMCalculator production implementation
// Following C++ Core Guidelines and PMI EVM standards

#include "core/deployment/evm_calculator.h"
#include <cmath>
#include <algorithm>
#include <memory>
#include <vector>

namespace wfm::deployment {

// ============================================================================
// DefaultEVMCalculator: Production implementation of EVMCalculator
// ============================================================================

class DefaultEVMCalculator : public EVMCalculator {
public:
    DefaultEVMCalculator() = default;

    // Calculate EVM metrics from milestones and budget
    EVMMetrics calculate_evm(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::vector<DeploymentPhase>& phases
    ) const override;

    // Calculate Planned Value (PV)
    double calculate_planned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::string& current_date
    ) const override;

    // Calculate Earned Value (EV)
    double calculate_earned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones
    ) const override;

    // Calculate Actual Cost (AC)
    double calculate_actual_cost(
        const std::vector<DeploymentPhase>& phases
    ) const override;

    // Calculate Schedule Performance Index (SPI = EV / PV)
    double calculate_spi(
        double earned_value,
        double planned_value
    ) const noexcept override;

    // Calculate Cost Performance Index (CPI = EV / AC)
    double calculate_cpi(
        double earned_value,
        double actual_cost
    ) const noexcept override;

    // Calculate Estimate at Completion (EAC = BAC / CPI)
    double calculate_eac(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept override;

    // Calculate Estimate to Complete (ETC = EAC - AC)
    double calculate_etc(
        double estimate_at_completion,
        double actual_cost
    ) const noexcept override;

    // Calculate Variance at Completion (VAC = BAC - EAC)
    double calculate_vac(
        double budget_at_completion,
        double estimate_at_completion
    ) const noexcept override;

    // Predict final cost variance
    double predict_cost_variance(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept override;

    // Check if project is on track
    bool is_on_track(
        double schedule_performance_index,
        double cost_performance_index
    ) const noexcept override;
};

// Calculate EVM metrics from milestones and budget
EVMMetrics DefaultEVMCalculator::calculate_evm(
    double budget_at_completion,
    const std::vector<Milestone>& milestones,
    const std::vector<DeploymentPhase>& phases
) const {
    EVMMetrics metrics;

    // Calculate base metrics
    metrics.planned_value = calculate_planned_value(
        budget_at_completion, milestones, "2024-06-01");
    metrics.earned_value = calculate_earned_value(
        budget_at_completion, milestones);
    metrics.actual_cost = calculate_actual_cost(phases);

    // Calculate variance metrics
    metrics.schedule_variance = metrics.earned_value - metrics.planned_value;
    metrics.cost_variance = metrics.earned_value - metrics.actual_cost;

    // Calculate performance indices
    metrics.schedule_performance_index = calculate_spi(
        metrics.earned_value, metrics.planned_value);
    metrics.cost_performance_index = calculate_cpi(
        metrics.earned_value, metrics.actual_cost);

    // Calculate forecasts
    metrics.estimate_at_completion = calculate_eac(
        budget_at_completion, metrics.cost_performance_index);
    metrics.estimate_to_complete = calculate_etc(
        metrics.estimate_at_completion, metrics.actual_cost);
    metrics.variance_at_completion = calculate_vac(
        budget_at_completion, metrics.estimate_at_completion);

    metrics.snapshot_date = "2024-06-01";

    return metrics;
}

// Calculate Planned Value (PV): budgeted cost of work scheduled
double DefaultEVMCalculator::calculate_planned_value(
    double budget_at_completion,
    const std::vector<Milestone>& milestones,
    const std::string& current_date
) const {
    double pv = 0.0;
    for (const auto& m : milestones) {
        // Include milestones that should be completed by current date
        if (m.planned_completion_date <= current_date) {
            pv += (m.weight / 100.0) * budget_at_completion;
        }
    }
    return pv;
}

// Calculate Earned Value (EV): budgeted cost of work performed
double DefaultEVMCalculator::calculate_earned_value(
    double budget_at_completion,
    const std::vector<Milestone>& milestones
) const {
    double ev = 0.0;
    for (const auto& m : milestones) {
        if (m.is_completed()) {
            ev += (m.weight / 100.0) * budget_at_completion;
        }
    }
    return ev;
}

// Calculate Actual Cost (AC): actual cost of work performed
double DefaultEVMCalculator::calculate_actual_cost(
    const std::vector<DeploymentPhase>& phases
) const {
    double ac = 0.0;
    for (const auto& p : phases) {
        ac += p.actual_cost;
    }
    return ac;
}

// Calculate Schedule Performance Index (SPI = EV / PV)
double DefaultEVMCalculator::calculate_spi(
    double earned_value,
    double planned_value
) const noexcept {
    if (planned_value == 0.0) {
        return 0.0;  // Avoid division by zero
    }
    return earned_value / planned_value;
}

// Calculate Cost Performance Index (CPI = EV / AC)
double DefaultEVMCalculator::calculate_cpi(
    double earned_value,
    double actual_cost
) const noexcept {
    if (actual_cost == 0.0) {
        return 0.0;  // Avoid division by zero
    }
    return earned_value / actual_cost;
}

// Calculate Estimate at Completion (EAC = BAC / CPI)
double DefaultEVMCalculator::calculate_eac(
    double budget_at_completion,
    double cost_performance_index
) const noexcept {
    if (cost_performance_index == 0.0) {
        return budget_at_completion;  // Default to BAC
    }
    return budget_at_completion / cost_performance_index;
}

// Calculate Estimate to Complete (ETC = EAC - AC)
double DefaultEVMCalculator::calculate_etc(
    double estimate_at_completion,
    double actual_cost
) const noexcept {
    return estimate_at_completion - actual_cost;
}

// Calculate Variance at Completion (VAC = BAC - EAC)
double DefaultEVMCalculator::calculate_vac(
    double budget_at_completion,
    double estimate_at_completion
) const noexcept {
    return budget_at_completion - estimate_at_completion;
}

// Predict final cost variance
double DefaultEVMCalculator::predict_cost_variance(
    double budget_at_completion,
    double cost_performance_index
) const noexcept {
    double eac = calculate_eac(budget_at_completion, cost_performance_index);
    return calculate_vac(budget_at_completion, eac);
}

// Check if project is on track (SPI >= 0.9 and CPI >= 0.9)
bool DefaultEVMCalculator::is_on_track(
    double schedule_performance_index,
    double cost_performance_index
) const noexcept {
    const double THRESHOLD = 0.9;
    return schedule_performance_index >= THRESHOLD &&
           cost_performance_index >= THRESHOLD;
}

// ============================================================================
// Factory function for creating EVMCalculator instances
// ============================================================================

std::unique_ptr<EVMCalculator> create_evm_calculator() {
    return std::make_unique<DefaultEVMCalculator>();
}

} // namespace wfm::deployment
