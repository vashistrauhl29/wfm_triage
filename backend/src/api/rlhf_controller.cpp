// RLHFController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/rlhf_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void RLHFController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<rlhf::FeedbackCollector> collector,
    std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
) {
    // POST /api/v1/rlhf/capture
    CROW_ROUTE(app, "/api/v1/rlhf/capture")
        .methods("POST"_method)
        ([collector, dataset_manager](const crow::request& req) {
            return capture_override(req, collector, dataset_manager);
        });

    // GET /api/v1/rlhf/dataset
    CROW_ROUTE(app, "/api/v1/rlhf/dataset")
        .methods("GET"_method)
        ([dataset_manager](const crow::request& req) {
            return export_dataset(req, dataset_manager);
        });

    // GET /api/v1/rlhf/stats
    CROW_ROUTE(app, "/api/v1/rlhf/stats")
        .methods("GET"_method)
        ([collector, dataset_manager](const crow::request& req) {
            return get_statistics(req, collector, dataset_manager);
        });
}

// POST /api/v1/rlhf/capture - Capture operator override event
crow::response RLHFController::capture_override(
    const crow::request& req,
    std::shared_ptr<rlhf::FeedbackCollector> collector,
    std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("ticket_id") ||
            !body.contains("operator_id") ||
            !body.contains("recommended_action") ||
            !body.contains("actual_action") ||
            !body.contains("disagreement_category")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required fields: ticket_id, operator_id, recommended_action, actual_action, disagreement_category"
            ));
        }

        // Extract fields
        std::string ticket_id = body["ticket_id"];
        std::string operator_id = body["operator_id"];
        std::string recommended_action = body["recommended_action"];
        std::string actual_action = body["actual_action"];
        std::string category_str = body["disagreement_category"];
        std::string notes = body.value("disagreement_notes", "");

        // Convert category string to enum
        DisagreementCategory category = string_to_disagreement_category(category_str);

        // Capture feedback event
        auto feedback_event = collector->capture_override(
            ticket_id,
            operator_id,
            recommended_action,
            actual_action,
            category,
            notes
        );

        // Create golden dataset entry if this is high-value feedback
        if (feedback_event.is_high_value_feedback()) {
            // Get ticket data from request (or fetch from repository)
            std::string ticket_data_json = body.value("ticket_data", "{}");

            auto dataset_entry = dataset_manager->create_entry_from_feedback(
                feedback_event,
                ticket_data_json
            );

            dataset_manager->add_to_dataset(dataset_entry);
        }

        // Build response
        crow::json::wvalue response_data;
        response_data["feedback_event_id"] = feedback_event.id;
        response_data["ticket_id"] = feedback_event.ticket_id;
        response_data["is_disagreement"] = feedback_event.is_disagreement();
        response_data["is_high_value_feedback"] = feedback_event.is_high_value_feedback();
        response_data["category"] = disagreement_category_to_string(feedback_event.category);
        response_data["message"] = "Override event captured successfully";

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

// GET /api/v1/rlhf/dataset - Export golden dataset for training
crow::response RLHFController::export_dataset(
    const crow::request& req,
    std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
) {
    try {
        (void)req;  // Unused for now

        // Get ready-for-training entries
        auto entries = dataset_manager->get_ready_for_training();

        if (entries.empty()) {
            return crow::response(200, error_response(
                "no_data",
                "No entries ready for training"
            ));
        }

        // Export to JSON
        std::string exported_json = dataset_manager->export_to_json(entries);

        // Mark as exported
        std::vector<std::string> entry_ids;
        for (const auto& entry : entries) {
            entry_ids.push_back(entry.id);
        }
        dataset_manager->mark_as_exported(entry_ids);

        // Build response
        crow::json::wvalue response_data;
        response_data["entries_count"] = entries.size();
        response_data["exported_json"] = exported_json;
        response_data["message"] = "Dataset exported successfully";

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// GET /api/v1/rlhf/stats - Get override statistics
crow::response RLHFController::get_statistics(
    const crow::request& req,
    std::shared_ptr<rlhf::FeedbackCollector> collector,
    std::shared_ptr<rlhf::GoldenDatasetManager> dataset_manager
) {
    try {
        (void)req;  // Unused for now

        // Get feedback statistics
        size_t total_disagreements = collector->count_disagreements();
        double disagreement_rate = collector->calculate_disagreement_rate();

        // Get category breakdown
        crow::json::wvalue category_breakdown;
        category_breakdown["policy_change"] =
            collector->count_disagreements_by_category(DisagreementCategory::PolicyChange);
        category_breakdown["edge_case"] =
            collector->count_disagreements_by_category(DisagreementCategory::EdgeCase);
        category_breakdown["model_error"] =
            collector->count_disagreements_by_category(DisagreementCategory::ModelError);
        category_breakdown["ambiguous_case"] =
            collector->count_disagreements_by_category(DisagreementCategory::AmbiguousCase);
        category_breakdown["context_missing"] =
            collector->count_disagreements_by_category(DisagreementCategory::ContextMissing);
        category_breakdown["user_error"] =
            collector->count_disagreements_by_category(DisagreementCategory::UserError);
        category_breakdown["other"] =
            collector->count_disagreements_by_category(DisagreementCategory::Other);

        // Get dataset statistics
        auto dataset_stats = dataset_manager->get_dataset_stats();

        // Build response
        crow::json::wvalue response_data;
        response_data["total_disagreements"] = total_disagreements;
        response_data["disagreement_rate"] = disagreement_rate;
        response_data["category_breakdown"] = std::move(category_breakdown);

        crow::json::wvalue dataset_info;
        dataset_info["total_entries"] = dataset_stats.total_entries;
        dataset_info["exported_entries"] = dataset_stats.exported_entries;
        dataset_info["ready_for_training"] = dataset_stats.ready_for_training;
        dataset_info["high_quality_entries"] = dataset_stats.high_quality_entries;
        dataset_info["avg_quality_score"] = dataset_stats.avg_quality_score;
        response_data["dataset_stats"] = std::move(dataset_info);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// Helper: Create success response
crow::json::wvalue RLHFController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue RLHFController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"] = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
