#ifndef WFM_API_RLHF_CONTROLLER_H
#define WFM_API_RLHF_CONTROLLER_H

#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include "core/rlhf/feedback_collector.h"
#include "core/rlhf/golden_dataset_manager.h"

namespace wfm::api {

// RLHFController: REST API endpoints for RLHF Capture Gate
// Following REST conventions and API design best practices
class RLHFController {
public:
    // Register routes with Crow app
    static void register_routes(
        crow::App<crow::CORSHandler>& app,
        std::shared_ptr<rlhf::FeedbackCollector> collector,
        std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
    );

private:
    // POST /api/v1/rlhf/capture - Capture operator override event
    static crow::response capture_override(
        const crow::request& req,
        std::shared_ptr<rlhf::FeedbackCollector> collector,
        std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
    );

    // GET /api/v1/rlhf/dataset - Export golden dataset for training
    static crow::response export_dataset(
        const crow::request& req,
        std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
    );

    // GET /api/v1/rlhf/stats - Get override statistics
    static crow::response get_statistics(
        const crow::request& req,
        std::shared_ptr<rlhf::FeedbackCollector> collector,
        std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
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

#endif // WFM_API_RLHF_CONTROLLER_H
