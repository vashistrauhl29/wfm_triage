import React from 'react'
import type { TicketWithContext } from '../../../types/ticket'

interface TicketDetailProps {
  ticket: TicketWithContext
}

export const TicketDetail: React.FC<TicketDetailProps> = ({ ticket }) => {
  const confidencePct = (ticket.confidence_score * 100).toFixed(2)
  const barWidth = `${Math.round(ticket.confidence_score * 100)}%`
  const thresholdPct = (ticket.confidence_override_threshold * 100).toFixed(2)

  const barColor =
    ticket.confidence_score < 0.7
      ? 'bg-red-500'
      : ticket.confidence_score < 0.9
      ? 'bg-yellow-500'
      : 'bg-green-500'

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6 space-y-4">
      {/* Ticket ID */}
      <div className="flex justify-between items-center">
        <span className="text-sm text-zinc-400">Ticket ID</span>
        <span className="font-mono text-sm text-white">{ticket.id}</span>
      </div>

      {/* Ticket Type */}
      <div className="flex justify-between items-center">
        <span className="text-sm text-zinc-400">Type</span>
        <span className="text-sm text-white">{ticket.ticket_type}</span>
      </div>

      {/* Confidence Score */}
      <div>
        <div className="flex justify-between items-center mb-1">
          <span className="text-sm text-zinc-400">Confidence</span>
          <span className="text-sm font-medium text-white">{`${confidencePct}%`}</span>
        </div>
        <div className="w-full bg-zinc-800 rounded-full h-2">
          <div
            data-testid="confidence-bar"
            className={`${barColor} h-2 rounded-full`}
            style={{ width: barWidth }}
          />
        </div>
      </div>

      {/* Routing Decision */}
      <div className="flex justify-between items-center">
        <span className="text-sm text-zinc-400">Routing</span>
        <span
          data-testid="routing-badge"
          className="bg-orange-500/10 text-orange-400 border border-orange-500/20 px-2 py-1 rounded text-xs font-medium"
        >
          Human Queue
        </span>
      </div>

      {/* Timestamp */}
      <div className="flex justify-between items-center">
        <span className="text-sm text-zinc-400">Created</span>
        <span data-testid="ticket-timestamp" className="text-sm text-white">
          {ticket.created_at ? new Date(ticket.created_at).toLocaleString() : '—'}
        </span>
      </div>

      {/* STP Threshold */}
      <div className="flex justify-between items-center">
        <span className="text-sm text-zinc-400">Threshold</span>
        <span className="text-sm text-white">{`${thresholdPct}%`}</span>
      </div>
    </div>
  )
}
