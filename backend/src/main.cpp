// WFM Triage & Unit Economics Engine — Backend Entry Point
// Initializes Crow HTTP server, wires up all six core modules,
// and registers REST API controllers.

#include <crow.h>
#include <nlohmann/json.hpp>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>
#include <stdexcept>
#include <chrono>
#include <ctime>

// ── Core module headers ──────────────────────────────────────────────────────
#include "core/router/confidence_evaluator.h"
#include "core/router/routing_engine.h"
#include "core/simulator/cost_calculator.h"
#include "core/simulator/threshold_optimizer.h"
#include "core/copilot/rag_service.h"
#include "core/copilot/context_generator.h"
#include "core/rlhf/feedback_collector.h"
#include "core/rlhf/golden_dataset_manager.h"
#include "core/analytics/drift_detector.h"
#include "core/analytics/metrics_aggregator.h"
#include "core/deployment/evm_calculator.h"
#include "core/deployment/phase_tracker.h"
#include "core/deployment/milestone_manager.h"

// ── Domain models ────────────────────────────────────────────────────────────
#include "models/cost_parameters.h"
#include "models/sop_document.h"
#include "models/feedback_event.h"

// ── Repository ───────────────────────────────────────────────────────────────
#include "repository/ticket_repository.h"

// ── API controllers ──────────────────────────────────────────────────────────
#include "api/router_controller.h"
#include "api/simulator_controller.h"
#include "api/copilot_controller.h"
#include "api/rlhf_controller.h"
#include "api/analytics_controller.h"
#include "api/deployment_controller.h"

// ── Factory function forward declarations ────────────────────────────────────
// These concrete factory functions are defined in their respective .cpp files.
namespace wfm::analytics {
    std::unique_ptr<DriftDetector> create_drift_detector(
        double baseline_accuracy = 0.90,
        double drift_threshold_percentage = 5.0);
    std::unique_ptr<MetricsAggregator> create_metrics_aggregator(
        std::unique_ptr<DriftDetector> detector);
} // namespace wfm::analytics

namespace wfm::deployment {
    std::unique_ptr<EVMCalculator>    create_evm_calculator();
    std::unique_ptr<PhaseTracker>     create_phase_tracker(
        std::unique_ptr<EVMCalculator> calculator);
    std::unique_ptr<MilestoneManager> create_milestone_manager();
} // namespace wfm::deployment

// ── In-memory stub implementations ──────────────────────────────────────────
// Concrete implementations for abstract interfaces that don't yet have
// a production database-backed implementation.

namespace wfm::copilot {

/// Stub RAGService — returns empty results; replace with vector DB integration.
class InMemoryRAGService final : public RAGService {
public:
    std::optional<SOPDocument> retrieve_sop_by_type(
        const std::string& /*ticket_type*/) const override {
        return std::nullopt;
    }

    std::vector<SOPSearchResult> search_similar_sops(
        const std::string& /*query*/,
        size_t /*top_k*/) const override {
        return {};
    }

    std::vector<SOPSearchResult> get_relevant_sops_for_ticket(
        const std::string& /*ticket_type*/,
        const std::string& /*ticket_content*/,
        size_t /*top_k*/) const override {
        return {};
    }

    bool has_sop_for_type(const std::string& /*ticket_type*/) const noexcept override {
        return false;
    }

    std::vector<std::string> get_available_ticket_types() const override {
        return {};
    }
};

} // namespace wfm::copilot

namespace wfm::rlhf {

/// Stub FeedbackCollector — stores events in memory.
class InMemoryFeedbackCollector final : public FeedbackCollector {
public:
    FeedbackEvent capture_override(
        const std::string& ticket_id,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes) const override {
        FeedbackEvent ev;
        ev.ticket_id           = ticket_id;
        ev.operator_id         = operator_id;
        ev.recommended_action  = recommended_action;
        ev.actual_action       = actual_action;
        ev.category            = category;
        ev.disagreement_notes  = notes;
        events_.push_back(ev);
        return ev;
    }

    FeedbackEvent capture_override_with_context(
        const Ticket& ticket,
        const std::string& operator_id,
        const std::string& recommended_action,
        const std::string& actual_action,
        DisagreementCategory category,
        const std::string& notes) const override {
        return capture_override(ticket.id(), operator_id,
            recommended_action, actual_action, category, notes);
    }

    std::vector<FeedbackEvent> get_feedback_for_ticket(
        const std::string& ticket_id) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& ev : events_) {
            if (ev.ticket_id == ticket_id) result.push_back(ev);
        }
        return result;
    }

    std::vector<FeedbackEvent> get_feedback_by_category(
        DisagreementCategory category) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& ev : events_) {
            if (ev.category == category) result.push_back(ev);
        }
        return result;
    }

    std::vector<FeedbackEvent> get_feedback_by_operator(
        const std::string& operator_id) const override {
        std::vector<FeedbackEvent> result;
        for (const auto& ev : events_) {
            if (ev.operator_id == operator_id) result.push_back(ev);
        }
        return result;
    }

    size_t count_disagreements() const noexcept override {
        size_t n = 0;
        for (const auto& ev : events_) {
            if (ev.is_disagreement()) ++n;
        }
        return n;
    }

    size_t count_disagreements_by_category(
        DisagreementCategory category) const noexcept override {
        size_t n = 0;
        for (const auto& ev : events_) {
            if (ev.category == category && ev.is_disagreement()) ++n;
        }
        return n;
    }

    double calculate_disagreement_rate() const noexcept override {
        if (events_.empty()) return 0.0;
        return static_cast<double>(count_disagreements()) /
               static_cast<double>(events_.size());
    }

    bool validate_feedback(const FeedbackEvent& ev) const noexcept override {
        return !ev.ticket_id.empty() && !ev.operator_id.empty();
    }

private:
    mutable std::vector<FeedbackEvent> events_;
};

} // namespace wfm::rlhf

namespace wfm::repository {

/// Stub TicketRepository — stores tickets in memory.
class InMemoryTicketRepository final : public TicketRepository {
public:
    std::optional<Ticket> find_by_id(const std::string& ticket_id) const override {
        for (const auto& t : tickets_) {
            if (t.id() == ticket_id) return t;
        }
        return std::nullopt;
    }

    void save(const Ticket& ticket) override {
        for (auto& t : tickets_) {
            if (t.id() == ticket.id()) { t = ticket; return; }
        }
        tickets_.push_back(ticket);
    }

    std::vector<Ticket> find_all() const override { return tickets_; }

    void remove(const std::string& ticket_id) override {
        tickets_.erase(
            std::remove_if(tickets_.begin(), tickets_.end(),
                [&ticket_id](const Ticket& t){ return t.id() == ticket_id; }),
            tickets_.end());
    }

    size_t count() const noexcept override { return tickets_.size(); }

private:
    std::vector<Ticket> tickets_;
};

} // namespace wfm::repository

// ── Global mutable threshold ──────────────────────────────────────────────────
// Updated atomically by POST /api/v1/router/threshold.
// Shared between the threshold-update endpoint and the SSE generator.
static std::atomic<double> g_stp_threshold{0.95};

// ── SSE ticket generator ──────────────────────────────────────────────────────
static std::string build_sse_body() {
    static const std::array<const char*, 3> TICKET_TYPES = {
        "safety_flag", "document_verification", "payment_dispute"
    };
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> conf_dist(0.78, 0.99);
    std::uniform_int_distribution<int>     type_dist(0, 2);
    std::uniform_int_distribution<int>     id_dist(1000, 9999);

    std::ostringstream body;
    // Comment heartbeat so the browser knows the connection is alive
    body << ": heartbeat\n\n";

    auto now = std::chrono::system_clock::now();

    for (int i = 0; i < 8; ++i) {
        double confidence   = conf_dist(rng);
        std::string decision = (confidence >= g_stp_threshold.load()) ? "stp" : "human_queue";
        const char* ttype   = TICKET_TYPES[type_dist(rng)];
        int ticket_num      = id_dist(rng);

        // Calculate a unique timestamp for each ticket (staggered by 5-15 seconds)
        auto ticket_time = now - std::chrono::seconds(i * 12 + (rng() % 5));
        std::time_t tt = std::chrono::system_clock::to_time_t(ticket_time);
        std::tm gmt;
        gmtime_r(&tt, &gmt);
        
        std::ostringstream ts;
        ts << std::put_time(&gmt, "%Y-%m-%dT%H:%M:%SZ");

        body << "data: {"
             << "\"id\":\"ticket-" << ticket_num << "\","
             << "\"ticket_type\":\"" << ttype << "\","
             << "\"confidence_score\":" << std::fixed << std::setprecision(4) << confidence << ","
             << "\"routing_decision\":\"" << decision << "\","
             << "\"raw_data\":\"{}\","
             << "\"created_at\":\"" << ts.str() << "\""
             << "}\n\n";
    }
    return body.str();
}

// ── main ─────────────────────────────────────────────────────────────────────
int main() {
    crow::App<crow::CORSHandler> app;

    // ── Configure CORS ───────────────────────────────────────────────────────
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .headers("Content-Type", "Authorization")
        .methods("GET"_method, "POST"_method, "PUT"_method,
                 "PATCH"_method, "DELETE"_method, "OPTIONS"_method)
        .origin("*");

    // ── 1. Live HITL Router ──────────────────────────────────────────────────
    // ConfidenceEvaluator is shared separately for the /evaluate endpoint.
    auto eval_for_engine     = std::make_unique<wfm::router::ConfidenceEvaluator>(0.95);
    auto eval_for_controller = std::make_shared<wfm::router::ConfidenceEvaluator>(0.95);
    auto routing_engine      = std::make_shared<wfm::router::RoutingEngine>(
        std::move(eval_for_engine));

    wfm::api::RouterController::register_routes(app, routing_engine, eval_for_controller);

    // ── 2. Dynamic Margin & Cost Simulator ──────────────────────────────────
    wfm::CostParameters params(
        /*api_cost_per_call=*/  0.002,   // $0.002 per API call
        /*avg_human_rate=*/    25.0,    // $25/hr for human operators
        /*avg_handling_time=*/ 180      // 3 minutes per ticket
    );
    auto cost_calculator    = std::make_shared<wfm::simulator::CostCalculator>(params);
    auto calc_for_optimizer = std::make_unique<wfm::simulator::CostCalculator>(params);
    auto threshold_optimizer = std::make_shared<wfm::simulator::ThresholdOptimizer>(
        std::move(calc_for_optimizer));

    wfm::api::SimulatorController::register_routes(app, cost_calculator, threshold_optimizer);

    // ── 3. Context-Engineered Operator Copilot ──────────────────────────────
    auto rag_service      = std::make_unique<wfm::copilot::InMemoryRAGService>();
    auto context_gen      = std::make_shared<wfm::copilot::ContextGenerator>(
        std::move(rag_service));
    auto ticket_repo      = std::make_shared<wfm::repository::InMemoryTicketRepository>();

    wfm::api::CopilotController::register_routes(app, context_gen, ticket_repo);

    // ── 4. RLHF Capture Gate ────────────────────────────────────────────────
    auto feedback_collector = std::make_unique<wfm::rlhf::InMemoryFeedbackCollector>();
    auto collector_for_ctrl = std::shared_ptr<wfm::rlhf::FeedbackCollector>(
        new wfm::rlhf::InMemoryFeedbackCollector());
    auto dataset_manager    = std::make_shared<wfm::rlhf::GoldenDatasetManager>(
        std::move(feedback_collector));

    wfm::api::RLHFController::register_routes(app, collector_for_ctrl, dataset_manager);

    // ── 5. Time-Lapse Analytics ─────────────────────────────────────────────
    auto drift_detector_owned = wfm::analytics::create_drift_detector(0.90, 5.0);
    auto drift_detector_ctrl  = std::shared_ptr<wfm::analytics::DriftDetector>(
        wfm::analytics::create_drift_detector(0.90, 5.0).release());
    auto metrics_aggregator   = std::shared_ptr<wfm::analytics::MetricsAggregator>(
        wfm::analytics::create_metrics_aggregator(std::move(drift_detector_owned)).release());

    wfm::api::AnalyticsController::register_routes(app, drift_detector_ctrl, metrics_aggregator);

    // ── 6. 5-Phase Deployment Dashboard ─────────────────────────────────────
    auto evm_for_tracker    = wfm::deployment::create_evm_calculator();
    auto evm_for_controller = std::shared_ptr<wfm::deployment::EVMCalculator>(
        wfm::deployment::create_evm_calculator().release());
    auto phase_tracker      = std::shared_ptr<wfm::deployment::PhaseTracker>(
        wfm::deployment::create_phase_tracker(std::move(evm_for_tracker)).release());
    auto milestone_manager  = std::shared_ptr<wfm::deployment::MilestoneManager>(
        wfm::deployment::create_milestone_manager().release());

    wfm::api::DeploymentController::register_routes(
        app, evm_for_controller, phase_tracker, milestone_manager);

    // ── Threshold update ─────────────────────────────────────────────────────
    // POST /api/v1/router/threshold  { "threshold": 0.80 }
    // Updates the global STP threshold used by both the SSE generator and
    // the /evaluate endpoint so the routing engine immediately reflects the
    // value the user set in the Simulator tab.
    CROW_ROUTE(app, "/api/v1/router/threshold")
        .methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                if (!body.contains("threshold")) {
                    crow::json::wvalue err;
                    err["error"]["code"]    = "validation_error";
                    err["error"]["message"] = "Missing field: threshold";
                    return crow::response(400, err);
                }
                double t = body["threshold"].get<double>();
                if (t < 0.0 || t > 1.0) {
                    crow::json::wvalue err;
                    err["error"]["code"]    = "validation_error";
                    err["error"]["message"] = "threshold must be in [0.0, 1.0]";
                    return crow::response(400, err);
                }
                g_stp_threshold.store(t);
                crow::json::wvalue data;
                data["threshold"] = t;
                crow::json::wvalue res;
                res["data"] = std::move(data);
                return crow::response(200, res);
            } catch (const std::exception& e) {
                crow::json::wvalue err;
                err["error"]["code"]    = "internal_error";
                err["error"]["message"] = e.what();
                return crow::response(500, err);
            }
        });

    // ── SSE: Live ticket queue stream ────────────────────────────────────────
    // Crow is a synchronous framework; we return a complete SSE body.
    // EventSource will auto-reconnect after the connection closes, providing
    // a continuous stream of simulated tickets for the demo.
    CROW_ROUTE(app, "/api/v1/stream/queue")
        .methods("GET"_method)
        ([](const crow::request&) {
            crow::response res(200, build_sse_body());
            res.add_header("Content-Type",  "text/event-stream");
            res.add_header("Cache-Control", "no-cache");
            res.add_header("Connection",    "keep-alive");
            res.add_header("X-Accel-Buffering", "no");
            return res;
        });

    // ── Health check ─────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/health")
        .methods("GET"_method)
        ([](const crow::request&) {
            crow::json::wvalue res;
            res["status"] = "ok";
            res["service"] = "wfm-backend";
            return crow::response(200, res);
        });

    // ── Start server ─────────────────────────────────────────────────────────
    app.loglevel(crow::LogLevel::Debug);
    const char* port_env = std::getenv("PORT");
    uint16_t port = port_env ? std::stoi(port_env) : 8080;
    app.bindaddr("0.0.0.0").port(port).multithreaded().run();

    return 0;
}
