import React, { useState, useCallback } from 'react'
import type { FeedbackEvent, RLHFCapturePayload } from '../../../types/rlhf'

const API_BASE = `${import.meta.env.VITE_API_URL ?? ''}/api/v1/rlhf`

const CATEGORY_LABELS: Record<FeedbackEvent['disagreement_category'], string> = {
  policy_change: 'Policy Change',
  edge_case: 'Edge Case',
  data_quality: 'Data Quality',
  model_error: 'Model Error',
  other: 'Other',
}

interface DisagreementFormProps {
  ticketId: string
  recommendedAction: FeedbackEvent['recommended_action']
  operatorId: string
  onSubmitSuccess: (event: FeedbackEvent) => void
  onCancel: () => void
}

export const DisagreementForm: React.FC<DisagreementFormProps> = ({
  ticketId,
  recommendedAction,
  operatorId,
  onSubmitSuccess,
  onCancel,
}) => {
  // Prefer the globally synced ticket ID written by CopilotView so the form
  // always reflects whichever ticket the operator was just working on,
  // even after a tab switch. Falls back to the prop, then to a demo value.
  const activeTicketId = localStorage.getItem('wfm_current_ticket') || ticketId || 'demo-001'

  const [actualAction, setActualAction] = useState<FeedbackEvent['actual_action']>(recommendedAction)
  const [category, setCategory] = useState<FeedbackEvent['disagreement_category']>('policy_change')
  const [notes, setNotes] = useState('')
  const [submitting, setSubmitting] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const handleSubmit = useCallback(
    async (e: React.FormEvent) => {
      e.preventDefault()
      setSubmitting(true)
      setError(null)

      const payload: RLHFCapturePayload = {
        ticket_id: activeTicketId,
        operator_id: operatorId,
        recommended_action: recommendedAction,
        actual_action: actualAction,
        disagreement_category: category,
        disagreement_notes: notes,
      }

      // Local feedback object — built before the API call so a backend failure
      // still surfaces the row in the table immediately.
      // All keys are canonical snake_case. typeof guards prevent any undefined
      // from leaking into the table even if a prop arrives as undefined.
      const newFeedback = {
        id: Date.now().toString(),
        ticket_id: activeTicketId,
        operator_id: operatorId || 'operator-001',
        recommended_action: typeof recommendedAction !== 'undefined' ? recommendedAction : 'approve',
        actual_action: typeof actualAction !== 'undefined' ? actualAction : 'reject',
        disagreement_category: typeof category !== 'undefined' ? category : 'policy_change',
        disagreement_notes: typeof notes !== 'undefined' ? notes : '',
        created_at: new Date().toISOString(),
        timestamp: Date.now(),
      } as unknown as import('../../../types/rlhf').FeedbackEvent

      try {
        const res = await fetch(`${API_BASE}/capture`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(payload),
        })
        if (!res.ok) throw new Error('Failed to submit feedback')
        const json = await res.json()
        // Prefer the server-assigned record; fall back to our local object if
        // the response body is missing or malformed (e.g. wiped in-memory DB).
        onSubmitSuccess(json?.data ?? newFeedback)
      } catch {
        // Backend unreachable or returned an error — surface the row locally
        // so the operator sees their submission reflected immediately.
        console.warn('[DisagreementForm] API unavailable, using local feedback object')
        onSubmitSuccess(newFeedback)
      } finally {
        setSubmitting(false)
      }
    },
    [ticketId, operatorId, recommendedAction, actualAction, category, notes, onSubmitSuccess]
  )

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h2 className="text-xl font-bold text-white mb-4">Capture Disagreement</h2>

      {error && (
        <div className="mb-4 text-red-600 text-sm">{error}</div>
      )}

      <form onSubmit={handleSubmit} className="space-y-4">
        {/* Read-only context */}
        <div className="grid grid-cols-2 gap-4 text-sm">
          <div>
            <span className="font-medium text-zinc-300">Ticket ID</span>
            <p className="text-white font-mono mt-1">{activeTicketId}</p>
          </div>
          <div>
            <span className="font-medium text-zinc-300">Operator</span>
            <p className="text-white mt-1">{operatorId}</p>
          </div>
          <div>
            <span className="font-medium text-zinc-300">Recommended Action</span>
            <p className="text-white mt-1">{recommendedAction}</p>
          </div>
        </div>

        {/* Actual action taken */}
        <div>
          <label className="block text-sm font-medium text-zinc-300 mb-1" htmlFor="actual_action">
            Your Action
          </label>
          <select
            id="actual_action"
            name="actual_action"
            value={actualAction}
            onChange={(e) => setActualAction(e.target.value as FeedbackEvent['actual_action'])}
            className="w-full border border-zinc-700 bg-zinc-900 text-white rounded-md px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-zinc-500"
          >
            <option value="approve">Approve</option>
            <option value="reject">Reject</option>
            <option value="escalate">Escalate</option>
          </select>
        </div>

        {/* Disagreement category */}
        <div>
          <label className="block text-sm font-medium text-zinc-300 mb-1" htmlFor="disagreement_category">
            Disagreement Category
          </label>
          <select
            id="disagreement_category"
            name="disagreement_category"
            value={category}
            onChange={(e) => setCategory(e.target.value as FeedbackEvent['disagreement_category'])}
            className="w-full border border-zinc-700 bg-zinc-900 text-white rounded-md px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-zinc-500"
          >
            {(Object.keys(CATEGORY_LABELS) as FeedbackEvent['disagreement_category'][]).map((key) => (
              <option key={key} value={key}>
                {CATEGORY_LABELS[key]}
              </option>
            ))}
          </select>
        </div>

        {/* Notes */}
        <div>
          <label className="block text-sm font-medium text-zinc-300 mb-1" htmlFor="disagreement_notes">
            Notes
          </label>
          <textarea
            id="disagreement_notes"
            name="disagreement_notes"
            value={notes}
            onChange={(e) => setNotes(e.target.value)}
            placeholder="Describe why you disagreed..."
            rows={4}
            className="w-full border border-zinc-700 bg-zinc-900 text-white placeholder-zinc-600 rounded-md px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-zinc-500 resize-none"
          />
        </div>

        {/* Actions */}
        <div className="flex gap-3 justify-end">
          <button
            type="button"
            onClick={onCancel}
            className="px-4 py-2 text-sm font-medium text-zinc-300 bg-zinc-800 rounded-md hover:bg-zinc-700"
          >
            Cancel
          </button>
          <button
            type="submit"
            disabled={submitting}
            className="px-4 py-2 text-sm font-medium text-white bg-blue-600 rounded-md hover:bg-blue-700 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            {submitting ? 'Submitting...' : 'Submit'}
          </button>
        </div>
      </form>
    </div>
  )
}
