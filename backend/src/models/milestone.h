#ifndef WFM_MODELS_MILESTONE_H
#define WFM_MODELS_MILESTONE_H

#include <string>
#include <vector>
#include <cstdint>

namespace wfm {

// Milestone status enumeration
enum class MilestoneStatus {
    Pending,
    InProgress,
    Completed,
    AtRisk
};

// Convert MilestoneStatus to string
inline std::string milestone_status_to_string(MilestoneStatus status) {
    switch (status) {
        case MilestoneStatus::Pending: return "pending";
        case MilestoneStatus::InProgress: return "in_progress";
        case MilestoneStatus::Completed: return "completed";
        case MilestoneStatus::AtRisk: return "at_risk";
        default: return "unknown";
    }
}

// Convert string to MilestoneStatus
inline MilestoneStatus string_to_milestone_status(const std::string& str) {
    if (str == "pending") return MilestoneStatus::Pending;
    if (str == "in_progress") return MilestoneStatus::InProgress;
    if (str == "completed") return MilestoneStatus::Completed;
    if (str == "at_risk") return MilestoneStatus::AtRisk;
    return MilestoneStatus::Pending;  // Default
}

// Domain model: Milestone within a deployment phase
struct Milestone {
    int64_t id;
    int64_t phase_id;                   // Foreign key to DeploymentPhase
    std::string milestone_name;
    std::string description;
    std::string planned_completion_date; // ISO 8601 date
    std::string actual_completion_date;  // ISO 8601 date (empty if not completed)
    MilestoneStatus status;
    double weight;                      // Weight for EV calculation (0-100)
    std::string owner;                  // Team/person responsible
    std::vector<int64_t> dependencies;  // IDs of milestones that must complete first

    // ES.20: Always initialize
    Milestone()
        : id(0),
          phase_id(0),
          milestone_name(),
          description(),
          planned_completion_date(),
          actual_completion_date(),
          status(MilestoneStatus::Pending),
          weight(1.0),
          owner(),
          dependencies() {}

    // Constructor with essential fields
    Milestone(
        int64_t milestone_id,
        int64_t phase_identifier,
        std::string name,
        double milestone_weight
    ) : id(milestone_id),
        phase_id(phase_identifier),
        milestone_name(std::move(name)),
        description(),
        planned_completion_date(),
        actual_completion_date(),
        status(MilestoneStatus::Pending),
        weight(milestone_weight),
        owner(),
        dependencies() {}

    // Check if milestone is completed
    bool is_completed() const noexcept {
        return status == MilestoneStatus::Completed;
    }

    // Check if milestone is at risk
    bool is_at_risk() const noexcept {
        return status == MilestoneStatus::AtRisk;
    }

    // Check if milestone has dependencies
    bool has_dependencies() const noexcept {
        return !dependencies.empty();
    }

    // Validate weight is in valid range
    bool has_valid_weight() const noexcept {
        return weight >= 0.0 && weight <= 100.0;
    }
};

} // namespace wfm

#endif // WFM_MODELS_MILESTONE_H
