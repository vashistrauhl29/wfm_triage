// CostCalculator implementation
// Following C++ Core Guidelines and TDD principles

#include "core/simulator/cost_calculator.h"
#include <stdexcept>
#include <string>

namespace wfm::simulator {

// C.46: Explicit constructor to prevent implicit conversion
CostCalculator::CostCalculator(const CostParameters& params)
    : params_(params) {
    // Validate cost parameters
    if (params_.api_cost_per_call < 0.0) {
        throw std::invalid_argument("API cost per call cannot be negative");
    }
    if (params_.avg_human_rate < 0.0) {
        throw std::invalid_argument("Average human rate cannot be negative");
    }
    if (params_.avg_handling_time_seconds < 0) {
        throw std::invalid_argument("Average handling time cannot be negative");
    }
}

// Calculate blended cost per resolution
// Formula: (STP_rate × API_cost) + ((1 - STP_rate) × Human_cost)
double CostCalculator::calculate_cost_per_resolution(
    const double stp_rate,
    [[maybe_unused]] const size_t total_tickets
) const {
    validate_stp_rate(stp_rate);

    // Calculate human cost per ticket
    const double human_cost = calculate_human_cost_per_ticket();

    // Blended cost: weighted average of API and human costs
    const double blended_cost = (stp_rate * params_.api_cost_per_call) +
                                ((1.0 - stp_rate) * human_cost);

    return blended_cost;
}

// Calculate total operational cost
double CostCalculator::calculate_total_cost(
    const double stp_rate,
    const size_t total_tickets
) const {
    validate_stp_rate(stp_rate);

    // Handle zero tickets edge case
    if (total_tickets == 0) {
        return 0.0;
    }

    const double cost_per_resolution = calculate_cost_per_resolution(stp_rate, total_tickets);
    return cost_per_resolution * static_cast<double>(total_tickets);
}

// Const accessor for cost parameters
const CostParameters& CostCalculator::get_parameters() const noexcept {
    return params_;
}

// Helper: Validate STP rate is in valid range [0.0, 1.0]
void CostCalculator::validate_stp_rate(const double stp_rate) const {
    if (stp_rate < 0.0 || stp_rate > 1.0) {
        throw std::invalid_argument(
            "STP rate must be between 0.0 and 1.0, got: " + std::to_string(stp_rate)
        );
    }
}

// Helper: Calculate human cost per ticket
// Formula: (handling_time_seconds / 3600) × hourly_rate
double CostCalculator::calculate_human_cost_per_ticket() const noexcept {
    const double hours = static_cast<double>(params_.avg_handling_time_seconds) / 3600.0;
    return hours * params_.avg_human_rate;
}

} // namespace wfm::simulator
