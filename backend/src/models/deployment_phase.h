#ifndef WFM_MODELS_DEPLOYMENT_PHASE_H
#define WFM_MODELS_DEPLOYMENT_PHASE_H

#include <string>
#include <cstdint>

namespace wfm {

// Phase status enumeration
enum class PhaseStatus {
    NotStarted,
    InProgress,
    Completed,
    Blocked
};

// Convert PhaseStatus to string
inline std::string phase_status_to_string(PhaseStatus status) {
    switch (status) {
        case PhaseStatus::NotStarted: return "not_started";
        case PhaseStatus::InProgress: return "in_progress";
        case PhaseStatus::Completed: return "completed";
        case PhaseStatus::Blocked: return "blocked";
        default: return "unknown";
    }
}

// Convert string to PhaseStatus
inline PhaseStatus string_to_phase_status(const std::string& str) {
    if (str == "not_started") return PhaseStatus::NotStarted;
    if (str == "in_progress") return PhaseStatus::InProgress;
    if (str == "completed") return PhaseStatus::Completed;
    if (str == "blocked") return PhaseStatus::Blocked;
    return PhaseStatus::NotStarted;  // Default
}

// Domain model: Deployment phase (5-phase rollout)
struct DeploymentPhase {
    int64_t id;
    int phase_number;                   // 1-5
    std::string phase_name;             // e.g., "Manual Baseline & Context Capture"
    std::string description;
    std::string start_date;             // ISO 8601 date
    std::string planned_end_date;       // ISO 8601 date
    std::string actual_end_date;        // ISO 8601 date (empty if not completed)
    PhaseStatus status;
    double budget_allocated;            // Planned budget for phase
    double actual_cost;                 // Actual spend

    // ES.20: Always initialize
    DeploymentPhase()
        : id(0),
          phase_number(0),
          phase_name(),
          description(),
          start_date(),
          planned_end_date(),
          actual_end_date(),
          status(PhaseStatus::NotStarted),
          budget_allocated(0.0),
          actual_cost(0.0) {}

    // Constructor with essential fields
    DeploymentPhase(
        int64_t phase_id,
        int number,
        std::string name,
        double budget
    ) : id(phase_id),
        phase_number(number),
        phase_name(std::move(name)),
        description(),
        start_date(),
        planned_end_date(),
        actual_end_date(),
        status(PhaseStatus::NotStarted),
        budget_allocated(budget),
        actual_cost(0.0) {}

    // Check if phase is completed
    bool is_completed() const noexcept {
        return status == PhaseStatus::Completed;
    }

    // Check if phase is in progress
    bool is_in_progress() const noexcept {
        return status == PhaseStatus::InProgress;
    }

    // Check if phase is blocked
    bool is_blocked() const noexcept {
        return status == PhaseStatus::Blocked;
    }
};

} // namespace wfm

#endif // WFM_MODELS_DEPLOYMENT_PHASE_H
