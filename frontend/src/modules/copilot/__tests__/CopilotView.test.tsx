import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, act } from '@testing-library/react'
import { CopilotView } from '../CopilotView'
import type { TicketWithContext } from '../../../types/ticket'

// ─── Mock child components ────────────────────────────────────────────────────
vi.mock('../components/TicketDetail', () => ({
  TicketDetail: ({ ticket }: { ticket: TicketWithContext }) => (
    <div
      data-testid="ticket-detail"
      data-ticket-id={ticket.id}
      data-confidence={ticket.confidence_score}
    >
      TicketDetail
    </div>
  ),
}))

vi.mock('../components/SOPDisplay', () => ({
  SOPDisplay: ({
    sop,
    searchTerms,
  }: {
    sop: { id: string; title: string; relevance_score: number }
    searchTerms?: string[]
  }) => (
    <div
      data-testid="sop-display"
      data-sop-id={sop.id}
      data-search-terms={searchTerms?.join(',') ?? ''}
    >
      {sop.title}
    </div>
  ),
}))

vi.mock('../components/RecommendationCard', () => ({
  RecommendationCard: ({
    summary,
    recommendation,
  }: {
    summary: string[]
    recommendation: string
  }) => (
    <div
      data-testid="recommendation-card"
      data-bullets={summary.length}
      data-recommendation={recommendation}
    >
      RecommendationCard
    </div>
  ),
}))

vi.mock('../components/ActionButtons', () => ({
  ActionButtons: ({
    onApprove,
    onReject,
    onEscalate,
    disabled,
  }: {
    onApprove: () => void
    onReject: () => void
    onEscalate: () => void
    disabled?: boolean
  }) => (
    <div data-testid="action-buttons" data-disabled={disabled ? 'true' : 'false'}>
      <button onClick={onApprove} data-testid="approve-btn">Approve</button>
      <button onClick={onReject} data-testid="reject-btn">Reject</button>
      <button onClick={onEscalate} data-testid="escalate-btn">Escalate</button>
    </div>
  ),
}))

// ─── API response fixtures ────────────────────────────────────────────────────
const mockTicketResponse = {
  data: {
    id: 'TKT-001',
    ticket_type: 'safety_flag',
    confidence_score: 0.87,
    routing_decision: 'human_queue',
    raw_data: JSON.stringify({ flag_reason: 'hate_speech' }),
    created_at: '2026-03-24T10:30:00Z',
    summary: [
      'User reported post violates community guidelines on hate speech.',
      'ML model confidence is 87%, below the 95% STP threshold.',
      'Similar cases in the past week have resulted in content removal.',
    ],
    relevant_sops: [
      {
        id: 'SOP-01',
        title: 'Hate Speech Removal Policy',
        summary: 'Remove content targeting protected characteristics.',
        relevance_score: 0.94,
      },
      {
        id: 'SOP-02',
        title: 'Account Warning Procedure',
        summary: 'Issue formal account warnings.',
        relevance_score: 0.72,
      },
    ],
    action_recommendation: 'Remove content and issue account warning',
    confidence_override_threshold: 0.95,
  },
}

const mockActionResponse = {
  data: {
    action_id: 'ACT-001',
    ticket_id: 'TKT-001',
    action: 'approve',
    recorded_at: '2026-03-24T10:35:00Z',
  },
}

function createFetchMock() {
  return vi.fn((url: string, init?: RequestInit) => {
    if (url.includes('/copilot/ticket/')) {
      return Promise.resolve({
        ok: true,
        json: () => Promise.resolve(mockTicketResponse),
      })
    }
    if (url.includes('/copilot/action') && init?.method === 'POST') {
      return Promise.resolve({
        ok: true,
        json: () => Promise.resolve(mockActionResponse),
      })
    }
    return Promise.resolve({
      ok: false,
      json: () =>
        Promise.resolve({ error: { code: 'not_found', message: 'Not found' } }),
    })
  }) as unknown as typeof globalThis.fetch
}

describe('CopilotView', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Static rendering ─────────────────────────────────────────────────────
  describe('initial render', () => {
    it('renders the "Operator Copilot" heading', () => {
      render(<CopilotView ticketId="TKT-001" />)
      expect(
        screen.getByRole('heading', { name: /operator copilot/i })
      ).toBeInTheDocument()
    })

    it('renders the TicketDetail child component', () => {
      render(<CopilotView ticketId="TKT-001" />)
      expect(screen.getByTestId('ticket-detail')).toBeInTheDocument()
    })

    it('renders the RecommendationCard child component', () => {
      render(<CopilotView ticketId="TKT-001" />)
      expect(screen.getByTestId('recommendation-card')).toBeInTheDocument()
    })

    it('renders the ActionButtons child component', () => {
      render(<CopilotView ticketId="TKT-001" />)
      expect(screen.getByTestId('action-buttons')).toBeInTheDocument()
    })
  })

  // ── API integration ──────────────────────────────────────────────────────
  describe('API integration', () => {
    it('fetches the ticket with context on mount', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/api/v1/copilot/ticket/TKT-001'),
          expect.anything()
        )
      })
    })

    it('populates TicketDetail with the fetched ticket', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })
    })

    it('passes the confidence score to TicketDetail', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-confidence',
          '0.87'
        )
      })
    })

    it('renders one SOPDisplay per relevant SOP', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getAllByTestId('sop-display')).toHaveLength(2)
      })
    })

    it('passes the correct SOP ids to SOPDisplay', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        const sopDisplays = screen.getAllByTestId('sop-display')
        const ids = sopDisplays.map((el) => el.getAttribute('data-sop-id'))
        expect(ids).toContain('SOP-01')
        expect(ids).toContain('SOP-02')
      })
    })

    it('passes the summary bullets to RecommendationCard', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('recommendation-card')).toHaveAttribute(
          'data-bullets',
          '3'
        )
      })
    })

    it('passes the action recommendation to RecommendationCard', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('recommendation-card')).toHaveAttribute(
          'data-recommendation',
          'Remove content and issue account warning'
        )
      })
    })

    it('re-fetches when the ticketId prop changes', async () => {
      const { rerender } = render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/copilot/ticket/TKT-001'),
          expect.anything()
        )
      })

      rerender(<CopilotView ticketId="TKT-002" />)
      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/copilot/ticket/TKT-002'),
          expect.anything()
        )
      })
    })
  })

  // ── Action submission ────────────────────────────────────────────────────
  describe('action submission', () => {
    it('posts to /api/v1/copilot/action when Approve is clicked', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('approve-btn').click()
      })

      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/api/v1/copilot/action'),
          expect.objectContaining({ method: 'POST' })
        )
      })
    })

    it('includes the ticket_id and action in the POST body for Approve', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('approve-btn').click()
      })

      await waitFor(() => {
        const postCall = (fetch as ReturnType<typeof vi.fn>).mock.calls.find(
          ([url, init]) =>
            url.includes('/copilot/action') && init?.method === 'POST'
        )
        expect(postCall).toBeDefined()
        const body = JSON.parse(postCall![1].body as string)
        expect(body.ticket_id).toBe('TKT-001')
        expect(body.action).toBe('approve')
      })
    })

    it('posts action "reject" when Reject is clicked', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('reject-btn').click()
      })

      await waitFor(() => {
        const postCall = (fetch as ReturnType<typeof vi.fn>).mock.calls.find(
          ([url, init]) =>
            url.includes('/copilot/action') && init?.method === 'POST'
        )
        const body = JSON.parse(postCall![1].body as string)
        expect(body.action).toBe('reject')
      })
    })

    it('posts action "escalate" when Escalate is clicked', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('escalate-btn').click()
      })

      await waitFor(() => {
        const postCall = (fetch as ReturnType<typeof vi.fn>).mock.calls.find(
          ([url, init]) =>
            url.includes('/copilot/action') && init?.method === 'POST'
        )
        const body = JSON.parse(postCall![1].body as string)
        expect(body.action).toBe('escalate')
      })
    })

    it('disables action buttons while submitting', async () => {
      // Use a fetch that resolves slowly for the action endpoint
      let resolveAction!: () => void
      vi.stubGlobal(
        'fetch',
        vi.fn((url: string, init?: RequestInit) => {
          if (url.includes('/copilot/ticket/')) {
            return Promise.resolve({
              ok: true,
              json: () => Promise.resolve(mockTicketResponse),
            })
          }
          if (url.includes('/copilot/action') && init?.method === 'POST') {
            return new Promise((resolve) => {
              resolveAction = () =>
                resolve({ ok: true, json: () => Promise.resolve(mockActionResponse) })
            })
          }
          return Promise.resolve({ ok: false, json: () => Promise.resolve({}) })
        }) as unknown as typeof globalThis.fetch
      )

      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('approve-btn').click()
      })

      expect(screen.getByTestId('action-buttons')).toHaveAttribute(
        'data-disabled',
        'true'
      )

      // Resolve the pending action request
      await act(async () => {
        resolveAction()
      })
    })

    it('re-enables action buttons after submission completes', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('approve-btn').click()
      })

      await waitFor(() => {
        expect(screen.getByTestId('action-buttons')).toHaveAttribute(
          'data-disabled',
          'false'
        )
      })
    })

    it('shows a success message after action is submitted', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })

      await act(async () => {
        screen.getByTestId('approve-btn').click()
      })

      await waitFor(() => {
        expect(screen.getByText(/action submitted|success/i)).toBeInTheDocument()
      })
    })
  })

  // ── Loading / error states ───────────────────────────────────────────────
  describe('loading and error states', () => {
    it('shows a loading indicator while fetching the ticket', () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() => new Promise(() => {})) as unknown as typeof globalThis.fetch
      )
      render(<CopilotView ticketId="TKT-001" />)
      expect(screen.getByText(/loading/i)).toBeInTheDocument()
    })

    it('hides the loading indicator once ticket data arrives', async () => {
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(screen.getByTestId('ticket-detail')).toHaveAttribute(
          'data-ticket-id',
          'TKT-001'
        )
      })
      expect(screen.queryByText(/loading/i)).not.toBeInTheDocument()
    })

    it('shows an error message when the ticket fetch fails', async () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() =>
          Promise.reject(new Error('Network error'))
        ) as unknown as typeof globalThis.fetch
      )
      render(<CopilotView ticketId="TKT-001" />)
      await waitFor(() => {
        expect(
          screen.getByText(/error|failed|unable/i)
        ).toBeInTheDocument()
      })
    })

    it('shows an error when the API returns a non-ok response', async () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() =>
          Promise.resolve({
            ok: false,
            json: () =>
              Promise.resolve({
                error: { code: 'not_found', message: 'Ticket not found' },
              }),
          })
        ) as unknown as typeof globalThis.fetch
      )
      render(<CopilotView ticketId="TKT-999" />)
      await waitFor(() => {
        expect(screen.getByText(/error|failed|unable/i)).toBeInTheDocument()
      })
    })
  })
})
