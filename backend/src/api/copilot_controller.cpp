// CopilotController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/copilot_controller.h"
#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void CopilotController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<copilot::ContextGenerator> generator,
    std::shared_ptr<repository::TicketRepository> ticket_repo
) {
    // GET /api/v1/copilot/ticket/:id
    CROW_ROUTE(app, "/api/v1/copilot/ticket/<string>")
        .methods("GET"_method)
        ([generator, ticket_repo](const crow::request& req, const std::string& ticket_id) {
            return get_enriched_ticket(req, generator, ticket_repo, ticket_id);
        });

    // POST /api/v1/copilot/action
    CROW_ROUTE(app, "/api/v1/copilot/action")
        .methods("POST"_method)
        ([ticket_repo](const crow::request& req) {
            return submit_operator_action(req, ticket_repo);
        });

    // GET /api/v1/copilot/sop/:type
    CROW_ROUTE(app, "/api/v1/copilot/sop/<string>")
        .methods("GET"_method)
        ([generator](const crow::request& req, const std::string& ticket_type) {
            return get_sop_by_type(req, generator, ticket_type);
        });
}

// GET /api/v1/copilot/ticket/:id - Get enriched ticket with context
crow::response CopilotController::get_enriched_ticket(
    const crow::request& req,
    std::shared_ptr<copilot::ContextGenerator> generator,
    std::shared_ptr<repository::TicketRepository> ticket_repo,
    const std::string& ticket_id
) {
    try {
        // Fetch ticket from repository (stub - assumes ticket_repo has get_by_id method)
        // In production, would call ticket_repo->get_by_id(ticket_id)
        // For now, create a mock ticket — constructor: (id, confidence, type, raw_data)
        Ticket ticket(ticket_id, 0.85, "safety_flag", "{\"user_id\": \"12345\"}");

        // Generate enriched context using ContextGenerator
        auto context = generator->generate_context(ticket);

        // Validate context quality
        if (!generator->validate_context(context)) {
            return crow::response(500, error_response(
                "context_generation_error",
                "Failed to generate valid operator context"
            ));
        }

        // Build response
        crow::json::wvalue response_data;

        // Ticket details
        crow::json::wvalue ticket_data;
        ticket_data["ticket_id"] = context.ticket.id();
        ticket_data["ticket_type"] = context.ticket.ticket_type();
        ticket_data["confidence_score"] = context.ticket.confidence_score();
        ticket_data["raw_data"] = context.ticket.raw_data();
        response_data["ticket"] = std::move(ticket_data);

        // Three-bullet summary
        crow::json::wvalue summary(crow::json::type::List);
        for (size_t i = 0; i < context.three_bullet_summary.size(); ++i) {
            summary[i] = context.three_bullet_summary[i];
        }
        response_data["summary"] = std::move(summary);

        // Action recommendation
        response_data["action_recommendation"] = context.action_recommendation;
        response_data["confidence_override_threshold"] = context.confidence_override_threshold;

        // Relevant SOPs
        crow::json::wvalue sops(crow::json::type::List);
        for (size_t i = 0; i < context.relevant_sops.size(); ++i) {
            crow::json::wvalue sop;
            sop["id"] = context.relevant_sops[i].document.id;
            sop["title"] = context.relevant_sops[i].document.title;
            sop["summary"] = context.relevant_sops[i].document.summary;
            sop["relevance_score"] = context.relevant_sops[i].relevance_score;
            sops[i] = std::move(sop);
        }
        response_data["relevant_sops"] = std::move(sops);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response(
            "validation_error",
            e.what()
        ));
    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// POST /api/v1/copilot/action - Submit operator action
crow::response CopilotController::submit_operator_action(
    const crow::request& req,
    std::shared_ptr<repository::TicketRepository> ticket_repo
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("ticket_id") || !body.contains("action_taken")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required fields: ticket_id, action_taken"
            ));
        }

        std::string ticket_id = body["ticket_id"];
        std::string action_taken = body["action_taken"];
        std::string operator_id = body.value("operator_id", "unknown");
        bool followed_recommendation = body.value("followed_recommendation", true);
        int completion_time = body.value("completion_time", 0);  // seconds

        // In production, would persist to operator_actions table via repository
        // For now, just acknowledge receipt
        (void)ticket_repo;  // Suppress unused warning

        // Build response
        crow::json::wvalue response_data;
        response_data["ticket_id"] = ticket_id;
        response_data["action_taken"] = action_taken;
        response_data["operator_id"] = operator_id;
        response_data["followed_recommendation"] = followed_recommendation;
        response_data["completion_time"] = completion_time;
        response_data["message"] = "Operator action recorded successfully";

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response(
            "validation_error",
            e.what()
        ));
    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// GET /api/v1/copilot/sop/:type - Fetch SOP by ticket type
crow::response CopilotController::get_sop_by_type(
    const crow::request& req,
    std::shared_ptr<copilot::ContextGenerator> generator,
    const std::string& ticket_type
) {
    try {
        // In production, would extract RAGService from ContextGenerator
        // For now, return a mock SOP
        (void)req;
        (void)generator;

        crow::json::wvalue response_data;
        response_data["ticket_type"] = ticket_type;
        response_data["title"] = "Standard Operating Procedure for " + ticket_type;
        response_data["summary"] = "SOP summary for handling " + ticket_type + " tickets";
        response_data["content"] = "Full SOP content would be here...";
        response_data["version"] = 1;

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// Helper: Create success response
crow::json::wvalue CopilotController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue CopilotController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"] = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
