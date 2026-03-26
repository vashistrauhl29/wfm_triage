#ifndef WFM_API_ROUTER_CONTROLLER_H
#define WFM_API_ROUTER_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/router/routing_engine.h"
#include "core/router/confidence_evaluator.h"

namespace wfm::api {

// RouterController: REST API endpoints for Live HITL Router
// Following REST conventions and API design best practices
class RouterController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<router::RoutingEngine> engine,
        std::shared_ptr<router::ConfidenceEvaluator> evaluator
    );

private:
    // POST /api/v1/router/evaluate - Evaluate incoming ticket
    static crow::response evaluate_ticket(
        const crow::request& req,
        std::shared_ptr<router::RoutingEngine> engine,
        std::shared_ptr<router::ConfidenceEvaluator> evaluator
    );

    // GET /api/v1/router/queue - Get current queue status
    static crow::response get_queue_status(
        std::shared_ptr<router::RoutingEngine> engine
    );

    // GET /api/v1/router/stats - Get routing statistics
    static crow::response get_routing_stats(
        std::shared_ptr<router::RoutingEngine> engine
    );

    // POST /api/v1/router/clear - Clear queues
    static crow::response clear_queues(
        std::shared_ptr<router::RoutingEngine> engine
    );

    // Helper: Create success response
    static crow::json::wvalue success_response(
        crow::json::wvalue data
    );

    // Helper: Create error response
    static crow::json::wvalue error_response(
        const std::string& code,
        const std::string& message
    );
};

} // namespace wfm::api

#endif // WFM_API_ROUTER_CONTROLLER_H
