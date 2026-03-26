#ifndef WFM_CORE_ANALYTICS_DRIFT_DETECTOR_H
#define WFM_CORE_ANALYTICS_DRIFT_DETECTOR_H

#include <string>
#include <vector>
#include "models/metrics_snapshot.h"

namespace wfm::analytics {

// DriftDetector: Monitors model performance degradation over time
// Following C++ Core Guidelines:
// - Con.2: Make member functions const by default
// - C.9: Minimize exposure of members
class DriftDetector {
public:
    // Constructor with baseline values
    explicit DriftDetector(
        double baseline_accuracy = 0.90,
        double drift_threshold_percentage = 5.0  // 5% degradation triggers alert
    );

    // Virtual destructor for potential inheritance
    virtual ~DriftDetector() = default;

    // Detect drift by comparing current metrics against baseline
    virtual DriftDetectionResult detect_drift(
        const std::vector<MetricSnapshot>& recent_metrics,
        MetricType metric_type
    ) const = 0;

    // Detect drift for a single metric value
    virtual DriftDetectionResult detect_drift_single(
        double current_value,
        MetricType metric_type
    ) const = 0;

    // Check if accuracy has fallen below baseline
    virtual bool is_accuracy_degraded(double current_accuracy) const noexcept = 0;

    // Calculate drift percentage
    virtual double calculate_drift_percentage(
        double current_value,
        double baseline_value
    ) const noexcept = 0;

    // Determine severity of drift
    virtual std::string determine_severity(
        double drift_percentage
    ) const noexcept = 0;

    // Update baseline values (for adaptive drift detection)
    virtual void update_baseline(
        MetricType metric_type,
        double new_baseline
    ) = 0;

    // Get current baseline for a metric type
    virtual double get_baseline(MetricType metric_type) const noexcept = 0;

    // Get drift threshold percentage
    virtual double get_drift_threshold() const noexcept = 0;

    // Check if drift detection is enabled
    virtual bool is_enabled() const noexcept = 0;

    // Enable/disable drift detection
    virtual void set_enabled(bool enabled) = 0;
};

} // namespace wfm::analytics

#endif // WFM_CORE_ANALYTICS_DRIFT_DETECTOR_H
