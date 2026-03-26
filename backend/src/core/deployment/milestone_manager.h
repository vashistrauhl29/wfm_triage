#ifndef WFM_CORE_DEPLOYMENT_MILESTONE_MANAGER_H
#define WFM_CORE_DEPLOYMENT_MILESTONE_MANAGER_H

#include <vector>
#include <string>
#include "models/milestone.h"

namespace wfm::deployment {

// MilestoneManager: Manages phase gates and milestone completion
// Following C++ Core Guidelines:
// - Con.2: Make member functions const by default
// - C.9: Minimize exposure of members
class MilestoneManager {
public:
    // Constructor
    MilestoneManager() = default;

    // Virtual destructor for potential inheritance
    virtual ~MilestoneManager() = default;

    // Add milestone to a phase
    virtual void add_milestone(const Milestone& milestone) const = 0;

    // Update milestone status
    virtual void update_milestone_status(
        int64_t milestone_id,
        MilestoneStatus new_status
    ) const = 0;

    // Complete milestone
    virtual void complete_milestone(
        int64_t milestone_id,
        const std::string& completion_date
    ) const = 0;

    // Mark milestone as at risk
    virtual void mark_at_risk(
        int64_t milestone_id,
        const std::string& risk_reason
    ) const = 0;

    // Get milestone by ID
    virtual Milestone get_milestone(int64_t milestone_id) const = 0;

    // Get all milestones for a phase
    virtual std::vector<Milestone> get_milestones_for_phase(
        int64_t phase_id
    ) const = 0;

    // Get milestones by status
    virtual std::vector<Milestone> get_milestones_by_status(
        MilestoneStatus status
    ) const = 0;

    // Check if milestone dependencies are satisfied
    virtual bool are_dependencies_satisfied(
        int64_t milestone_id
    ) const noexcept = 0;

    // Check if milestone can be completed
    virtual bool can_complete_milestone(
        int64_t milestone_id
    ) const noexcept = 0;

    // Add dependency to milestone
    virtual void add_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const = 0;

    // Remove dependency from milestone
    virtual void remove_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const = 0;

    // Get dependency chain for milestone
    virtual std::vector<int64_t> get_dependency_chain(
        int64_t milestone_id
    ) const = 0;

    // Calculate weighted completion for phase (sum of completed milestone weights)
    virtual double calculate_phase_completion(
        int64_t phase_id
    ) const noexcept = 0;

    // Get total milestone count
    virtual size_t get_milestone_count() const noexcept = 0;

    // Get completed milestone count
    virtual size_t get_completed_count() const noexcept = 0;

    // Get at-risk milestone count
    virtual size_t get_at_risk_count() const noexcept = 0;

    // Validate milestone weight (0-100)
    virtual bool validate_milestone_weight(double weight) const noexcept = 0;

    // Clear all milestones (for testing)
    virtual void clear_milestones() const = 0;
};

} // namespace wfm::deployment

#endif // WFM_CORE_DEPLOYMENT_MILESTONE_MANAGER_H
