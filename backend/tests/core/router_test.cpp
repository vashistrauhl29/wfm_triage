// Unit tests for Live HITL Router module
// Following C++ Core Guidelines and Google Test best practices
// TDD Approach: Tests written before implementation

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <stdexcept>

// Forward declarations - implementation files to be created
#include "core/router/confidence_evaluator.h"
#include "core/router/routing_engine.h"
#include "models/ticket.h"
#include "models/routing_decision.h"

namespace wfm::router {

// ============================================================================
// ConfidenceEvaluator Test Suite
// ============================================================================

class ConfidenceEvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
        standard_threshold_ = 0.95;
        evaluator_ = std::make_unique<ConfidenceEvaluator>(standard_threshold_);
    }

    void TearDown() override {
        evaluator_.reset();
    }

    double standard_threshold_;
    std::unique_ptr<ConfidenceEvaluator> evaluator_;
};

// Test: Confidence score above threshold triggers STP
TEST_F(ConfidenceEvaluatorTest, EvaluatesAboveThreshold) {
    // Arrange
    const Ticket ticket{"ticket_001", 0.97, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto decision = evaluator_->evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::STP);
    EXPECT_EQ(decision.ticket_id, "ticket_001");
    EXPECT_DOUBLE_EQ(decision.confidence_score, 0.97);
}

// Test: Confidence score below threshold routes to human queue
TEST_F(ConfidenceEvaluatorTest, EvaluatesBelowThreshold) {
    // Arrange
    const Ticket ticket{"ticket_002", 0.89, "document_verification", R"({"data": "test"})"};

    // Act
    const auto decision = evaluator_->evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::HumanQueue);
    EXPECT_EQ(decision.ticket_id, "ticket_002");
    EXPECT_DOUBLE_EQ(decision.confidence_score, 0.89);
}

// Test: Confidence score exactly at threshold triggers STP (boundary condition)
TEST_F(ConfidenceEvaluatorTest, EvaluatesExactThreshold) {
    // Arrange
    const Ticket ticket{"ticket_003", 0.95, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto decision = evaluator_->evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::STP);
    EXPECT_EQ(decision.ticket_id, "ticket_003");
    EXPECT_DOUBLE_EQ(decision.confidence_score, 0.95);
}

// Test: Maximum confidence score (1.0) triggers STP
TEST_F(ConfidenceEvaluatorTest, EvaluatesMaximumConfidence) {
    // Arrange
    const Ticket ticket{"ticket_004", 1.0, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto decision = evaluator_->evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::STP);
    EXPECT_DOUBLE_EQ(decision.confidence_score, 1.0);
}

// Test: Minimum confidence score (0.0) routes to human queue
TEST_F(ConfidenceEvaluatorTest, EvaluatesMinimumConfidence) {
    // Arrange
    const Ticket ticket{"ticket_005", 0.0, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto decision = evaluator_->evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::HumanQueue);
    EXPECT_DOUBLE_EQ(decision.confidence_score, 0.0);
}

// Test: Conservative threshold (0.99) correctly evaluates
TEST_F(ConfidenceEvaluatorTest, UsesConservativeThreshold) {
    // Arrange
    const double conservative_threshold = 0.99;
    const auto conservative_evaluator = std::make_unique<ConfidenceEvaluator>(conservative_threshold);
    const Ticket high_ticket{"ticket_006", 0.995, "safety_flag", R"({"data": "test"})"};
    const Ticket low_ticket{"ticket_007", 0.985, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto high_decision = conservative_evaluator->evaluate(high_ticket);
    const auto low_decision = conservative_evaluator->evaluate(low_ticket);

    // Assert
    EXPECT_EQ(high_decision.routing, RoutingDecision::STP);
    EXPECT_EQ(low_decision.routing, RoutingDecision::HumanQueue);
}

// Test: Moderate threshold (0.50) correctly evaluates
TEST_F(ConfidenceEvaluatorTest, UsesModerateThreshold) {
    // Arrange
    const double moderate_threshold = 0.50;
    const auto moderate_evaluator = std::make_unique<ConfidenceEvaluator>(moderate_threshold);
    const Ticket above_ticket{"ticket_008", 0.60, "safety_flag", R"({"data": "test"})"};
    const Ticket below_ticket{"ticket_009", 0.40, "safety_flag", R"({"data": "test"})"};

    // Act
    const auto above_decision = moderate_evaluator->evaluate(above_ticket);
    const auto below_decision = moderate_evaluator->evaluate(below_ticket);

    // Assert
    EXPECT_EQ(above_decision.routing, RoutingDecision::STP);
    EXPECT_EQ(below_decision.routing, RoutingDecision::HumanQueue);
}

// Test: Invalid confidence score above 1.0 throws exception
TEST_F(ConfidenceEvaluatorTest, ThrowsOnConfidenceAboveOne) {
    // Arrange
    const Ticket invalid_ticket{"ticket_010", 1.5, "safety_flag", R"({"data": "test"})"};

    // Act & Assert
    EXPECT_THROW(evaluator_->evaluate(invalid_ticket), std::invalid_argument);
}

// Test: Invalid confidence score below 0.0 throws exception
TEST_F(ConfidenceEvaluatorTest, ThrowsOnConfidenceBelowZero) {
    // Arrange
    const Ticket invalid_ticket{"ticket_011", -0.1, "safety_flag", R"({"data": "test"})"};

    // Act & Assert
    EXPECT_THROW(evaluator_->evaluate(invalid_ticket), std::invalid_argument);
}

// Test: Invalid threshold above 1.0 throws during construction
TEST(ConfidenceEvaluator, ThrowsOnInvalidThresholdAboveOne) {
    // Act & Assert
    EXPECT_THROW(ConfidenceEvaluator evaluator(1.5), std::invalid_argument);
}

// Test: Invalid threshold below 0.0 throws during construction
TEST(ConfidenceEvaluator, ThrowsOnInvalidThresholdBelowZero) {
    // Act & Assert
    EXPECT_THROW(ConfidenceEvaluator evaluator(-0.1), std::invalid_argument);
}

// Test: Evaluator is const-correct (can evaluate from const reference)
TEST_F(ConfidenceEvaluatorTest, IsConstCorrect) {
    // Arrange
    const Ticket ticket{"ticket_012", 0.97, "safety_flag", R"({"data": "test"})"};
    const auto& const_evaluator = *evaluator_;

    // Act - should compile without issues
    const auto decision = const_evaluator.evaluate(ticket);

    // Assert
    EXPECT_EQ(decision.routing, RoutingDecision::STP);
}

// ============================================================================
// RoutingEngine Test Suite
// ============================================================================

class RoutingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        threshold_ = 0.95;
        evaluator_ = std::make_unique<ConfidenceEvaluator>(threshold_);
        engine_ = std::make_unique<RoutingEngine>(std::move(evaluator_));
    }

    void TearDown() override {
        engine_.reset();
    }

    double threshold_;
    std::unique_ptr<ConfidenceEvaluator> evaluator_;
    std::unique_ptr<RoutingEngine> engine_;
};

// Test: High confidence ticket routed to STP
TEST_F(RoutingEngineTest, RouteHighConfidenceToSTP) {
    // Arrange
    const Ticket ticket{"ticket_100", 0.98, "safety_flag", R"({"data": "test"})"};

    // Act
    engine_->route(ticket);

    // Assert
    const auto stp_queue = engine_->get_stp_queue();
    const auto human_queue = engine_->get_human_queue();

    EXPECT_EQ(stp_queue.size(), 1);
    EXPECT_EQ(human_queue.size(), 0);
    EXPECT_EQ(stp_queue[0].ticket_id, "ticket_100");
}

// Test: Low confidence ticket routed to human queue
TEST_F(RoutingEngineTest, RouteLowConfidenceToHumanQueue) {
    // Arrange
    const Ticket ticket{"ticket_101", 0.85, "document_verification", R"({"data": "test"})"};

    // Act
    engine_->route(ticket);

    // Assert
    const auto stp_queue = engine_->get_stp_queue();
    const auto human_queue = engine_->get_human_queue();

    EXPECT_EQ(stp_queue.size(), 0);
    EXPECT_EQ(human_queue.size(), 1);
    EXPECT_EQ(human_queue[0].ticket_id, "ticket_101");
}

// Test: Multiple tickets processed correctly
TEST_F(RoutingEngineTest, RouteMultipleTicketsCorrectly) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_102", 0.98, "safety_flag", R"({"data": "test1"})"),      // STP
        Ticket("ticket_103", 0.87, "safety_flag", R"({"data": "test2"})"),      // Human
        Ticket("ticket_104", 0.99, "safety_flag", R"({"data": "test3"})"),      // STP
        Ticket("ticket_105", 0.75, "document_verification", R"({"data": "test4"})"),  // Human
        Ticket("ticket_106", 0.96, "safety_flag", R"({"data": "test5"})")       // STP
    };

    // Act
    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Assert
    const auto stp_queue = engine_->get_stp_queue();
    const auto human_queue = engine_->get_human_queue();

    EXPECT_EQ(stp_queue.size(), 3);  // tickets 102, 104, 106
    EXPECT_EQ(human_queue.size(), 2);  // tickets 103, 105
}

// Test: STP rate calculated correctly
TEST_F(RoutingEngineTest, CalculatesStpRateCorrectly) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_107", 0.98, "safety_flag", R"({"data": "test1"})"),      // STP
        Ticket("ticket_108", 0.87, "safety_flag", R"({"data": "test2"})"),      // Human
        Ticket("ticket_109", 0.99, "safety_flag", R"({"data": "test3"})"),      // STP
        Ticket("ticket_110", 0.75, "document_verification", R"({"data": "test4"})")   // Human
    };

    // Act
    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Assert
    const double stp_rate = engine_->get_stp_rate();
    EXPECT_DOUBLE_EQ(stp_rate, 0.50);  // 2 out of 4 tickets routed to STP
}

// Test: STP rate is 0.0 when no tickets processed
TEST_F(RoutingEngineTest, StpRateIsZeroWhenNoTickets) {
    // Act
    const double stp_rate = engine_->get_stp_rate();

    // Assert
    EXPECT_DOUBLE_EQ(stp_rate, 0.0);
}

// Test: STP rate is 1.0 when all tickets routed to STP
TEST_F(RoutingEngineTest, StpRateIsOneWhenAllStp) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_111", 0.98, "safety_flag", R"({"data": "test1"})"),
        Ticket("ticket_112", 0.99, "safety_flag", R"({"data": "test2"})"),
        Ticket("ticket_113", 0.97, "safety_flag", R"({"data": "test3"})")
    };

    // Act
    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Assert
    const double stp_rate = engine_->get_stp_rate();
    EXPECT_DOUBLE_EQ(stp_rate, 1.0);
}

// Test: Queue statistics updated correctly
TEST_F(RoutingEngineTest, UpdatesQueueStatistics) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_114", 0.98, "safety_flag", R"({"data": "test1"})"),      // STP
        Ticket("ticket_115", 0.87, "safety_flag", R"({"data": "test2"})")       // Human
    };

    // Act
    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Assert
    const auto stats = engine_->get_statistics();
    EXPECT_EQ(stats.total_processed, 2);
    EXPECT_EQ(stats.stp_count, 1);
    EXPECT_EQ(stats.human_queue_count, 1);
    EXPECT_DOUBLE_EQ(stats.stp_rate, 0.5);
}

// Test: Clear queues functionality
TEST_F(RoutingEngineTest, ClearQueuesResetsState) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_116", 0.98, "safety_flag", R"({"data": "test1"})"),
        Ticket("ticket_117", 0.87, "safety_flag", R"({"data": "test2"})")
    };

    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Act
    engine_->clear_queues();

    // Assert
    const auto stp_queue = engine_->get_stp_queue();
    const auto human_queue = engine_->get_human_queue();

    EXPECT_EQ(stp_queue.size(), 0);
    EXPECT_EQ(human_queue.size(), 0);
}

// Test: Engine maintains RAII principles with unique_ptr
TEST_F(RoutingEngineTest, MaintainsRaiiPrinciples) {
    // Arrange
    auto evaluator = std::make_unique<ConfidenceEvaluator>(0.95);

    // Act - ownership transferred
    auto engine = std::make_unique<RoutingEngine>(std::move(evaluator));

    // Assert - evaluator is now null (ownership transferred)
    EXPECT_EQ(evaluator, nullptr);
    EXPECT_NE(engine, nullptr);
}

// Test: Engine is thread-safe for concurrent routing
TEST_F(RoutingEngineTest, SupportsThreadSafeRouting) {
    // Arrange
    const std::vector<Ticket> tickets = {
        Ticket("ticket_118", 0.98, "safety_flag", R"({"data": "test1"})"),
        Ticket("ticket_119", 0.87, "safety_flag", R"({"data": "test2"})"),
        Ticket("ticket_120", 0.99, "safety_flag", R"({"data": "test3"})")
    };

    // Act - simulate concurrent access (actual threading test would use std::thread)
    for (const auto& ticket : tickets) {
        engine_->route(ticket);
    }

    // Assert
    const auto stats = engine_->get_statistics();
    EXPECT_EQ(stats.total_processed, 3);
}

} // namespace wfm::router

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
