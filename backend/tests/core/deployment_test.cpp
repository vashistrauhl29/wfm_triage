// 5-Phase Deployment Dashboard Module Tests
// Following TDD principles and C++ Core Guidelines
// Test coverage for EVMCalculator, PhaseTracker, and MilestoneManager

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include "core/deployment/evm_calculator.h"
#include "core/deployment/phase_tracker.h"
#include "core/deployment/milestone_manager.h"
#include "models/evm_metrics.h"
#include "models/deployment_phase.h"
#include "models/milestone.h"

using namespace wfm;
using namespace wfm::deployment;

// ============================================================================
// Mock Implementations (for testing purposes only)
// ============================================================================

// MockEVMCalculator: Test implementation of EVMCalculator interface
class MockEVMCalculator : public EVMCalculator {
public:
    MockEVMCalculator() = default;

    // Calculate EVM metrics from milestones and budget
    EVMMetrics calculate_evm(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::vector<DeploymentPhase>& phases
    ) const override {
        EVMMetrics metrics;

        metrics.planned_value = calculate_planned_value(
            budget_at_completion, milestones, "2024-06-01");
        metrics.earned_value = calculate_earned_value(
            budget_at_completion, milestones);
        metrics.actual_cost = calculate_actual_cost(phases);

        metrics.schedule_variance = metrics.earned_value - metrics.planned_value;
        metrics.cost_variance = metrics.earned_value - metrics.actual_cost;

        metrics.schedule_performance_index = calculate_spi(
            metrics.earned_value, metrics.planned_value);
        metrics.cost_performance_index = calculate_cpi(
            metrics.earned_value, metrics.actual_cost);

        metrics.estimate_at_completion = calculate_eac(
            budget_at_completion, metrics.cost_performance_index);
        metrics.estimate_to_complete = calculate_etc(
            metrics.estimate_at_completion, metrics.actual_cost);
        metrics.variance_at_completion = calculate_vac(
            budget_at_completion, metrics.estimate_at_completion);

        metrics.snapshot_date = "2024-06-01";

        return metrics;
    }

    // Calculate Planned Value (PV)
    double calculate_planned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones,
        const std::string& current_date
    ) const override {
        // PV = sum of weights for milestones that should be completed by current date
        double pv = 0.0;
        for (const auto& m : milestones) {
            if (m.planned_completion_date <= current_date) {
                pv += (m.weight / 100.0) * budget_at_completion;
            }
        }
        return pv;
    }

    // Calculate Earned Value (EV)
    double calculate_earned_value(
        double budget_at_completion,
        const std::vector<Milestone>& milestones
    ) const override {
        // EV = sum of weights for completed milestones
        double ev = 0.0;
        for (const auto& m : milestones) {
            if (m.is_completed()) {
                ev += (m.weight / 100.0) * budget_at_completion;
            }
        }
        return ev;
    }

    // Calculate Actual Cost (AC)
    double calculate_actual_cost(
        const std::vector<DeploymentPhase>& phases
    ) const override {
        double ac = 0.0;
        for (const auto& p : phases) {
            ac += p.actual_cost;
        }
        return ac;
    }

    // Calculate Schedule Performance Index (SPI = EV / PV)
    double calculate_spi(
        double earned_value,
        double planned_value
    ) const noexcept override {
        if (planned_value == 0.0) {
            return 0.0;
        }
        return earned_value / planned_value;
    }

    // Calculate Cost Performance Index (CPI = EV / AC)
    double calculate_cpi(
        double earned_value,
        double actual_cost
    ) const noexcept override {
        if (actual_cost == 0.0) {
            return 0.0;
        }
        return earned_value / actual_cost;
    }

    // Calculate Estimate at Completion (EAC = BAC / CPI)
    double calculate_eac(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept override {
        if (cost_performance_index == 0.0) {
            return budget_at_completion;
        }
        return budget_at_completion / cost_performance_index;
    }

    // Calculate Estimate to Complete (ETC = EAC - AC)
    double calculate_etc(
        double estimate_at_completion,
        double actual_cost
    ) const noexcept override {
        return estimate_at_completion - actual_cost;
    }

    // Calculate Variance at Completion (VAC = BAC - EAC)
    double calculate_vac(
        double budget_at_completion,
        double estimate_at_completion
    ) const noexcept override {
        return budget_at_completion - estimate_at_completion;
    }

    // Predict final cost variance
    double predict_cost_variance(
        double budget_at_completion,
        double cost_performance_index
    ) const noexcept override {
        double eac = calculate_eac(budget_at_completion, cost_performance_index);
        return calculate_vac(budget_at_completion, eac);
    }

    // Check if project is on track
    bool is_on_track(
        double schedule_performance_index,
        double cost_performance_index
    ) const noexcept override {
        return schedule_performance_index >= 0.9 &&
               cost_performance_index >= 0.9;
    }
};

// MockPhaseTracker: Test implementation of PhaseTracker interface
class MockPhaseTracker : public PhaseTracker {
public:
    explicit MockPhaseTracker(std::unique_ptr<EVMCalculator> calculator)
        : PhaseTracker(std::move(calculator)),
          phases_() {}

    void add_phase(const DeploymentPhase& phase) const override {
        phases_.push_back(phase);
    }

    void update_phase_status(
        int64_t phase_id,
        PhaseStatus new_status
    ) const override {
        for (auto& p : phases_) {
            if (p.id == phase_id) {
                p.status = new_status;
                break;
            }
        }
    }

    void update_phase_cost(
        int64_t phase_id,
        double actual_cost
    ) const override {
        for (auto& p : phases_) {
            if (p.id == phase_id) {
                p.actual_cost = actual_cost;
                break;
            }
        }
    }

    void complete_phase(int64_t phase_id) const override {
        update_phase_status(phase_id, PhaseStatus::Completed);
    }

    void block_phase(
        int64_t phase_id,
        const std::string& reason
    ) const override {
        update_phase_status(phase_id, PhaseStatus::Blocked);
        (void)reason;  // In production, would store reason
    }

    DeploymentPhase get_phase(int64_t phase_id) const override {
        for (const auto& p : phases_) {
            if (p.id == phase_id) {
                return p;
            }
        }
        return DeploymentPhase();
    }

    DeploymentPhase get_phase_by_number(int phase_number) const override {
        for (const auto& p : phases_) {
            if (p.phase_number == phase_number) {
                return p;
            }
        }
        return DeploymentPhase();
    }

    std::vector<DeploymentPhase> get_all_phases() const override {
        return phases_;
    }

    std::vector<DeploymentPhase> get_phases_by_status(
        PhaseStatus status
    ) const override {
        std::vector<DeploymentPhase> result;
        for (const auto& p : phases_) {
            if (p.status == status) {
                result.push_back(p);
            }
        }
        return result;
    }

    DeploymentPhase get_current_phase() const override {
        for (const auto& p : phases_) {
            if (p.is_in_progress()) {
                return p;
            }
        }
        // Return first non-completed phase
        for (const auto& p : phases_) {
            if (!p.is_completed()) {
                return p;
            }
        }
        return DeploymentPhase();
    }

    double calculate_progress() const noexcept override {
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

    bool can_transition(
        int64_t phase_id,
        PhaseStatus new_status
    ) const noexcept override {
        auto phase = get_phase(phase_id);
        return validate_phase_transition(
            phase.phase_number,
            phase.status,
            new_status
        );
    }

    bool validate_phase_transition(
        int phase_number,
        PhaseStatus current_status,
        PhaseStatus new_status
    ) const noexcept override {
        // NotStarted -> InProgress: OK
        if (current_status == PhaseStatus::NotStarted &&
            new_status == PhaseStatus::InProgress) {
            return true;
        }
        // InProgress -> Completed: OK
        if (current_status == PhaseStatus::InProgress &&
            new_status == PhaseStatus::Completed) {
            return true;
        }
        // Any -> Blocked: OK
        if (new_status == PhaseStatus::Blocked) {
            return true;
        }
        // Blocked -> InProgress: OK
        if (current_status == PhaseStatus::Blocked &&
            new_status == PhaseStatus::InProgress) {
            return true;
        }
        return false;
    }

    size_t count_phases_by_status(PhaseStatus status) const noexcept override {
        size_t count = 0;
        for (const auto& p : phases_) {
            if (p.status == status) {
                ++count;
            }
        }
        return count;
    }

    void clear_phases() const override {
        phases_.clear();
    }

    size_t get_phase_count() const noexcept override {
        return phases_.size();
    }

private:
    mutable std::vector<DeploymentPhase> phases_;
};

// MockMilestoneManager: Test implementation of MilestoneManager interface
class MockMilestoneManager : public MilestoneManager {
public:
    MockMilestoneManager() : milestones_() {}

    void add_milestone(const Milestone& milestone) const override {
        milestones_.push_back(milestone);
    }

    void update_milestone_status(
        int64_t milestone_id,
        MilestoneStatus new_status
    ) const override {
        for (auto& m : milestones_) {
            if (m.id == milestone_id) {
                m.status = new_status;
                break;
            }
        }
    }

    void complete_milestone(
        int64_t milestone_id,
        const std::string& completion_date
    ) const override {
        for (auto& m : milestones_) {
            if (m.id == milestone_id) {
                m.status = MilestoneStatus::Completed;
                m.actual_completion_date = completion_date;
                break;
            }
        }
    }

    void mark_at_risk(
        int64_t milestone_id,
        const std::string& risk_reason
    ) const override {
        update_milestone_status(milestone_id, MilestoneStatus::AtRisk);
        (void)risk_reason;  // In production, would store reason
    }

    Milestone get_milestone(int64_t milestone_id) const override {
        for (const auto& m : milestones_) {
            if (m.id == milestone_id) {
                return m;
            }
        }
        return Milestone();
    }

    std::vector<Milestone> get_milestones_for_phase(
        int64_t phase_id
    ) const override {
        std::vector<Milestone> result;
        for (const auto& m : milestones_) {
            if (m.phase_id == phase_id) {
                result.push_back(m);
            }
        }
        return result;
    }

    std::vector<Milestone> get_milestones_by_status(
        MilestoneStatus status
    ) const override {
        std::vector<Milestone> result;
        for (const auto& m : milestones_) {
            if (m.status == status) {
                result.push_back(m);
            }
        }
        return result;
    }

    bool are_dependencies_satisfied(
        int64_t milestone_id
    ) const noexcept override {
        auto milestone = get_milestone(milestone_id);
        if (milestone.dependencies.empty()) {
            return true;
        }

        for (auto dep_id : milestone.dependencies) {
            auto dep = get_milestone(dep_id);
            if (!dep.is_completed()) {
                return false;
            }
        }
        return true;
    }

    bool can_complete_milestone(
        int64_t milestone_id
    ) const noexcept override {
        auto milestone = get_milestone(milestone_id);
        if (milestone.is_completed()) {
            return false;  // Already completed
        }
        return are_dependencies_satisfied(milestone_id);
    }

    void add_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const override {
        for (auto& m : milestones_) {
            if (m.id == milestone_id) {
                // Check if dependency already exists
                if (std::find(m.dependencies.begin(), m.dependencies.end(),
                             dependency_id) == m.dependencies.end()) {
                    m.dependencies.push_back(dependency_id);
                }
                break;
            }
        }
    }

    void remove_dependency(
        int64_t milestone_id,
        int64_t dependency_id
    ) const override {
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

    std::vector<int64_t> get_dependency_chain(
        int64_t milestone_id
    ) const override {
        auto milestone = get_milestone(milestone_id);
        return milestone.dependencies;
    }

    double calculate_phase_completion(
        int64_t phase_id
    ) const noexcept override {
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

    size_t get_milestone_count() const noexcept override {
        return milestones_.size();
    }

    size_t get_completed_count() const noexcept override {
        size_t count = 0;
        for (const auto& m : milestones_) {
            if (m.is_completed()) {
                ++count;
            }
        }
        return count;
    }

    size_t get_at_risk_count() const noexcept override {
        size_t count = 0;
        for (const auto& m : milestones_) {
            if (m.is_at_risk()) {
                ++count;
            }
        }
        return count;
    }

    bool validate_milestone_weight(double weight) const noexcept override {
        return weight >= 0.0 && weight <= 100.0;
    }

    void clear_milestones() const override {
        milestones_.clear();
    }

private:
    mutable std::vector<Milestone> milestones_;
};

// ============================================================================
// EVMCalculator Test Suite
// ============================================================================

class EVMCalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        calculator_ = std::make_unique<MockEVMCalculator>();
    }

    std::unique_ptr<EVMCalculator> calculator_;
};

// Test: Calculate Planned Value (PV)
TEST_F(EVMCalculatorTest, CalculatePlannedValue) {
    std::vector<Milestone> milestones = {
        Milestone(1, 1, "M1", 50.0),
        Milestone(2, 1, "M2", 50.0)
    };
    milestones[0].planned_completion_date = "2024-05-01";
    milestones[1].planned_completion_date = "2024-07-01";

    double pv = calculator_->calculate_planned_value(100000.0, milestones, "2024-06-01");

    // Only M1 should be planned by June 1
    EXPECT_DOUBLE_EQ(pv, 50000.0);
}

// Test: Calculate Earned Value (EV)
TEST_F(EVMCalculatorTest, CalculateEarnedValue) {
    std::vector<Milestone> milestones = {
        Milestone(1, 1, "M1", 50.0),
        Milestone(2, 1, "M2", 50.0)
    };
    milestones[0].status = MilestoneStatus::Completed;
    milestones[1].status = MilestoneStatus::InProgress;

    double ev = calculator_->calculate_earned_value(100000.0, milestones);

    // Only M1 is completed
    EXPECT_DOUBLE_EQ(ev, 50000.0);
}

// Test: Calculate Actual Cost (AC)
TEST_F(EVMCalculatorTest, CalculateActualCost) {
    std::vector<DeploymentPhase> phases = {
        DeploymentPhase(1, 1, "Phase 1", 50000.0),
        DeploymentPhase(2, 2, "Phase 2", 50000.0)
    };
    phases[0].actual_cost = 45000.0;
    phases[1].actual_cost = 30000.0;

    double ac = calculator_->calculate_actual_cost(phases);

    EXPECT_DOUBLE_EQ(ac, 75000.0);
}

// Test: Calculate SPI (ahead of schedule)
TEST_F(EVMCalculatorTest, CalculateSpiAheadOfSchedule) {
    double spi = calculator_->calculate_spi(60000.0, 50000.0);

    EXPECT_DOUBLE_EQ(spi, 1.2);  // 20% ahead
}

// Test: Calculate SPI (behind schedule)
TEST_F(EVMCalculatorTest, CalculateSpiBehindSchedule) {
    double spi = calculator_->calculate_spi(40000.0, 50000.0);

    EXPECT_DOUBLE_EQ(spi, 0.8);  // 20% behind
}

// Test: Calculate CPI (under budget)
TEST_F(EVMCalculatorTest, CalculateCpiUnderBudget) {
    double cpi = calculator_->calculate_cpi(50000.0, 45000.0);

    EXPECT_NEAR(cpi, 1.111, 0.001);  // ~11% under budget
}

// Test: Calculate CPI (over budget)
TEST_F(EVMCalculatorTest, CalculateCpiOverBudget) {
    double cpi = calculator_->calculate_cpi(50000.0, 55000.0);

    EXPECT_NEAR(cpi, 0.909, 0.001);  // ~9% over budget
}

// Test: Handle zero PV in SPI calculation
TEST_F(EVMCalculatorTest, HandleZeroPvInSpi) {
    double spi = calculator_->calculate_spi(50000.0, 0.0);

    EXPECT_DOUBLE_EQ(spi, 0.0);  // Avoid division by zero
}

// Test: Handle zero AC in CPI calculation
TEST_F(EVMCalculatorTest, HandleZeroAcInCpi) {
    double cpi = calculator_->calculate_cpi(50000.0, 0.0);

    EXPECT_DOUBLE_EQ(cpi, 0.0);  // Avoid division by zero
}

// Test: Calculate EAC (Estimate at Completion)
TEST_F(EVMCalculatorTest, CalculateEac) {
    double eac = calculator_->calculate_eac(100000.0, 0.9);

    EXPECT_NEAR(eac, 111111.11, 1.0);  // BAC / CPI
}

// Test: Calculate ETC (Estimate to Complete)
TEST_F(EVMCalculatorTest, CalculateEtc) {
    double etc = calculator_->calculate_etc(110000.0, 50000.0);

    EXPECT_DOUBLE_EQ(etc, 60000.0);  // EAC - AC
}

// Test: Calculate VAC (Variance at Completion)
TEST_F(EVMCalculatorTest, CalculateVac) {
    double vac = calculator_->calculate_vac(100000.0, 110000.0);

    EXPECT_DOUBLE_EQ(vac, -10000.0);  // BAC - EAC (negative = overrun)
}

// Test: Predict cost variance (positive = under budget at completion)
TEST_F(EVMCalculatorTest, PredictCostVarianceUnderBudget) {
    double variance = calculator_->predict_cost_variance(100000.0, 1.1);

    EXPECT_NEAR(variance, 9090.91, 1.0);  // Positive variance
}

// Test: Predict cost variance (negative = over budget at completion)
TEST_F(EVMCalculatorTest, PredictCostVarianceOverBudget) {
    double variance = calculator_->predict_cost_variance(100000.0, 0.9);

    EXPECT_NEAR(variance, -11111.11, 1.0);  // Negative variance
}

// Test: Project is on track
TEST_F(EVMCalculatorTest, IsOnTrackTrue) {
    bool on_track = calculator_->is_on_track(1.0, 1.0);

    EXPECT_TRUE(on_track);
}

// Test: Project behind schedule
TEST_F(EVMCalculatorTest, IsOnTrackFalseBehindSchedule) {
    bool on_track = calculator_->is_on_track(0.85, 1.0);

    EXPECT_FALSE(on_track);  // SPI below 0.9 threshold
}

// Test: Project over budget
TEST_F(EVMCalculatorTest, IsOnTrackFalseOverBudget) {
    bool on_track = calculator_->is_on_track(1.0, 0.85);

    EXPECT_FALSE(on_track);  // CPI below 0.9 threshold
}

// Test: Calculate full EVM metrics
TEST_F(EVMCalculatorTest, CalculateFullEvmMetrics) {
    std::vector<Milestone> milestones = {
        Milestone(1, 1, "M1", 50.0),
        Milestone(2, 1, "M2", 50.0)
    };
    milestones[0].planned_completion_date = "2024-05-01";
    milestones[0].status = MilestoneStatus::Completed;
    milestones[1].planned_completion_date = "2024-07-01";

    std::vector<DeploymentPhase> phases = {
        DeploymentPhase(1, 1, "Phase 1", 100000.0)
    };
    phases[0].actual_cost = 45000.0;

    EVMMetrics metrics = calculator_->calculate_evm(100000.0, milestones, phases);

    EXPECT_DOUBLE_EQ(metrics.planned_value, 50000.0);  // M1 planned by June 1
    EXPECT_DOUBLE_EQ(metrics.earned_value, 50000.0);   // M1 completed
    EXPECT_DOUBLE_EQ(metrics.actual_cost, 45000.0);
    EXPECT_DOUBLE_EQ(metrics.schedule_variance, 0.0);  // EV - PV
    EXPECT_DOUBLE_EQ(metrics.cost_variance, 5000.0);   // EV - AC (under budget)
    EXPECT_DOUBLE_EQ(metrics.schedule_performance_index, 1.0);  // On schedule
    EXPECT_NEAR(metrics.cost_performance_index, 1.111, 0.001);  // Under budget
}

// Test: EVM health score calculation
TEST_F(EVMCalculatorTest, EvmHealthScore) {
    EVMMetrics metrics;
    metrics.schedule_performance_index = 1.0;
    metrics.cost_performance_index = 1.0;
    metrics.variance_at_completion = 0.0;

    double score = metrics.get_health_score();

    EXPECT_DOUBLE_EQ(score, 100.0);  // Perfect score
}

// Test: EVM is_on_track method
TEST_F(EVMCalculatorTest, EvmIsOnTrackMethod) {
    EVMMetrics metrics;
    metrics.schedule_performance_index = 0.95;
    metrics.cost_performance_index = 0.95;

    EXPECT_TRUE(metrics.is_on_track());
}

// ============================================================================
// PhaseTracker Test Suite
// ============================================================================

class PhaseTrackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto evm_calculator = std::make_unique<MockEVMCalculator>();
        tracker_ = std::make_unique<MockPhaseTracker>(std::move(evm_calculator));
    }

    std::unique_ptr<PhaseTracker> tracker_;
};

// Test: Add phase
TEST_F(PhaseTrackerTest, AddPhase) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);

    tracker_->add_phase(phase);

    EXPECT_EQ(tracker_->get_phase_count(), 1);
}

// Test: Add multiple phases
TEST_F(PhaseTrackerTest, AddMultiplePhases) {
    for (int i = 1; i <= 5; ++i) {
        DeploymentPhase phase(i, i, "Phase " + std::to_string(i), 20000.0);
        tracker_->add_phase(phase);
    }

    EXPECT_EQ(tracker_->get_phase_count(), 5);
}

// Test: Get phase by ID
TEST_F(PhaseTrackerTest, GetPhaseById) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    tracker_->add_phase(phase);

    auto retrieved = tracker_->get_phase(1);

    EXPECT_EQ(retrieved.id, 1);
    EXPECT_EQ(retrieved.phase_name, "Phase 1");
}

// Test: Get phase by number
TEST_F(PhaseTrackerTest, GetPhaseByNumber) {
    DeploymentPhase phase(10, 3, "Phase 3", 50000.0);
    tracker_->add_phase(phase);

    auto retrieved = tracker_->get_phase_by_number(3);

    EXPECT_EQ(retrieved.phase_number, 3);
    EXPECT_EQ(retrieved.id, 10);
}

// Test: Update phase status
TEST_F(PhaseTrackerTest, UpdatePhaseStatus) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    tracker_->add_phase(phase);

    tracker_->update_phase_status(1, PhaseStatus::InProgress);

    auto updated = tracker_->get_phase(1);
    EXPECT_EQ(updated.status, PhaseStatus::InProgress);
}

// Test: Update phase cost
TEST_F(PhaseTrackerTest, UpdatePhaseCost) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    tracker_->add_phase(phase);

    tracker_->update_phase_cost(1, 45000.0);

    auto updated = tracker_->get_phase(1);
    EXPECT_DOUBLE_EQ(updated.actual_cost, 45000.0);
}

// Test: Complete phase
TEST_F(PhaseTrackerTest, CompletePhase) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    phase.status = PhaseStatus::InProgress;
    tracker_->add_phase(phase);

    tracker_->complete_phase(1);

    auto completed = tracker_->get_phase(1);
    EXPECT_EQ(completed.status, PhaseStatus::Completed);
}

// Test: Block phase
TEST_F(PhaseTrackerTest, BlockPhase) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    phase.status = PhaseStatus::InProgress;
    tracker_->add_phase(phase);

    tracker_->block_phase(1, "Missing dependency");

    auto blocked = tracker_->get_phase(1);
    EXPECT_EQ(blocked.status, PhaseStatus::Blocked);
}

// Test: Get all phases
TEST_F(PhaseTrackerTest, GetAllPhases) {
    for (int i = 1; i <= 3; ++i) {
        DeploymentPhase phase(i, i, "Phase " + std::to_string(i), 30000.0);
        tracker_->add_phase(phase);
    }

    auto all_phases = tracker_->get_all_phases();

    EXPECT_EQ(all_phases.size(), 3);
}

// Test: Get phases by status
TEST_F(PhaseTrackerTest, GetPhasesByStatus) {
    DeploymentPhase phase1(1, 1, "Phase 1", 50000.0);
    phase1.status = PhaseStatus::Completed;
    DeploymentPhase phase2(2, 2, "Phase 2", 50000.0);
    phase2.status = PhaseStatus::InProgress;
    DeploymentPhase phase3(3, 3, "Phase 3", 50000.0);
    phase3.status = PhaseStatus::Completed;

    tracker_->add_phase(phase1);
    tracker_->add_phase(phase2);
    tracker_->add_phase(phase3);

    auto completed = tracker_->get_phases_by_status(PhaseStatus::Completed);

    EXPECT_EQ(completed.size(), 2);
}

// Test: Get current active phase
TEST_F(PhaseTrackerTest, GetCurrentPhase) {
    DeploymentPhase phase1(1, 1, "Phase 1", 50000.0);
    phase1.status = PhaseStatus::Completed;
    DeploymentPhase phase2(2, 2, "Phase 2", 50000.0);
    phase2.status = PhaseStatus::InProgress;

    tracker_->add_phase(phase1);
    tracker_->add_phase(phase2);

    auto current = tracker_->get_current_phase();

    EXPECT_EQ(current.phase_number, 2);
    EXPECT_TRUE(current.is_in_progress());
}

// Test: Calculate overall progress
TEST_F(PhaseTrackerTest, CalculateProgress) {
    for (int i = 1; i <= 5; ++i) {
        DeploymentPhase phase(i, i, "Phase " + std::to_string(i), 20000.0);
        if (i <= 2) {
            phase.status = PhaseStatus::Completed;
        }
        tracker_->add_phase(phase);
    }

    double progress = tracker_->calculate_progress();

    EXPECT_DOUBLE_EQ(progress, 40.0);  // 2 out of 5 phases = 40%
}

// Test: Validate phase transition (NotStarted -> InProgress)
TEST_F(PhaseTrackerTest, ValidateTransitionNotStartedToInProgress) {
    bool valid = tracker_->validate_phase_transition(
        1,
        PhaseStatus::NotStarted,
        PhaseStatus::InProgress
    );

    EXPECT_TRUE(valid);
}

// Test: Validate phase transition (InProgress -> Completed)
TEST_F(PhaseTrackerTest, ValidateTransitionInProgressToCompleted) {
    bool valid = tracker_->validate_phase_transition(
        1,
        PhaseStatus::InProgress,
        PhaseStatus::Completed
    );

    EXPECT_TRUE(valid);
}

// Test: Invalid phase transition (NotStarted -> Completed)
TEST_F(PhaseTrackerTest, ValidateTransitionInvalid) {
    bool valid = tracker_->validate_phase_transition(
        1,
        PhaseStatus::NotStarted,
        PhaseStatus::Completed
    );

    EXPECT_FALSE(valid);  // Cannot skip InProgress
}

// Test: Can transition check
TEST_F(PhaseTrackerTest, CanTransition) {
    DeploymentPhase phase(1, 1, "Phase 1", 50000.0);
    phase.status = PhaseStatus::NotStarted;
    tracker_->add_phase(phase);

    bool can = tracker_->can_transition(1, PhaseStatus::InProgress);

    EXPECT_TRUE(can);
}

// Test: Count phases by status
TEST_F(PhaseTrackerTest, CountPhasesByStatus) {
    for (int i = 1; i <= 5; ++i) {
        DeploymentPhase phase(i, i, "Phase " + std::to_string(i), 20000.0);
        if (i <= 2) {
            phase.status = PhaseStatus::Completed;
        } else if (i == 3) {
            phase.status = PhaseStatus::InProgress;
        }
        tracker_->add_phase(phase);
    }

    EXPECT_EQ(tracker_->count_phases_by_status(PhaseStatus::Completed), 2);
    EXPECT_EQ(tracker_->count_phases_by_status(PhaseStatus::InProgress), 1);
    EXPECT_EQ(tracker_->count_phases_by_status(PhaseStatus::NotStarted), 2);
}

// Test: Clear phases
TEST_F(PhaseTrackerTest, ClearPhases) {
    for (int i = 1; i <= 3; ++i) {
        DeploymentPhase phase(i, i, "Phase " + std::to_string(i), 30000.0);
        tracker_->add_phase(phase);
    }

    tracker_->clear_phases();

    EXPECT_EQ(tracker_->get_phase_count(), 0);
}

// ============================================================================
// MilestoneManager Test Suite
// ============================================================================

class MilestoneManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager_ = std::make_unique<MockMilestoneManager>();
    }

    std::unique_ptr<MilestoneManager> manager_;
};

// Test: Add milestone
TEST_F(MilestoneManagerTest, AddMilestone) {
    Milestone milestone(1, 1, "Milestone 1", 50.0);

    manager_->add_milestone(milestone);

    EXPECT_EQ(manager_->get_milestone_count(), 1);
}

// Test: Add multiple milestones
TEST_F(MilestoneManagerTest, AddMultipleMilestones) {
    for (int i = 1; i <= 5; ++i) {
        Milestone milestone(i, 1, "Milestone " + std::to_string(i), 20.0);
        manager_->add_milestone(milestone);
    }

    EXPECT_EQ(manager_->get_milestone_count(), 5);
}

// Test: Update milestone status
TEST_F(MilestoneManagerTest, UpdateMilestoneStatus) {
    Milestone milestone(1, 1, "Milestone 1", 50.0);
    manager_->add_milestone(milestone);

    manager_->update_milestone_status(1, MilestoneStatus::InProgress);

    auto updated = manager_->get_milestone(1);
    EXPECT_EQ(updated.status, MilestoneStatus::InProgress);
}

// Test: Complete milestone
TEST_F(MilestoneManagerTest, CompleteMilestone) {
    Milestone milestone(1, 1, "Milestone 1", 50.0);
    manager_->add_milestone(milestone);

    manager_->complete_milestone(1, "2024-06-01");

    auto completed = manager_->get_milestone(1);
    EXPECT_EQ(completed.status, MilestoneStatus::Completed);
    EXPECT_EQ(completed.actual_completion_date, "2024-06-01");
}

// Test: Mark milestone as at risk
TEST_F(MilestoneManagerTest, MarkAtRisk) {
    Milestone milestone(1, 1, "Milestone 1", 50.0);
    manager_->add_milestone(milestone);

    manager_->mark_at_risk(1, "Resource constraint");

    auto at_risk = manager_->get_milestone(1);
    EXPECT_EQ(at_risk.status, MilestoneStatus::AtRisk);
}

// Test: Get milestones for phase
TEST_F(MilestoneManagerTest, GetMilestonesForPhase) {
    Milestone m1(1, 1, "M1", 30.0);
    Milestone m2(2, 1, "M2", 40.0);
    Milestone m3(3, 2, "M3", 30.0);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);
    manager_->add_milestone(m3);

    auto phase1_milestones = manager_->get_milestones_for_phase(1);

    EXPECT_EQ(phase1_milestones.size(), 2);
}

// Test: Get milestones by status
TEST_F(MilestoneManagerTest, GetMilestonesByStatus) {
    Milestone m1(1, 1, "M1", 30.0);
    m1.status = MilestoneStatus::Completed;
    Milestone m2(2, 1, "M2", 40.0);
    m2.status = MilestoneStatus::InProgress;
    Milestone m3(3, 1, "M3", 30.0);
    m3.status = MilestoneStatus::Completed;

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);
    manager_->add_milestone(m3);

    auto completed = manager_->get_milestones_by_status(MilestoneStatus::Completed);

    EXPECT_EQ(completed.size(), 2);
}

// Test: Dependencies satisfied (no dependencies)
TEST_F(MilestoneManagerTest, DependenciesSatisfiedNoDependencies) {
    Milestone milestone(1, 1, "M1", 50.0);
    manager_->add_milestone(milestone);

    bool satisfied = manager_->are_dependencies_satisfied(1);

    EXPECT_TRUE(satisfied);
}

// Test: Dependencies satisfied (all completed)
TEST_F(MilestoneManagerTest, DependenciesSatisfiedAllCompleted) {
    Milestone m1(1, 1, "M1", 30.0);
    m1.status = MilestoneStatus::Completed;
    Milestone m2(2, 1, "M2", 40.0);
    m2.dependencies.push_back(1);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    bool satisfied = manager_->are_dependencies_satisfied(2);

    EXPECT_TRUE(satisfied);
}

// Test: Dependencies not satisfied
TEST_F(MilestoneManagerTest, DependenciesNotSatisfied) {
    Milestone m1(1, 1, "M1", 30.0);
    Milestone m2(2, 1, "M2", 40.0);
    m2.dependencies.push_back(1);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    bool satisfied = manager_->are_dependencies_satisfied(2);

    EXPECT_FALSE(satisfied);  // M1 not completed
}

// Test: Can complete milestone (dependencies satisfied)
TEST_F(MilestoneManagerTest, CanCompleteMilestoneDependenciesSatisfied) {
    Milestone m1(1, 1, "M1", 30.0);
    m1.status = MilestoneStatus::Completed;
    Milestone m2(2, 1, "M2", 40.0);
    m2.dependencies.push_back(1);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    bool can_complete = manager_->can_complete_milestone(2);

    EXPECT_TRUE(can_complete);
}

// Test: Cannot complete milestone (already completed)
TEST_F(MilestoneManagerTest, CannotCompleteMilestoneAlreadyCompleted) {
    Milestone milestone(1, 1, "M1", 50.0);
    milestone.status = MilestoneStatus::Completed;
    manager_->add_milestone(milestone);

    bool can_complete = manager_->can_complete_milestone(1);

    EXPECT_FALSE(can_complete);
}

// Test: Add dependency
TEST_F(MilestoneManagerTest, AddDependency) {
    Milestone m1(1, 1, "M1", 30.0);
    Milestone m2(2, 1, "M2", 40.0);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    manager_->add_dependency(2, 1);

    auto m2_updated = manager_->get_milestone(2);
    EXPECT_EQ(m2_updated.dependencies.size(), 1);
    EXPECT_EQ(m2_updated.dependencies[0], 1);
}

// Test: Remove dependency
TEST_F(MilestoneManagerTest, RemoveDependency) {
    Milestone m1(1, 1, "M1", 30.0);
    Milestone m2(2, 1, "M2", 40.0);
    m2.dependencies.push_back(1);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    manager_->remove_dependency(2, 1);

    auto m2_updated = manager_->get_milestone(2);
    EXPECT_TRUE(m2_updated.dependencies.empty());
}

// Test: Get dependency chain
TEST_F(MilestoneManagerTest, GetDependencyChain) {
    Milestone m1(1, 1, "M1", 30.0);
    Milestone m2(2, 1, "M2", 40.0);
    m2.dependencies.push_back(1);

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    auto chain = manager_->get_dependency_chain(2);

    EXPECT_EQ(chain.size(), 1);
    EXPECT_EQ(chain[0], 1);
}

// Test: Calculate phase completion (weighted)
TEST_F(MilestoneManagerTest, CalculatePhaseCompletion) {
    Milestone m1(1, 1, "M1", 60.0);
    m1.status = MilestoneStatus::Completed;
    Milestone m2(2, 1, "M2", 40.0);
    m2.status = MilestoneStatus::InProgress;

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);

    double completion = manager_->calculate_phase_completion(1);

    EXPECT_DOUBLE_EQ(completion, 60.0);  // 60 out of 100 weight completed
}

// Test: Get completed milestone count
TEST_F(MilestoneManagerTest, GetCompletedCount) {
    Milestone m1(1, 1, "M1", 30.0);
    m1.status = MilestoneStatus::Completed;
    Milestone m2(2, 1, "M2", 40.0);
    Milestone m3(3, 1, "M3", 30.0);
    m3.status = MilestoneStatus::Completed;

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);
    manager_->add_milestone(m3);

    EXPECT_EQ(manager_->get_completed_count(), 2);
}

// Test: Get at-risk milestone count
TEST_F(MilestoneManagerTest, GetAtRiskCount) {
    Milestone m1(1, 1, "M1", 30.0);
    m1.status = MilestoneStatus::AtRisk;
    Milestone m2(2, 1, "M2", 40.0);
    Milestone m3(3, 1, "M3", 30.0);
    m3.status = MilestoneStatus::AtRisk;

    manager_->add_milestone(m1);
    manager_->add_milestone(m2);
    manager_->add_milestone(m3);

    EXPECT_EQ(manager_->get_at_risk_count(), 2);
}

// Test: Validate milestone weight (valid)
TEST_F(MilestoneManagerTest, ValidateMilestoneWeightValid) {
    EXPECT_TRUE(manager_->validate_milestone_weight(50.0));
    EXPECT_TRUE(manager_->validate_milestone_weight(0.0));
    EXPECT_TRUE(manager_->validate_milestone_weight(100.0));
}

// Test: Validate milestone weight (invalid)
TEST_F(MilestoneManagerTest, ValidateMilestoneWeightInvalid) {
    EXPECT_FALSE(manager_->validate_milestone_weight(-1.0));
    EXPECT_FALSE(manager_->validate_milestone_weight(101.0));
}

// Test: Clear milestones
TEST_F(MilestoneManagerTest, ClearMilestones) {
    for (int i = 1; i <= 3; ++i) {
        Milestone milestone(i, 1, "M" + std::to_string(i), 30.0);
        manager_->add_milestone(milestone);
    }

    manager_->clear_milestones();

    EXPECT_EQ(manager_->get_milestone_count(), 0);
}

// Test: Deployment health status determination
TEST_F(MilestoneManagerTest, DeploymentHealthStatus) {
    DeploymentHealth health;

    health.overall_health_score = 85.0;
    EXPECT_EQ(health.determine_health_status(), "excellent");

    health.overall_health_score = 65.0;
    EXPECT_EQ(health.determine_health_status(), "good");

    health.overall_health_score = 45.0;
    EXPECT_EQ(health.determine_health_status(), "at_risk");

    health.overall_health_score = 25.0;
    EXPECT_EQ(health.determine_health_status(), "critical");
}

// Test: Deployment health completion percentage
TEST_F(MilestoneManagerTest, DeploymentHealthCompletionPercentage) {
    DeploymentHealth health;
    health.completed_milestones = 7;
    health.total_milestones = 10;

    double completion = health.get_completion_percentage();

    EXPECT_DOUBLE_EQ(completion, 70.0);
}
