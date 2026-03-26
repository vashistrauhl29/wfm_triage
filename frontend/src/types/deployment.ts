export type PhaseStatus = 'not_started' | 'in_progress' | 'completed' | 'blocked'
export type MilestoneStatus = 'pending' | 'in_progress' | 'completed' | 'at_risk'

export interface Milestone {
  id: string
  phase_id: string
  milestone_name: string
  description?: string
  planned_completion_date: string  // ISO date
  actual_completion_date?: string
  status: MilestoneStatus
  weight: number                   // 0–100, for EV calculation
  owner?: string
  dependencies?: string[]          // Array of milestone IDs that must be completed first
}

export interface DeploymentPhase {
  id: string
  phase_number: number             // 1–5
  phase_name: string
  description?: string
  start_date?: string              // ISO date
  planned_end_date?: string        // ISO date
  actual_end_date?: string         // ISO date
  status: PhaseStatus
  budget_allocated: number
  actual_cost: number
  milestones: Milestone[]
}

export interface EVMSnapshot {
  planned_value: number            // PV: Budgeted cost of work scheduled
  earned_value: number             // EV: Budgeted cost of work performed
  actual_cost: number              // AC: Actual cost of work performed
  schedule_variance: number        // SV = EV - PV
  cost_variance: number            // CV = EV - AC
  schedule_performance_index: number  // SPI = EV / PV  (>1 ahead of schedule)
  cost_performance_index: number      // CPI = EV / AC  (>1 under budget)
  estimate_at_completion: number      // EAC = BAC / CPI
  estimate_to_complete: number        // ETC = EAC - AC
  variance_at_completion: number      // VAC = BAC - EAC
  budget_at_completion: number        // BAC: total budget
}
