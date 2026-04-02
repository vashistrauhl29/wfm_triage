import React from 'react'
import { motion, AnimatePresence } from 'framer-motion'
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
      <motion.div
        initial={{ opacity: 0 }}
        animate={{ opacity: 1 }}
        className="flex flex-col items-center justify-center p-12 text-center border border-zinc-800 rounded-lg bg-surface"
      >
        <div className="w-16 h-16 rounded-full bg-zinc-800/50 flex items-center justify-center mb-4">
          <svg className="w-8 h-8 text-zinc-600" fill="none" viewBox="0 0 24 24" stroke="currentColor">
            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={1.5} d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
          </svg>
        </div>
        <p className="text-zinc-400 font-medium mb-1">No tickets in queue</p>
        <p className="text-xs text-zinc-600">Waiting for incoming tickets...</p>
      </motion.div>
    )
  }

  return (
    <ul className="space-y-2">
      <AnimatePresence initial={false}>
        {sortedTickets.map((ticket, index) => (
          <motion.li
            key={ticket.id}
            role="listitem"
            initial={{ opacity: 0, y: -20, scale: 0.95 }}
            animate={{
              opacity: 1,
              y: 0,
              scale: 1,
              transition: {
                duration: 0.3,
                delay: Math.min(index * 0.05, 0.5), // Stagger animation
                ease: 'easeOut',
              },
            }}
            exit={{
              opacity: 0,
              x: 100,
              transition: { duration: 0.2 },
            }}
            whileHover={{
              scale: 1.01,
              y: -2,
              transition: { duration: 0.2 },
            }}
            className="border border-zinc-800 rounded-lg p-4 bg-gradient-to-br from-surface to-surface-light hover:border-zinc-700 transition-all cursor-pointer shadow-sm hover:shadow-premium"
          >
            <div className="flex items-center justify-between">
              <div className="flex-1">
                <div className="flex items-center gap-3 mb-3">
                  <span className="font-mono font-bold text-white text-sm tracking-tight">
                    {ticket.id}
                  </span>
                  <span className="px-2 py-0.5 rounded text-xs font-medium bg-zinc-800/50 text-zinc-400 border border-zinc-700/50">
                    {ticket.ticket_type}
                  </span>
                </div>

                <div className="flex items-center gap-4">
                  {/* Confidence Score with Visual Indicator */}
                  <div className="flex items-center gap-2">
                    <div className="relative w-12 h-1.5 bg-zinc-800 rounded-full overflow-hidden">
                      <motion.div
                        initial={{ width: 0 }}
                        animate={{ width: `${ticket.confidence_score * 100}%` }}
                        transition={{ duration: 0.5, delay: 0.2 }}
                        className={`h-full ${
                          ticket.confidence_score >= 0.9
                            ? 'bg-emerald-500'
                            : ticket.confidence_score >= 0.7
                            ? 'bg-yellow-500'
                            : 'bg-red-500'
                        }`}
                      />
                    </div>
                    <span className="text-xs font-bold text-zinc-300 tabular-nums">
                      {(ticket.confidence_score * 100).toFixed(1)}%
                    </span>
                  </div>

                  {/* Routing Decision Badge */}
                  {ticket.routing_decision === 'stp' ? (
                    <motion.span
                      initial={{ scale: 0.8, opacity: 0 }}
                      animate={{ scale: 1, opacity: 1 }}
                      transition={{ delay: 0.15, type: 'spring', stiffness: 200 }}
                      className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-md text-xs font-bold bg-emerald-500/10 text-emerald-400 border border-emerald-500/20 shadow-sm"
                    >
                      <div className="w-1.5 h-1.5 rounded-full bg-emerald-500" />
                      STP
                    </motion.span>
                  ) : (
                    <motion.span
                      initial={{ scale: 0.8, opacity: 0 }}
                      animate={{ scale: 1, opacity: 1 }}
                      transition={{ delay: 0.15, type: 'spring', stiffness: 200 }}
                      className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-md text-xs font-bold bg-orange-500/10 text-orange-400 border border-orange-500/20 shadow-sm"
                    >
                      <div className="w-1.5 h-1.5 rounded-full bg-orange-500 animate-pulse" />
                      Manual Review
                    </motion.span>
                  )}
                </div>
              </div>

              {/* Timestamp */}
              <div className="flex flex-col items-end gap-1">
                <div className="text-[11px] text-zinc-600 uppercase tracking-wide font-medium">
                  {new Date(ticket.created_at).toLocaleTimeString('en-US', {
                    hour: '2-digit',
                    minute: '2-digit',
                  })}
                </div>
                <div className="text-[10px] text-zinc-700">
                  {new Date(ticket.created_at).toLocaleDateString('en-US', {
                    month: 'short',
                    day: 'numeric',
                  })}
                </div>
              </div>
            </div>
          </motion.li>
        ))}
      </AnimatePresence>
    </ul>
  )
}
