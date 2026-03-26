/**
 * Type definitions for the Context-Engineered Operator View (Copilot) module
 */

export interface OperatorAction {
  ticket_id: string
  action: 'approve' | 'reject' | 'escalate'
  notes?: string
  timestamp: string  // ISO 8601
}

export interface ActionResponse {
  action_id: string
  ticket_id: string
  action: OperatorAction['action']
  recorded_at: string
}
