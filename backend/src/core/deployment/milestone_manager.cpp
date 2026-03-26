// MilestoneManager production implementation
// Following C++ Core Guidelines and TDD principles

#include "core/deployment/milestone_manager.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace wfm::deployment {

// ============================================================================
// DefaultMilestoneManager: Production implementation of MilestoneManager
// ============================================================================

class DefaultMilestoneManager : public MilestoneManager {
public:
    DefaultMilestoneManager() = default;

    // Add milestone to a phase
    void add_milestone(const Milestone& milestone) const override;

    // Update milestone status
    void update_milestone_status(
        int64_t milestone_id,
        MilestoneStatus new_status
    ) const override;

    // Complete milestone
    void complete_milestone(
        int64_t milestone_id,
        const std::string& completion_date
    ) const override;

    // Mark milestone as at risk
    void mark_at_risk(
        int64_t milestone_id,
        const std::string& risk_reason
    ) const override;

    // Get milestone by ID
    Milestone get_milestone(int64_t milestone_id) const override;

    // Get all milestones for a phase
    std::vector<Milestone> get_milestones_for_phase(
        int64_t phase_id
    ) const override;

    // Get milestones by status
    std::vector<Milestone> get_milestones_by_status(
        MilestoneStatus status
    ) const override;

    // Check if milestone dependencies are satisfied
    bool are_dependencies_satisfied(
        int64_t milestone_id
    ) const noexcept override;

    // Check if milestone can be completed
    bool can_complete_milestone(
        int64_t milestone_id
    ) const noexcept override;

    // Add dependency to milestone
    void add_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const override;

    // Remove dependency from milestone
    void remove_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const override;

    // Get dependency chain for milestone
    std::vector<int64_t> get_dependency_chain(
        int64_t milestone_id
    ) const override;

    // Calculate weighted completion for phase
    double calculate_phase_completion(
        int64_t phase_id
    ) const noexcept override;

    // Get total milestone count
    size_t get_milestone_count() const noexcept override;

    // Get completed milestone count
    size_t get_completed_count() const noexcept override;

    // Get at-risk milestone count
    size_t get_at_risk_count() const noexcept override;

    // Validate milestone weight
    bool validate_milestone_weight(double weight) const noexcept override;

    // Clear all milestones
    void clear_milestones() const override;

private:
    mutable std::vector<Milestone> milestones_;
};

// Add milestone to a phase
void DefaultMilestoneManager::add_milestone(const Milestone& milestone) const {
    milestones_.push_back(milestone);
}

// Update milestone status
void DefaultMilestoneManager::update_milestone_status(
    int64_t milestone_id,
    MilestoneStatus new_status
) const {
    for (auto& m : milestones_) {
        if (m.id == milestone_id) {
            m.status = new_status;
            break;
        }
    }
}

// Complete milestone
void DefaultMilestoneManager::complete_milestone(
    int64_t milestone_id,
    const std::string& completion_date
) const {
    for (auto& m : milestones_) {
        if (m.id == milestone_id) {
            m.status = MilestoneStatus::Completed;
            m.actual_completion_date = completion_date;
            break;
        }
    }
}

// Mark milestone as at risk
void DefaultMilestoneManager::mark_at_risk(
    int64_t milestone_id,
    const std::string& risk_reason
) const {
    update_milestone_status(milestone_id, MilestoneStatus::AtRisk);
    // In production, would store the risk_reason in a separate field or log
    (void)risk_reason;
}

// Get milestone by ID
Milestone DefaultMilestoneManager::get_milestone(int64_t milestone_id) const {
    for (const auto& m : milestones_) {
        if (m.id == milestone_id) {
            return m;
        }
    }
    // Return default-constructed milestone if not found
    return Milestone();
}

// Get all milestones for a phase
std::vector<Milestone> DefaultMilestoneManager::get_milestones_for_phase(
    int64_t phase_id
) const {
    std::vector<Milestone> result;
    for (const auto& m : milestones_) {
        if (m.phase_id == phase_id) {
            result.push_back(m);
        }
    }
    return result;
}

// Get milestones by status
std::vector<Milestone> DefaultMilestoneManager::get_milestones_by_status(
    MilestoneStatus status
) const {
    std::vector<Milestone> result;
    for (const auto& m : milestones_) {
        if (m.status == status) {
            result.push_back(m);
        }
    }
    return result;
}

// Check if milestone dependencies are satisfied
bool DefaultMilestoneManager::are_dependencies_satisfied(
    int64_t milestone_id
) const noexcept {
    auto milestone = get_milestone(milestone_id);

    // No dependencies means satisfied
    if (milestone.dependencies.empty()) {
        return true;
    }

    // Check if all dependencies are completed
    for (auto dep_id : milestone.dependencies) {
        auto dep = get_milestone(dep_id);
        if (!dep.is_completed()) {
            return false;
        }
    }

    return true;
}

// Check if milestone can be completed
bool DefaultMilestoneManager::can_complete_milestone(
    int64_t milestone_id
) const noexcept {
    auto milestone = get_milestone(milestone_id);

    // Cannot complete if already completed
    if (milestone.is_completed()) {
        return false;
    }

    // Can only complete if dependencies are satisfied
    return are_dependencies_satisfied(milestone_id);
}

// Add dependency to milestone
void DefaultMilestoneManager::add_dependency(
    int64_t milestone_id,
    int64_t dependency_id
) const {
    for (auto& m : milestones_) {
        if (m.id == milestone_id) {
            // Check if dependency already exists
            auto it = std::find(m.dependencies.begin(),
                              m.dependencies.end(),
                              dependency_id);
            if (it == m.dependencies.end()) {
                m.dependencies.push_back(dependency_id);
            }
            break;
        }
    }
}

// Remove dependency from milestone
void DefaultMilestoneManager::remove_dependency(
    int64_t milestone_id,
    int64_t dependency_id
) const {
    for (auto& m : milestones_) {
        if (m.id == milestone_id) {
            auto it = std::find(m.dependencies.begin(),
                              m.dependencies.end(),
                              dependency_id);
            if (it != m.dependencies.end()) {
                m.dependencies.erase(it);
            }
            break;
        }
    }
}

// Get dependency chain for milestone
std::vector<int64_t> DefaultMilestoneManager::get_dependency_chain(
    int64_t milestone_id
) const {
    auto milestone = get_milestone(milestone_id);
    return milestone.dependencies;
}

// Calculate weighted completion for phase
double DefaultMilestoneManager::calculate_phase_completion(
    int64_t phase_id
) const noexcept {
    auto phase_milestones = get_milestones_for_phase(phase_id);

    if (phase_milestones.empty()) {
        return 0.0;
    }

    double total_weight = 0.0;
    double completed_weight = 0.0;

    for (const auto& m : phase_milestones) {
        total_weight += m.weight;
        if (m.is_completed()) {
            completed_weight += m.weight;
        }
    }

    if (total_weight == 0.0) {
        return 0.0;
    }

    return (completed_weight / total_weight) * 100.0;
}

// Get total milestone count
size_t DefaultMilestoneManager::get_milestone_count() const noexcept {
    return milestones_.size();
}

// Get completed milestone count
size_t DefaultMilestoneManager::get_completed_count() const noexcept {
    size_t count = 0;
    for (const auto& m : milestones_) {
        if (m.is_completed()) {
            ++count;
        }
    }
    return count;
}

// Get at-risk milestone count
size_t DefaultMilestoneManager::get_at_risk_count() const noexcept {
    size_t count = 0;
    for (const auto& m : milestones_) {
        if (m.is_at_risk()) {
            ++count;
        }
    }
    return count;
}

// Validate milestone weight (0-100)
bool DefaultMilestoneManager::validate_milestone_weight(double weight) const noexcept {
    return weight >= 0.0 && weight <= 100.0;
}

// Clear all milestones (for testing)
void DefaultMilestoneManager::clear_milestones() const {
    milestones_.clear();
}

// ============================================================================
// Factory function for creating MilestoneManager instances
// ============================================================================

std::unique_ptr<MilestoneManager> create_milestone_manager() {
    return std::make_unique<DefaultMilestoneManager>();
}

} // namespace wfm::deployment
