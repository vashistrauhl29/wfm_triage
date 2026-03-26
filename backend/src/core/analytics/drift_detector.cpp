// DriftDetector production implementation
// Following C++ Core Guidelines and TDD principles

#include "core/analytics/drift_detector.h"
#include <cmath>
#include <algorithm>
#include <map>
#include <memory>

namespace wfm::analytics {

// Constructor with baseline values
DriftDetector::DriftDetector(
    double baseline_accuracy,
    double drift_threshold_percentage
) {
    // Constructor body intentionally empty - abstract base class
    // Subclasses will handle actual initialization
    (void)baseline_accuracy;
    (void)drift_threshold_percentage;
}

// ============================================================================
// DefaultDriftDetector: Production implementation of DriftDetector
// ============================================================================

class DefaultDriftDetector : public DriftDetector {
public:
    explicit DefaultDriftDetector(
        double baseline_accuracy = 0.90,
        double drift_threshold_percentage = 5.0
    );

    // Detect drift by comparing recent metrics against baseline
    DriftDetectionResult detect_drift(
        const std::vector<MetricSnapshot>& recent_metrics,
        MetricType metric_type
    ) const override;

    // Detect drift for a single metric value
    DriftDetectionResult detect_drift_single(
        double current_value,
        MetricType metric_type
    ) const override;

    // Check if accuracy has fallen below baseline
    bool is_accuracy_degraded(double current_accuracy) const noexcept override;

    // Calculate drift percentage
    double calculate_drift_percentage(
        double current_value,
        double baseline_value
    ) const noexcept override;

    // Determine severity of drift
    std::string determine_severity(
        double drift_percentage
    ) const noexcept override;

    // Update baseline values
    void update_baseline(
        MetricType metric_type,
        double new_baseline
    ) override;

    // Get current baseline for a metric type
    double get_baseline(MetricType metric_type) const noexcept override;

    // Get drift threshold percentage
    double get_drift_threshold() const noexcept override;

    // Check if drift detection is enabled
    bool is_enabled() const noexcept override;

    // Enable/disable drift detection
    void set_enabled(bool enabled) override;

private:
    double baseline_accuracy_;
    double drift_threshold_;
    bool enabled_;
    mutable std::map<MetricType, double> baselines_;
};

// Constructor
DefaultDriftDetector::DefaultDriftDetector(
    double baseline_accuracy,
    double drift_threshold_percentage
) : DriftDetector(baseline_accuracy, drift_threshold_percentage),
    baseline_accuracy_(baseline_accuracy),
    drift_threshold_(drift_threshold_percentage),
    enabled_(true) {
}

// Detect drift by comparing recent metrics against baseline
DriftDetectionResult DefaultDriftDetector::detect_drift(
    const std::vector<MetricSnapshot>& recent_metrics,
    MetricType metric_type
) const {
    if (recent_metrics.empty()) {
        return DriftDetectionResult();
    }

    // Calculate average of recent metrics for the specified type
    double sum = 0.0;
    size_t count = 0;
    for (const auto& m : recent_metrics) {
        if (m.metric_type == metric_type) {
            sum += m.metric_value;
            ++count;
        }
    }

    if (count == 0) {
        return DriftDetectionResult();
    }

    double current_value = sum / static_cast<double>(count);
    return detect_drift_single(current_value, metric_type);
}

// Detect drift for a single metric value
DriftDetectionResult DefaultDriftDetector::detect_drift_single(
    double current_value,
    MetricType metric_type
) const {
    DriftDetectionResult result;
    result.metric_type = metric_type;
    result.current_value = current_value;
    result.baseline_value = get_baseline(metric_type);
    result.detection_timestamp = "2024-01-01T00:00:00Z";  // In production, use actual timestamp

    double drift_pct = calculate_drift_percentage(current_value, result.baseline_value);
    result.drift_percentage = drift_pct;

    // Drift detected if percentage exceeds threshold
    // For accuracy, negative drift (degradation) is concerning
    if (metric_type == MetricType::Accuracy) {
        result.drift_detected = (drift_pct < -drift_threshold_);
    } else {
        // For other metrics, any drift beyond threshold is concerning
        result.drift_detected = (std::abs(drift_pct) > drift_threshold_);
    }

    result.severity = determine_severity(std::abs(drift_pct));

    return result;
}

// Check if accuracy has fallen below baseline
bool DefaultDriftDetector::is_accuracy_degraded(double current_accuracy) const noexcept {
    double drift_pct = calculate_drift_percentage(current_accuracy, baseline_accuracy_);
    return drift_pct < -drift_threshold_;
}

// Calculate drift percentage
double DefaultDriftDetector::calculate_drift_percentage(
    double current_value,
    double baseline_value
) const noexcept {
    if (baseline_value == 0.0) {
        return 0.0;  // Avoid division by zero
    }
    return ((current_value - baseline_value) / baseline_value) * 100.0;
}

// Determine severity of drift
std::string DefaultDriftDetector::determine_severity(
    double drift_percentage
) const noexcept {
    double abs_drift = std::abs(drift_percentage);
    if (abs_drift >= 20.0) {
        return "critical";
    } else if (abs_drift >= 10.0) {
        return "high";
    } else if (abs_drift >= 5.0) {
        return "medium";
    } else {
        return "low";
    }
}

// Update baseline values
void DefaultDriftDetector::update_baseline(
    MetricType metric_type,
    double new_baseline
) {
    baselines_[metric_type] = new_baseline;
}

// Get current baseline for a metric type
double DefaultDriftDetector::get_baseline(MetricType metric_type) const noexcept {
    auto it = baselines_.find(metric_type);
    if (it != baselines_.end()) {
        return it->second;
    }
    // Default baselines
    if (metric_type == MetricType::Accuracy) {
        return baseline_accuracy_;
    }
    return 0.0;
}

// Get drift threshold percentage
double DefaultDriftDetector::get_drift_threshold() const noexcept {
    return drift_threshold_;
}

// Check if drift detection is enabled
bool DefaultDriftDetector::is_enabled() const noexcept {
    return enabled_;
}

// Enable/disable drift detection
void DefaultDriftDetector::set_enabled(bool enabled) {
    enabled_ = enabled;
}

// ============================================================================
// Factory function for creating DriftDetector instances
// ============================================================================

std::unique_ptr<DriftDetector> create_drift_detector(
    double baseline_accuracy,
    double drift_threshold_percentage
) {
    return std::make_unique<DefaultDriftDetector>(
        baseline_accuracy,
        drift_threshold_percentage
    );
}

} // namespace wfm::analytics
