import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { TicketDetail } from '../TicketDetail'
import type { TicketWithContext } from '../../../../types/ticket'

const baseTicket: TicketWithContext = {
  id: 'TKT-001',
  ticket_type: 'safety_flag',
  confidence_score: 0.87,
  routing_decision: 'human_queue',
  raw_data: JSON.stringify({ flag_reason: 'inappropriate_content', severity: 'high' }),
  created_at: '2026-03-24T10:30:00Z',
  summary: [
    'User reported post violates community guidelines on hate speech.',
    'ML model confidence is 87%, below the 95% STP threshold.',
    'Similar cases in the past week have resulted in content removal.',
  ],
  relevant_sops: [
    { id: 'SOP-01', title: 'Hate Speech Removal', summary: 'Policy for removing hate speech content.', relevance_score: 0.94 },
    { id: 'SOP-02', title: 'Account Warning Procedure', summary: 'Steps for issuing account warnings.', relevance_score: 0.72 },
  ],
  action_recommendation: 'Remove content and issue account warning',
  confidence_override_threshold: 0.95,
}

describe('TicketDetail', () => {
  // ── Identity ─────────────────────────────────────────────────────────────
  it('renders the ticket ID', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/TKT-001/)).toBeInTheDocument()
  })

  it('renders the ticket type', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/safety_flag/i)).toBeInTheDocument()
  })

  // ── Confidence score ─────────────────────────────────────────────────────
  it('renders the confidence score as a percentage', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/87\.00%/)).toBeInTheDocument()
  })

  it('renders a confidence score progress bar', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('confidence-bar')).toBeInTheDocument()
  })

  it('sets the confidence bar width proportional to the score', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('confidence-bar')).toHaveStyle({ width: '87%' })
  })

  it('applies a warning color to the confidence bar when score is below 90%', () => {
    render(<TicketDetail ticket={baseTicket} />)
    // 87% < 90% → yellow/warning
    const bar = screen.getByTestId('confidence-bar')
    expect(bar.className).toMatch(/yellow|amber|warning/i)
  })

  it('applies a danger color to the confidence bar when score is below 70%', () => {
    const lowConfTicket: TicketWithContext = { ...baseTicket, confidence_score: 0.65 }
    render(<TicketDetail ticket={lowConfTicket} />)
    const bar = screen.getByTestId('confidence-bar')
    expect(bar.className).toMatch(/red|danger/i)
  })

  it('applies a success color to the confidence bar when score is 90% or above', () => {
    const highConfTicket: TicketWithContext = { ...baseTicket, confidence_score: 0.92 }
    render(<TicketDetail ticket={highConfTicket} />)
    const bar = screen.getByTestId('confidence-bar')
    expect(bar.className).toMatch(/green|success/i)
  })

  // ── Routing decision ─────────────────────────────────────────────────────
  it('renders the routing decision label', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('routing-badge')).toBeInTheDocument()
  })

  it('shows "Human Queue" for human_queue routing decision', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('routing-badge')).toHaveTextContent(/human queue/i)
  })

  it('shows "STP" for stp routing decision', () => {
    const stpTicket: TicketWithContext = { ...baseTicket, routing_decision: 'stp' }
    render(<TicketDetail ticket={stpTicket} />)
    expect(screen.getByTestId('routing-badge')).toHaveTextContent(/stp/i)
  })

  it('applies an orange color for human_queue routing decision', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('routing-badge')).toHaveClass('bg-orange-100')
  })

  it('applies a green color for stp routing decision', () => {
    const stpTicket: TicketWithContext = { ...baseTicket, routing_decision: 'stp' }
    render(<TicketDetail ticket={stpTicket} />)
    expect(screen.getByTestId('routing-badge')).toHaveClass('bg-green-100')
  })

  // ── Timestamp ────────────────────────────────────────────────────────────
  it('renders a formatted timestamp', () => {
    render(<TicketDetail ticket={baseTicket} />)
    // Displays the date in some human-readable format
    expect(screen.getByTestId('ticket-timestamp')).toBeInTheDocument()
  })

  it('timestamp element is not empty', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByTestId('ticket-timestamp').textContent?.trim()).not.toBe('')
  })

  // ── Labels ───────────────────────────────────────────────────────────────
  it('labels the ticket ID field', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/ticket id/i)).toBeInTheDocument()
  })

  it('labels the confidence score field', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/confidence/i)).toBeInTheDocument()
  })

  it('labels the routing decision field', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/routing/i)).toBeInTheDocument()
  })

  // ── Override threshold ───────────────────────────────────────────────────
  it('renders the STP threshold value', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/95\.00%/)).toBeInTheDocument()
  })

  it('labels the STP threshold field', () => {
    render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/threshold/i)).toBeInTheDocument()
  })

  // ── Re-render ────────────────────────────────────────────────────────────
  it('updates all values when the ticket prop changes', () => {
    const { rerender } = render(<TicketDetail ticket={baseTicket} />)
    expect(screen.getByText(/TKT-001/)).toBeInTheDocument()

    const updatedTicket: TicketWithContext = {
      ...baseTicket,
      id: 'TKT-002',
      ticket_type: 'document_verification',
      confidence_score: 0.72,
    }
    rerender(<TicketDetail ticket={updatedTicket} />)
    expect(screen.getByText(/TKT-002/)).toBeInTheDocument()
    expect(screen.getByText(/document_verification/i)).toBeInTheDocument()
    expect(screen.getByText(/72\.00%/)).toBeInTheDocument()
  })
})
