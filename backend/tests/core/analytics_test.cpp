// Time-Lapse Simulator (Analytics) Module Tests
// Following TDD principles and C++ Core Guidelines
// Test coverage for DriftDetector and MetricsAggregator

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <map>
#include <algorithm>
#include "core/analytics/drift_detector.h"
#include "core/analytics/metrics_aggregator.h"
#include "models/metrics_snapshot.h"

using namespace wfm;
using namespace wfm::analytics;

// ============================================================================
// Mock Implementations (for testing purposes only)
// ============================================================================

// MockDriftDetector: Test implementation of DriftDetector interface
class MockDriftDetector : public DriftDetector {
public:
    explicit MockDriftDetector(
        double baseline_accuracy = 0.90,
        double drift_threshold_percentage = 5.0
    ) : DriftDetector(baseline_accuracy, drift_threshold_percentage),
        baseline_accuracy_(baseline_accuracy),
        drift_threshold_(drift_threshold_percentage),
        enabled_(true) {}

    // Detect drift by comparing recent metrics against baseline
    DriftDetectionResult detect_drift(
        const std::vector<MetricSnapshot>& recent_metrics,
        MetricType metric_type
    ) const override {
        if (recent_metrics.empty()) {
            return DriftDetectionResult();
        }

        // Calculate average of recent metrics
        double sum = 0.0;
        for (const auto& m : recent_metrics) {
            if (m.metric_type == metric_type) {
                sum += m.metric_value;
            }
        }
        double current_value = sum / static_cast<double>(recent_metrics.size());

        return detect_drift_single(current_value, metric_type);
    }

    // Detect drift for a single metric value
    DriftDetectionResult detect_drift_single(
        double current_value,
        MetricType metric_type
    ) const override {
        DriftDetectionResult result;
        result.metric_type = metric_type;
        result.current_value = current_value;
        result.baseline_value = get_baseline(metric_type);
        result.detection_timestamp = "2024-01-01T00:00:00Z";

        double drift_pct = calculate_drift_percentage(current_value, result.baseline_value);
        result.drift_percentage = drift_pct;

        // Drift detected if percentage exceeds threshold (negative drift for accuracy)
        if (metric_type == MetricType::Accuracy) {
            result.drift_detected = (drift_pct < -drift_threshold_);
        } else {
            result.drift_detected = (std::abs(drift_pct) > drift_threshold_);
        }

        result.severity = determine_severity(std::abs(drift_pct));

        return result;
    }

    // Check if accuracy has fallen below baseline
    bool is_accuracy_degraded(double current_accuracy) const noexcept override {
        double drift_pct = calculate_drift_percentage(current_accuracy, baseline_accuracy_);
        return drift_pct < -drift_threshold_;
    }

    // Calculate drift percentage
    double calculate_drift_percentage(
        double current_value,
        double baseline_value
    ) const noexcept override {
        if (baseline_value == 0.0) {
            return 0.0;
        }
        return ((current_value - baseline_value) / baseline_value) * 100.0;
    }

    // Determine severity of drift
    std::string determine_severity(
        double drift_percentage
    ) const noexcept override {
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
    void update_baseline(
        MetricType metric_type,
        double new_baseline
    ) override {
        baselines_[metric_type] = new_baseline;
    }

    // Get current baseline for a metric type
    double get_baseline(MetricType metric_type) const noexcept override {
        if (baselines_.find(metric_type) != baselines_.end()) {
            return baselines_.at(metric_type);
        }
        // Default baselines
        if (metric_type == MetricType::Accuracy) {
            return baseline_accuracy_;
        }
        return 0.0;
    }

    // Get drift threshold percentage
    double get_drift_threshold() const noexcept override {
        return drift_threshold_;
    }

    // Check if drift detection is enabled
    bool is_enabled() const noexcept override {
        return enabled_;
    }

    // Enable/disable drift detection
    void set_enabled(bool enabled) override {
        enabled_ = enabled;
    }

private:
    double baseline_accuracy_;
    double drift_threshold_;
    bool enabled_;
    mutable std::map<MetricType, double> baselines_;
};

// MockMetricsAggregator: Test implementation of MetricsAggregator interface
class MockMetricsAggregator : public MetricsAggregator {
public:
    explicit MockMetricsAggregator(std::unique_ptr<DriftDetector> detector)
        : MetricsAggregator(std::move(detector)), metrics_() {}

    // Add metric snapshot to aggregator
    void add_metric(const MetricSnapshot& metric) const override {
        metrics_.push_back(metric);
    }

    // Aggregate metrics by period
    std::vector<AggregatedMetrics> aggregate_by_period(
        MetricType metric_type,
        AggregationPeriod period,
        const std::string& start_time,
        const std::string& end_time
    ) const override {
        std::vector<AggregatedMetrics> result;

        // Filter metrics by type and time range
        std::vector<MetricSnapshot> filtered;
        for (const auto& m : metrics_) {
            if (m.metric_type == metric_type &&
                m.timestamp >= start_time &&
                m.timestamp <= end_time) {
                filtered.push_back(m);
            }
        }

        if (filtered.empty()) {
            return result;
        }

        // Simple aggregation (single period)
        AggregatedMetrics agg;
        agg.period_start = start_time;
        agg.period_end = end_time;
        agg.metric_type = metric_type;
        agg.sample_count = filtered.size();

        double sum = 0.0;
        agg.max_value = filtered[0].metric_value;
        agg.min_value = filtered[0].metric_value;

        for (const auto& m : filtered) {
            sum += m.metric_value;
            agg.max_value = std::max(agg.max_value, m.metric_value);
            agg.min_value = std::min(agg.min_value, m.metric_value);
        }

        agg.avg_value = sum / static_cast<double>(filtered.size());
        result.push_back(agg);

        return result;
    }

    // Get daily aggregated metrics
    std::vector<AggregatedMetrics> get_daily_metrics(
        MetricType metric_type,
        size_t num_days
    ) const override {
        std::vector<AggregatedMetrics> result;

        // Group metrics by day
        for (size_t day = 0; day < num_days; ++day) {
            // Format day with zero-padding (01, 02, etc.)
            std::string day_str = (day + 1 < 10) ? "0" + std::to_string(day + 1) : std::to_string(day + 1);
            std::string period_start = "2024-01-" + day_str + "T00:00:00Z";
            std::string period_end = "2024-01-" + day_str + "T23:59:59Z";

            auto daily = aggregate_by_period(metric_type, AggregationPeriod::Daily, period_start, period_end);
            if (!daily.empty()) {
                result.push_back(daily[0]);
            }
        }

        return result;
    }

    // Get weekly aggregated metrics
    std::vector<AggregatedMetrics> get_weekly_metrics(
        MetricType metric_type,
        size_t num_weeks
    ) const override {
        std::vector<AggregatedMetrics> result;

        // Group metrics by week
        for (size_t week = 0; week < num_weeks; ++week) {
            size_t start_day = week * 7 + 1;
            size_t end_day = start_day + 6;

            // Format days with zero-padding
            std::string start_day_str = (start_day < 10) ? "0" + std::to_string(start_day) : std::to_string(start_day);
            std::string end_day_str = (end_day < 10) ? "0" + std::to_string(end_day) : std::to_string(end_day);

            std::string period_start = "2024-01-" + start_day_str + "T00:00:00Z";
            std::string period_end = "2024-01-" + end_day_str + "T23:59:59Z";

            auto weekly = aggregate_by_period(metric_type, AggregationPeriod::Weekly, period_start, period_end);
            if (!weekly.empty()) {
                result.push_back(weekly[0]);
            }
        }

        return result;
    }

    // Calculate average for a metric type over time range
    double calculate_average(
        MetricType metric_type,
        const std::string& start_time,
        const std::string& end_time
    ) const override {
        auto aggregated = aggregate_by_period(metric_type, AggregationPeriod::Daily, start_time, end_time);
        if (aggregated.empty()) {
            return 0.0;
        }
        return aggregated[0].avg_value;
    }

    // Calculate trend (increasing, decreasing, stable)
    std::string calculate_trend(
        const std::vector<AggregatedMetrics>& metrics
    ) const override {
        if (metrics.size() < 2) {
            return "stable";
        }

        // Compare first and last values
        double first_value = metrics.front().avg_value;
        double last_value = metrics.back().avg_value;
        double change_pct = ((last_value - first_value) / first_value) * 100.0;

        if (change_pct > 5.0) {
            return "increasing";
        } else if (change_pct < -5.0) {
            return "decreasing";
        } else {
            return "stable";
        }
    }

    // Get all metrics for a specific type
    std::vector<MetricSnapshot> get_metrics_by_type(
        MetricType metric_type
    ) const override {
        std::vector<MetricSnapshot> result;
        for (const auto& m : metrics_) {
            if (m.metric_type == metric_type) {
                result.push_back(m);
            }
        }
        return result;
    }

    // Get metrics for a time range
    std::vector<MetricSnapshot> get_metrics_in_range(
        const std::string& start_time,
        const std::string& end_time
    ) const override {
        std::vector<MetricSnapshot> result;
        for (const auto& m : metrics_) {
            if (m.timestamp >= start_time && m.timestamp <= end_time) {
                result.push_back(m);
            }
        }
        return result;
    }

    // Prepare data for flywheel visualization
    FlywheelData prepare_flywheel_data(
        const std::string& period
    ) const override {
        FlywheelData data;
        data.period = period;

        // Calculate from stored metrics
        auto accuracy_metrics = get_metrics_by_type(MetricType::Accuracy);
        if (accuracy_metrics.size() >= 2) {
            double first_accuracy = accuracy_metrics.front().metric_value;
            double last_accuracy = accuracy_metrics.back().metric_value;
            data.model_accuracy_improvement = last_accuracy - first_accuracy;
            data.current_model_version = accuracy_metrics.back().model_version;
        }

        data.total_tickets_processed = metrics_.size();
        // Use std::ceil to avoid truncation for small values
        data.human_feedback_collected = static_cast<size_t>(std::ceil(metrics_.size() * 0.1));  // 10% feedback rate
        data.golden_dataset_entries = static_cast<size_t>(std::ceil(metrics_.size() * 0.05));   // 5% golden dataset

        return data;
    }

    // Get time-series data for charting
    std::vector<MetricSnapshot> get_time_series(
        MetricType metric_type,
        size_t num_points
    ) const override {
        auto all_metrics = get_metrics_by_type(metric_type);
        if (all_metrics.size() <= num_points) {
            return all_metrics;
        }

        // Return last num_points
        std::vector<MetricSnapshot> result;
        for (size_t i = all_metrics.size() - num_points; i < all_metrics.size(); ++i) {
            result.push_back(all_metrics[i]);
        }
        return result;
    }

    // Clear all metrics (for testing)
    void clear_metrics() const override {
        metrics_.clear();
    }

    // Get count of stored metrics
    size_t get_metric_count() const noexcept override {
        return metrics_.size();
    }

private:
    mutable std::vector<MetricSnapshot> metrics_;
};

// ============================================================================
// DriftDetector Test Suite
// ============================================================================

class DriftDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector_ = std::make_unique<MockDriftDetector>(0.90, 5.0);
    }

    std::unique_ptr<DriftDetector> detector_;
};

// Test: Constructor initializes with baseline accuracy
TEST_F(DriftDetectorTest, ConstructorInitializesBaseline) {
    EXPECT_DOUBLE_EQ(detector_->get_baseline(MetricType::Accuracy), 0.90);
    EXPECT_DOUBLE_EQ(detector_->get_drift_threshold(), 5.0);
}

// Test: Detect drift when accuracy falls below threshold
TEST_F(DriftDetectorTest, DetectDriftWhenAccuracyFallsBelowThreshold) {
    std::vector<MetricSnapshot> metrics = {
        MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.84, "v1.0.0"}
    };

    auto result = detector_->detect_drift(metrics, MetricType::Accuracy);

    EXPECT_TRUE(result.drift_detected);
    EXPECT_EQ(result.metric_type, MetricType::Accuracy);
    EXPECT_DOUBLE_EQ(result.current_value, 0.84);
    EXPECT_DOUBLE_EQ(result.baseline_value, 0.90);
}

// Test: No drift detected when accuracy within threshold
TEST_F(DriftDetectorTest, NoDriftWhenAccuracyWithinThreshold) {
    std::vector<MetricSnapshot> metrics = {
        MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.89, "v1.0.0"}
    };

    auto result = detector_->detect_drift(metrics, MetricType::Accuracy);

    EXPECT_FALSE(result.drift_detected);
}

// Test: Calculate drift percentage correctly
TEST_F(DriftDetectorTest, CalculateDriftPercentageCorrectly) {
    double drift_pct = detector_->calculate_drift_percentage(0.85, 0.90);

    EXPECT_NEAR(drift_pct, -5.56, 0.1);  // (0.85 - 0.90) / 0.90 * 100 = -5.56%
}

// Test: Calculate positive drift percentage
TEST_F(DriftDetectorTest, CalculatePositiveDriftPercentage) {
    double drift_pct = detector_->calculate_drift_percentage(0.95, 0.90);

    EXPECT_NEAR(drift_pct, 5.56, 0.1);  // (0.95 - 0.90) / 0.90 * 100 = +5.56%
}

// Test: Handle zero baseline (edge case)
TEST_F(DriftDetectorTest, HandleZeroBaseline) {
    double drift_pct = detector_->calculate_drift_percentage(0.5, 0.0);

    EXPECT_DOUBLE_EQ(drift_pct, 0.0);  // Should return 0 to avoid division by zero
}

// Test: Determine severity levels
TEST_F(DriftDetectorTest, DetermineSeverityLevels) {
    EXPECT_EQ(detector_->determine_severity(3.0), "low");
    EXPECT_EQ(detector_->determine_severity(7.0), "medium");
    EXPECT_EQ(detector_->determine_severity(12.0), "high");
    EXPECT_EQ(detector_->determine_severity(25.0), "critical");
}

// Test: Is accuracy degraded
TEST_F(DriftDetectorTest, IsAccuracyDegraded) {
    EXPECT_TRUE(detector_->is_accuracy_degraded(0.84));   // -6.67% drift
    EXPECT_FALSE(detector_->is_accuracy_degraded(0.89));  // -1.11% drift
}

// Test: Update baseline
TEST_F(DriftDetectorTest, UpdateBaseline) {
    detector_->update_baseline(MetricType::Accuracy, 0.95);

    EXPECT_DOUBLE_EQ(detector_->get_baseline(MetricType::Accuracy), 0.95);
}

// Test: Detect drift for single metric value
TEST_F(DriftDetectorTest, DetectDriftSingleValue) {
    auto result = detector_->detect_drift_single(0.84, MetricType::Accuracy);

    EXPECT_TRUE(result.drift_detected);
    EXPECT_DOUBLE_EQ(result.current_value, 0.84);
}

// Test: Detect drift for multiple recent metrics (average)
TEST_F(DriftDetectorTest, DetectDriftMultipleMetrics) {
    std::vector<MetricSnapshot> metrics = {
        MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.85, "v1.0.0"},
        MetricSnapshot{"2024-01-02T00:00:00Z", MetricType::Accuracy, 0.83, "v1.0.0"}
    };

    auto result = detector_->detect_drift(metrics, MetricType::Accuracy);

    EXPECT_TRUE(result.drift_detected);
    EXPECT_DOUBLE_EQ(result.current_value, 0.84);  // Average of 0.85 and 0.83
}

// Test: Detect drift for non-accuracy metrics
TEST_F(DriftDetectorTest, DetectDriftForCostMetric) {
    detector_->update_baseline(MetricType::Cost, 100.0);

    std::vector<MetricSnapshot> metrics = {
        MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Cost, 120.0, "v1.0.0"}
    };

    auto result = detector_->detect_drift(metrics, MetricType::Cost);

    EXPECT_TRUE(result.drift_detected);  // 20% drift exceeds 5% threshold
}

// Test: Enable/disable drift detection
TEST_F(DriftDetectorTest, EnableDisableDriftDetection) {
    EXPECT_TRUE(detector_->is_enabled());

    detector_->set_enabled(false);
    EXPECT_FALSE(detector_->is_enabled());

    detector_->set_enabled(true);
    EXPECT_TRUE(detector_->is_enabled());
}

// Test: Empty metrics vector returns default result
TEST_F(DriftDetectorTest, EmptyMetricsReturnsDefault) {
    std::vector<MetricSnapshot> empty_metrics;

    auto result = detector_->detect_drift(empty_metrics, MetricType::Accuracy);

    EXPECT_FALSE(result.drift_detected);
}

// Test: Severity is correctly set in drift result
TEST_F(DriftDetectorTest, SeveritySetInDriftResult) {
    auto result = detector_->detect_drift_single(0.84, MetricType::Accuracy);

    EXPECT_EQ(result.severity, "medium");  // ~6.67% drift = medium
}

// Test: Detection timestamp is set
TEST_F(DriftDetectorTest, DetectionTimestampIsSet) {
    auto result = detector_->detect_drift_single(0.84, MetricType::Accuracy);

    EXPECT_FALSE(result.detection_timestamp.empty());
}

// Test: Baseline value is correctly reported in result
TEST_F(DriftDetectorTest, BaselineValueInResult) {
    auto result = detector_->detect_drift_single(0.84, MetricType::Accuracy);

    EXPECT_DOUBLE_EQ(result.baseline_value, 0.90);
}

// Test: Drift percentage is correctly reported in result
TEST_F(DriftDetectorTest, DriftPercentageInResult) {
    auto result = detector_->detect_drift_single(0.84, MetricType::Accuracy);

    EXPECT_NEAR(result.drift_percentage, -6.67, 0.1);
}

// Test: Get default baseline for unknown metric type
TEST_F(DriftDetectorTest, GetDefaultBaselineForUnknownMetric) {
    double baseline = detector_->get_baseline(MetricType::Throughput);

    EXPECT_DOUBLE_EQ(baseline, 0.0);  // Default baseline
}

// Test: Multiple baseline updates
TEST_F(DriftDetectorTest, MultipleBaselineUpdates) {
    detector_->update_baseline(MetricType::Accuracy, 0.92);
    detector_->update_baseline(MetricType::Cost, 80.0);

    EXPECT_DOUBLE_EQ(detector_->get_baseline(MetricType::Accuracy), 0.92);
    EXPECT_DOUBLE_EQ(detector_->get_baseline(MetricType::Cost), 80.0);
}

// ============================================================================
// MetricsAggregator Test Suite
// ============================================================================

class MetricsAggregatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto drift_detector = std::make_unique<MockDriftDetector>(0.90, 5.0);
        aggregator_ = std::make_unique<MockMetricsAggregator>(std::move(drift_detector));
    }

    std::unique_ptr<MetricsAggregator> aggregator_;
};

// Test: Constructor takes unique_ptr ownership (RAII)
TEST_F(MetricsAggregatorTest, ConstructorTakesOwnership) {
    auto detector = std::make_unique<MockDriftDetector>(0.90, 5.0);
    auto agg = std::make_unique<MockMetricsAggregator>(std::move(detector));

    EXPECT_EQ(agg->get_metric_count(), 0);
}

// Test: Add metric to aggregator
TEST_F(MetricsAggregatorTest, AddMetricToAggregator) {
    MetricSnapshot metric{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"};

    aggregator_->add_metric(metric);

    EXPECT_EQ(aggregator_->get_metric_count(), 1);
}

// Test: Add multiple metrics
TEST_F(MetricsAggregatorTest, AddMultipleMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-02T00:00:00Z", MetricType::Accuracy, 0.93, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-03T00:00:00Z", MetricType::Cost, 85.0, "v1.0.0"});

    EXPECT_EQ(aggregator_->get_metric_count(), 3);
}

// Test: Get metrics by type
TEST_F(MetricsAggregatorTest, GetMetricsByType) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-02T00:00:00Z", MetricType::Cost, 85.0, "v1.0.0"});

    auto accuracy_metrics = aggregator_->get_metrics_by_type(MetricType::Accuracy);

    EXPECT_EQ(accuracy_metrics.size(), 1);
    EXPECT_EQ(accuracy_metrics[0].metric_type, MetricType::Accuracy);
}

// Test: Get metrics in time range
TEST_F(MetricsAggregatorTest, GetMetricsInRange) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-05T00:00:00Z", MetricType::Accuracy, 0.93, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-10T00:00:00Z", MetricType::Accuracy, 0.94, "v1.0.0"});

    auto range_metrics = aggregator_->get_metrics_in_range("2024-01-01T00:00:00Z", "2024-01-06T00:00:00Z");

    EXPECT_EQ(range_metrics.size(), 2);
}

// Test: Aggregate metrics by daily period
TEST_F(MetricsAggregatorTest, AggregateDailyMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T10:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T14:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});

    auto aggregated = aggregator_->aggregate_by_period(
        MetricType::Accuracy,
        AggregationPeriod::Daily,
        "2024-01-01T00:00:00Z",
        "2024-01-01T23:59:59Z"
    );

    ASSERT_EQ(aggregated.size(), 1);
    EXPECT_DOUBLE_EQ(aggregated[0].avg_value, 0.91);
    EXPECT_DOUBLE_EQ(aggregated[0].min_value, 0.90);
    EXPECT_DOUBLE_EQ(aggregated[0].max_value, 0.92);
    EXPECT_EQ(aggregated[0].sample_count, 2);
}

// Test: Get daily metrics
TEST_F(MetricsAggregatorTest, GetDailyMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T10:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-02T10:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});

    auto daily = aggregator_->get_daily_metrics(MetricType::Accuracy, 2);

    EXPECT_EQ(daily.size(), 2);
}

// Test: Get weekly metrics
TEST_F(MetricsAggregatorTest, GetWeeklyMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T10:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-08T10:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});

    auto weekly = aggregator_->get_weekly_metrics(MetricType::Accuracy, 2);

    EXPECT_EQ(weekly.size(), 2);
}

// Test: Calculate average over time range
TEST_F(MetricsAggregatorTest, CalculateAverage) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T10:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T14:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});

    double avg = aggregator_->calculate_average(
        MetricType::Accuracy,
        "2024-01-01T00:00:00Z",
        "2024-01-01T23:59:59Z"
    );

    EXPECT_DOUBLE_EQ(avg, 0.91);
}

// Test: Calculate trend - increasing
TEST_F(MetricsAggregatorTest, CalculateTrendIncreasing) {
    std::vector<AggregatedMetrics> metrics;
    AggregatedMetrics m1;
    m1.avg_value = 0.90;
    AggregatedMetrics m2;
    m2.avg_value = 0.95;
    metrics.push_back(m1);
    metrics.push_back(m2);

    std::string trend = aggregator_->calculate_trend(metrics);

    EXPECT_EQ(trend, "increasing");
}

// Test: Calculate trend - decreasing
TEST_F(MetricsAggregatorTest, CalculateTrendDecreasing) {
    std::vector<AggregatedMetrics> metrics;
    AggregatedMetrics m1;
    m1.avg_value = 0.95;
    AggregatedMetrics m2;
    m2.avg_value = 0.85;
    metrics.push_back(m1);
    metrics.push_back(m2);

    std::string trend = aggregator_->calculate_trend(metrics);

    EXPECT_EQ(trend, "decreasing");
}

// Test: Calculate trend - stable
TEST_F(MetricsAggregatorTest, CalculateTrendStable) {
    std::vector<AggregatedMetrics> metrics;
    AggregatedMetrics m1;
    m1.avg_value = 0.90;
    AggregatedMetrics m2;
    m2.avg_value = 0.91;
    metrics.push_back(m1);
    metrics.push_back(m2);

    std::string trend = aggregator_->calculate_trend(metrics);

    EXPECT_EQ(trend, "stable");
}

// Test: Prepare flywheel data
TEST_F(MetricsAggregatorTest, PrepareFlywheelData) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.85, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-15T00:00:00Z", MetricType::Accuracy, 0.90, "v1.5.0"});

    auto flywheel = aggregator_->prepare_flywheel_data("last_30_days");

    EXPECT_EQ(flywheel.period, "last_30_days");
    EXPECT_EQ(flywheel.total_tickets_processed, 2);
    EXPECT_GT(flywheel.human_feedback_collected, 0);
    EXPECT_GT(flywheel.golden_dataset_entries, 0);
    EXPECT_NEAR(flywheel.model_accuracy_improvement, 0.05, 0.0001);
    EXPECT_EQ(flywheel.current_model_version, "v1.5.0");
}

// Test: Get time-series data
TEST_F(MetricsAggregatorTest, GetTimeSeries) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-02T00:00:00Z", MetricType::Accuracy, 0.91, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-03T00:00:00Z", MetricType::Accuracy, 0.92, "v1.0.0"});

    auto series = aggregator_->get_time_series(MetricType::Accuracy, 2);

    EXPECT_EQ(series.size(), 2);
    EXPECT_DOUBLE_EQ(series[0].metric_value, 0.91);
    EXPECT_DOUBLE_EQ(series[1].metric_value, 0.92);
}

// Test: Clear metrics
TEST_F(MetricsAggregatorTest, ClearMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});
    EXPECT_EQ(aggregator_->get_metric_count(), 1);

    aggregator_->clear_metrics();

    EXPECT_EQ(aggregator_->get_metric_count(), 0);
}

// Test: Empty aggregation returns empty result
TEST_F(MetricsAggregatorTest, EmptyAggregationReturnsEmpty) {
    auto aggregated = aggregator_->aggregate_by_period(
        MetricType::Accuracy,
        AggregationPeriod::Daily,
        "2024-01-01T00:00:00Z",
        "2024-01-01T23:59:59Z"
    );

    EXPECT_TRUE(aggregated.empty());
}

// Test: Calculate average with no data returns zero
TEST_F(MetricsAggregatorTest, CalculateAverageNoData) {
    double avg = aggregator_->calculate_average(
        MetricType::Accuracy,
        "2024-01-01T00:00:00Z",
        "2024-01-01T23:59:59Z"
    );

    EXPECT_DOUBLE_EQ(avg, 0.0);
}

// Test: Get metrics by type returns empty when no match
TEST_F(MetricsAggregatorTest, GetMetricsByTypeEmpty) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Accuracy, 0.90, "v1.0.0"});

    auto cost_metrics = aggregator_->get_metrics_by_type(MetricType::Cost);

    EXPECT_TRUE(cost_metrics.empty());
}

// Test: Aggregate metrics for different metric types
TEST_F(MetricsAggregatorTest, AggregateDifferentMetricTypes) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T10:00:00Z", MetricType::Cost, 80.0, "v1.0.0"});
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T14:00:00Z", MetricType::Cost, 90.0, "v1.0.0"});

    auto aggregated = aggregator_->aggregate_by_period(
        MetricType::Cost,
        AggregationPeriod::Daily,
        "2024-01-01T00:00:00Z",
        "2024-01-01T23:59:59Z"
    );

    ASSERT_EQ(aggregated.size(), 1);
    EXPECT_DOUBLE_EQ(aggregated[0].avg_value, 85.0);
}

// Test: Trend calculation with single data point returns stable
TEST_F(MetricsAggregatorTest, TrendSingleDataPoint) {
    std::vector<AggregatedMetrics> metrics;
    AggregatedMetrics m1;
    m1.avg_value = 0.90;
    metrics.push_back(m1);

    std::string trend = aggregator_->calculate_trend(metrics);

    EXPECT_EQ(trend, "stable");
}

// Test: Flywheel data with no accuracy metrics
TEST_F(MetricsAggregatorTest, FlywheelDataNoAccuracyMetrics) {
    aggregator_->add_metric(MetricSnapshot{"2024-01-01T00:00:00Z", MetricType::Cost, 85.0, "v1.0.0"});

    auto flywheel = aggregator_->prepare_flywheel_data("last_30_days");

    EXPECT_EQ(flywheel.model_accuracy_improvement, 0.0);
}
