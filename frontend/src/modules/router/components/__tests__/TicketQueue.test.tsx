import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { TicketQueue } from '../TicketQueue'
import type { Ticket } from '../../../../types/ticket'

describe('TicketQueue', () => {
  it('renders empty state when no tickets provided', () => {
    render(<TicketQueue tickets={[]} />)

    expect(screen.getByText(/no pending tickets/i)).toBeInTheDocument()
  })

  it('renders list of pending tickets with ticket IDs', () => {
    const tickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-002',
        ticket_type: 'document_verification',
        confidence_score: 0.72,
        routing_decision: 'human_queue',
        raw_data: '{}',
        created_at: '2026-03-23T10:01:00Z'
      }
    ]

    render(<TicketQueue tickets={tickets} />)

    expect(screen.getByText('ticket-001')).toBeInTheDocument()
    expect(screen.getByText('ticket-002')).toBeInTheDocument()
  })

  it('displays confidence scores for each ticket', () => {
    const tickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-002',
        ticket_type: 'document_verification',
        confidence_score: 0.92,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:01:00Z'
      }
    ]

    render(<TicketQueue tickets={tickets} />)

    // Confidence scores should be displayed as percentages
    expect(screen.getByText(/85\.00%/)).toBeInTheDocument()
    expect(screen.getByText(/92\.00%/)).toBeInTheDocument()
  })

  it('shows ticket type for each ticket', () => {
    const tickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-002',
        ticket_type: 'document_verification',
        confidence_score: 0.72,
        routing_decision: 'human_queue',
        raw_data: '{}',
        created_at: '2026-03-23T10:01:00Z'
      }
    ]

    render(<TicketQueue tickets={tickets} />)

    expect(screen.getByText('safety_flag')).toBeInTheDocument()
    expect(screen.getByText('document_verification')).toBeInTheDocument()
  })

  it('updates when tickets prop changes', () => {
    const initialTickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      }
    ]

    const { rerender } = render(<TicketQueue tickets={initialTickets} />)
    expect(screen.getByText('ticket-001')).toBeInTheDocument()

    const updatedTickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-003',
        ticket_type: 'document_verification',
        confidence_score: 0.78,
        routing_decision: 'human_queue',
        raw_data: '{}',
        created_at: '2026-03-23T10:02:00Z'
      }
    ]

    rerender(<TicketQueue tickets={updatedTickets} />)

    expect(screen.getByText('ticket-001')).toBeInTheDocument()
    expect(screen.getByText('ticket-003')).toBeInTheDocument()
  })

  it('handles real-time ticket stream updates', () => {
    const tickets: Ticket[] = []

    const { rerender } = render(<TicketQueue tickets={tickets} />)
    expect(screen.getByText(/no pending tickets/i)).toBeInTheDocument()

    // Simulate new ticket arriving via stream
    const newTickets: Ticket[] = [
      {
        id: 'ticket-live-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.89,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:05:00Z'
      }
    ]

    rerender(<TicketQueue tickets={newTickets} />)

    expect(screen.queryByText(/no pending tickets/i)).not.toBeInTheDocument()
    expect(screen.getByText('ticket-live-001')).toBeInTheDocument()
    expect(screen.getByText(/89\.00%/)).toBeInTheDocument()
  })

  it('displays routing decision for each ticket', () => {
    const tickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.95,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-002',
        ticket_type: 'document_verification',
        confidence_score: 0.72,
        routing_decision: 'human_queue',
        raw_data: '{}',
        created_at: '2026-03-23T10:01:00Z'
      }
    ]

    render(<TicketQueue tickets={tickets} />)

    // Should display routing decision badges
    expect(screen.getByText(/STP/i)).toBeInTheDocument()
    expect(screen.getByText(/Human Queue/i)).toBeInTheDocument()
  })

  it('sorts tickets by created_at timestamp (newest first)', () => {
    const tickets: Ticket[] = [
      {
        id: 'ticket-001',
        ticket_type: 'safety_flag',
        confidence_score: 0.85,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:00:00Z'
      },
      {
        id: 'ticket-002',
        ticket_type: 'document_verification',
        confidence_score: 0.72,
        routing_decision: 'human_queue',
        raw_data: '{}',
        created_at: '2026-03-23T10:05:00Z'
      },
      {
        id: 'ticket-003',
        ticket_type: 'safety_flag',
        confidence_score: 0.88,
        routing_decision: 'stp',
        raw_data: '{}',
        created_at: '2026-03-23T10:02:00Z'
      }
    ]

    render(<TicketQueue tickets={tickets} />)

    const ticketElements = screen.getAllByRole('listitem')

    // ticket-002 (10:05) should be first, then ticket-003 (10:02), then ticket-001 (10:00)
    expect(ticketElements[0]).toHaveTextContent('ticket-002')
    expect(ticketElements[1]).toHaveTextContent('ticket-003')
    expect(ticketElements[2]).toHaveTextContent('ticket-001')
  })
})
