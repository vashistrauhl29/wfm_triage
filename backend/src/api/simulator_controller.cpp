// SimulatorController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/simulator_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void SimulatorController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<simulator::CostCalculator> calculator,
    std::shared_ptr<simulator::ThresholdOptimizer> optimizer
) {
    // POST /api/v1/simulator/calculate
    CROW_ROUTE(app, "/api/v1/simulator/calculate")
        .methods("POST"_method)
        ([calculator](const crow::request& req) {
            return calculate_cost(req, calculator);
        });

    // POST /api/v1/simulator/optimize
    CROW_ROUTE(app, "/api/v1/simulator/optimize")
        .methods("POST"_method)
        ([optimizer](const crow::request& req) {
            return optimize_threshold(req, optimizer);
        });

    // GET /api/v1/simulator/scenarios
    CROW_ROUTE(app, "/api/v1/simulator/scenarios")
        .methods("GET"_method)
        ([optimizer](const crow::request&) {
            return get_scenarios(optimizer);
        });
}

// POST /api/v1/simulator/calculate - Calculate cost for given threshold
crow::response SimulatorController::calculate_cost(
    const crow::request& req,
    std::shared_ptr<simulator::CostCalculator> /*calculator*/
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("threshold") || !body.contains("total_tickets")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required fields: threshold, total_tickets"
            ));
        }

        double threshold     = body["threshold"];
        size_t total_tickets = body["total_tickets"];

        // Inverse STP model: higher threshold = stricter = fewer auto-resolved tickets.
        // Formula: STP% = max(0, 1 - (threshold - 0.80) * 4)
        //   threshold 0.80 -> 100% STP  (lenient — routes everything automatically)
        //   threshold 0.90 ->  60% STP
        //   threshold 0.95 ->  40% STP
        //   threshold 1.00 ->  20% STP  (strict  — almost everything goes to humans)
        double projected_stp = std::max(0.0, std::min(1.0, 1.0 - ((threshold - 0.80) * 4.0)));

        double api_tickets   = static_cast<double>(total_tickets) * projected_stp;
        double human_tickets = static_cast<double>(total_tickets) * (1.0 - projected_stp);

        // Simple per-ticket costs: $0.05 per API call, $3.00 per human review
        double api_cost   = api_tickets   * 0.05;
        double human_cost = human_tickets * 3.00;
        double total_cost = api_cost + human_cost;
        double cost_per_resolution = total_tickets > 0
            ? total_cost / static_cast<double>(total_tickets)
            : 0.0;

        // Build response
        crow::json::wvalue response_data;
        response_data["api_cost"]            = api_cost;
        response_data["human_cost"]          = human_cost;
        response_data["total_cost"]          = total_cost;
        response_data["cost_per_resolution"] = cost_per_resolution;
        response_data["stp_rate"]            = projected_stp;
        response_data["total_tickets"]       = static_cast<int>(total_tickets);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response("validation_error", e.what()));
    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// POST /api/v1/simulator/optimize - Find optimal threshold
crow::response SimulatorController::optimize_threshold(
    const crow::request& req,
    std::shared_ptr<simulator::ThresholdOptimizer> optimizer
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        size_t total_tickets    = body.value("total_tickets",  static_cast<size_t>(10000));
        double min_threshold    = body.value("min_threshold",  0.90);
        double max_threshold    = body.value("max_threshold",  0.99);
        double max_cost         = body.value("max_cost",       1000000.0);

        // Find optimal threshold — correct signature:
        // find_optimal_threshold(min, max, max_acceptable_cost, total_tickets)
        auto result = optimizer->find_optimal_threshold(
            min_threshold, max_threshold, max_cost, total_tickets);

        // Build response
        crow::json::wvalue response_data;
        response_data["optimal_threshold"]  = result.threshold;
        response_data["projected_stp_rate"] = result.projected_stp_rate;
        response_data["cost_per_resolution"]= result.projected_cost_per_resolution;
        response_data["total_tickets"]      = static_cast<int>(result.total_tickets);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// GET /api/v1/simulator/scenarios - Get pre-computed scenarios
crow::response SimulatorController::get_scenarios(
    std::shared_ptr<simulator::ThresholdOptimizer> /*optimizer*/
) {
    try {
        // Cover the full domain the chart displays: [0.80, 1.00]
        // STP rate is inversely proportional to threshold using the same formula
        // as /calculate: STP% = max(0, 1 - (threshold - 0.80) * 4)
        // 21 data points: 0.80, 0.81, ..., 1.00 (step = 0.01)
        const size_t total_tickets = 10000;
        const double start = 0.80;
        const double step  = 0.01;
        const int    steps = 21;   // inclusive: (1.00 - 0.80) / 0.01 + 1

        crow::json::wvalue scenarios(crow::json::type::List);
        for (int i = 0; i < steps; ++i) {
            double t            = start + i * step;
            double projected_stp = std::max(0.0, std::min(1.0, 1.0 - ((t - 0.80) * 4.0)));
            double api_tickets   = static_cast<double>(total_tickets) * projected_stp;
            double human_tickets = static_cast<double>(total_tickets) * (1.0 - projected_stp);
            double api_cost      = api_tickets   * 0.05;
            double human_cost    = human_tickets * 3.00;
            double total_cost    = api_cost + human_cost;
            double cost_per_res  = total_cost / static_cast<double>(total_tickets);

            crow::json::wvalue scenario;
            scenario["threshold"]           = t;
            scenario["projected_stp_rate"]  = projected_stp;
            scenario["cost_per_resolution"] = cost_per_res;
            scenario["total_tickets"]       = static_cast<int>(total_tickets);
            scenarios[i] = std::move(scenario);
        }

        crow::json::wvalue response_data;
        response_data["scenarios"] = std::move(scenarios);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// Helper: Create success response
crow::json::wvalue SimulatorController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue SimulatorController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"]    = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
