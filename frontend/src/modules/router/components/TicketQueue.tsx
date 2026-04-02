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
        className="flex items-center justify-center p-8 text-zinc-500"
      >
        <p>No pending tickets</p>
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
              scale: 1.02,
              transition: { duration: 0.2 },
            }}
            className="border border-zinc-800 rounded-lg p-4 bg-[#0A0A0A] hover:bg-zinc-900 hover:border-zinc-700 transition-colors cursor-pointer"
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
                    <motion.span
                      initial={{ scale: 0.8 }}
                      animate={{ scale: 1 }}
                      transition={{ delay: 0.1, type: 'spring', stiffness: 200 }}
                      className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-emerald-500/10 text-emerald-400 border border-emerald-500/20"
                    >
                      STP
                    </motion.span>
                  ) : (
                    <motion.span
                      initial={{ scale: 0.8 }}
                      animate={{ scale: 1 }}
                      transition={{ delay: 0.1, type: 'spring', stiffness: 200 }}
                      className="inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium bg-orange-500/10 text-orange-400 border border-orange-500/20"
                    >
                      Human Queue
                    </motion.span>
                  )}
                </div>
              </div>

              <div className="text-xs text-zinc-500">
                {new Date(ticket.created_at).toLocaleTimeString()}
              </div>
            </div>
          </motion.li>
        ))}
      </AnimatePresence>
    </ul>
  )
}
