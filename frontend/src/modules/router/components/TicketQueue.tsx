import React from 'react'
import type { Ticket } from '../../../types/ticket'

interface TicketQueueProps {
  tickets: Ticket[]
}

export const TicketQueue: React.FC<TicketQueueProps> = ({ tickets }) => {
  // Sort tickets by created_at (newest first)
  const sortedTickets = React.useMemo(() => {
    return [...tickets].sort((a, b) => {
      return new Date(b.created_at).getTime() - new Date(a.created_at).getTime()
    })
  }, [tickets])

  if (tickets.length === 0) {
    return (
      <div className="flex items-center justify-center p-8 text-zinc-500">
        <p>No pending tickets</p>
      </div>
    )
  }

  return (
    <ul className="space-y-2">
      {sortedTickets.map((ticket) => (
        <li
          key={ticket.id}
          role="listitem"
          className="border border-zinc-800 rounded-lg p-4 bg-[#0A0A0A] hover:bg-zinc-900 transition-colors"
        >
          <div className="flex items-center justify-between">
            <div className="flex-1">
              <div className="flex items-center gap-3">
                <span className="font-mono font-semibold text-white">
                  {ticket.id}
                </span>
                <span className="text-sm text-zinc-400">
                  {ticket.ticket_type}
                </span>
              </div>

              <div className="mt-2 flex items-center gap-4">
                <span className="text-sm font-medium text-zinc-300">
                  {(ticket.confidence_score * 100).toFixed(2)}%
                </span>

                {ticket.routing_decision === 'stp' ? (
                  <span className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-green-100 text-green-800">
                    STP
                  </span>
                ) : (
                  <span className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-orange-100 text-orange-800">
                    Human Queue
                  </span>
                )}
              </div>
            </div>

            <div className="text-xs text-zinc-500">
              {new Date(ticket.created_at).toLocaleTimeString()}
            </div>
          </div>
        </li>
      ))}
    </ul>
  )
}
