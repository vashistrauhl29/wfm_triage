#ifndef WFM_CORE_ANALYTICS_METRICS_AGGREGATOR_H
#define WFM_CORE_ANALYTICS_METRICS_AGGREGATOR_H

#include <string>
#include <vector>
#include <memory>
#include "models/metrics_snapshot.h"
#include "core/analytics/drift_detector.h"

namespace wfm::analytics {

// Time period for aggregation
enum class AggregationPeriod {
    Hourly,
    Daily,
    Weekly,
    Monthly
};

// MetricsAggregator: Aggregates time-series metrics for analysis
// Following C++ Core Guidelines:
// - R.20: Use unique_ptr to represent ownership
// - Con.2: Make member functions const by default
class MetricsAggregator {
public:
    // I.11: Take ownership by unique_ptr (following RAII)
    explicit MetricsAggregator(std::unique_ptr<DriftDetector> detector);

    // Virtual destructor for potential inheritance
    virtual ~MetricsAggregator() = default;

    // Add metric snapshot to aggregator
    virtual void add_metric(const MetricSnapshot& metric) const = 0;

    // Aggregate metrics by period
    virtual std::vector<AggregatedMetrics> aggregate_by_period(
        MetricType metric_type,
        AggregationPeriod period,
        const std::string& start_time,
        const std::string& end_time
    ) const = 0;

    // Get daily aggregated metrics
    virtual std::vector<AggregatedMetrics> get_daily_metrics(
        MetricType metric_type,
        size_t num_days
    ) const = 0;

    // Get weekly aggregated metrics
    virtual std::vector<AggregatedMetrics> get_weekly_metrics(
        MetricType metric_type,
        size_t num_weeks
    ) const = 0;

    // Calculate average for a metric type over time range
    virtual double calculate_average(
        MetricType metric_type,
        const std::string& start_time,
        const std::string& end_time
    ) const = 0;

    // Calculate trend (increasing, decreasing, stable)
    virtual std::string calculate_trend(
        const std::vector<AggregatedMetrics>& metrics
    ) const = 0;

    // Get all metrics for a specific type
    virtual std::vector<MetricSnapshot> get_metrics_by_type(
        MetricType metric_type
    ) const = 0;

    // Get metrics for a time range
    virtual std::vector<MetricSnapshot> get_metrics_in_range(
        const std::string& start_time,
        const std::string& end_time
    ) const = 0;

    // Prepare data for flywheel visualization
    virtual FlywheelData prepare_flywheel_data(
        const std::string& period = "last_30_days"
    ) const = 0;

    // Get time-series data for charting
    virtual std::vector<MetricSnapshot> get_time_series(
        MetricType metric_type,
        size_t num_points
    ) const = 0;

    // Clear all metrics (for testing)
    virtual void clear_metrics() const = 0;

    // Get count of stored metrics
    virtual size_t get_metric_count() const noexcept = 0;
};

} // namespace wfm::analytics

#endif // WFM_CORE_ANALYTICS_METRICS_AGGREGATOR_H
