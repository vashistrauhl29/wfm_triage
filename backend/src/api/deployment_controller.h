#ifndef WFM_API_DEPLOYMENT_CONTROLLER_H
#define WFM_API_DEPLOYMENT_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/deployment/evm_calculator.h"
#include "core/deployment/phase_tracker.h"
#include "core/deployment/milestone_manager.h"

namespace wfm::api {

// DeploymentController: REST API endpoints for 5-Phase Deployment Dashboard
// Following REST conventions and API design best practices
class DeploymentController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<deployment::EVMCalculator> evm_calculator,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker,
        std::shared_ptr<deployment::MilestoneManager> milestone_manager
    );

private:
    // GET /api/v1/deployment/phases - Get all deployment phases
    static crow::response get_all_phases(
        const crow::request& req,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker
    );

    // GET /api/v1/deployment/phase/:id - Get specific phase details
    static crow::response get_phase_details(
        const crow::request& req,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker,
        std::shared_ptr<deployment::MilestoneManager> milestone_manager,
        const std::string& phase_id
    );

    // POST /api/v1/deployment/milestone - Complete a milestone
    static crow::response complete_milestone(
        const crow::request& req,
        std::shared_ptr<deployment::MilestoneManager> milestone_manager
    );

    // GET /api/v1/deployment/evm - Get EVM metrics
    static crow::response get_evm_metrics(
        const crow::request& req,
        std::shared_ptr<deployment::EVMCalculator> evm_calculator,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker,
        std::shared_ptr<deployment::MilestoneManager> milestone_manager
    );

    // GET /api/v1/deployment/health - Get deployment health score
    static crow::response get_deployment_health(
        const crow::request& req,
        std::shared_ptr<deployment::EVMCalculator> evm_calculator,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker,
        std::shared_ptr<deployment::MilestoneManager> milestone_manager
    );

    // PATCH /api/v1/deployment/phase/:id - Update phase status
    static crow::response update_phase_status(
        const crow::request& req,
        std::shared_ptr<deployment::PhaseTracker> phase_tracker,
        const std::string& phase_id
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

#endif // WFM_API_DEPLOYMENT_CONTROLLER_H
