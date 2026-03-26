#ifndef WFM_API_ANALYTICS_CONTROLLER_H
#define WFM_API_ANALYTICS_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/analytics/drift_detector.h"
#include "core/analytics/metrics_aggregator.h"

namespace wfm::api {

// AnalyticsController: REST API endpoints for Time-Lapse Simulator
// Following REST conventions and API design best practices
class AnalyticsController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<analytics::DriftDetector> drift_detector,
        std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
    );

private:
    // GET /api/v1/analytics/metrics - Get time-series metrics
    static crow::response get_metrics(
        const crow::request& req,
        std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
    );

    // GET /api/v1/analytics/drift - Get model drift analysis
    static crow::response get_drift_analysis(
        const crow::request& req,
        std::shared_ptr<analytics::DriftDetector> drift_detector
    );

    // GET /api/v1/analytics/flywheel - Get flywheel visualization data
    static crow::response get_flywheel_data(
        const crow::request& req,
        std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
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

#endif // WFM_API_ANALYTICS_CONTROLLER_H
