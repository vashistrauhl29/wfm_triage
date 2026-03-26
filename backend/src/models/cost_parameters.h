#ifndef WFM_MODELS_COST_PARAMETERS_H
#define WFM_MODELS_COST_PARAMETERS_H

namespace wfm {

// Domain model: Cost parameters for economic calculations
// Following C++ Core Guidelines (Con.4: const for values that don't change)
struct CostParameters {
    double api_cost_per_call;           // Cost per API inference call (dollars)
    double avg_human_rate;              // Average hourly rate for human operators (dollars/hour)
    int avg_handling_time_seconds;      // Average time to handle one ticket (seconds)

    // ES.20: Always initialize
    CostParameters()
        : api_cost_per_call(0.0),
          avg_human_rate(0.0),
          avg_handling_time_seconds(0) {}

    // Designated initialization constructor (C++20)
    CostParameters(double api_cost, double human_rate, int handling_time)
        : api_cost_per_call(api_cost),
          avg_human_rate(human_rate),
          avg_handling_time_seconds(handling_time) {}
};

} // namespace wfm

#endif // WFM_MODELS_COST_PARAMETERS_H
