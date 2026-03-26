#ifndef WFM_MODELS_METRICS_SNAPSHOT_H
#define WFM_MODELS_METRICS_SNAPSHOT_H

#include <string>
#include <cstdint>

namespace wfm {

// Metric type enumeration
enum class MetricType {
    Accuracy,           // Model accuracy percentage
    EscalationVolume,   // Number of escalations to human
    Cost,               // Operational cost (dollars)
    StpRate,            // Straight-through processing rate
    LatencyMs,          // Processing latency (milliseconds)
    Throughput          // Tickets processed per hour
};

// Convert MetricType to string
inline std::string metric_type_to_string(MetricType type) {
    switch (type) {
        case MetricType::Accuracy: return "accuracy";
        case MetricType::EscalationVolume: return "escalation_volume";
        case MetricType::Cost: return "cost";
        case MetricType::StpRate: return "stp_rate";
        case MetricType::LatencyMs: return "latency_ms";
        case MetricType::Throughput: return "throughput";
        default: return "unknown";
    }
}

// Convert string to MetricType
inline MetricType string_to_metric_type(const std::string& str) {
    if (str == "accuracy") return MetricType::Accuracy;
    if (str == "escalation_volume") return MetricType::EscalationVolume;
    if (str == "cost") return MetricType::Cost;
    if (str == "stp_rate") return MetricType::StpRate;
    if (str == "latency_ms") return MetricType::LatencyMs;
    if (str == "throughput") return MetricType::Throughput;
    return MetricType::Accuracy;  // Default
}

// Domain model: Single point-in-time metric measurement
struct MetricSnapshot {
    std::string timestamp;          // ISO 8601 timestamp
    MetricType metric_type;         // Type of metric
    double metric_value;            // Metric value
    std::string model_version;      // Model version (e.g., "v1.2.3")
    std::string metadata;           // Additional metadata (JSON)

    // ES.20: Always initialize
    MetricSnapshot()
        : timestamp(),
          metric_type(MetricType::Accuracy),
          metric_value(0.0),
          model_version(),
          metadata() {}

    // Constructor with essential fields
    MetricSnapshot(
        std::string ts,
        MetricType type,
        double value,
        std::string version = "v1.0.0"
    ) : timestamp(std::move(ts)),
        metric_type(type),
        metric_value(value),
        model_version(std::move(version)),
        metadata("{}") {}
};

// Aggregated metrics for a time period (daily/weekly)
struct AggregatedMetrics {
    std::string period_start;       // Period start timestamp
    std::string period_end;         // Period end timestamp
    MetricType metric_type;         // Type of metric
    double avg_value;               // Average value
    double max_value;               // Maximum value
    double min_value;               // Minimum value
    size_t sample_count;            // Number of samples

    AggregatedMetrics()
        : period_start(),
          period_end(),
          metric_type(MetricType::Accuracy),
          avg_value(0.0),
          max_value(0.0),
          min_value(0.0),
          sample_count(0) {}
};

// Drift detection result
struct DriftDetectionResult {
    bool drift_detected;            // Whether drift was detected
    MetricType metric_type;         // Metric that drifted
    double current_value;           // Current metric value
    double baseline_value;          // Baseline (expected) value
    double drift_percentage;        // Percentage drift from baseline
    std::string detection_timestamp; // When drift was detected
    std::string severity;           // "low", "medium", "high", "critical"

    DriftDetectionResult()
        : drift_detected(false),
          metric_type(MetricType::Accuracy),
          current_value(0.0),
          baseline_value(0.0),
          drift_percentage(0.0),
          detection_timestamp(),
          severity("none") {}
};

// Data flywheel visualization data
struct FlywheelData {
    size_t total_tickets_processed;     // Total tickets in period
    size_t human_feedback_collected;    // Feedback events collected
    size_t golden_dataset_entries;      // New training data entries
    double model_accuracy_improvement;  // Accuracy improvement percentage
    std::string current_model_version;  // Current model version
    std::string period;                 // Time period (e.g., "last_30_days")

    FlywheelData()
        : total_tickets_processed(0),
          human_feedback_collected(0),
          golden_dataset_entries(0),
          model_accuracy_improvement(0.0),
          current_model_version("v1.0.0"),
          period("last_30_days") {}
};

} // namespace wfm

#endif // WFM_MODELS_METRICS_SNAPSHOT_H
