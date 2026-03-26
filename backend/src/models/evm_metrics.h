#ifndef WFM_MODELS_EVM_METRICS_H
#define WFM_MODELS_EVM_METRICS_H

#include <string>

namespace wfm {

// Earned Value Management (EVM) metrics for deployment tracking
// Following PMI standard formulas
struct EVMMetrics {
    double planned_value;               // PV: Budgeted cost of work scheduled
    double earned_value;                // EV: Budgeted cost of work performed
    double actual_cost;                 // AC: Actual cost of work performed
    double schedule_variance;           // SV = EV - PV (>0 is ahead of schedule)
    double cost_variance;               // CV = EV - AC (>0 is under budget)
    double schedule_performance_index;  // SPI = EV / PV (>1 is ahead)
    double cost_performance_index;      // CPI = EV / AC (>1 is under budget)
    double estimate_at_completion;      // EAC = BAC / CPI
    double estimate_to_complete;        // ETC = EAC - AC
    double variance_at_completion;      // VAC = BAC - EAC
    std::string snapshot_date;          // Date of calculation

    // ES.20: Always initialize
    EVMMetrics()
        : planned_value(0.0),
          earned_value(0.0),
          actual_cost(0.0),
          schedule_variance(0.0),
          cost_variance(0.0),
          schedule_performance_index(0.0),
          cost_performance_index(0.0),
          estimate_at_completion(0.0),
          estimate_to_complete(0.0),
          variance_at_completion(0.0),
          snapshot_date() {}

    // Check if project is ahead of schedule
    bool is_ahead_of_schedule() const noexcept {
        return schedule_performance_index > 1.0;
    }

    // Check if project is under budget
    bool is_under_budget() const noexcept {
        return cost_performance_index > 1.0;
    }

    // Check if project is on track (within 10% tolerance)
    bool is_on_track() const noexcept {
        return schedule_performance_index >= 0.9 &&
               cost_performance_index >= 0.9;
    }

    // Get overall health score (0-100)
    double get_health_score() const noexcept {
        // Weighted score: 40% schedule + 40% cost + 20% variance
        double spi_score = schedule_performance_index * 40.0;
        double cpi_score = cost_performance_index * 40.0;
        double variance_score = (variance_at_completion >= 0) ? 20.0 : 0.0;

        double total = spi_score + cpi_score + variance_score;
        return (total > 100.0) ? 100.0 : total;
    }
};

// Deployment health snapshot
struct DeploymentHealth {
    double overall_health_score;        // 0-100 score
    std::string health_status;          // "excellent", "good", "at_risk", "critical"
    int completed_milestones;
    int total_milestones;
    int blocked_phases;
    int critical_risks;
    std::string snapshot_date;

    DeploymentHealth()
        : overall_health_score(0.0),
          health_status("unknown"),
          completed_milestones(0),
          total_milestones(0),
          blocked_phases(0),
          critical_risks(0),
          snapshot_date() {}

    // Determine health status from score
    std::string determine_health_status() const noexcept {
        if (overall_health_score >= 80.0) {
            return "excellent";
        } else if (overall_health_score >= 60.0) {
            return "good";
        } else if (overall_health_score >= 40.0) {
            return "at_risk";
        } else {
            return "critical";
        }
    }

    // Calculate completion percentage
    double get_completion_percentage() const noexcept {
        if (total_milestones == 0) {
            return 0.0;
        }
        return (static_cast<double>(completed_milestones) /
                static_cast<double>(total_milestones)) * 100.0;
    }
};

} // namespace wfm

#endif // WFM_MODELS_EVM_METRICS_H
