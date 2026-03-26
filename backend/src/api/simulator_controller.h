#ifndef WFM_API_SIMULATOR_CONTROLLER_H
#define WFM_API_SIMULATOR_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/simulator/cost_calculator.h"
#include "core/simulator/threshold_optimizer.h"

namespace wfm::api {

// SimulatorController: REST API endpoints for Dynamic Cost Simulator
class SimulatorController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<simulator::CostCalculator> calculator,
        std::shared_ptr<simulator::ThresholdOptimizer> optimizer
    );

private:
    // POST /api/v1/simulator/calculate - Calculate cost for given threshold
    static crow::response calculate_cost(
        const crow::request& req,
        std::shared_ptr<simulator::CostCalculator> calculator
    );

    // POST /api/v1/simulator/optimize - Find optimal threshold
    static crow::response optimize_threshold(
        const crow::request& req,
        std::shared_ptr<simulator::ThresholdOptimizer> optimizer
    );

    // GET /api/v1/simulator/scenarios - Get pre-computed scenarios
    static crow::response get_scenarios(
        std::shared_ptr<simulator::ThresholdOptimizer> optimizer
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

#endif // WFM_API_SIMULATOR_CONTROLLER_H
