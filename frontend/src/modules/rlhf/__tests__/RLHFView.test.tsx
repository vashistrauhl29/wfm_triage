import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, act } from '@testing-library/react'
import { RLHFView } from '../RLHFView'
import type { FeedbackEvent } from '../../../types/rlhf'

// ─── Mock child components ─────────────────────────────────────────────────────

vi.mock('../components/DisagreementForm', () => ({
  DisagreementForm: ({
    ticketId,
    recommendedAction,
    operatorId,
    onSubmitSuccess,
    onCancel,
  }: any) => (
    <div
      data-testid="disagreement-form"
      data-ticket-id={ticketId}
      data-recommended-action={recommendedAction}
      data-operator-id={operatorId}
    >
      <button
        onClick={() =>
          onSubmitSuccess({
            id: 'evt-1',
            ticket_id: ticketId,
            operator_id: operatorId,
            recommended_action: recommendedAction,
            actual_action: 'reject',
            disagreement_category: 'edge_case',
            disagreement_notes: '',
            created_at: new Date().toISOString(),
          } satisfies FeedbackEvent)
        }
      >
        MockSubmit
      </button>
      <button onClick={onCancel}>MockCancel</button>
    </div>
  ),
}))

vi.mock('../components/FeedbackHistory', () => ({
  FeedbackHistory: ({ events }: any) => (
    <div data-testid="feedback-history" data-event-count={events.length}>
      {events.map((e: FeedbackEvent) => (
        <div key={e.id} data-testid="mock-event">
          {e.id}
        </div>
      ))}
    </div>
  ),
}))

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const existingEvent: FeedbackEvent = {
  id: 'evt-existing',
  ticket_id: 'TKT-001',
  operator_id: 'OP-001',
  recommended_action: 'approve',
  actual_action: 'approve',
  disagreement_category: 'policy_change',
  disagreement_notes: '',
  created_at: '2026-03-20T08:00:00Z',
}

const defaultProps = {
  ticketId: 'TKT-001',
  recommendedAction: 'approve' as FeedbackEvent['recommended_action'],
  operatorId: 'OP-001',
}

function createFetchMock(events: FeedbackEvent[] = [existingEvent]) {
  return vi.fn(() =>
    Promise.resolve({
      ok: true,
      json: () => Promise.resolve({ data: events }),
    })
  ) as unknown as typeof globalThis.fetch
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('RLHFView', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Heading ───────────────────────────────────────────────────────────────

  describe('heading', () => {
    it('renders a "RLHF Capture Gate" h1 heading', () => {
      render(<RLHFView {...defaultProps} />)
      expect(
        screen.getByRole('heading', { name: /rlhf capture gate/i, level: 1 })
      ).toBeInTheDocument()
    })
  })

  // ── Initial render ────────────────────────────────────────────────────────

  describe('initial render', () => {
    it('renders the DisagreementForm from the first render', () => {
      render(<RLHFView {...defaultProps} />)
      expect(screen.getByTestId('disagreement-form')).toBeInTheDocument()
    })

    it('renders the FeedbackHistory from the first render', () => {
      render(<RLHFView {...defaultProps} />)
      expect(screen.getByTestId('feedback-history')).toBeInTheDocument()
    })

    it('passes ticketId to DisagreementForm', () => {
      render(<RLHFView {...defaultProps} ticketId="TKT-XYZ" />)
      expect(screen.getByTestId('disagreement-form')).toHaveAttribute('data-ticket-id', 'TKT-XYZ')
    })

    it('passes recommendedAction to DisagreementForm', () => {
      render(<RLHFView {...defaultProps} recommendedAction="escalate" />)
      expect(screen.getByTestId('disagreement-form')).toHaveAttribute(
        'data-recommended-action',
        'escalate'
      )
    })

    it('passes operatorId to DisagreementForm', () => {
      render(<RLHFView {...defaultProps} operatorId="OP-999" />)
      expect(screen.getByTestId('disagreement-form')).toHaveAttribute('data-operator-id', 'OP-999')
    })
  })

  // ── API integration ────────────────────────────────────────────────────────

  describe('API integration', () => {
    it('fetches GET /api/v1/rlhf/dataset on mount', async () => {
      render(<RLHFView {...defaultProps} />)
      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/api/v1/rlhf/dataset'),
          expect.anything()
        )
      })
    })

    it('passes fetched events to FeedbackHistory', async () => {
      render(<RLHFView {...defaultProps} />)
      await waitFor(() => {
        expect(screen.getByTestId('feedback-history')).toHaveAttribute(
          'data-event-count',
          '1'
        )
      })
    })

    it('renders fetched event ids in the history', async () => {
      render(<RLHFView {...defaultProps} />)
      await waitFor(() => {
        expect(screen.getByText('evt-existing')).toBeInTheDocument()
      })
    })
  })

  // ── Loading / error states ─────────────────────────────────────────────────

  describe('loading and error states', () => {
    it('shows "Loading history..." while the GET is in-flight', () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() => new Promise(() => {})) as unknown as typeof globalThis.fetch
      )
      render(<RLHFView {...defaultProps} />)
      expect(screen.getByText(/loading history/i)).toBeInTheDocument()
    })

    it('hides "Loading history..." after fetch resolves', async () => {
      render(<RLHFView {...defaultProps} />)
      await waitFor(() => {
        expect(screen.queryByText(/loading history/i)).not.toBeInTheDocument()
      })
    })

    it('shows "Failed to load feedback history" when GET fails', async () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() => Promise.resolve({ ok: false, json: () => Promise.resolve({}) })) as unknown as typeof globalThis.fetch
      )
      render(<RLHFView {...defaultProps} />)
      await waitFor(() => {
        expect(screen.getByText(/failed to load feedback history/i)).toBeInTheDocument()
      })
    })
  })

  // ── onSubmitSuccess ────────────────────────────────────────────────────────

  describe('onSubmitSuccess', () => {
    it('prepends the new event to FeedbackHistory when onSubmitSuccess fires', async () => {
      render(<RLHFView {...defaultProps} />)

      // Wait for initial history to load (1 existing event)
      await waitFor(() => {
        expect(screen.getByTestId('feedback-history')).toHaveAttribute('data-event-count', '1')
      })

      // Fire MockSubmit → triggers onSubmitSuccess with evt-1
      await act(async () => {
        screen.getByText('MockSubmit').click()
      })

      // Now history should have 2 events (new one prepended)
      await waitFor(() => {
        expect(screen.getByTestId('feedback-history')).toHaveAttribute('data-event-count', '2')
      })
    })

    it('renders the new event id at the top of the history', async () => {
      render(<RLHFView {...defaultProps} />)

      await waitFor(() => {
        expect(screen.getByTestId('feedback-history')).toHaveAttribute('data-event-count', '1')
      })

      await act(async () => {
        screen.getByText('MockSubmit').click()
      })

      await waitFor(() => {
        const events = screen.getAllByTestId('mock-event')
        expect(events[0].textContent).toBe('evt-1')
      })
    })
  })

  // ── onCancel ───────────────────────────────────────────────────────────────

  describe('onCancel', () => {
    it('keeps DisagreementForm mounted after Cancel is clicked', async () => {
      render(<RLHFView {...defaultProps} />)

      await act(async () => {
        screen.getByText('MockCancel').click()
      })

      expect(screen.getByTestId('disagreement-form')).toBeInTheDocument()
    })
  })
})
