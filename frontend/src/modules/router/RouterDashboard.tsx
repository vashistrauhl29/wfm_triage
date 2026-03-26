import { useState, useEffect, useCallback, useRef } from 'react'
import { TicketQueue } from './components/TicketQueue'
import { RouteStatus } from './components/RouteStatus'
import { ConfidenceGauge } from './components/ConfidenceGauge'
import type { Ticket } from '../../types/ticket'
import type { QueueStatus } from '../../types/router'

const BACKEND = import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'
const SSE_URL = `${BACKEND}/api/v1/stream/queue`

const readThreshold = () => parseFloat(localStorage.getItem('wfm_threshold') || '0.95')

// Fire-and-forget: tell the backend to use the new threshold so the SSE
// stream's routing_decision labels stay consistent with the frontend gauge.
const syncThresholdToBackend = (t: number) => {
  fetch(`${BACKEND}/api/v1/router/threshold`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ threshold: t }),
  }).catch(() => { /* backend unreachable — SSE labels may lag, frontend gauge is authoritative */ })
}

export function RouterDashboard() {
  const [tickets, setTickets] = useState<Ticket[]>([])
  const [latestConfidence, setLatestConfidence] = useState(0)
  const [threshold, setThreshold] = useState(readThreshold)
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
  const thresholdRef = useRef(threshold)

  // Keep ref in sync so handleTicket always sees the latest threshold
  // without needing to be recreated on every threshold change.
  useEffect(() => {
    thresholdRef.current = threshold
  }, [threshold])

  // Subscribe to localStorage changes made by the Simulator tab.
  useEffect(() => {
    const onStorage = (e: StorageEvent) => {
      if (e.key === 'wfm_threshold' && e.newValue) {
        const t = parseFloat(e.newValue)
        if (!isNaN(t)) {
          setThreshold(t)
          syncThresholdToBackend(t)
          // Reset counters so the STP rate reflects the new threshold
          countersRef.current = { stp: 0, human: 0, total: 0 }
          setQueueStatus({ stp_queue_size: 0, human_queue_size: 0, total_processed: 0, stp_rate: 0 })
        }
      }
    }
    window.addEventListener('storage', onStorage)
    // Sync current value to backend on mount
    syncThresholdToBackend(threshold)
    return () => window.removeEventListener('storage', onStorage)
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const handleTicket = useCallback((ticket: Ticket) => {
    // Re-classify using the frontend threshold so the gauge and counters
    // immediately reflect user changes even before the backend SSE batch regenerates.
    const decision: Ticket['routing_decision'] =
      ticket.confidence_score >= thresholdRef.current ? 'stp' : 'human_queue'
    const classified: Ticket = { ...ticket, routing_decision: decision }

    setTickets((prev) => [...prev, classified])
    setLatestConfidence(classified.confidence_score)
    setConnectionError(false)

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
            threshold={threshold}
          />
        </div>
      </div>
    </div>
  )
}
