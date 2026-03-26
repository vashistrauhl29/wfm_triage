/**
 * Simulator type definitions for the Dynamic Margin & Cost Simulator module.
 * Aligned with backend CostSimulationResult, CostParameters, and API responses.
 */

export interface CostParameters {
  api_cost: number        // Cost per API inference call ($)
  human_rate: number      // Average human operator hourly rate ($/hr)
  handling_time: number   // Average handling time per ticket (seconds)
}

export interface CostBreakdownData {
  api_cost: number        // Total API cost ($)
  human_cost: number      // Total human cost ($)
  total_cost: number      // Combined total cost ($)
  cost_per_ticket: number // Average cost per ticket ($)
  stp_rate: number        // Straight-through processing rate (0-1)
}

export interface ScenarioDataPoint {
  threshold: number           // AI confidence threshold (0-1)
  projected_stp_rate: number  // Projected STP rate at this threshold
  projected_cost: number      // Projected total cost ($)
  cost_per_ticket: number     // Cost per ticket ($)
}

export interface OptimizeResult {
  optimal_threshold: number
  projected_stp_rate: number
  projected_cost: number
  cost_per_ticket: number
}
