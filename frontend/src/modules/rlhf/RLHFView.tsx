import React, { useState, useEffect, useCallback } from 'react'
import { DisagreementForm } from './components/DisagreementForm'
import { FeedbackHistory } from './components/FeedbackHistory'
import type { FeedbackEvent } from '../../types/rlhf'

const API_BASE = `${import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'}/api/v1/rlhf`

interface RLHFViewProps {
  ticketId: string
  recommendedAction: FeedbackEvent['recommended_action']
  operatorId: string
}

export const RLHFView: React.FC<RLHFViewProps> = ({
  ticketId,
  recommendedAction,
  operatorId,
}) => {
  const [events, setEvents] = useState<FeedbackEvent[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    setLoading(true)
    setError(null)

    fetch(`${API_BASE}/dataset`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('Failed to load feedback history')
        return res.json()
      })
      .then((json) => {
        const raw = json.data
        const safeEvents = Array.isArray(raw) ? raw : (raw?.events ?? [])
        setEvents(safeEvents)
        setLoading(false)
      })
      .catch(() => {
        setError('Failed to load feedback history')
        setLoading(false)
      })
  }, [])

  const handleSubmitSuccess = useCallback((event: FeedbackEvent) => {
    setEvents((prev) => [event, ...prev])
  }, [])

  const handleCancel = useCallback(() => {
    // Cancel is handled internally by DisagreementForm — form stays mounted
  }, [])

  return (
    <div className="p-6 space-y-6 animate-fade-in-up">
      <h1 className="text-2xl font-bold text-white">RLHF Capture Gate</h1>

      <DisagreementForm
        ticketId={ticketId}
        recommendedAction={recommendedAction}
        operatorId={operatorId}
        onSubmitSuccess={handleSubmitSuccess}
        onCancel={handleCancel}
      />

      {loading && (
        <div className="text-center py-2 text-zinc-500">Loading history...</div>
      )}

      {error && (
        <div className="text-center py-2 text-red-500">{error}</div>
      )}

      <FeedbackHistory events={events} />
    </div>
  )
}
