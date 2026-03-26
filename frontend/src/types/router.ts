/**
 * Router type definitions for the Live HITL Router module
 */

export interface QueueStatus {
  stp_queue_size: number
  human_queue_size: number
  total_processed: number
  stp_rate: number  // 0.0 to 1.0
}

export interface RoutingStats extends QueueStatus {
  avg_confidence: number  // Average confidence score
}

export interface EvaluateTicketRequest {
  ticket_id: string
  confidence_score: number
  ticket_type?: string
  raw_data?: string
}

export interface EvaluateTicketResponse {
  ticket_id: string
  routing_decision: 'stp' | 'human_queue'
  confidence_score: number
  threshold: number
  timestamp: string
}

export interface ClearQueuesResponse {
  message: string
}
