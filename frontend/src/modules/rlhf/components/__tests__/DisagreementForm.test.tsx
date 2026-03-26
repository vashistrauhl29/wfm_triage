import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, fireEvent, waitFor, act } from '@testing-library/react'
import { DisagreementForm } from '../DisagreementForm'
import type { FeedbackEvent } from '../../../../types/rlhf'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const mockFeedbackEvent: FeedbackEvent = {
  id: 'evt-001',
  ticket_id: 'TKT-123',
  operator_id: 'OP-456',
  recommended_action: 'approve',
  actual_action: 'reject',
  disagreement_category: 'policy_change',
  disagreement_notes: 'Policy was recently updated.',
  created_at: '2026-03-24T10:00:00Z',
}

const defaultProps = {
  ticketId: 'TKT-123',
  recommendedAction: 'approve' as FeedbackEvent['recommended_action'],
  operatorId: 'OP-456',
  onSubmitSuccess: vi.fn(),
  onCancel: vi.fn(),
}

function createFetchMock(ok = true) {
  return vi.fn(() =>
    Promise.resolve({
      ok,
      json: () => Promise.resolve({ data: mockFeedbackEvent }),
    })
  ) as unknown as typeof globalThis.fetch
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('DisagreementForm', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Structure ────────────────────────────────────────────────────────────

  describe('structure', () => {
    it('renders a <form> element', () => {
      const { container } = render(<DisagreementForm {...defaultProps} />)
      expect(container.querySelector('form')).toBeInTheDocument()
    })

    it('renders the heading "Capture Disagreement"', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(
        screen.getByRole('heading', { name: /capture disagreement/i })
      ).toBeInTheDocument()
    })

    it('displays the ticketId value', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByText('TKT-123')).toBeInTheDocument()
    })

    it('displays a "Ticket ID" label', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByText(/ticket id/i)).toBeInTheDocument()
    })

    it('displays the operatorId value', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByText('OP-456')).toBeInTheDocument()
    })

    it('displays an "Operator" label', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByText(/operator/i)).toBeInTheDocument()
    })

    it('displays the recommendedAction value under a "Recommended Action" label', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByText(/recommended action/i)).toBeInTheDocument()
      expect(screen.getByText('approve')).toBeInTheDocument()
    })

    it('renders a <select> for actual_action with approve/reject/escalate options', () => {
      render(<DisagreementForm {...defaultProps} />)
      const select = document.querySelector('select[name="actual_action"]') as HTMLSelectElement
      expect(select).toBeInTheDocument()
      const values = Array.from(select.options).map((o) => o.value)
      expect(values).toContain('approve')
      expect(values).toContain('reject')
      expect(values).toContain('escalate')
    })

    it('renders a <select> for disagreement_category with all 5 options', () => {
      render(<DisagreementForm {...defaultProps} />)
      const select = document.querySelector('select[name="disagreement_category"]') as HTMLSelectElement
      expect(select).toBeInTheDocument()
      const values = Array.from(select.options).map((o) => o.value)
      expect(values).toContain('policy_change')
      expect(values).toContain('edge_case')
      expect(values).toContain('data_quality')
      expect(values).toContain('model_error')
      expect(values).toContain('other')
    })

    it('renders human-readable labels for disagreement_category options', () => {
      render(<DisagreementForm {...defaultProps} />)
      const select = document.querySelector('select[name="disagreement_category"]') as HTMLSelectElement
      const labels = Array.from(select.options).map((o) => o.text)
      expect(labels).toContain('Policy Change')
      expect(labels).toContain('Edge Case')
      expect(labels).toContain('Data Quality')
      expect(labels).toContain('Model Error')
      expect(labels).toContain('Other')
    })

    it('renders a <textarea> with name="disagreement_notes" and correct placeholder', () => {
      render(<DisagreementForm {...defaultProps} />)
      const textarea = document.querySelector('textarea[name="disagreement_notes"]') as HTMLTextAreaElement
      expect(textarea).toBeInTheDocument()
      expect(textarea.placeholder).toBe('Describe why you disagreed...')
    })

    it('renders a "Submit" button', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByRole('button', { name: /submit/i })).toBeInTheDocument()
    })

    it('renders a "Cancel" button', () => {
      render(<DisagreementForm {...defaultProps} />)
      expect(screen.getByRole('button', { name: /cancel/i })).toBeInTheDocument()
    })
  })

  // ── Defaults ─────────────────────────────────────────────────────────────

  describe('defaults', () => {
    it('pre-selects actual_action to the recommendedAction prop value', () => {
      render(<DisagreementForm {...defaultProps} recommendedAction="reject" />)
      const select = document.querySelector('select[name="actual_action"]') as HTMLSelectElement
      expect(select.value).toBe('reject')
    })

    it('defaults disagreement_category to "policy_change"', () => {
      render(<DisagreementForm {...defaultProps} />)
      const select = document.querySelector('select[name="disagreement_category"]') as HTMLSelectElement
      expect(select.value).toBe('policy_change')
    })
  })

  // ── Interactions ──────────────────────────────────────────────────────────

  describe('interactions', () => {
    it('calls onCancel when the Cancel button is clicked', () => {
      const onCancel = vi.fn()
      render(<DisagreementForm {...defaultProps} onCancel={onCancel} />)
      fireEvent.click(screen.getByRole('button', { name: /cancel/i }))
      expect(onCancel).toHaveBeenCalledTimes(1)
    })

    it('posts to /api/v1/rlhf/capture on Submit with correct payload', async () => {
      render(<DisagreementForm {...defaultProps} />)

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          '/api/v1/rlhf/capture',
          expect.objectContaining({ method: 'POST' })
        )
      })

      const [, init] = (fetch as ReturnType<typeof vi.fn>).mock.calls[0]
      const body = JSON.parse(init.body as string)
      expect(body.ticket_id).toBe('TKT-123')
      expect(body.operator_id).toBe('OP-456')
      expect(body.recommended_action).toBe('approve')
      expect(body.actual_action).toBe('approve')
      expect(body.disagreement_category).toBe('policy_change')
    })

    it('calls onSubmitSuccess with the returned FeedbackEvent on success', async () => {
      const onSubmitSuccess = vi.fn()
      render(<DisagreementForm {...defaultProps} onSubmitSuccess={onSubmitSuccess} />)

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      await waitFor(() => {
        expect(onSubmitSuccess).toHaveBeenCalledWith(mockFeedbackEvent)
      })
    })

    it('shows "Submitting..." while the POST is in-flight', async () => {
      let resolvePost!: () => void
      vi.stubGlobal(
        'fetch',
        vi.fn(
          () =>
            new Promise((resolve) => {
              resolvePost = () =>
                resolve({ ok: true, json: () => Promise.resolve({ data: mockFeedbackEvent }) })
            })
        ) as unknown as typeof globalThis.fetch
      )

      render(<DisagreementForm {...defaultProps} />)

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      expect(screen.getByText(/submitting/i)).toBeInTheDocument()

      await act(async () => {
        resolvePost()
      })
    })

    it('disables the Submit button while submitting', async () => {
      let resolvePost!: () => void
      vi.stubGlobal(
        'fetch',
        vi.fn(
          () =>
            new Promise((resolve) => {
              resolvePost = () =>
                resolve({ ok: true, json: () => Promise.resolve({ data: mockFeedbackEvent }) })
            })
        ) as unknown as typeof globalThis.fetch
      )

      render(<DisagreementForm {...defaultProps} />)

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      expect(screen.getByRole('button', { name: /submitting/i })).toBeDisabled()

      await act(async () => {
        resolvePost()
      })
    })

    it('shows "Failed to submit feedback" when the POST fails', async () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() => Promise.resolve({ ok: false, json: () => Promise.resolve({}) })) as unknown as typeof globalThis.fetch
      )

      render(<DisagreementForm {...defaultProps} />)

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      await waitFor(() => {
        expect(screen.getByText(/failed to submit feedback/i)).toBeInTheDocument()
      })
    })

    it('includes updated textarea content in the submission payload', async () => {
      render(<DisagreementForm {...defaultProps} />)

      const textarea = document.querySelector('textarea[name="disagreement_notes"]') as HTMLTextAreaElement
      fireEvent.change(textarea, { target: { value: 'The policy changed last week.' } })

      await act(async () => {
        fireEvent.click(screen.getByRole('button', { name: /submit/i }))
      })

      await waitFor(() => {
        const [, init] = (fetch as ReturnType<typeof vi.fn>).mock.calls[0]
        const body = JSON.parse(init.body as string)
        expect(body.disagreement_notes).toBe('The policy changed last week.')
      })
    })
  })
})
