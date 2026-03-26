// Unit tests for Dynamic Margin & Cost Simulator module
// Following C++ Core Guidelines and Google Test best practices
// TDD Approach: Tests written before implementation

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <cmath>

// Forward declarations - implementation files to be created
#include "core/simulator/cost_calculator.h"
#include "core/simulator/threshold_optimizer.h"
#include "models/cost_parameters.h"
#include "models/cost_simulation.h"

namespace wfm::simulator {

// ============================================================================
// CostCalculator Test Suite
// ============================================================================

class CostCalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard cost parameters for testing
        // API cost: $0.001 per call
        // Human rate: $25/hour = $0.00694 per second
        // Avg handling time: 180 seconds (3 minutes)
        params_ = CostParameters(0.001, 25.0, 180);

        calculator_ = std::make_unique<CostCalculator>(params_);
    }

    void TearDown() override {
        calculator_.reset();
    }

    CostParameters params_;
    std::unique_ptr<CostCalculator> calculator_;
};

// Test: Calculate API cost only (100% STP rate)
TEST_F(CostCalculatorTest, CalculatesApiCostOnly) {
    // Arrange
    const double stp_rate = 1.0;  // 100% STP, no human intervention
    const size_t total_tickets = 1000;

    // Act
    const double cost = calculator_->calculate_cost_per_resolution(stp_rate, total_tickets);

    // Assert
    // All tickets go through API: 1000 * $0.001 = $1.00
    EXPECT_DOUBLE_EQ(cost, 0.001);
}

// Test: Calculate human cost only (0% STP rate)
TEST_F(CostCalculatorTest, CalculatesHumanCostOnly) {
    // Arrange
    const double stp_rate = 0.0;  // 0% STP, all human review
    const size_t total_tickets = 1000;

    // Act
    const double cost = calculator_->calculate_cost_per_resolution(stp_rate, total_tickets);

    // Assert
    // Human cost per ticket: (180 seconds / 3600) * $25 = $1.25
    const double expected_cost = (180.0 / 3600.0) * 25.0;
    EXPECT_DOUBLE_EQ(cost, expected_cost);
}

// Test: Calculate blended cost (50% STP rate)
TEST_F(CostCalculatorTest, CalculatesBlendedCost) {
    // Arrange
    const double stp_rate = 0.5;  // 50% STP, 50% human
    const size_t total_tickets = 1000;

    // Act
    const double cost = calculator_->calculate_cost_per_resolution(stp_rate, total_tickets);

    // Assert
    // API cost: 500 * $0.001 = $0.50
    // Human cost: 500 * $1.25 = $625.00
    // Total: $625.50 / 1000 = $0.62550
    const double api_cost = 0.001;
    const double human_cost = (180.0 / 3600.0) * 25.0;
    const double expected_cost = (stp_rate * api_cost) + ((1.0 - stp_rate) * human_cost);
    EXPECT_DOUBLE_EQ(cost, expected_cost);
}

// Test: Calculate total operational cost
TEST_F(CostCalculatorTest, CalculatesTotalOperationalCost) {
    // Arrange
    const double stp_rate = 0.8;  // 80% STP
    const size_t total_tickets = 10000;

    // Act
    const double total_cost = calculator_->calculate_total_cost(stp_rate, total_tickets);

    // Assert
    // STP tickets: 8000 * $0.001 = $8.00
    // Human tickets: 2000 * $1.25 = $2500.00
    // Total: $2508.00
    const double stp_tickets = total_tickets * stp_rate;
    const double human_tickets = total_tickets * (1.0 - stp_rate);
    const double expected_total = (stp_tickets * 0.001) + (human_tickets * (180.0 / 3600.0) * 25.0);
    EXPECT_NEAR(total_cost, expected_total, 0.01);
}

// Test: Cost decreases as STP rate increases
TEST_F(CostCalculatorTest, CostDecreasesWithHigherStpRate) {
    // Arrange
    const size_t tickets = 1000;
    const double low_stp = 0.3;
    const double high_stp = 0.9;

    // Act
    const double low_cost = calculator_->calculate_cost_per_resolution(low_stp, tickets);
    const double high_cost = calculator_->calculate_cost_per_resolution(high_stp, tickets);

    // Assert
    EXPECT_LT(high_cost, low_cost);
}

// Test: Calculate with different API costs
TEST_F(CostCalculatorTest, HandlesVariableApiCosts) {
    // Arrange - expensive API
    const CostParameters expensive_params(0.01, 25.0, 180);  // 10x more expensive
    const auto expensive_calc = std::make_unique<CostCalculator>(expensive_params);

    // Act
    const double cheap_cost = calculator_->calculate_cost_per_resolution(1.0, 1000);
    const double expensive_cost = expensive_calc->calculate_cost_per_resolution(1.0, 1000);

    // Assert
    EXPECT_DOUBLE_EQ(cheap_cost, 0.001);
    EXPECT_DOUBLE_EQ(expensive_cost, 0.01);
}

// Test: Calculate with different human rates
TEST_F(CostCalculatorTest, HandlesVariableHumanRates) {
    // Arrange - higher paid operators
    const CostParameters premium_params(0.001, 50.0, 180);  // 2x higher rate
    const auto premium_calc = std::make_unique<CostCalculator>(premium_params);

    // Act
    const double standard_cost = calculator_->calculate_cost_per_resolution(0.0, 1000);
    const double premium_cost = premium_calc->calculate_cost_per_resolution(0.0, 1000);

    // Assert
    EXPECT_DOUBLE_EQ(standard_cost, (180.0 / 3600.0) * 25.0);
    EXPECT_DOUBLE_EQ(premium_cost, (180.0 / 3600.0) * 50.0);
    EXPECT_DOUBLE_EQ(premium_cost, standard_cost * 2.0);
}

// Test: Calculate with different handling times
TEST_F(CostCalculatorTest, HandlesVariableHandlingTimes) {
    // Arrange - faster handling time
    const CostParameters fast_params(0.001, 25.0, 90);  // Half the time
    const auto fast_calc = std::make_unique<CostCalculator>(fast_params);

    // Act
    const double slow_cost = calculator_->calculate_cost_per_resolution(0.0, 1000);
    const double fast_cost = fast_calc->calculate_cost_per_resolution(0.0, 1000);

    // Assert
    EXPECT_DOUBLE_EQ(slow_cost, (180.0 / 3600.0) * 25.0);
    EXPECT_DOUBLE_EQ(fast_cost, (90.0 / 3600.0) * 25.0);
    EXPECT_DOUBLE_EQ(fast_cost, slow_cost * 0.5);
}

// Test: Invalid STP rate above 1.0 throws exception
TEST_F(CostCalculatorTest, ThrowsOnStpRateAboveOne) {
    // Arrange
    const double invalid_rate = 1.5;
    const size_t tickets = 1000;

    // Act & Assert
    EXPECT_THROW(
        calculator_->calculate_cost_per_resolution(invalid_rate, tickets),
        std::invalid_argument
    );
}

// Test: Invalid STP rate below 0.0 throws exception
TEST_F(CostCalculatorTest, ThrowsOnStpRateBelowZero) {
    // Arrange
    const double invalid_rate = -0.1;
    const size_t tickets = 1000;

    // Act & Assert
    EXPECT_THROW(
        calculator_->calculate_cost_per_resolution(invalid_rate, tickets),
        std::invalid_argument
    );
}

// Test: Zero tickets handled correctly
TEST_F(CostCalculatorTest, HandlesZeroTickets) {
    // Arrange
    const double stp_rate = 0.5;
    const size_t zero_tickets = 0;

    // Act
    const double total_cost = calculator_->calculate_total_cost(stp_rate, zero_tickets);

    // Assert
    EXPECT_DOUBLE_EQ(total_cost, 0.0);
}

// Test: Calculate cost savings from increased STP rate
TEST_F(CostCalculatorTest, CalculatesCostSavings) {
    // Arrange
    const size_t tickets = 10000;
    const double baseline_stp = 0.5;
    const double improved_stp = 0.8;

    // Act
    const double baseline_cost = calculator_->calculate_total_cost(baseline_stp, tickets);
    const double improved_cost = calculator_->calculate_total_cost(improved_stp, tickets);
    const double savings = baseline_cost - improved_cost;

    // Assert
    EXPECT_GT(savings, 0.0);  // Should have positive savings
}

// Test: Const-correctness
TEST_F(CostCalculatorTest, IsConstCorrect) {
    // Arrange
    const auto& const_calc = *calculator_;
    const double stp_rate = 0.5;
    const size_t tickets = 1000;

    // Act - should compile without issues
    const double cost = const_calc.calculate_cost_per_resolution(stp_rate, tickets);

    // Assert
    EXPECT_GT(cost, 0.0);
}

// ============================================================================
// ThresholdOptimizer Test Suite
// ============================================================================

class ThresholdOptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard cost parameters
        params_ = CostParameters(0.001, 25.0, 180);

        calculator_ = std::make_unique<CostCalculator>(params_);
        optimizer_ = std::make_unique<ThresholdOptimizer>(std::move(calculator_));
    }

    void TearDown() override {
        optimizer_.reset();
    }

    CostParameters params_;
    std::unique_ptr<CostCalculator> calculator_;
    std::unique_ptr<ThresholdOptimizer> optimizer_;
};

// Test: Simulate single threshold scenario
TEST_F(ThresholdOptimizerTest, SimulatesSingleThreshold) {
    // Arrange
    const double threshold = 0.95;
    const size_t total_tickets = 10000;
    const double expected_stp_rate = 0.75;  // 75% of tickets above 0.95

    // Act
    const auto result = optimizer_->simulate_threshold(
        threshold,
        total_tickets,
        expected_stp_rate
    );

    // Assert
    EXPECT_DOUBLE_EQ(result.threshold, 0.95);
    EXPECT_DOUBLE_EQ(result.projected_stp_rate, 0.75);
    EXPECT_GT(result.projected_cost_per_resolution, 0.0);
    EXPECT_EQ(result.total_tickets, 10000);
}

// Test: Simulate multiple threshold scenarios
TEST_F(ThresholdOptimizerTest, SimulatesMultipleThresholds) {
    // Arrange
    const std::vector<double> thresholds = {0.90, 0.95, 0.98, 0.99};
    const std::vector<double> stp_rates = {0.85, 0.75, 0.60, 0.45};
    const size_t tickets = 10000;

    // Act
    const auto results = optimizer_->simulate_scenarios(
        thresholds,
        tickets,
        stp_rates
    );

    // Assert
    EXPECT_EQ(results.size(), 4);

    // Verify each scenario
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_DOUBLE_EQ(results[i].threshold, thresholds[i]);
        EXPECT_DOUBLE_EQ(results[i].projected_stp_rate, stp_rates[i]);
    }
}

// Test: Higher thresholds result in lower STP rates
TEST_F(ThresholdOptimizerTest, HigherThresholdsLowerStpRates) {
    // Arrange
    const size_t tickets = 10000;
    const double low_threshold = 0.90;
    const double high_threshold = 0.99;

    // Simulate with realistic STP rates
    const auto low_result = optimizer_->simulate_threshold(low_threshold, tickets, 0.85);
    const auto high_result = optimizer_->simulate_threshold(high_threshold, tickets, 0.45);

    // Assert
    EXPECT_LT(high_result.projected_stp_rate, low_result.projected_stp_rate);
}

// Test: Lower STP rates result in higher costs
TEST_F(ThresholdOptimizerTest, LowerStpRatesIncreasesCosts) {
    // Arrange
    const double threshold = 0.95;
    const size_t tickets = 10000;
    const double high_stp = 0.85;
    const double low_stp = 0.45;

    // Act
    const auto high_stp_result = optimizer_->simulate_threshold(threshold, tickets, high_stp);
    const auto low_stp_result = optimizer_->simulate_threshold(threshold, tickets, low_stp);

    // Assert
    EXPECT_LT(
        high_stp_result.projected_cost_per_resolution,
        low_stp_result.projected_cost_per_resolution
    );
}

// Test: Find optimal threshold within constraints
TEST_F(ThresholdOptimizerTest, FindsOptimalThreshold) {
    // Arrange
    const double min_threshold = 0.90;
    const double max_threshold = 0.99;
    const double max_acceptable_cost = 0.50;  // $0.50 per resolution
    const size_t tickets = 10000;

    // Act
    const auto optimal = optimizer_->find_optimal_threshold(
        min_threshold,
        max_threshold,
        max_acceptable_cost,
        tickets
    );

    // Assert
    EXPECT_GE(optimal.threshold, min_threshold);
    EXPECT_LE(optimal.threshold, max_threshold);
    EXPECT_LE(optimal.projected_cost_per_resolution, max_acceptable_cost);
}

// Test: Calculate break-even point (API cost vs human cost)
TEST_F(ThresholdOptimizerTest, CalculatesBreakEvenPoint) {
    // Arrange
    // API cost: $0.001
    // Human cost: $1.25
    // Break-even STP rate: where blended cost equals human-only cost

    const size_t tickets = 10000;

    // Act
    const auto break_even = optimizer_->calculate_break_even_stp_rate();

    // Assert
    // Break-even should be very high since API is much cheaper
    EXPECT_GT(break_even, 0.0);
    EXPECT_LE(break_even, 1.0);
}

// Test: Optimize for cost minimization
TEST_F(ThresholdOptimizerTest, OptimizesForMinimumCost) {
    // Arrange
    const std::vector<double> thresholds = {0.85, 0.90, 0.95, 0.98, 0.99};
    const std::vector<double> stp_rates = {0.90, 0.85, 0.75, 0.60, 0.45};
    const size_t tickets = 10000;

    // Act
    const auto results = optimizer_->simulate_scenarios(thresholds, tickets, stp_rates);
    const auto min_cost_scenario = optimizer_->find_minimum_cost_scenario(results);

    // Assert
    // Lowest threshold should have lowest cost (highest STP rate)
    EXPECT_DOUBLE_EQ(min_cost_scenario.threshold, 0.85);
    EXPECT_DOUBLE_EQ(min_cost_scenario.projected_stp_rate, 0.90);
}

// Test: Simulate with capacity constraints
TEST_F(ThresholdOptimizerTest, SimulatesWithCapacityConstraints) {
    // Arrange
    const double threshold = 0.95;
    const size_t total_tickets = 10000;
    const double stp_rate = 0.75;
    const size_t max_human_capacity = 2000;  // Can only handle 2000 human reviews

    // Act
    const auto result = optimizer_->simulate_with_capacity_constraint(
        threshold,
        total_tickets,
        stp_rate,
        max_human_capacity
    );

    // Assert - check that result respects capacity constraint
    const size_t result_human_tickets = static_cast<size_t>(
        static_cast<double>(result.total_tickets) * (1.0 - result.projected_stp_rate)
    );
    EXPECT_LE(result_human_tickets, max_human_capacity);
}

// Test: Calculate projected monthly costs
TEST_F(ThresholdOptimizerTest, CalculatesMonthlyProjections) {
    // Arrange
    const double threshold = 0.95;
    const size_t daily_tickets = 10000;
    const double stp_rate = 0.75;
    const size_t days_per_month = 30;

    // Act
    const auto monthly = optimizer_->project_monthly_cost(
        threshold,
        daily_tickets,
        stp_rate,
        days_per_month
    );

    // Assert
    EXPECT_GT(monthly.total_cost, 0.0);
    EXPECT_EQ(monthly.total_tickets, daily_tickets * days_per_month);
}

// Test: Compare two threshold strategies
TEST_F(ThresholdOptimizerTest, ComparesThresholdStrategies) {
    // Arrange
    const size_t tickets = 10000;

    // Conservative strategy: high threshold, low STP rate, high quality
    const auto conservative = optimizer_->simulate_threshold(0.99, tickets, 0.45);

    // Aggressive strategy: low threshold, high STP rate, lower cost
    const auto aggressive = optimizer_->simulate_threshold(0.90, tickets, 0.85);

    // Act
    const auto comparison = optimizer_->compare_strategies(conservative, aggressive);

    // Assert
    EXPECT_GT(comparison.cost_difference, 0.0);  // Conservative costs more
    EXPECT_LT(comparison.stp_rate_difference, 0.0);  // Aggressive has higher STP (negative diff means B > A)
}

// Test: Invalid threshold above 1.0 throws exception
TEST_F(ThresholdOptimizerTest, ThrowsOnInvalidThresholdAboveOne) {
    // Arrange
    const double invalid_threshold = 1.5;
    const size_t tickets = 1000;
    const double stp_rate = 0.75;

    // Act & Assert
    EXPECT_THROW(
        optimizer_->simulate_threshold(invalid_threshold, tickets, stp_rate),
        std::invalid_argument
    );
}

// Test: Invalid threshold below 0.0 throws exception
TEST_F(ThresholdOptimizerTest, ThrowsOnInvalidThresholdBelowZero) {
    // Arrange
    const double invalid_threshold = -0.1;
    const size_t tickets = 1000;
    const double stp_rate = 0.75;

    // Act & Assert
    EXPECT_THROW(
        optimizer_->simulate_threshold(invalid_threshold, tickets, stp_rate),
        std::invalid_argument
    );
}

// Test: RAII principles with unique_ptr
TEST_F(ThresholdOptimizerTest, MaintainsRaiiPrinciples) {
    // Arrange
    auto calculator = std::make_unique<CostCalculator>(params_);

    // Act - ownership transferred
    auto optimizer = std::make_unique<ThresholdOptimizer>(std::move(calculator));

    // Assert - calculator is now null (ownership transferred)
    EXPECT_EQ(calculator, nullptr);
    EXPECT_NE(optimizer, nullptr);
}

// Test: Const-correctness
TEST_F(ThresholdOptimizerTest, IsConstCorrect) {
    // Arrange
    const auto& const_optimizer = *optimizer_;
    const double threshold = 0.95;
    const size_t tickets = 1000;
    const double stp_rate = 0.75;

    // Act - should compile without issues
    const auto result = const_optimizer.simulate_threshold(threshold, tickets, stp_rate);

    // Assert
    EXPECT_GT(result.projected_cost_per_resolution, 0.0);
}

} // namespace wfm::simulator

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
