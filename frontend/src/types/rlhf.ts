export interface FeedbackEvent {
  id: string
  ticket_id: string
  operator_id: string
  recommended_action: 'approve' | 'reject' | 'escalate'
  actual_action: 'approve' | 'reject' | 'escalate'
  disagreement_category: 'policy_change' | 'edge_case' | 'data_quality' | 'model_error' | 'other'
  disagreement_notes: string
  created_at: string
}

export interface RLHFCapturePayload {
  ticket_id: string
  operator_id: string
  recommended_action: FeedbackEvent['recommended_action']
  actual_action: FeedbackEvent['actual_action']
  disagreement_category: FeedbackEvent['disagreement_category']
  disagreement_notes: string
}
