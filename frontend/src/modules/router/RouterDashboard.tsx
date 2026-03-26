import { useState, useEffect, useCallback, useRef } from 'react'
import { TicketQueue } from './components/TicketQueue'
import { RouteStatus } from './components/RouteStatus'
import { ConfidenceGauge } from './components/ConfidenceGauge'
import type { Ticket } from '../../types/ticket'
import type { QueueStatus } from '../../types/router'

const SSE_BASE = import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'
const SSE_URL = `${SSE_BASE}/api/v1/stream/queue`
const DEFAULT_THRESHOLD = 0.95

export function RouterDashboard() {
  const [tickets, setTickets] = useState<Ticket[]>([])
  const [latestConfidence, setLatestConfidence] = useState(0)
  const [queueStatus, setQueueStatus] = useState<QueueStatus>({
    stp_queue_size: 0,
    human_queue_size: 0,
    total_processed: 0,
    stp_rate: 0,
  })
  const [connectionError, setConnectionError] = useState(false)

  // Mutable counters that survive across onmessage calls inside the same
  // render-cycle batch (multiple dispatchMessage calls inside a single act()).
  const countersRef = useRef({ stp: 0, human: 0, total: 0 })
  const errorTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null)

  const handleTicket = useCallback((ticket: Ticket) => {
    setTickets((prev) => [...prev, ticket])
    setLatestConfidence(ticket.confidence_score)
    setConnectionError(false)

    const c = countersRef.current
    c.total += 1
    if (ticket.routing_decision === 'stp') {
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
    <div className="min-h-screen bg-black p-6 animate-fade-in-up">
      <h1 className="text-2xl font-bold text-white mb-6">
        Live HITL Router
      </h1>

      {connectionError && (
        <div className="mb-4 rounded-md border border-red-900/50 p-3 text-sm text-red-400">
          Connection error — attempting to reconnect...
        </div>
      )}

      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        <div className="lg:col-span-2">
          <TicketQueue tickets={tickets} />
        </div>

        <div className="space-y-6">
          <RouteStatus queueStatus={queueStatus} />
          <ConfidenceGauge
            confidenceScore={latestConfidence}
            threshold={DEFAULT_THRESHOLD}
          />
        </div>
      </div>
    </div>
  )
}
