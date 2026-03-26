// AnalyticsController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/analytics_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void AnalyticsController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<analytics::DriftDetector> drift_detector,
    std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
) {
    // GET /api/v1/analytics/metrics
    CROW_ROUTE(app, "/api/v1/analytics/metrics")
        .methods("GET"_method)
        ([metrics_aggregator](const crow::request& req) {
            return get_metrics(req, metrics_aggregator);
        });

    // GET /api/v1/analytics/drift
    CROW_ROUTE(app, "/api/v1/analytics/drift")
        .methods("GET"_method)
        ([drift_detector](const crow::request& req) {
            return get_drift_analysis(req, drift_detector);
        });

    // GET /api/v1/analytics/flywheel
    CROW_ROUTE(app, "/api/v1/analytics/flywheel")
        .methods("GET"_method)
        ([metrics_aggregator](const crow::request& req) {
            return get_flywheel_data(req, metrics_aggregator);
        });
}

// GET /api/v1/analytics/metrics - Get time-series metrics
crow::response AnalyticsController::get_metrics(
    const crow::request& req,
    std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
) {
    try {
        // Parse query parameters
        auto period_param      = req.url_params.get("period");
        auto metric_type_param = req.url_params.get("metric_type");

        std::string period_str = period_param ? period_param : "daily";

        // Determine metric type (default: accuracy)
        MetricType metric_type = MetricType::Accuracy;
        if (metric_type_param) {
            metric_type = string_to_metric_type(std::string(metric_type_param));
        }

        // Actual API: get_daily_metrics(MetricType, size_t) → vector<AggregatedMetrics>
        std::vector<AggregatedMetrics> data_points;
        if (period_str == "daily") {
            data_points = metrics_aggregator->get_daily_metrics(metric_type, 7);
        } else if (period_str == "weekly") {
            data_points = metrics_aggregator->get_weekly_metrics(metric_type, 4);
        } else {
            return crow::response(400, error_response(
                "validation_error",
                "Invalid period. Must be 'daily' or 'weekly'"
            ));
        }

        // calculate_trend takes vector<AggregatedMetrics> and returns string
        std::string trend = metrics_aggregator->calculate_trend(data_points);

        // Build response
        crow::json::wvalue response_data;
        response_data["period"]      = period_str;
        response_data["metric_type"] = metric_type_to_string(metric_type);
        response_data["data_points"] = static_cast<int>(data_points.size());
        response_data["trend"]       = trend;

        crow::json::wvalue points(crow::json::type::List);
        for (size_t i = 0; i < data_points.size(); ++i) {
            crow::json::wvalue pt;
            pt["period_start"] = data_points[i].period_start;
            pt["period_end"]   = data_points[i].period_end;
            pt["avg_value"]    = data_points[i].avg_value;
            pt["max_value"]    = data_points[i].max_value;
            pt["min_value"]    = data_points[i].min_value;
            pt["sample_count"] = static_cast<int>(data_points[i].sample_count);
            points[i] = std::move(pt);
        }
        response_data["data_points_list"] = std::move(points);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// GET /api/v1/analytics/drift - Get model drift analysis
crow::response AnalyticsController::get_drift_analysis(
    const crow::request& req,
    std::shared_ptr<analytics::DriftDetector> drift_detector
) {
    try {
        (void)req;

        // Actual API: get_baseline(MetricType) → double
        double baseline_value = drift_detector->get_baseline(MetricType::Accuracy);

        // Actual API: detect_drift_single(double, MetricType) → DriftDetectionResult
        auto drift_result = drift_detector->detect_drift_single(
            baseline_value, MetricType::Accuracy);

        // Build response — DriftDetectionResult fields:
        // drift_detected, metric_type, current_value, baseline_value,
        // drift_percentage, detection_timestamp, severity
        crow::json::wvalue response_data;
        response_data["drift_detected"]      = drift_result.drift_detected;
        response_data["drift_percentage"]    = drift_result.drift_percentage;
        response_data["severity"]            = drift_result.severity;
        response_data["current_value"]       = drift_result.current_value;
        response_data["baseline_value"]      = drift_result.baseline_value;
        response_data["detection_timestamp"] = drift_result.detection_timestamp;
        response_data["metric_type"]         = metric_type_to_string(drift_result.metric_type);
        response_data["drift_threshold"]     = drift_detector->get_drift_threshold();

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// GET /api/v1/analytics/flywheel - Get flywheel visualization data
crow::response AnalyticsController::get_flywheel_data(
    const crow::request& req,
    std::shared_ptr<analytics::MetricsAggregator> metrics_aggregator
) {
    try {
        (void)req;

        // Actual API: prepare_flywheel_data(period) → FlywheelData
        auto flywheel = metrics_aggregator->prepare_flywheel_data("last_30_days");

        // FlywheelData fields: total_tickets_processed, human_feedback_collected,
        // golden_dataset_entries, model_accuracy_improvement,
        // current_model_version, period
        crow::json::wvalue response_data;
        response_data["total_tickets_processed"]    = static_cast<int>(flywheel.total_tickets_processed);
        response_data["human_feedback_collected"]   = static_cast<int>(flywheel.human_feedback_collected);
        response_data["golden_dataset_entries"]     = static_cast<int>(flywheel.golden_dataset_entries);
        response_data["model_accuracy_improvement"] = flywheel.model_accuracy_improvement;
        response_data["current_model_version"]      = flywheel.current_model_version;
        response_data["period"]                     = flywheel.period;

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response("internal_error", e.what()));
    }
}

// Helper: Create success response
crow::json::wvalue AnalyticsController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue AnalyticsController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"]    = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
