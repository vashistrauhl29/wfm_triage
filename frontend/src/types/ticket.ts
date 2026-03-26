/**
 * Ticket type definitions for the Live HITL Router module
 */

export interface Ticket {
  id: string
  ticket_type: string
  confidence_score: number  // 0.0 to 1.0
  routing_decision: 'stp' | 'human_queue'
  raw_data: string  // JSON string
  created_at: string  // ISO 8601 timestamp
}

export interface TicketWithContext extends Ticket {
  summary: string[]  // 3-bullet summary
  relevant_sops: SOPSearchResult[]
  action_recommendation: string
  confidence_override_threshold: number
}

export interface SOPSearchResult {
  id: string
  title: string
  summary: string
  relevance_score: number  // 0.0 to 1.0
}
