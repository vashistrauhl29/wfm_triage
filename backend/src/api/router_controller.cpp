// RouterController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/router_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void RouterController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<router::RoutingEngine> engine,
    std::shared_ptr<router::ConfidenceEvaluator> evaluator
) {
    // POST /api/v1/router/evaluate
    CROW_ROUTE(app, "/api/v1/router/evaluate")
        .methods("POST"_method)
        ([engine, evaluator](const crow::request& req) {
            return evaluate_ticket(req, engine, evaluator);
        });

    // GET /api/v1/router/queue
    CROW_ROUTE(app, "/api/v1/router/queue")
        .methods("GET"_method)
        ([engine](const crow::request&) {
            return get_queue_status(engine);
        });

    // GET /api/v1/router/stats
    CROW_ROUTE(app, "/api/v1/router/stats")
        .methods("GET"_method)
        ([engine](const crow::request&) {
            return get_routing_stats(engine);
        });

    // POST /api/v1/router/clear
    CROW_ROUTE(app, "/api/v1/router/clear")
        .methods("POST"_method)
        ([engine](const crow::request&) {
            return clear_queues(engine);
        });
}

// POST /api/v1/router/evaluate - Evaluate incoming ticket
crow::response RouterController::evaluate_ticket(
    const crow::request& req,
    std::shared_ptr<router::RoutingEngine> engine,
    std::shared_ptr<router::ConfidenceEvaluator> evaluator
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("ticket_id") || !body.contains("confidence_score")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required fields: ticket_id, confidence_score"
            ));
        }

        // Extract ticket data
        std::string ticket_id  = body["ticket_id"];
        double confidence      = body["confidence_score"];
        std::string ticket_type = body.value("ticket_type", "unknown");
        std::string raw_data   = body.value("raw_data", "{}");

        // Create ticket — constructor order: id, confidence, type, raw_data
        Ticket ticket(ticket_id, confidence, ticket_type, raw_data);

        // Evaluate confidence (returns Decision)
        const Decision decision = evaluator->evaluate(ticket);

        // Route ticket through engine
        engine->route(ticket);

        // Build response
        crow::json::wvalue response_data;
        response_data["ticket_id"]         = ticket_id;
        response_data["routing_decision"]  = (decision.routing == RoutingDecision::STP)
                                               ? "stp" : "human_queue";
        response_data["confidence_score"]  = confidence;
        response_data["threshold"]         = evaluator->threshold();

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response("validation_error", e.what()));
    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// GET /api/v1/router/queue - Get current queue status
crow::response RouterController::get_queue_status(
    std::shared_ptr<router::RoutingEngine> engine
) {
    try {
        const auto stats = engine->get_statistics();

        crow::json::wvalue response_data;
        response_data["stp_queue_size"]   = static_cast<int>(stats.stp_count);
        response_data["human_queue_size"] = static_cast<int>(stats.human_queue_count);
        response_data["total_processed"]  = static_cast<int>(stats.total_processed);
        response_data["stp_rate"]         = stats.stp_rate;

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// GET /api/v1/router/stats - Get routing statistics
crow::response RouterController::get_routing_stats(
    std::shared_ptr<router::RoutingEngine> engine
) {
    try {
        const auto stats = engine->get_statistics();

        crow::json::wvalue response_data;
        response_data["total_processed"] = static_cast<int>(stats.total_processed);
        response_data["stp_count"]       = static_cast<int>(stats.stp_count);
        response_data["human_count"]     = static_cast<int>(stats.human_queue_count);
        response_data["stp_rate"]        = stats.stp_rate;

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// POST /api/v1/router/clear - Clear queues
crow::response RouterController::clear_queues(
    std::shared_ptr<router::RoutingEngine> engine
) {
    try {
        engine->clear_queues();

        crow::json::wvalue response_data;
        response_data["message"] = "Queues cleared successfully";

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// Helper: Create success response
crow::json::wvalue RouterController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue RouterController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"]    = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
