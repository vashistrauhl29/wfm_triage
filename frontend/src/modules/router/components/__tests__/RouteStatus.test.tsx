import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { RouteStatus } from '../RouteStatus'
import type { QueueStatus } from '../../../../types/router'

describe('RouteStatus', () => {
  it('displays "STP" visual indicator when STP queue has tickets', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 15,
      human_queue_size: 5,
      total_processed: 100,
      stp_rate: 0.75
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText('STP')).toBeInTheDocument()
  })

  it('displays "Human Queue" visual indicator when human queue has tickets', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 8,
      human_queue_size: 12,
      total_processed: 100,
      stp_rate: 0.40
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText(/Human Queue/i)).toBeInTheDocument()
  })

  it('shows correct color coding for STP queue (green)', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 20,
      human_queue_size: 5,
      total_processed: 100,
      stp_rate: 0.80
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const stpIndicator = screen.getByTestId('stp-indicator')
    expect(stpIndicator).toHaveClass('bg-green-500')
  })

  it('shows correct color coding for Human Queue (orange/yellow)', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 5,
      human_queue_size: 15,
      total_processed: 100,
      stp_rate: 0.25
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const humanIndicator = screen.getByTestId('human-indicator')
    expect(humanIndicator).toHaveClass('bg-orange-500')
  })

  it('displays ticket count for STP queue', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 42,
      human_queue_size: 8,
      total_processed: 100,
      stp_rate: 0.84
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText('42')).toBeInTheDocument()
  })

  it('displays ticket count for Human Queue', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 10,
      human_queue_size: 23,
      total_processed: 100,
      stp_rate: 0.30
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText('23')).toBeInTheDocument()
  })

  it('shows STP rate as percentage', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 75,
      human_queue_size: 25,
      total_processed: 100,
      stp_rate: 0.75
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText(/75\.00%/)).toBeInTheDocument()
  })

  it('updates in real-time when routing decisions change', () => {
    const initialStatus: QueueStatus = {
      stp_queue_size: 10,
      human_queue_size: 5,
      total_processed: 50,
      stp_rate: 0.67
    }

    const { rerender } = render(<RouteStatus queueStatus={initialStatus} />)

    expect(screen.getByText('10')).toBeInTheDocument()
    expect(screen.getByText('5')).toBeInTheDocument()
    expect(screen.getByText(/67\.00%/)).toBeInTheDocument()

    // Simulate real-time update
    const updatedStatus: QueueStatus = {
      stp_queue_size: 15,
      human_queue_size: 8,
      total_processed: 75,
      stp_rate: 0.65
    }

    rerender(<RouteStatus queueStatus={updatedStatus} />)

    expect(screen.getByText('15')).toBeInTheDocument()
    expect(screen.getByText('8')).toBeInTheDocument()
    expect(screen.getByText(/65\.00%/)).toBeInTheDocument()
  })

  it('displays total processed tickets count', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 30,
      human_queue_size: 20,
      total_processed: 500,
      stp_rate: 0.60
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    expect(screen.getByText(/500/)).toBeInTheDocument()
    expect(screen.getByText(/Total Processed/i)).toBeInTheDocument()
  })

  it('handles zero tickets in both queues', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 0,
      human_queue_size: 0,
      total_processed: 0,
      stp_rate: 0
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const zeroElements = screen.getAllByText('0')
    expect(zeroElements.length).toBeGreaterThan(0)
    expect(screen.getByText(/0\.00%/)).toBeInTheDocument()
  })

  it('displays high STP rate with green indicator', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 90,
      human_queue_size: 10,
      total_processed: 100,
      stp_rate: 0.90
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const stpRateElement = screen.getByTestId('stp-rate')
    expect(stpRateElement).toHaveClass('text-green-600')
    expect(screen.getByText(/90\.00%/)).toBeInTheDocument()
  })

  it('displays low STP rate with red indicator', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 40,
      human_queue_size: 60,
      total_processed: 100,
      stp_rate: 0.40
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const stpRateElement = screen.getByTestId('stp-rate')
    expect(stpRateElement).toHaveClass('text-red-600')
    expect(screen.getByText(/40\.00%/)).toBeInTheDocument()
  })

  it('displays medium STP rate with yellow indicator', () => {
    const queueStatus: QueueStatus = {
      stp_queue_size: 70,
      human_queue_size: 30,
      total_processed: 100,
      stp_rate: 0.70
    }

    render(<RouteStatus queueStatus={queueStatus} />)

    const stpRateElement = screen.getByTestId('stp-rate')
    expect(stpRateElement).toHaveClass('text-yellow-600')
    expect(screen.getByText(/70\.00%/)).toBeInTheDocument()
  })
})
