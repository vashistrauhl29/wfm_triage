import React, { useState, useEffect, useRef } from 'react'
import { TicketDetail } from './components/TicketDetail'
import { SOPDisplay } from './components/SOPDisplay'
import { ActionButtons } from './components/ActionButtons'
import type { TicketWithContext } from '../../types/ticket'

const API_BASE = `${import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'}/api/v1/copilot`

// ── Local demo queue ──────────────────────────────────────────────────────────
// These are displayed whenever the backend is unavailable (404, network error,
// in-memory DB wipe). All fields required by TicketWithContext are filled so
// child components never receive undefined props.
const DEMO_QUEUE: TicketWithContext[] = [
  {
    id: 'demo-001',
    ticket_type: 'safety_flag',
    confidence_score: 0.82,
    routing_decision: 'human_queue',
    raw_data: '{}',
    created_at: new Date().toISOString(),
    summary: [
      'Passenger complaint flagged for aggressive driving',
      'Third incident in 90-day rolling window',
      'GPS trace shows 3 hard-brake events on trip',
    ],
    relevant_sops: [],
    action_recommendation: 'Issue formal warning and schedule mandatory safety review.',
    confidence_override_threshold: 0.95,
  },
  {
    id: 'demo-002',
    ticket_type: 'payment_dispute',
    confidence_score: 0.74,
    routing_decision: 'human_queue',
    raw_data: '{}',
    created_at: new Date().toISOString(),
    summary: [
      'Rider disputes $14.20 surge charge',
      'Trip completed normally — no route deviation detected',
      'Surge pricing was active at trip start (2.1×)',
    ],
    relevant_sops: [],
    action_recommendation: 'Confirm surge was correctly applied and close dispute.',
    confidence_override_threshold: 0.95,
  },
  {
    id: 'demo-003',
    ticket_type: 'document_verification',
    confidence_score: 0.68,
    routing_decision: 'human_queue',
    raw_data: '{}',
    created_at: new Date().toISOString(),
    summary: [
      'Driver license expiry detected within 30 days',
      'Background check status: pending renewal',
      'Address verification required',
    ],
    relevant_sops: [],
    action_recommendation: 'Request updated documentation from driver before approving.',
    confidence_override_threshold: 0.95,
  },
]

interface CopilotViewProps {
  ticketId: string
}

export const CopilotView: React.FC<CopilotViewProps> = ({ ticketId }) => {
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const [ticket, setTicket] = useState<any>(null)
  const [queueIndex, setQueueIndex] = useState(0)
  const [elapsedSeconds, setElapsedSeconds] = useState(0)
  const [loading, setLoading] = useState(true)
  const [actionFeedback, setActionFeedback] = useState<string | null>(null)

  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null)

  // ── Defensive fetch ───────────────────────────────────────────────────────
  // Any failure (network, 404, empty body) immediately falls back to
  // DEMO_QUEUE[queueIndex] so the UI never renders null/NaN state.
  useEffect(() => {
    let cancelled = false
    setLoading(true)

    ;(async () => {
      try {
        const res = await fetch(`${API_BASE}/ticket/${ticketId}`)
        if (!res.ok) throw new Error(`HTTP ${res.status}`)
        const json = await res.json()
        const data = json?.data ?? json
        if (!data?.id) throw new Error('Empty response')
        if (!cancelled) setTicket(data)
      } catch (err) {
        console.warn('[CopilotView] Backend unavailable, using demo queue:', err)
        if (!cancelled) setTicket(DEMO_QUEUE[queueIndex])
      } finally {
        if (!cancelled) setLoading(false)
      }
    })()

    return () => { cancelled = true }
  // queueIndex intentionally omitted — fetch only re-runs when ticketId changes
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ticketId])

  // ── Cognitive Load Timer + global ticket ID sync ─────────────────────────
  // Resets and restarts whenever a new ticket is displayed.
  // Also writes the active ticket ID to localStorage so other tabs (RLHF) can read it.
  useEffect(() => {
    if (!ticket?.id) return

    localStorage.setItem('wfm_current_ticket', ticket.id)
    setElapsedSeconds(0)
    if (timerRef.current) clearInterval(timerRef.current)

    timerRef.current = setInterval(() => {
      setElapsedSeconds((s) => s + 1)
    }, 1000)

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current)
        timerRef.current = null
      }
    }
  }, [ticket?.id])

  // ── Action handler ────────────────────────────────────────────────────────
  // Stops the timer, advances the queue, and loads the next demo ticket.
  // Does NOT block on the API — the queue cycles immediately so buttons
  // are always responsive even when the backend is down.
  const handleAction = (action: string) => {
    // Stop timer and capture elapsed for logging
    if (timerRef.current) {
      clearInterval(timerRef.current)
      timerRef.current = null
    }
    console.log(`[Copilot] action=${action} ticket=${ticket?.id ?? '--'} elapsed=${elapsedSeconds}s`)

    // Fire-and-forget API call — we don't await so UX is never blocked
    fetch(`${API_BASE}/action`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ticket_id: ticket?.id, action, timestamp: new Date().toISOString() }),
    }).catch(() => { /* backend unavailable — silently ignored */ })

    // Advance queue and reset state
    const nextIndex = (queueIndex + 1) % DEMO_QUEUE.length
    setQueueIndex(nextIndex)
    setTicket(DEMO_QUEUE[nextIndex])
    setElapsedSeconds(0)

    // Flash feedback banner for 1.2s
    setActionFeedback(action)
    setTimeout(() => setActionFeedback(null), 1200)
  }

  // ── Global threshold from localStorage (set by SimulatorView) ───────────
  // Falls back to 0.95 if the user has never moved the Simulator slider.
  const globalThreshold = parseFloat(localStorage.getItem('wfm_threshold') || '0.95')

  // ── Safe ticket values for rendering ─────────────────────────────────────
  // Optional chaining + logical-OR defaults ensure zero NaN/blank renders.
  // Inject globalThreshold so TicketDetail always reflects the Simulator value.
  const safeTicket: TicketWithContext = {
    ...(ticket ?? DEMO_QUEUE[0]),
    confidence_override_threshold: globalThreshold,
  }
  const confidencePct = ((ticket?.confidence_score ?? 0) * 100).toFixed(2)
  const ticketId_display = ticket?.id || '--'

  return (
    <div className="p-6 space-y-6 animate-fade-in-up">

      {/* ── Header ── */}
      <div className="flex items-center justify-between">
        <h1 className="text-2xl font-bold text-white">Operator Copilot</h1>
        <span className="text-xs font-mono text-zinc-600 tabular-nums">
          {ticketId_display}
        </span>
      </div>

      {/* ── Loading indicator ── */}
      {loading && (
        <div className="bg-[#0A0A0A] border border-zinc-800 rounded-lg px-4 py-3 text-sm text-zinc-500">
          Fetching ticket…
        </div>
      )}

      {/* ── Action feedback banner ── */}
      {actionFeedback && (
        <div className="bg-emerald-950/40 border border-emerald-700/40 rounded-lg px-4 py-3 text-sm font-semibold text-emerald-400">
          ✓ {actionFeedback.charAt(0).toUpperCase() + actionFeedback.slice(1)} submitted — loading next ticket
        </div>
      )}

      {/* ── Ticket metadata card ── */}
      <TicketDetail ticket={safeTicket} />

      {/* ── Recommendation card (with timer inlined in header) ── */}
      <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6 space-y-4">

        {/* Card header: title left, cognitive load timer right */}
        <div className="flex items-center justify-between">
          <h3 className="text-lg font-semibold text-white">Recommendation</h3>
          <span className="text-emerald-400 font-mono text-sm">⏱ Active: {elapsedSeconds}s</span>
        </div>

        {/* Confidence callout */}
        <div className="flex items-center gap-3 bg-zinc-900/60 border border-zinc-800 rounded-md px-4 py-2">
          <span className="text-sm text-zinc-400">Confidence</span>
          <span className="font-mono text-sm font-semibold text-white">
            {confidencePct}%
          </span>
        </div>

        {/* Context summary bullets */}
        <div>
          <p className="text-sm font-medium text-zinc-300 mb-2">Context Summary</p>
          <ul className="space-y-2 list-none">
            {(safeTicket.summary ?? []).map((bullet, i) => (
              <li key={i} className="flex items-start gap-2 text-sm text-zinc-400">
                <span className="text-zinc-600 shrink-0 mt-0.5">•</span>
                <span>{bullet}</span>
              </li>
            ))}
          </ul>
        </div>

        {/* Recommended action */}
        <div>
          <p className="text-sm font-medium text-zinc-300 mb-2">Recommended Action</p>
          <div
            data-testid="recommendation-action"
            className="bg-blue-950/30 border border-blue-900/50 rounded-md p-3 text-sm font-medium text-blue-300"
          >
            {safeTicket.action_recommendation || 'No recommendation available.'}
          </div>
        </div>
      </div>

      {/* ── SOPs (if any) ── */}
      {(safeTicket.relevant_sops ?? []).map((sop) => (
        <SOPDisplay key={sop.id} sop={sop} searchTerms={safeTicket.summary} />
      ))}

      {/* ── Action buttons ── */}
      <ActionButtons
        onApprove={() => handleAction('approve')}
        onReject={() => handleAction('reject')}
        onEscalate={() => handleAction('escalate')}
        disabled={loading}
      />
    </div>
  )
}
