#ifndef WFM_CORE_SIMULATOR_COST_CALCULATOR_H
#define WFM_CORE_SIMULATOR_COST_CALCULATOR_H

#include <cstddef>
#include "models/cost_parameters.h"

namespace wfm::simulator {

// CostCalculator: Calculates blended cost (API + human labor)
// Following C++ Core Guidelines:
// - C.2: Use class if invariant exists (cost parameters invariant)
// - C.9: Minimize exposure of members
// - Con.2: Make member functions const by default
class CostCalculator {
public:
    // C.46: Declare single-argument constructors explicit
    explicit CostCalculator(const CostParameters& params);

    // Calculate cost per resolution given STP rate
    // Con.2: Const member function (does not modify state)
    // F.16: Pass by value for small types (double, size_t)
    double calculate_cost_per_resolution(
        double stp_rate,
        size_t total_tickets
    ) const;

    // Calculate total operational cost
    double calculate_total_cost(
        double stp_rate,
        size_t total_tickets
    ) const;

    // Get cost parameters (const accessor)
    const CostParameters& get_parameters() const noexcept;

private:
    const CostParameters params_;  // Con.4: const for values that don't change

    // Helper: Validate STP rate
    void validate_stp_rate(double stp_rate) const;

    // Helper: Calculate human cost per ticket
    double calculate_human_cost_per_ticket() const noexcept;
};

} // namespace wfm::simulator

#endif // WFM_CORE_SIMULATOR_COST_CALCULATOR_H
