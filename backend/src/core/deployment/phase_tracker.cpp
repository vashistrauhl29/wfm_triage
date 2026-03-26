// PhaseTracker production implementation
// Following C++ Core Guidelines and TDD principles

#include "core/deployment/phase_tracker.h"
#include <algorithm>
#include <stdexcept>

namespace wfm::deployment {

// I.11: Take ownership by unique_ptr (following RAII)
PhaseTracker::PhaseTracker(std::unique_ptr<EVMCalculator> calculator) {
    // Constructor body intentionally empty - abstract base class
    // Subclasses will handle actual initialization
    (void)calculator;
}

// ============================================================================
// DefaultPhaseTracker: Production implementation of PhaseTracker
// ============================================================================

class DefaultPhaseTracker : public PhaseTracker {
public:
    explicit DefaultPhaseTracker(std::unique_ptr<EVMCalculator> calculator);

    // Add deployment phase
    void add_phase(const DeploymentPhase& phase) const override;

    // Update phase status
    void update_phase_status(
        int64_t phase_id,
        PhaseStatus new_status
    ) const override;

    // Update phase actual cost
    void update_phase_cost(
        int64_t phase_id,
        double actual_cost
    ) const override;

    // Complete phase
    void complete_phase(int64_t phase_id) const override;

    // Block phase
    void block_phase(
        int64_t phase_id,
        const std::string& reason
    ) const override;

    // Get phase by ID
    DeploymentPhase get_phase(int64_t phase_id) const override;

    // Get phase by number (1-5)
    DeploymentPhase get_phase_by_number(int phase_number) const override;

    // Get all phases
    std::vector<DeploymentPhase> get_all_phases() const override;

    // Get phases by status
    std::vector<DeploymentPhase> get_phases_by_status(
        PhaseStatus status
    ) const override;

    // Get current active phase
    DeploymentPhase get_current_phase() const override;

    // Calculate overall progress percentage (0-100)
    double calculate_progress() const noexcept override;

    // Check if phase can transition to new status
    bool can_transition(
        int64_t phase_id,
        PhaseStatus new_status
    ) const noexcept override;

    // Validate phase transition
    bool validate_phase_transition(
        int phase_number,
        PhaseStatus current_status,
        PhaseStatus new_status
    ) const noexcept override;

    // Get count of phases by status
    size_t count_phases_by_status(PhaseStatus status) const noexcept override;

    // Clear all phases
    void clear_phases() const override;

    // Get phase count
    size_t get_phase_count() const noexcept override;

private:
    std::unique_ptr<EVMCalculator> evm_calculator_;
    mutable std::vector<DeploymentPhase> phases_;
};

// Constructor
DefaultPhaseTracker::DefaultPhaseTracker(std::unique_ptr<EVMCalculator> calculator)
    : PhaseTracker(std::move(calculator)),
      evm_calculator_(std::move(calculator)),
      phases_() {
}

// Add deployment phase
void DefaultPhaseTracker::add_phase(const DeploymentPhase& phase) const {
    phases_.push_back(phase);
}

// Update phase status
void DefaultPhaseTracker::update_phase_status(
    int64_t phase_id,
    PhaseStatus new_status
) const {
    for (auto& p : phases_) {
        if (p.id == phase_id) {
            p.status = new_status;
            break;
        }
    }
}

// Update phase actual cost
void DefaultPhaseTracker::update_phase_cost(
    int64_t phase_id,
    double actual_cost
) const {
    for (auto& p : phases_) {
        if (p.id == phase_id) {
            p.actual_cost = actual_cost;
            break;
        }
    }
}

// Complete phase (transition to Completed status)
void DefaultPhaseTracker::complete_phase(int64_t phase_id) const {
    update_phase_status(phase_id, PhaseStatus::Completed);
}

// Block phase (transition to Blocked status)
void DefaultPhaseTracker::block_phase(
    int64_t phase_id,
    const std::string& reason
) const {
    update_phase_status(phase_id, PhaseStatus::Blocked);
    // In production, would store the reason in a separate field or log
    (void)reason;
}

// Get phase by ID
DeploymentPhase DefaultPhaseTracker::get_phase(int64_t phase_id) const {
    for (const auto& p : phases_) {
        if (p.id == phase_id) {
            return p;
        }
    }
    // Return default-constructed phase if not found
    return DeploymentPhase();
}

// Get phase by number (1-5)
DeploymentPhase DefaultPhaseTracker::get_phase_by_number(int phase_number) const {
    for (const auto& p : phases_) {
        if (p.phase_number == phase_number) {
            return p;
        }
    }
    return DeploymentPhase();
}

// Get all phases
std::vector<DeploymentPhase> DefaultPhaseTracker::get_all_phases() const {
    return phases_;
}

// Get phases by status
std::vector<DeploymentPhase> DefaultPhaseTracker::get_phases_by_status(
    PhaseStatus status
) const {
    std::vector<DeploymentPhase> result;
    for (const auto& p : phases_) {
        if (p.status == status) {
            result.push_back(p);
        }
    }
    return result;
}

// Get current active phase
DeploymentPhase DefaultPhaseTracker::get_current_phase() const {
    // First, look for phase in progress
    for (const auto& p : phases_) {
        if (p.is_in_progress()) {
            return p;
        }
    }
    // If no phase in progress, return first non-completed phase
    for (const auto& p : phases_) {
        if (!p.is_completed()) {
            return p;
        }
    }
    // All phases completed or no phases
    return DeploymentPhase();
}

// Calculate overall progress percentage (0-100)
double DefaultPhaseTracker::calculate_progress() const noexcept {
    if (phases_.empty()) {
        return 0.0;
    }

    size_t completed = 0;
    for (const auto& p : phases_) {
        if (p.is_completed()) {
            ++completed;
        }
    }

    return (static_cast<double>(completed) / static_cast<double>(phases_.size())) * 100.0;
}

// Check if phase can transition to new status
bool DefaultPhaseTracker::can_transition(
    int64_t phase_id,
    PhaseStatus new_status
) const noexcept {
    auto phase = get_phase(phase_id);
    return validate_phase_transition(
        phase.phase_number,
        phase.status,
        new_status
    );
}

// Validate phase transition
bool DefaultPhaseTracker::validate_phase_transition(
    int phase_number,
    PhaseStatus current_status,
    PhaseStatus new_status
) const noexcept {
    (void)phase_number;  // Phase number not used in validation logic

    // Valid transitions:
    // NotStarted -> InProgress
    if (current_status == PhaseStatus::NotStarted &&
        new_status == PhaseStatus::InProgress) {
        return true;
    }

    // InProgress -> Completed
    if (current_status == PhaseStatus::InProgress &&
        new_status == PhaseStatus::Completed) {
        return true;
    }

    // Any -> Blocked (can block from any state)
    if (new_status == PhaseStatus::Blocked) {
        return true;
    }

    // Blocked -> InProgress (can unblock)
    if (current_status == PhaseStatus::Blocked &&
        new_status == PhaseStatus::InProgress) {
        return true;
    }

    // All other transitions are invalid
    return false;
}

// Get count of phases by status
size_t DefaultPhaseTracker::count_phases_by_status(PhaseStatus status) const noexcept {
    size_t count = 0;
    for (const auto& p : phases_) {
        if (p.status == status) {
            ++count;
        }
    }
    return count;
}

// Clear all phases (for testing)
void DefaultPhaseTracker::clear_phases() const {
    phases_.clear();
}

// Get phase count
size_t DefaultPhaseTracker::get_phase_count() const noexcept {
    return phases_.size();
}

// ============================================================================
// Factory function for creating PhaseTracker instances
// ============================================================================

std::unique_ptr<PhaseTracker> create_phase_tracker(
    std::unique_ptr<EVMCalculator> calculator
) {
    return std::make_unique<DefaultPhaseTracker>(std::move(calculator));
}

} // namespace wfm::deployment
