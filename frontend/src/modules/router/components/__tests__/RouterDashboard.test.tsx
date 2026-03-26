import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, act } from '@testing-library/react'
import { RouterDashboard } from '../../RouterDashboard'

// ─── Mock child components ───────────────────────────────────────────────────
vi.mock('../TicketQueue', () => ({
  TicketQueue: ({ tickets }: { tickets: unknown[] }) => (
    <div data-testid="ticket-queue" data-count={tickets.length}>
      TicketQueue
    </div>
  ),
}))

vi.mock('../RouteStatus', () => ({
  RouteStatus: ({ queueStatus }: { queueStatus: unknown }) => (
    <div data-testid="route-status" data-status={JSON.stringify(queueStatus)}>
      RouteStatus
    </div>
  ),
}))

vi.mock('../ConfidenceGauge', () => ({
  ConfidenceGauge: ({
    confidenceScore,
    threshold,
  }: {
    confidenceScore: number
    threshold: number
  }) => (
    <div
      data-testid="confidence-gauge"
      data-score={confidenceScore}
      data-threshold={threshold}
    >
      ConfidenceGauge
    </div>
  ),
}))

// ─── EventSource mock ────────────────────────────────────────────────────────
class MockEventSource {
  static CONNECTING = 0
  static OPEN = 1
  static CLOSED = 2

  url: string
  readyState: number = MockEventSource.CONNECTING
  onmessage: ((event: MessageEvent) => void) | null = null
  onerror: ((event: Event) => void) | null = null
  onopen: ((event: Event) => void) | null = null

  private listeners: Map<string, EventListener[]> = new Map()

  constructor(url: string) {
    this.url = url
    MockEventSource.instances.push(this)
    // Immediately open
    this.readyState = MockEventSource.OPEN
  }

  addEventListener(type: string, listener: EventListener) {
    if (!this.listeners.has(type)) {
      this.listeners.set(type, [])
    }
    this.listeners.get(type)!.push(listener)
  }

  removeEventListener(type: string, listener: EventListener) {
    const existing = this.listeners.get(type) ?? []
    this.listeners.set(
      type,
      existing.filter((l) => l !== listener)
    )
  }

  dispatchMessage(data: unknown) {
    const event = new MessageEvent('message', { data: JSON.stringify(data) })
    this.onmessage?.(event)
    this.listeners.get('message')?.forEach((l) => l(event as unknown as Event))
  }

  dispatchError() {
    const event = new Event('error')
    this.onerror?.(event)
    this.listeners.get('error')?.forEach((l) => l(event))
  }

  close() {
    this.readyState = MockEventSource.CLOSED
  }

  // ─── Test helpers ───────────────────────────────────────────────────────
  static instances: MockEventSource[] = []
  static reset() {
    MockEventSource.instances = []
  }
  static latest(): MockEventSource {
    return MockEventSource.instances[MockEventSource.instances.length - 1]
  }
}

// ─── Test suite ──────────────────────────────────────────────────────────────
describe('RouterDashboard', () => {
  beforeEach(() => {
    MockEventSource.reset()
    vi.stubGlobal('EventSource', MockEventSource)
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Static rendering ────────────────────────────────────────────────────
  describe('initial render', () => {
    it('renders the dashboard heading', () => {
      render(<RouterDashboard />)
      expect(
        screen.getByRole('heading', { name: /live hitl router/i })
      ).toBeInTheDocument()
    })

    it('renders the TicketQueue child component', () => {
      render(<RouterDashboard />)
      expect(screen.getByTestId('ticket-queue')).toBeInTheDocument()
    })

    it('renders the RouteStatus child component', () => {
      render(<RouterDashboard />)
      expect(screen.getByTestId('route-status')).toBeInTheDocument()
    })

    it('renders the ConfidenceGauge child component', () => {
      render(<RouterDashboard />)
      expect(screen.getByTestId('confidence-gauge')).toBeInTheDocument()
    })

    it('passes an empty ticket list to TicketQueue on mount', () => {
      render(<RouterDashboard />)
      expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
        'data-count',
        '0'
      )
    })

    it('passes a default confidence score to ConfidenceGauge on mount', () => {
      render(<RouterDashboard />)
      const gauge = screen.getByTestId('confidence-gauge')
      const score = parseFloat(gauge.getAttribute('data-score') ?? 'NaN')
      expect(score).toBeGreaterThanOrEqual(0)
      expect(score).toBeLessThanOrEqual(1)
    })

    it('passes a default threshold to ConfidenceGauge on mount', () => {
      render(<RouterDashboard />)
      const gauge = screen.getByTestId('confidence-gauge')
      const threshold = parseFloat(
        gauge.getAttribute('data-threshold') ?? 'NaN'
      )
      expect(threshold).toBeGreaterThan(0)
      expect(threshold).toBeLessThanOrEqual(1)
    })
  })

  // ── SSE lifecycle ───────────────────────────────────────────────────────
  describe('Server-Sent Events lifecycle', () => {
    it('creates an EventSource connection on mount', () => {
      render(<RouterDashboard />)
      expect(MockEventSource.instances).toHaveLength(1)
    })

    it('connects to the queue SSE endpoint', () => {
      render(<RouterDashboard />)
      expect(MockEventSource.latest().url).toMatch(/\/api\/v1\/stream\/queue/)
    })

    it('closes the EventSource connection on unmount', () => {
      const { unmount } = render(<RouterDashboard />)
      const source = MockEventSource.latest()
      unmount()
      expect(source.readyState).toBe(MockEventSource.CLOSED)
    })

    it('does not create more than one EventSource per mount', () => {
      const { unmount } = render(<RouterDashboard />)
      unmount()
      render(<RouterDashboard />)
      // Each mount should create exactly one connection; two mounts → two instances
      expect(MockEventSource.instances).toHaveLength(2)
    })
  })

  // ── Real-time ticket updates ────────────────────────────────────────────
  describe('real-time ticket stream updates', () => {
    it('adds a ticket to TicketQueue when an SSE message arrives', async () => {
      render(<RouterDashboard />)

      expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
        'data-count',
        '0'
      )

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.88,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
          'data-count',
          '1'
        )
      })
    })

    it('accumulates multiple SSE tickets without losing earlier ones', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.88,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-002',
          ticket_type: 'document_verification',
          confidence_score: 0.72,
          routing_decision: 'human_queue',
          raw_data: '{}',
          created_at: '2026-03-24T08:01:00Z',
        })
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-003',
          ticket_type: 'safety_flag',
          confidence_score: 0.95,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:02:00Z',
        })
      })

      await waitFor(() => {
        expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
          'data-count',
          '3'
        )
      })
    })

    it('updates the ConfidenceGauge score from the latest SSE ticket', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.92,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        const gauge = screen.getByTestId('confidence-gauge')
        expect(gauge.getAttribute('data-score')).toBe('0.92')
      })
    })

    it('updates RouteStatus queue counts when an STP ticket arrives', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.97,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        const statusEl = screen.getByTestId('route-status')
        const status = JSON.parse(statusEl.getAttribute('data-status') ?? '{}')
        expect(status.stp_queue_size).toBeGreaterThan(0)
      })
    })

    it('updates RouteStatus queue counts when a human-queue ticket arrives', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'document_verification',
          confidence_score: 0.65,
          routing_decision: 'human_queue',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        const statusEl = screen.getByTestId('route-status')
        const status = JSON.parse(statusEl.getAttribute('data-status') ?? '{}')
        expect(status.human_queue_size).toBeGreaterThan(0)
      })
    })

    it('increments total_processed on every SSE ticket', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.88,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-002',
          ticket_type: 'safety_flag',
          confidence_score: 0.91,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:01:00Z',
        })
      })

      await waitFor(() => {
        const statusEl = screen.getByTestId('route-status')
        const status = JSON.parse(statusEl.getAttribute('data-status') ?? '{}')
        expect(status.total_processed).toBe(2)
      })
    })

    it('recalculates stp_rate correctly after mixed routing decisions', async () => {
      render(<RouterDashboard />)

      act(() => {
        // 2 STP + 1 human = stp_rate 0.67 (rounded)
        MockEventSource.latest().dispatchMessage({
          id: 't1',
          ticket_type: 'safety_flag',
          confidence_score: 0.9,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
        MockEventSource.latest().dispatchMessage({
          id: 't2',
          ticket_type: 'safety_flag',
          confidence_score: 0.95,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:01:00Z',
        })
        MockEventSource.latest().dispatchMessage({
          id: 't3',
          ticket_type: 'document_verification',
          confidence_score: 0.6,
          routing_decision: 'human_queue',
          raw_data: '{}',
          created_at: '2026-03-24T08:02:00Z',
        })
      })

      await waitFor(() => {
        const statusEl = screen.getByTestId('route-status')
        const status = JSON.parse(statusEl.getAttribute('data-status') ?? '{}')
        expect(status.stp_rate).toBeCloseTo(2 / 3, 5)
      })
    })
  })

  // ── SSE error handling ──────────────────────────────────────────────────
  describe('SSE error handling', () => {
    it('shows a connection error indicator when EventSource fires an error', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchError()
      })

      await waitFor(() => {
        expect(
          screen.getByText(/connection error|reconnecting|stream error/i)
        ).toBeInTheDocument()
      })
    })

    it('continues to display existing tickets after an SSE error', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.88,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
          'data-count',
          '1'
        )
      })

      act(() => {
        MockEventSource.latest().dispatchError()
      })

      // Tickets must not disappear after an SSE error
      expect(screen.getByTestId('ticket-queue')).toHaveAttribute(
        'data-count',
        '1'
      )
    })

    it('clears the connection error indicator when a new message arrives', async () => {
      render(<RouterDashboard />)

      act(() => {
        MockEventSource.latest().dispatchError()
      })

      await waitFor(() => {
        expect(
          screen.getByText(/connection error|reconnecting|stream error/i)
        ).toBeInTheDocument()
      })

      act(() => {
        MockEventSource.latest().dispatchMessage({
          id: 'ticket-sse-001',
          ticket_type: 'safety_flag',
          confidence_score: 0.88,
          routing_decision: 'stp',
          raw_data: '{}',
          created_at: '2026-03-24T08:00:00Z',
        })
      })

      await waitFor(() => {
        expect(
          screen.queryByText(/connection error|reconnecting|stream error/i)
        ).not.toBeInTheDocument()
      })
    })
  })
})
