export interface MetricsSnapshot {
  date: string           // ISO date string e.g. "2026-03-24"
  name?: string          // display label used as XAxis key (e.g. "Mon") — preferred over date for charts
  accuracy: number       // model classification accuracy 0.0–1.0
  stp_rate: number       // straight-through processing rate 0.0–1.0
  override_rate: number  // operator override rate 0.0–1.0
  volume: number         // total ticket count for the day
  total_cost: number     // total WFM cost for the day (USD)
  api_cost: number       // API inference cost for the day (USD)
  human_cost: number     // human labor cost for the day (USD)
}

// Used by CostTrend when rendered from a separate state to the metrics chart
export interface CostDataPoint {
  name: string
  total_cost: number
  api_cost: number
  human_cost: number
}

export interface DriftReport {
  detected: boolean
  drift_magnitude: number    // e.g. 0.08 = 8% drift from baseline
  baseline_accuracy: number
  current_accuracy: number
  model_version: string
  last_checked: string       // ISO timestamp
}

export interface FlywheelData {
  feedback_count: number
  model_version: string
  last_retrained: string     // ISO timestamp
  stp_improvement: number    // e.g. 0.05 = 5% improvement since last training
}
