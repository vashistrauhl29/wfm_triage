// Unit tests for Context-Engineered Operator View module
// Following C++ Core Guidelines and Google Test best practices
// TDD Approach: Tests written before implementation

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <optional>

// Forward declarations - implementation files to be created
#include "core/copilot/rag_service.h"
#include "core/copilot/context_generator.h"
#include "models/ticket.h"
#include "models/sop_document.h"
#include "models/operator_context.h"

namespace wfm::copilot {

// ============================================================================
// Mock RAGService for Testing
// ============================================================================

class MockRAGService : public RAGService {
public:
    MockRAGService() {
        // Populate with test SOPs
        sop_safety_ = SOPDocument(
            "sop_001",
            "safety_flag",
            "Safety Flag Response Protocol",
            "Step 1: Verify safety concern legitimacy. "
            "Step 2: Escalate to safety team immediately. "
            "Step 3: Document incident details."
        );
        sop_safety_.summary = "Verify, escalate, document safety concerns";
        sop_safety_.embedding = std::vector<double>(1536, 0.5);  // Mock embedding

        sop_document_ = SOPDocument(
            "sop_002",
            "document_verification",
            "Document Verification Protocol",
            "Step 1: Check document authenticity markers. "
            "Step 2: Verify against database records. "
            "Step 3: Flag discrepancies."
        );
        sop_document_.summary = "Authenticate, verify, flag documents";
        sop_document_.embedding = std::vector<double>(1536, 0.7);

        sop_generic_ = SOPDocument(
            "sop_003",
            "generic",
            "Generic Ticket Handling",
            "Standard operating procedure for general tickets."
        );
        sop_generic_.summary = "Standard ticket handling";
        sop_generic_.embedding = std::vector<double>(1536, 0.3);
    }

    std::optional<SOPDocument> retrieve_sop_by_type(
        const std::string& ticket_type
    ) const override {
        if (ticket_type == "safety_flag") {
            return sop_safety_;
        } else if (ticket_type == "document_verification") {
            return sop_document_;
        } else if (ticket_type == "generic") {
            return sop_generic_;
        }
        return std::nullopt;
    }

    std::vector<SOPSearchResult> search_similar_sops(
        const std::string& query,
        size_t top_k
    ) const override {
        std::vector<SOPSearchResult> results;

        if (query.find("safety") != std::string::npos) {
            results.emplace_back(sop_safety_, 0.95);
        }
        if (query.find("document") != std::string::npos) {
            results.emplace_back(sop_document_, 0.90);
        }
        if (results.empty() || query.find("generic") != std::string::npos) {
            results.emplace_back(sop_generic_, 0.60);
        }

        // Trim to top_k
        if (results.size() > top_k) {
            results.resize(top_k);
        }

        return results;
    }

    std::vector<SOPSearchResult> get_relevant_sops_for_ticket(
        const std::string& ticket_type,
        const std::string& ticket_content,
        size_t top_k
    ) const override {
        std::vector<SOPSearchResult> results;

        // First try exact type match
        auto exact_match = retrieve_sop_by_type(ticket_type);
        if (exact_match.has_value()) {
            results.emplace_back(exact_match.value(), 1.0);
        }

        // Then add content-based matches
        auto similar = search_similar_sops(ticket_content, top_k);
        for (const auto& result : similar) {
            // Avoid duplicates
            bool is_duplicate = false;
            for (const auto& existing : results) {
                if (existing.document.id == result.document.id) {
                    is_duplicate = true;
                    break;
                }
            }
            if (!is_duplicate && results.size() < top_k) {
                results.push_back(result);
            }
        }

        return results;
    }

    bool has_sop_for_type(const std::string& ticket_type) const noexcept override {
        return ticket_type == "safety_flag" ||
               ticket_type == "document_verification" ||
               ticket_type == "generic";
    }

    std::vector<std::string> get_available_ticket_types() const override {
        return {"safety_flag", "document_verification", "generic"};
    }

private:
    SOPDocument sop_safety_;
    SOPDocument sop_document_;
    SOPDocument sop_generic_;
};

// ============================================================================
// RAGService Test Suite
// ============================================================================

class RAGServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        rag_service_ = std::make_unique<MockRAGService>();
    }

    void TearDown() override {
        rag_service_.reset();
    }

    std::unique_ptr<RAGService> rag_service_;
};

// Test: Retrieve SOP by exact ticket type match
TEST_F(RAGServiceTest, RetrievesSopByExactType) {
    // Arrange
    const std::string ticket_type = "safety_flag";

    // Act
    const auto sop = rag_service_->retrieve_sop_by_type(ticket_type);

    // Assert
    ASSERT_TRUE(sop.has_value());
    EXPECT_EQ(sop->ticket_type, "safety_flag");
    EXPECT_EQ(sop->id, "sop_001");
    EXPECT_FALSE(sop->content.empty());
}

// Test: Returns nullopt for non-existent ticket type
TEST_F(RAGServiceTest, ReturnsNulloptForNonExistentType) {
    // Arrange
    const std::string invalid_type = "nonexistent_type";

    // Act
    const auto sop = rag_service_->retrieve_sop_by_type(invalid_type);

    // Assert
    EXPECT_FALSE(sop.has_value());
}

// Test: Retrieves multiple SOPs for different types
TEST_F(RAGServiceTest, RetrievesMultipleSopTypes) {
    // Arrange
    const std::vector<std::string> types = {
        "safety_flag",
        "document_verification",
        "generic"
    };

    // Act & Assert
    for (const auto& type : types) {
        const auto sop = rag_service_->retrieve_sop_by_type(type);
        ASSERT_TRUE(sop.has_value());
        EXPECT_EQ(sop->ticket_type, type);
    }
}

// Test: Search finds similar SOPs by query
TEST_F(RAGServiceTest, SearchesSimilarSopsByQuery) {
    // Arrange
    const std::string query = "safety concern escalation";
    const size_t top_k = 3;

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert
    EXPECT_FALSE(results.empty());
    EXPECT_LE(results.size(), top_k);

    // Results should be ordered by relevance score (highest first)
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_GE(results[i-1].relevance_score, results[i].relevance_score);
    }
}

// Test: Search limits results to top_k
TEST_F(RAGServiceTest, SearchLimitsResultsToTopK) {
    // Arrange
    const std::string query = "document safety generic";
    const size_t top_k = 2;

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert
    EXPECT_LE(results.size(), top_k);
}

// Test: Search returns empty for non-matching query
TEST_F(RAGServiceTest, SearchHandlesNonMatchingQuery) {
    // Arrange
    const std::string query = "completely unrelated topic xyz";
    const size_t top_k = 5;

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert - Mock returns generic SOP as fallback
    EXPECT_FALSE(results.empty());
}

// Test: Get relevant SOPs combines type and content matching
TEST_F(RAGServiceTest, GetsRelevantSopsForTicket) {
    // Arrange
    const std::string ticket_type = "safety_flag";
    const std::string ticket_content = "User reported safety issue with document";
    const size_t top_k = 3;

    // Act
    const auto results = rag_service_->get_relevant_sops_for_ticket(
        ticket_type,
        ticket_content,
        top_k
    );

    // Assert
    EXPECT_FALSE(results.empty());
    EXPECT_LE(results.size(), top_k);

    // First result should be exact type match with highest score
    EXPECT_EQ(results[0].document.ticket_type, ticket_type);
    EXPECT_EQ(results[0].relevance_score, 1.0);
}

// Test: Check if SOP exists for ticket type
TEST_F(RAGServiceTest, ChecksSopExistsForType) {
    // Arrange & Act & Assert
    EXPECT_TRUE(rag_service_->has_sop_for_type("safety_flag"));
    EXPECT_TRUE(rag_service_->has_sop_for_type("document_verification"));
    EXPECT_FALSE(rag_service_->has_sop_for_type("nonexistent"));
}

// Test: Get all available ticket types
TEST_F(RAGServiceTest, GetsAvailableTicketTypes) {
    // Act
    const auto types = rag_service_->get_available_ticket_types();

    // Assert
    EXPECT_FALSE(types.empty());
    EXPECT_GE(types.size(), 2);  // At least safety_flag and document_verification

    // Verify expected types are present
    bool has_safety = false;
    bool has_document = false;
    for (const auto& type : types) {
        if (type == "safety_flag") has_safety = true;
        if (type == "document_verification") has_document = true;
    }
    EXPECT_TRUE(has_safety);
    EXPECT_TRUE(has_document);
}

// Test: Relevance scores are in valid range [0.0, 1.0]
TEST_F(RAGServiceTest, RelevanceScoresInValidRange) {
    // Arrange
    const std::string query = "safety document verification";
    const size_t top_k = 5;

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert
    for (const auto& result : results) {
        EXPECT_GE(result.relevance_score, 0.0);
        EXPECT_LE(result.relevance_score, 1.0);
    }
}

// Test: Empty query handles gracefully
TEST_F(RAGServiceTest, HandlesEmptyQuery) {
    // Arrange
    const std::string empty_query = "";
    const size_t top_k = 3;

    // Act
    const auto results = rag_service_->search_similar_sops(empty_query, top_k);

    // Assert - Should not crash, may return generic results
    EXPECT_TRUE(results.empty() || !results[0].document.content.empty());
}

// Test: Zero top_k returns empty results
TEST_F(RAGServiceTest, HandlesZeroTopK) {
    // Arrange
    const std::string query = "safety";
    const size_t top_k = 0;

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert
    EXPECT_TRUE(results.empty());
}

// Test: Large top_k doesn't exceed available SOPs
TEST_F(RAGServiceTest, HandlesLargeTopK) {
    // Arrange
    const std::string query = "safety document generic";
    const size_t top_k = 1000;  // Very large k

    // Act
    const auto results = rag_service_->search_similar_sops(query, top_k);

    // Assert - Should return all available, not crash
    EXPECT_FALSE(results.empty());
    EXPECT_LE(results.size(), 10);  // Reasonable upper bound
}

// Test: Const-correctness
TEST_F(RAGServiceTest, IsConstCorrect) {
    // Arrange
    const auto& const_service = *rag_service_;
    const std::string query = "safety";

    // Act - should compile without issues
    const auto results = const_service.search_similar_sops(query, 3);
    const auto has_sop = const_service.has_sop_for_type("safety_flag");

    // Assert
    EXPECT_FALSE(results.empty());
    EXPECT_TRUE(has_sop);
}

// ============================================================================
// ContextGenerator Test Suite
// ============================================================================

class ContextGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto mock_rag = std::make_unique<MockRAGService>();
        generator_ = std::make_unique<ContextGenerator>(std::move(mock_rag));
    }

    void TearDown() override {
        generator_.reset();
    }

    // Helper to create test ticket
    Ticket create_test_ticket() const {
        return Ticket(
            "ticket_001",
            0.87,
            "safety_flag",
            R"({"user_id": "user_123", "issue": "safety concern reported"})"
        );
    }

    std::unique_ptr<ContextGenerator> generator_;
};

// Test: Generate complete operator context
TEST_F(ContextGeneratorTest, GeneratesCompleteContext) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    EXPECT_EQ(context.ticket.id(), create_test_ticket().id());
    EXPECT_FALSE(context.three_bullet_summary.empty());
    EXPECT_FALSE(context.relevant_sops.empty());
}

// Test: Three-bullet summary has exactly 3 bullets
TEST_F(ContextGeneratorTest, GeneratesExactlyThreeBullets) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    EXPECT_EQ(context.three_bullet_summary.size(), 3);
    EXPECT_TRUE(context.has_valid_summary());
}

// Test: Each bullet is non-empty
TEST_F(ContextGeneratorTest, BulletsAreNonEmpty) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    for (const auto& bullet : context.three_bullet_summary) {
        EXPECT_FALSE(bullet.empty());
        EXPECT_GT(bullet.length(), 5);  // Meaningful content
    }
}

// Test: Bullets are concise (reasonable max length)
TEST_F(ContextGeneratorTest, BulletsAreConcise) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    for (const auto& bullet : context.three_bullet_summary) {
        EXPECT_LE(bullet.length(), 150);  // Concise, not verbose
    }
}

// Test: Generate summary integrates SOP data
TEST_F(ContextGeneratorTest, IntegratesSopDataIntoSummary) {
    // Arrange
    const std::vector<SOPSearchResult> sops = {
        {SOPDocument("sop_001", "safety_flag", "Safety Protocol",
                     "Verify, escalate, document"), 0.95}
    };

    // Act
    const auto summary = generator_->generate_three_bullet_summary(create_test_ticket(), sops);

    // Assert
    EXPECT_EQ(summary.size(), 3);

    // Summary should contain context from ticket and SOP
    const std::string combined = summary[0] + summary[1] + summary[2];
    // Should mention ticket details or SOP guidance
    EXPECT_GT(combined.length(), 20);
}

// Test: Generate action recommendation
TEST_F(ContextGeneratorTest, GeneratesActionRecommendation) {
    // Arrange
    const std::vector<SOPSearchResult> sops = {
        {SOPDocument("sop_001", "safety_flag", "Safety Protocol",
                     "Escalate to safety team immediately"), 0.95}
    };

    // Act
    const auto recommendation = generator_->generate_action_recommendation(
        create_test_ticket(),
        sops
    );

    // Assert
    EXPECT_FALSE(recommendation.empty());
    EXPECT_GT(recommendation.length(), 10);
}

// Test: Validate valid context
TEST_F(ContextGeneratorTest, ValidatesValidContext) {
    // Arrange
    const auto context = generator_->generate_context(create_test_ticket());

    // Act
    const bool is_valid = generator_->validate_context(context);

    // Assert
    EXPECT_TRUE(is_valid);
}

// Test: Invalidate context with wrong number of bullets
TEST_F(ContextGeneratorTest, InvalidatesContextWithWrongBulletCount) {
    // Arrange
    OperatorContext invalid_context(create_test_ticket());
    invalid_context.three_bullet_summary = {"Bullet 1", "Bullet 2"};  // Only 2

    // Act
    const bool is_valid = generator_->validate_context(invalid_context);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Invalidate context with empty bullets
TEST_F(ContextGeneratorTest, InvalidatesContextWithEmptyBullets) {
    // Arrange
    OperatorContext invalid_context(create_test_ticket());
    invalid_context.three_bullet_summary = {"Bullet 1", "", "Bullet 3"};

    // Act
    const bool is_valid = generator_->validate_context(invalid_context);

    // Assert
    EXPECT_FALSE(is_valid);
}

// Test: Handle ticket with minimal data
TEST_F(ContextGeneratorTest, HandlesTicketWithMinimalData) {
    // Arrange
    const Ticket minimal_ticket("minimal_001", 0.5, "generic", "{}");

    // Act
    const auto context = generator_->generate_context(minimal_ticket);

    // Assert - Should not crash, generate generic context
    EXPECT_EQ(context.three_bullet_summary.size(), 3);
    EXPECT_FALSE(context.relevant_sops.empty());
}

// Test: Handle different ticket types
TEST_F(ContextGeneratorTest, HandlesDifferentTicketTypes) {
    // Arrange
    const std::vector<std::string> types = {
        "safety_flag",
        "document_verification",
        "generic"
    };

    // Act & Assert
    for (const auto& type : types) {
        const Ticket ticket("ticket_" + type, 0.8, type, "{}");
        const auto context = generator_->generate_context(ticket);

        EXPECT_EQ(context.three_bullet_summary.size(), 3);
        EXPECT_EQ(context.ticket.ticket_type(), type);
    }
}

// Test: Context includes relevant SOPs
TEST_F(ContextGeneratorTest, ContextIncludesRelevantSops) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    EXPECT_FALSE(context.relevant_sops.empty());
    EXPECT_LE(context.relevant_sops.size(), 5);  // Top-K constraint

    // SOPs should be relevant to ticket type
    bool found_relevant = false;
    for (const auto& sop_result : context.relevant_sops) {
        if (sop_result.document.ticket_type == create_test_ticket().ticket_type()) {
            found_relevant = true;
            break;
        }
    }
    EXPECT_TRUE(found_relevant);
}

// Test: Relevant SOPs are ordered by relevance
TEST_F(ContextGeneratorTest, SopsOrderedByRelevance) {
    // Act
    const auto context = generator_->generate_context(create_test_ticket());

    // Assert
    ASSERT_GT(context.relevant_sops.size(), 1);

    // Check descending relevance scores
    for (size_t i = 1; i < context.relevant_sops.size(); ++i) {
        EXPECT_GE(
            context.relevant_sops[i-1].relevance_score,
            context.relevant_sops[i].relevance_score
        );
    }
}

// Test: Generate summary with no SOPs available
TEST_F(ContextGeneratorTest, GeneratesSummaryWithNoSops) {
    // Arrange
    const std::vector<SOPSearchResult> empty_sops;

    // Act
    const auto summary = generator_->generate_three_bullet_summary(
        create_test_ticket(),
        empty_sops
    );

    // Assert - Should still generate 3 bullets from ticket data
    EXPECT_EQ(summary.size(), 3);
    for (const auto& bullet : summary) {
        EXPECT_FALSE(bullet.empty());
    }
}

// Test: RAII principles with unique_ptr
TEST_F(ContextGeneratorTest, MaintainsRaiiPrinciples) {
    // Arrange
    auto rag_service = std::make_unique<MockRAGService>();

    // Act - ownership transferred
    auto generator = std::make_unique<ContextGenerator>(std::move(rag_service));

    // Assert - rag_service is now null (ownership transferred)
    EXPECT_EQ(rag_service, nullptr);
    EXPECT_NE(generator, nullptr);
}

// Test: Const-correctness
TEST_F(ContextGeneratorTest, IsConstCorrect) {
    // Arrange
    const auto& const_generator = *generator_;

    // Act - should compile without issues
    const auto context = const_generator.generate_context(create_test_ticket());

    // Assert
    EXPECT_EQ(context.three_bullet_summary.size(), 3);
}

// Test: Multiple tickets generate different contexts
TEST_F(ContextGeneratorTest, GeneratesDifferentContextsForDifferentTickets) {
    // Arrange
    const Ticket ticket1("t1", 0.9, "safety_flag", R"({"issue": "safety"})");
    const Ticket ticket2("t2", 0.7, "document_verification", R"({"issue": "doc"})");

    // Act
    const auto context1 = generator_->generate_context(ticket1);
    const auto context2 = generator_->generate_context(ticket2);

    // Assert - Contexts should be different
    EXPECT_NE(context1.ticket.id(), context2.ticket.id());
    EXPECT_NE(context1.ticket.ticket_type(), context2.ticket.ticket_type());
}

// Test: High confidence tickets get appropriate context
TEST_F(ContextGeneratorTest, HighConfidenceTicketsGetContext) {
    // Arrange
    const Ticket high_conf_ticket("high_001", 0.99, "safety_flag", "{}");

    // Act
    const auto context = generator_->generate_context(high_conf_ticket);

    // Assert
    EXPECT_EQ(context.three_bullet_summary.size(), 3);
    EXPECT_DOUBLE_EQ(context.ticket.confidence_score(), 0.99);
}

// Test: Low confidence tickets get appropriate context
TEST_F(ContextGeneratorTest, LowConfidenceTicketsGetContext) {
    // Arrange
    const Ticket low_conf_ticket("low_001", 0.60, "safety_flag", "{}");

    // Act
    const auto context = generator_->generate_context(low_conf_ticket);

    // Assert
    EXPECT_EQ(context.three_bullet_summary.size(), 3);
    EXPECT_DOUBLE_EQ(context.ticket.confidence_score(), 0.60);
}

} // namespace wfm::copilot

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
