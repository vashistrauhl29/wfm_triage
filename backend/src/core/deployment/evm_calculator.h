#ifndef WFM_CORE_DEPLOYMENT_EVM_CALCULATOR_H
#define WFM_CORE_DEPLOYMENT_EVM_CALCULATOR_H

#include <vector>
#include "models/evm_metrics.h"
#include "models/milestone.h"
#include "models/deployment_phase.h"

namespace wfm::deployment {

// EVMCalculator: Calculates Earned Value Management metrics
// Following PMI (Project Management Institute) EVM standards
// Following C++ Core Guidelines:
// - Con.2: Make member functions const by default
// - C.9: Minimize exposure of members
class EVMCalculator {
public:
    // Constructor
    EVMCalculator() = default;

    // Virtual destructor for potential inheritance
    virtual ~EVMCalculator() = default;

    // Calculate EVM metrics from milestones and budget
    virtual EVMMetrics calculate_evm(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::vector<DeploymentPhase>& phases
    ) const = 0;

    // Calculate Planned Value (PV): budgeted cost of work scheduled
    virtual double calculate_planned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::string& current_date
    ) const = 0;

    // Calculate Earned Value (EV): budgeted cost of work performed
    virtual double calculate_earned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones
    ) const = 0;

    // Calculate Actual Cost (AC): actual cost of work performed
    virtual double calculate_actual_cost(
        const std::vector<DeploymentPhase>& phases
    ) const = 0;

    // Calculate Schedule Performance Index (SPI = EV / PV)
    virtual double calculate_spi(
        double earned_value,
        double planned_value
    ) const noexcept = 0;

    // Calculate Cost Performance Index (CPI = EV / AC)
    virtual double calculate_cpi(
        double earned_value,
        double actual_cost
    ) const noexcept = 0;

    // Calculate Estimate at Completion (EAC = BAC / CPI)
    virtual double calculate_eac(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept = 0;

    // Calculate Estimate to Complete (ETC = EAC - AC)
    virtual double calculate_etc(
        double estimate_at_completion,
        double actual_cost
    ) const noexcept = 0;

    // Calculate Variance at Completion (VAC = BAC - EAC)
    virtual double calculate_vac(
        double budget_at_completion,
        double estimate_at_completion
    ) const noexcept = 0;

    // Predict final cost overrun/underrun
    virtual double predict_cost_variance(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept = 0;

    // Check if project is on track (SPI >= 0.9 and CPI >= 0.9)
    virtual bool is_on_track(
        double schedule_performance_index,
        double cost_performance_index
    ) const noexcept = 0;
};

} // namespace wfm::deployment

#endif // WFM_CORE_DEPLOYMENT_EVM_CALCULATOR_H
