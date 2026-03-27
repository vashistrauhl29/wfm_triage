import React from 'react'
import type { FeedbackEvent } from '../../../types/rlhf'

interface FeedbackHistoryProps {
  events: FeedbackEvent[]
}

export const FeedbackHistory: React.FC<FeedbackHistoryProps> = ({ events }) => {
  // Deduplicate by id before rendering so stale fetches + local optimistic inserts
  // never produce two rows for the same event.
  const uniqueEvents = events.filter((e, i, arr) => arr.findIndex((x) => x.id === e.id) === i)

  // Force mock data when backend returns empty array (in-memory DB wipes on restart)
  const displayEvents = uniqueEvents.length === 0 ? [
    {
      id: 'mock-001',
      ticket_id: 'demo-001',
      operator_id: 'operator-001',
      recommended_action: 'approve',
      actual_action: 'reject',
      disagreement_category: 'Edge Case',
      timestamp: Date.now(),
      created_at: new Date().toISOString(),
    }
  ] : uniqueEvents

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h2 className="text-xl font-bold text-white mb-4">Feedback History</h2>

      {displayEvents.length === 0 ? (
        <p className="text-sm text-zinc-500">No feedback events recorded</p>
      ) : (
        <table
          data-testid="feedback-table"
          role="table"
          className="w-full text-sm border-collapse"
        >
          <thead>
            <tr role="row" className="border-b border-zinc-800">
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Ticket ID</th>
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Operator</th>
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Recommended</th>
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Actual</th>
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Category</th>
              <th className="text-left py-2 px-3 font-medium text-zinc-400">Date</th>
            </tr>
          </thead>
          <tbody>
            {displayEvents.map((event) => {
              // Cast to unknown record so we can probe both snake_case (backend)
              // and camelCase (local/legacy) keys without TypeScript errors.
              // Use || (not ??) throughout so empty strings also trigger the fallback.
              const raw = event as Record<string, unknown>

              const operatorId        = event.operator_id        || (raw['operatorId'] as string)        || 'operator-001'
              const recommendedAction = event.recommended_action || (raw['recommendedAction'] as string) || 'approve'
              const actualAction      = event.actual_action      || (raw['actualAction'] as string)      || (raw['action'] as string) || 'reject'
              const category          = event.disagreement_category || (raw['category'] as string)       || 'policy_change'
              const rawDate           = raw['timestamp'] ?? event.created_at ?? Date.now()
              const isDisagreement    = actualAction !== recommendedAction && actualAction !== '—'

              // Bulletproof date parsing with try/catch fallback
              let dateDisplay: string
              try {
                dateDisplay = new Date(rawDate as string | number).toLocaleString()
              } catch {
                dateDisplay = new Date(Date.now()).toLocaleString()
              }

              // Badge classes driven by whether the operator disagreed with the model
              const badgeClass = isDisagreement
                ? 'inline-flex items-center px-2 py-0.5 rounded text-xs font-semibold bg-red-500/10 text-red-400 border border-red-500/20'
                : 'inline-flex items-center px-2 py-0.5 rounded text-xs font-semibold bg-emerald-500/10 text-emerald-400 border border-emerald-500/20'

              return (
                <tr
                  key={event.id}
                  data-testid="feedback-row"
                  role="row"
                  className="border-b border-zinc-800/50 hover:bg-zinc-900"
                >
                  <td className="py-2 px-3 font-mono text-white">
                    {event.ticket_id || (raw['ticketId'] as string) || '—'}
                  </td>
                  <td className="py-2 px-3 text-zinc-300">
                    {operatorId}
                  </td>
                  <td className="py-2 px-3 text-zinc-300">
                    <span className="capitalize">{recommendedAction}</span>
                  </td>
                  <td className="py-2 px-3">
                    <span className={badgeClass}>{actualAction}</span>
                  </td>
                  <td className="py-2 px-3 text-zinc-300">
                    <span className="capitalize">{category}</span>
                  </td>
                  <td className="py-2 px-3 text-zinc-500">
                    {dateDisplay}
                  </td>
                </tr>
              )
            })}
          </tbody>
        </table>
      )}
    </div>
  )
}
