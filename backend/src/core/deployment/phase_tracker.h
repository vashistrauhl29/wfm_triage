#ifndef WFM_CORE_DEPLOYMENT_PHASE_TRACKER_H
#define WFM_CORE_DEPLOYMENT_PHASE_TRACKER_H

#include <vector>
#include <string>
#include <memory>
#include "models/deployment_phase.h"
#include "core/deployment/evm_calculator.h"

namespace wfm::deployment {

// PhaseTracker: Tracks progress across 5 deployment phases
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - Con.2: Make member functions const by default
class PhaseTracker {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit PhaseTracker(std::unique_ptr<EVMCalculator> calculator);

    // Virtual destructor for potential inheritance
    virtual ~PhaseTracker() = default;

    // Add deployment phase
    virtual void add_phase(const DeploymentPhase& phase) const = 0;

    // Update phase status
    virtual void update_phase_status(
        int64_t phase_id,
        PhaseStatus new_status
    ) const = 0;

    // Update phase actual cost
    virtual void update_phase_cost(
        int64_t phase_id,
        double actual_cost
    ) const = 0;

    // Complete phase (transition to Completed status)
    virtual void complete_phase(int64_t phase_id) const = 0;

    // Block phase (transition to Blocked status)
    virtual void block_phase(
        int64_t phase_id,
        const std::string& reason
    ) const = 0;

    // Get phase by ID
    virtual DeploymentPhase get_phase(int64_t phase_id) const = 0;

    // Get phase by number (1-5)
    virtual DeploymentPhase get_phase_by_number(int phase_number) const = 0;

    // Get all phases
    virtual std::vector<DeploymentPhase> get_all_phases() const = 0;

    // Get phases by status
    virtual std::vector<DeploymentPhase> get_phases_by_status(
        PhaseStatus status
    ) const = 0;

    // Get current active phase
    virtual DeploymentPhase get_current_phase() const = 0;

    // Calculate overall progress percentage (0-100)
    virtual double calculate_progress() const noexcept = 0;

    // Check if phase can transition to new status
    virtual bool can_transition(
        int64_t phase_id,
        PhaseStatus new_status
    ) const noexcept = 0;

    // Validate phase transition (e.g., sequential phase completion)
    virtual bool validate_phase_transition(
        int phase_number,
        PhaseStatus current_status,
        PhaseStatus new_status
    ) const noexcept = 0;

    // Get count of phases by status
    virtual size_t count_phases_by_status(PhaseStatus status) const noexcept = 0;

    // Clear all phases (for testing)
    virtual void clear_phases() const = 0;

    // Get phase count
    virtual size_t get_phase_count() const noexcept = 0;
};

} // namespace wfm::deployment

#endif // WFM_CORE_DEPLOYMENT_PHASE_TRACKER_H
