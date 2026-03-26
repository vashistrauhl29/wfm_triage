#ifndef WFM_API_COPILOT_CONTROLLER_H
#define WFM_API_COPILOT_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/copilot/context_generator.h"
#include "repository/ticket_repository.h"

namespace wfm::api {

// CopilotController: REST API endpoints for Context-Engineered Operator View
// Following REST conventions and API design best practices
class CopilotController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<copilot::ContextGenerator> generator,
        std::shared_ptr<repository::TicketRepository> ticket_repo
    );

private:
    // GET /api/v1/copilot/ticket/:id - Get enriched ticket with context
    static crow::response get_enriched_ticket(
        const crow::request& req,
        std::shared_ptr<copilot::ContextGenerator> generator,
        std::shared_ptr<repository::TicketRepository> ticket_repo,
        const std::string& ticket_id
    );

    // POST /api/v1/copilot/action - Submit operator action
    static crow::response submit_operator_action(
        const crow::request& req,
        std::shared_ptr<repository::TicketRepository> ticket_repo
    );

    // GET /api/v1/copilot/sop/:type - Fetch SOP by ticket type
    static crow::response get_sop_by_type(
        const crow::request& req,
        std::shared_ptr<copilot::ContextGenerator> generator,
        const std::string& ticket_type
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

#endif // WFM_API_COPILOT_CONTROLLER_H
