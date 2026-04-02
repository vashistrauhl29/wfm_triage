import { useState, useEffect, useCallback, useRef } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { Activity, TrendingUp, Zap } from 'lucide-react'
import { TicketQueue } from './components/TicketQueue'
import { RouteStatus } from './components/RouteStatus'
import { ConfidenceGauge } from './components/ConfidenceGauge'
import { useThreshold } from '../../context/ThresholdContext'
import { SkeletonLoader } from '../../components/SkeletonLoader'
import type { Ticket } from '../../types/ticket'
import type { QueueStatus } from '../../types/router'

const BACKEND = import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'
const SSE_URL = `${BACKEND}/api/v1/stream/queue`

export function RouterDashboard() {
  const { threshold } = useThreshold()
  const [tickets, setTickets] = useState<Ticket[]>([])
  const [latestConfidence, setLatestConfidence] = useState(0)
  const [queueStatus, setQueueStatus] = useState<QueueStatus>({
    stp_queue_size: 0,
    human_queue_size: 0,
    total_processed: 0,
    stp_rate: 0,
  })
  const [connectionError, setConnectionError] = useState(false)
  const [isLoading, setIsLoading] = useState(true)

  // Mutable counters that survive across onmessage calls inside the same
  // render-cycle batch (multiple dispatchMessage calls inside a single act()).
  const countersRef = useRef({ stp: 0, human: 0, total: 0 })
  const errorTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null)
  const thresholdRef = useRef(threshold)

  // Keep ref in sync so handleTicket always sees the latest threshold
  // without needing to be recreated on every threshold change.
  useEffect(() => {
    thresholdRef.current = threshold
    // Reset counters whenever threshold changes so STP rate recalculates cleanly
    countersRef.current = { stp: 0, human: 0, total: 0 }
    setQueueStatus({ stp_queue_size: 0, human_queue_size: 0, total_processed: 0, stp_rate: 0 })
  }, [threshold])

  const handleTicket = useCallback((ticket: Ticket) => {
    // Re-classify using the frontend threshold so the gauge and counters
    // immediately reflect user changes even before the backend SSE batch regenerates.
    const decision: Ticket['routing_decision'] =
      ticket.confidence_score >= thresholdRef.current ? 'stp' : 'human_queue'
    const classified: Ticket = { ...ticket, routing_decision: decision }

    setTickets((prev) => [...prev, classified])
    setLatestConfidence(classified.confidence_score)
    setConnectionError(false)
    setIsLoading(false)

    const c = countersRef.current
    c.total += 1
    if (decision === 'stp') {
      c.stp += 1
    } else {
      c.human += 1
    }

    setQueueStatus({
      stp_queue_size: c.stp,
      human_queue_size: c.human,
      total_processed: c.total,
      stp_rate: c.total > 0 ? c.stp / c.total : 0,
    })
  }, [])

  useEffect(() => {
    const source = new EventSource(SSE_URL)

    source.onmessage = (event: MessageEvent) => {
      const ticket = JSON.parse(event.data) as Ticket
      handleTicket(ticket)
    }

    source.onerror = () => {
      if (source.readyState === EventSource.CONNECTING) {
        // Normal auto-reconnect after batch closes — delay banner by 3 s
        if (errorTimerRef.current) clearTimeout(errorTimerRef.current)
        errorTimerRef.current = setTimeout(() => {
          if (source.readyState !== EventSource.OPEN) {
            setConnectionError(true)
          }
        }, 3000)
      } else {
        // CLOSED or mock OPEN error — show banner immediately
        if (errorTimerRef.current) {
          clearTimeout(errorTimerRef.current)
          errorTimerRef.current = null
        }
        setConnectionError(true)
      }
    }

    source.onopen = () => {
      if (errorTimerRef.current) {
        clearTimeout(errorTimerRef.current)
        errorTimerRef.current = null
      }
      setConnectionError(false)
    }

    return () => {
      source.close()
      if (errorTimerRef.current) clearTimeout(errorTimerRef.current)
    }
  }, [handleTicket])

  return (
    <div className="min-h-screen bg-black p-6">
      {/* Header with Stats */}
      <motion.div
        initial={{ opacity: 0, y: -10 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.4 }}
        className="mb-8"
      >
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-primary-600 flex items-center justify-center">
              <Activity className="w-5 h-5 text-white" strokeWidth={2} />
            </div>
            <div>
              <h1 className="text-2xl font-bold text-white tracking-tight">
                Live HITL Router
              </h1>
              <p className="text-sm text-zinc-400 mt-0.5">
                Real-time ticket classification & routing
              </p>
            </div>
          </div>

          {/* Quick Stats */}
          <div className="flex items-center gap-4">
            <motion.div
              initial={{ scale: 0.9, opacity: 0 }}
              animate={{ scale: 1, opacity: 1 }}
              transition={{ delay: 0.2 }}
              className="flex items-center gap-2 px-4 py-2 rounded-lg bg-surface-light border border-zinc-800"
            >
              <Zap className="w-4 h-4 text-primary-400" />
              <span className="text-xs text-zinc-400">Processing</span>
              <span className="text-sm font-bold text-white">
                {queueStatus.total_processed}
              </span>
            </motion.div>
            <motion.div
              initial={{ scale: 0.9, opacity: 0 }}
              animate={{ scale: 1, opacity: 1 }}
              transition={{ delay: 0.3 }}
              className="flex items-center gap-2 px-4 py-2 rounded-lg bg-surface-light border border-zinc-800"
            >
              <TrendingUp className="w-4 h-4 text-emerald-400" />
              <span className="text-xs text-zinc-400">STP Rate</span>
              <span className="text-sm font-bold text-emerald-400">
                {(queueStatus.stp_rate * 100).toFixed(1)}%
              </span>
            </motion.div>
          </div>
        </div>
      </motion.div>

      {/* Error Banner */}
      <AnimatePresence>
        {connectionError && (
          <motion.div
            initial={{ opacity: 0, height: 0, marginBottom: 0 }}
            animate={{ opacity: 1, height: 'auto', marginBottom: 24 }}
            exit={{ opacity: 0, height: 0, marginBottom: 0 }}
            transition={{ duration: 0.3, ease: 'easeInOut' }}
            className="rounded-lg border border-red-900/50 bg-red-950/20 p-4 text-sm text-red-400 flex items-center gap-3"
          >
            <div className="w-2 h-2 rounded-full bg-red-500 animate-pulse" />
            <span className="font-medium">Connection lost</span>
            <span className="text-red-400/70">— Attempting to reconnect...</span>
          </motion.div>
        )}
      </AnimatePresence>

      {/* Main Content */}
      <motion.div
        initial={{ opacity: 0, y: 20 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.5, delay: 0.1 }}
        className="grid grid-cols-1 lg:grid-cols-3 gap-6"
      >
        {/* Ticket Queue */}
        <div className="lg:col-span-2">
          {isLoading && tickets.length === 0 ? (
            <SkeletonLoader variant="ticket" count={5} />
          ) : (
            <TicketQueue tickets={tickets} />
          )}
        </div>

        {/* Sidebar */}
        <div className="space-y-6">
          <RouteStatus queueStatus={queueStatus} />
          <ConfidenceGauge
            confidenceScore={latestConfidence}
            threshold={threshold}
          />
        </div>
      </motion.div>
    </div>
  )
}
