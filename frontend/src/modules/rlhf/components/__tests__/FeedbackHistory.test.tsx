import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen } from '@testing-library/react'
import { FeedbackHistory } from '../FeedbackHistory'
import type { FeedbackEvent } from '../../../../types/rlhf'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const makeEvent = (overrides: Partial<FeedbackEvent> = {}): FeedbackEvent => ({
  id: 'evt-001',
  ticket_id: 'TKT-100',
  operator_id: 'OP-001',
  recommended_action: 'approve',
  actual_action: 'approve',
  disagreement_category: 'policy_change',
  disagreement_notes: 'Some notes here.',
  created_at: '2026-03-24T10:00:00Z',
  ...overrides,
})

const disagreedEvent = makeEvent({
  id: 'evt-002',
  ticket_id: 'TKT-200',
  operator_id: 'OP-002',
  recommended_action: 'approve',
  actual_action: 'reject',
  disagreement_category: 'edge_case',
  disagreement_notes: 'Edge case not covered by policy.',
})

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('FeedbackHistory', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Structure ─────────────────────────────────────────────────────────────

  describe('structure', () => {
    it('renders a "Feedback History" heading', () => {
      render(<FeedbackHistory events={[]} />)
      expect(
        screen.getByRole('heading', { name: /feedback history/i })
      ).toBeInTheDocument()
    })

    it('renders a table with data-testid="feedback-table"', () => {
      render(<FeedbackHistory events={[makeEvent()]} />)
      expect(screen.getByTestId('feedback-table')).toBeInTheDocument()
    })

    it('renders the table with role="table"', () => {
      render(<FeedbackHistory events={[makeEvent()]} />)
      expect(screen.getByRole('table')).toBeInTheDocument()
    })

    it('renders column headers: Ticket ID, Operator, Recommended, Actual, Category, Date', () => {
      render(<FeedbackHistory events={[makeEvent()]} />)
      expect(screen.getByText(/ticket id/i)).toBeInTheDocument()
      expect(screen.getByText(/operator/i)).toBeInTheDocument()
      expect(screen.getByText(/recommended/i)).toBeInTheDocument()
      expect(screen.getByText(/actual/i)).toBeInTheDocument()
      expect(screen.getByText(/category/i)).toBeInTheDocument()
      expect(screen.getByText(/date/i)).toBeInTheDocument()
    })
  })

  // ── Empty state ───────────────────────────────────────────────────────────

  describe('empty state', () => {
    it('shows "No feedback events recorded" when events is empty', () => {
      render(<FeedbackHistory events={[]} />)
      expect(screen.getByText(/no feedback events recorded/i)).toBeInTheDocument()
    })

    it('renders no data rows when events is empty', () => {
      render(<FeedbackHistory events={[]} />)
      expect(screen.queryAllByTestId('feedback-row')).toHaveLength(0)
    })
  })

  // ── Data rows ─────────────────────────────────────────────────────────────

  describe('data rows', () => {
    it('renders one row per event with data-testid="feedback-row"', () => {
      const events = [makeEvent({ id: 'evt-1' }), makeEvent({ id: 'evt-2' }), makeEvent({ id: 'evt-3' })]
      render(<FeedbackHistory events={events} />)
      expect(screen.getAllByTestId('feedback-row')).toHaveLength(3)
    })

    it('displays the ticket_id in each row', () => {
      render(<FeedbackHistory events={[makeEvent({ ticket_id: 'TKT-999' })]} />)
      expect(screen.getByText('TKT-999')).toBeInTheDocument()
    })

    it('displays the operator_id in each row', () => {
      render(<FeedbackHistory events={[makeEvent({ operator_id: 'OP-777' })]} />)
      expect(screen.getByText('OP-777')).toBeInTheDocument()
    })

    it('displays the recommended_action in each row', () => {
      render(<FeedbackHistory events={[makeEvent({ recommended_action: 'escalate' })]} />)
      expect(screen.getByText('escalate')).toBeInTheDocument()
    })

    it('formats the date as a locale string (not raw ISO)', () => {
      render(<FeedbackHistory events={[makeEvent({ created_at: '2026-03-24T10:00:00Z' })]} />)
      // Should NOT render the raw ISO string
      expect(screen.queryByText('2026-03-24T10:00:00Z')).not.toBeInTheDocument()
    })

    it('does NOT display disagreement_notes in the table', () => {
      render(<FeedbackHistory events={[makeEvent({ disagreement_notes: 'super secret note xyz987' })]} />)
      expect(screen.queryByText('super secret note xyz987')).not.toBeInTheDocument()
    })
  })

  // ── Disagreement highlighting ─────────────────────────────────────────────

  describe('disagreement highlighting', () => {
    it('applies text-red-600 to actual_action cell when it differs from recommended_action', () => {
      render(<FeedbackHistory events={[disagreedEvent]} />)
      const row = screen.getByTestId('feedback-row')
      const cells = row.querySelectorAll('td')
      // actual_action column — find cell containing "reject" with red class
      const actualCell = Array.from(cells).find((c) => c.textContent === 'reject')
      expect(actualCell).toBeDefined()
      expect(actualCell!.className).toContain('text-red-600')
    })

    it('does not apply a red class to actual_action cell when it matches recommended_action', () => {
      const agreedEvent = makeEvent({ recommended_action: 'approve', actual_action: 'reject' === 'reject' ? 'approve' : 'approve' })
      render(<FeedbackHistory events={[agreedEvent]} />)
      const row = screen.getByTestId('feedback-row')
      const cells = row.querySelectorAll('td')
      // actual_action is the 4th cell (index 3)
      const actualCell = cells[3]
      expect(actualCell).toBeDefined()
      expect(actualCell.className).not.toContain('text-red')
    })
  })
})
