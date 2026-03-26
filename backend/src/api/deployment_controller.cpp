// DeploymentController implementation
// Following REST conventions and C++ Core Guidelines

#include "api/deployment_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wfm::api {

// Register routes with Crow app
void DeploymentController::register_routes(
    crow::App<crow::CORSHandler>& app,
    std::shared_ptr<deployment::EVMCalculator> evm_calculator,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker,
    std::shared_ptr<deployment::MilestoneManager> milestone_manager
) {
    // GET /api/v1/deployment/phases
    CROW_ROUTE(app, "/api/v1/deployment/phases")
        .methods("GET"_method)
        ([phase_tracker](const crow::request& req) {
            return get_all_phases(req, phase_tracker);
        });

    // GET /api/v1/deployment/phase/:id
    CROW_ROUTE(app, "/api/v1/deployment/phase/<string>")
        .methods("GET"_method)
        ([phase_tracker, milestone_manager](const crow::request& req, const std::string& phase_id) {
            return get_phase_details(req, phase_tracker, milestone_manager, phase_id);
        });

    // POST /api/v1/deployment/milestone
    CROW_ROUTE(app, "/api/v1/deployment/milestone")
        .methods("POST"_method)
        ([milestone_manager](const crow::request& req) {
            return complete_milestone(req, milestone_manager);
        });

    // GET /api/v1/deployment/evm
    CROW_ROUTE(app, "/api/v1/deployment/evm")
        .methods("GET"_method)
        ([evm_calculator, phase_tracker, milestone_manager](const crow::request& req) {
            return get_evm_metrics(req, evm_calculator, phase_tracker, milestone_manager);
        });

    // GET /api/v1/deployment/health
    CROW_ROUTE(app, "/api/v1/deployment/health")
        .methods("GET"_method)
        ([evm_calculator, phase_tracker, milestone_manager](const crow::request& req) {
            return get_deployment_health(req, evm_calculator, phase_tracker, milestone_manager);
        });

    // PATCH /api/v1/deployment/phase/:id
    CROW_ROUTE(app, "/api/v1/deployment/phase/<string>")
        .methods("PATCH"_method)
        ([phase_tracker](const crow::request& req, const std::string& phase_id) {
            return update_phase_status(req, phase_tracker, phase_id);
        });
}

// GET /api/v1/deployment/phases - Get all deployment phases
crow::response DeploymentController::get_all_phases(
    const crow::request& req,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker
) {
    try {
        (void)req;  // Unused for now

        auto phases = phase_tracker->get_all_phases();

        // Build response
        crow::json::wvalue response_data;
        crow::json::wvalue phases_list(crow::json::type::List);

        for (size_t i = 0; i < phases.size(); ++i) {
            crow::json::wvalue phase;
            phase["id"] = phases[i].id;
            phase["phase_number"] = phases[i].phase_number;
            phase["phase_name"] = phases[i].phase_name;
            phase["description"] = phases[i].description;
            phase["start_date"] = phases[i].start_date;
            phase["planned_end_date"] = phases[i].planned_end_date;
            phase["actual_end_date"] = phases[i].actual_end_date;
            phase["status"] = phase_status_to_string(phases[i].status);
            phase["budget_allocated"] = phases[i].budget_allocated;
            phase["actual_cost"] = phases[i].actual_cost;
            phases_list[i] = std::move(phase);
        }

        response_data["phases"] = std::move(phases_list);
        response_data["total_phases"] = static_cast<int>(phases.size());
        response_data["overall_progress"] = phase_tracker->calculate_progress();

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// GET /api/v1/deployment/phase/:id - Get specific phase details
crow::response DeploymentController::get_phase_details(
    const crow::request& req,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker,
    std::shared_ptr<deployment::MilestoneManager> milestone_manager,
    const std::string& phase_id
) {
    try {
        (void)req;  // Unused for now

        // Parse phase ID
        int64_t id = std::stoll(phase_id);

        // Get phase
        auto phase = phase_tracker->get_phase(id);

        // Get milestones for this phase
        auto milestones = milestone_manager->get_milestones_for_phase(id);

        // Build response
        crow::json::wvalue response_data;

        // Phase details
        crow::json::wvalue phase_data;
        phase_data["id"] = phase.id;
        phase_data["phase_number"] = phase.phase_number;
        phase_data["phase_name"] = phase.phase_name;
        phase_data["description"] = phase.description;
        phase_data["start_date"] = phase.start_date;
        phase_data["planned_end_date"] = phase.planned_end_date;
        phase_data["actual_end_date"] = phase.actual_end_date;
        phase_data["status"] = phase_status_to_string(phase.status);
        phase_data["budget_allocated"] = phase.budget_allocated;
        phase_data["actual_cost"] = phase.actual_cost;
        response_data["phase"] = std::move(phase_data);

        // Milestones
        crow::json::wvalue milestones_list(crow::json::type::List);
        for (size_t i = 0; i < milestones.size(); ++i) {
            crow::json::wvalue milestone;
            milestone["id"] = milestones[i].id;
            milestone["milestone_name"] = milestones[i].milestone_name;
            milestone["description"] = milestones[i].description;
            milestone["planned_completion_date"] = milestones[i].planned_completion_date;
            milestone["actual_completion_date"] = milestones[i].actual_completion_date;
            milestone["status"] = milestone_status_to_string(milestones[i].status);
            milestone["weight"] = milestones[i].weight;
            milestone["owner"] = milestones[i].owner;

            // Dependencies
            crow::json::wvalue deps(crow::json::type::List);
            for (size_t j = 0; j < milestones[i].dependencies.size(); ++j) {
                deps[j] = milestones[i].dependencies[j];
            }
            milestone["dependencies"] = std::move(deps);

            milestones_list[i] = std::move(milestone);
        }
        response_data["milestones"] = std::move(milestones_list);

        // Calculate phase completion
        double phase_completion = milestone_manager->calculate_phase_completion(id);
        response_data["phase_completion_percentage"] = phase_completion;

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response(
            "validation_error",
            "Invalid phase ID"
        ));
    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// POST /api/v1/deployment/milestone - Complete a milestone
crow::response DeploymentController::complete_milestone(
    const crow::request& req,
    std::shared_ptr<deployment::MilestoneManager> milestone_manager
) {
    try {
        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("milestone_id")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required field: milestone_id"
            ));
        }

        int64_t milestone_id = body["milestone_id"];
        std::string completion_date = body.value("completion_date", "2026-03-23");

        // Check if milestone can be completed
        if (!milestone_manager->can_complete_milestone(milestone_id)) {
            return crow::response(400, error_response(
                "validation_error",
                "Milestone cannot be completed. Dependencies may not be satisfied."
            ));
        }

        // Complete milestone
        milestone_manager->complete_milestone(milestone_id, completion_date);

        // Get updated milestone
        auto milestone = milestone_manager->get_milestone(milestone_id);

        // Build response
        crow::json::wvalue response_data;
        response_data["milestone_id"] = milestone.id;
        response_data["milestone_name"] = milestone.milestone_name;
        response_data["status"] = milestone_status_to_string(milestone.status);
        response_data["completion_date"] = milestone.actual_completion_date;
        response_data["message"] = "Milestone completed successfully";

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

// GET /api/v1/deployment/evm - Get EVM metrics
crow::response DeploymentController::get_evm_metrics(
    const crow::request& req,
    std::shared_ptr<deployment::EVMCalculator> evm_calculator,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker,
    std::shared_ptr<deployment::MilestoneManager> milestone_manager
) {
    try {
        (void)req;  // Unused for now

        // Get all phases and milestones
        auto phases = phase_tracker->get_all_phases();

        // Calculate total budget
        double budget_at_completion = 0.0;
        for (const auto& phase : phases) {
            budget_at_completion += phase.budget_allocated;
        }

        // Get all milestones (need to collect from all phases)
        std::vector<Milestone> all_milestones;
        for (const auto& phase : phases) {
            auto phase_milestones = milestone_manager->get_milestones_for_phase(phase.id);
            all_milestones.insert(all_milestones.end(),
                                 phase_milestones.begin(),
                                 phase_milestones.end());
        }

        // Calculate EVM metrics
        auto evm = evm_calculator->calculate_evm(
            budget_at_completion,
            all_milestones,
            phases
        );

        // Build response
        crow::json::wvalue response_data;
        response_data["planned_value"] = evm.planned_value;
        response_data["earned_value"] = evm.earned_value;
        response_data["actual_cost"] = evm.actual_cost;
        response_data["schedule_variance"] = evm.schedule_variance;
        response_data["cost_variance"] = evm.cost_variance;
        response_data["schedule_performance_index"] = evm.schedule_performance_index;
        response_data["cost_performance_index"] = evm.cost_performance_index;
        response_data["estimate_at_completion"] = evm.estimate_at_completion;
        response_data["estimate_to_complete"] = evm.estimate_to_complete;
        response_data["variance_at_completion"] = evm.variance_at_completion;
        response_data["snapshot_date"] = evm.snapshot_date;

        // Add health indicators
        response_data["is_ahead_of_schedule"] = evm.is_ahead_of_schedule();
        response_data["is_under_budget"] = evm.is_under_budget();
        response_data["is_on_track"] = evm.is_on_track();
        response_data["health_score"] = evm.get_health_score();

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// GET /api/v1/deployment/health - Get deployment health score
crow::response DeploymentController::get_deployment_health(
    const crow::request& req,
    std::shared_ptr<deployment::EVMCalculator> evm_calculator,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker,
    std::shared_ptr<deployment::MilestoneManager> milestone_manager
) {
    try {
        (void)req;  // Unused for now

        // Get all phases
        auto phases = phase_tracker->get_all_phases();

        // Calculate total budget
        double budget_at_completion = 0.0;
        for (const auto& phase : phases) {
            budget_at_completion += phase.budget_allocated;
        }

        // Get all milestones
        std::vector<Milestone> all_milestones;
        for (const auto& phase : phases) {
            auto phase_milestones = milestone_manager->get_milestones_for_phase(phase.id);
            all_milestones.insert(all_milestones.end(),
                                 phase_milestones.begin(),
                                 phase_milestones.end());
        }

        // Calculate EVM metrics
        auto evm = evm_calculator->calculate_evm(
            budget_at_completion,
            all_milestones,
            phases
        );

        // Build health snapshot
        DeploymentHealth health;
        health.overall_health_score = evm.get_health_score();
        health.health_status = health.determine_health_status();
        health.completed_milestones = static_cast<int>(milestone_manager->get_completed_count());
        health.total_milestones = static_cast<int>(milestone_manager->get_milestone_count());
        health.blocked_phases = static_cast<int>(phase_tracker->count_phases_by_status(PhaseStatus::Blocked));
        health.critical_risks = 0;  // Would come from risk repository
        health.snapshot_date = "2026-03-23";

        // Build response
        crow::json::wvalue response_data;
        response_data["overall_health_score"] = health.overall_health_score;
        response_data["health_status"] = health.health_status;
        response_data["completed_milestones"] = health.completed_milestones;
        response_data["total_milestones"] = health.total_milestones;
        response_data["completion_percentage"] = health.get_completion_percentage();
        response_data["blocked_phases"] = health.blocked_phases;
        response_data["critical_risks"] = health.critical_risks;
        response_data["snapshot_date"] = health.snapshot_date;

        // EVM summary
        crow::json::wvalue evm_summary;
        evm_summary["spi"] = evm.schedule_performance_index;
        evm_summary["cpi"] = evm.cost_performance_index;
        evm_summary["is_on_track"] = evm.is_on_track();
        response_data["evm_summary"] = std::move(evm_summary);

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// PATCH /api/v1/deployment/phase/:id - Update phase status
crow::response DeploymentController::update_phase_status(
    const crow::request& req,
    std::shared_ptr<deployment::PhaseTracker> phase_tracker,
    const std::string& phase_id
) {
    try {
        // Parse phase ID
        int64_t id = std::stoll(phase_id);

        // Parse request body
        auto body = json::parse(req.body);

        // Validate required fields
        if (!body.contains("status")) {
            return crow::response(400, error_response(
                "validation_error",
                "Missing required field: status"
            ));
        }

        std::string status_str = body["status"];
        PhaseStatus new_status = string_to_phase_status(status_str);

        // Check if transition is valid
        if (!phase_tracker->can_transition(id, new_status)) {
            return crow::response(400, error_response(
                "validation_error",
                "Invalid phase status transition"
            ));
        }

        // Update phase status
        phase_tracker->update_phase_status(id, new_status);

        // Get updated phase
        auto phase = phase_tracker->get_phase(id);

        // Build response
        crow::json::wvalue response_data;
        response_data["phase_id"] = phase.id;
        response_data["phase_number"] = phase.phase_number;
        response_data["phase_name"] = phase.phase_name;
        response_data["status"] = phase_status_to_string(phase.status);
        response_data["message"] = "Phase status updated successfully";

        return crow::response(200, success_response(std::move(response_data)));

    } catch (const std::invalid_argument& e) {
        return crow::response(400, error_response(
            "validation_error",
            "Invalid phase ID or status"
        ));
    } catch (const std::exception& e) {
        return crow::response(500, error_response(
            "internal_error",
            e.what()
        ));
    }
}

// Helper: Create success response
crow::json::wvalue DeploymentController::success_response(
    crow::json::wvalue data
) {
    crow::json::wvalue response;
    response["data"] = std::move(data);
    return response;
}

// Helper: Create error response
crow::json::wvalue DeploymentController::error_response(
    const std::string& code,
    const std::string& message
) {
    crow::json::wvalue response;
    response["error"]["code"] = code;
    response["error"]["message"] = message;
    return response;
}

} // namespace wfm::api
