import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { ConfidenceGauge } from '../ConfidenceGauge'

describe('ConfidenceGauge', () => {
  it('renders confidence score as percentage (0-100%)', () => {
    render(<ConfidenceGauge confidenceScore={0.85} threshold={0.95} />)

    expect(screen.getByText(/85\.00%/)).toBeInTheDocument()
  })

  it('displays visual gauge/progress bar', () => {
    render(<ConfidenceGauge confidenceScore={0.75} threshold={0.95} />)

    const progressBar = screen.getByRole('progressbar')
    expect(progressBar).toBeInTheDocument()
    expect(progressBar).toHaveAttribute('aria-valuenow', '75')
  })

  it('shows threshold line on gauge', () => {
    render(<ConfidenceGauge confidenceScore={0.88} threshold={0.95} />)

    const thresholdLine = screen.getByTestId('threshold-line')
    expect(thresholdLine).toBeInTheDocument()
    expect(thresholdLine).toHaveStyle({ left: '95%' })
  })

  it('color codes red when confidence < 70%', () => {
    render(<ConfidenceGauge confidenceScore={0.65} threshold={0.95} />)

    const gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-red-500')
  })

  it('color codes yellow when confidence 70-90%', () => {
    render(<ConfidenceGauge confidenceScore={0.80} threshold={0.95} />)

    const gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-yellow-500')
  })

  it('color codes green when confidence > 90%', () => {
    render(<ConfidenceGauge confidenceScore={0.95} threshold={0.95} />)

    const gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-green-500')
  })

  it('updates when confidence score changes', () => {
    const { rerender } = render(
      <ConfidenceGauge confidenceScore={0.70} threshold={0.95} />
    )

    expect(screen.getByText(/70\.00%/)).toBeInTheDocument()

    rerender(<ConfidenceGauge confidenceScore={0.92} threshold={0.95} />)

    expect(screen.getByText(/92\.00%/)).toBeInTheDocument()
  })

  it('displays confidence score with 2 decimal precision', () => {
    render(<ConfidenceGauge confidenceScore={0.8567} threshold={0.95} />)

    expect(screen.getByText(/85\.67%/)).toBeInTheDocument()
  })

  it('handles edge case of 0% confidence', () => {
    render(<ConfidenceGauge confidenceScore={0.0} threshold={0.95} />)

    expect(screen.getByText(/0\.00%/)).toBeInTheDocument()

    const progressBar = screen.getByRole('progressbar')
    expect(progressBar).toHaveAttribute('aria-valuenow', '0')
  })

  it('handles edge case of 100% confidence', () => {
    render(<ConfidenceGauge confidenceScore={1.0} threshold={0.95} />)

    expect(screen.getByText(/100\.00%/)).toBeInTheDocument()

    const progressBar = screen.getByRole('progressbar')
    expect(progressBar).toHaveAttribute('aria-valuenow', '100')
  })

  it('displays threshold value as label', () => {
    render(<ConfidenceGauge confidenceScore={0.88} threshold={0.95} />)

    expect(screen.getByText(/Threshold: 95\.00%/)).toBeInTheDocument()
  })

  it('highlights when confidence exceeds threshold', () => {
    render(<ConfidenceGauge confidenceScore={0.97} threshold={0.95} />)

    const gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-green-500')

    const statusBadge = screen.getByTestId('threshold-status')
    expect(statusBadge).toHaveTextContent('Above Threshold')
  })

  it('highlights when confidence is below threshold', () => {
    render(<ConfidenceGauge confidenceScore={0.88} threshold={0.95} />)

    const statusBadge = screen.getByTestId('threshold-status')
    expect(statusBadge).toHaveTextContent('Below Threshold')
    expect(statusBadge).toHaveClass('text-orange-600')
  })

  it('updates color when confidence crosses threshold boundaries', () => {
    const { rerender } = render(
      <ConfidenceGauge confidenceScore={0.65} threshold={0.95} />
    )

    let gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-red-500')

    // Cross into yellow zone
    rerender(<ConfidenceGauge confidenceScore={0.75} threshold={0.95} />)

    gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-yellow-500')

    // Cross into green zone
    rerender(<ConfidenceGauge confidenceScore={0.92} threshold={0.95} />)

    gauge = screen.getByTestId('confidence-gauge')
    expect(gauge).toHaveClass('bg-green-500')
  })

  it('animates gauge fill with smooth transition', () => {
    render(<ConfidenceGauge confidenceScore={0.85} threshold={0.95} />)

    const gaugeFill = screen.getByTestId('gauge-fill')
    expect(gaugeFill).toHaveStyle({ transition: 'width 0.3s ease-in-out' })
  })

  it('displays aria-label for accessibility', () => {
    render(<ConfidenceGauge confidenceScore={0.88} threshold={0.95} />)

    const progressBar = screen.getByRole('progressbar')
    expect(progressBar).toHaveAttribute(
      'aria-label',
      'Confidence score: 88.00%'
    )
  })

  it('shows confidence score difference from threshold', () => {
    render(<ConfidenceGauge confidenceScore={0.88} threshold={0.95} />)

    // 0.95 - 0.88 = 0.07 = 7%
    expect(screen.getByText(/-7\.00% from threshold/)).toBeInTheDocument()
  })

  it('handles threshold updates dynamically', () => {
    const { rerender } = render(
      <ConfidenceGauge confidenceScore={0.88} threshold={0.95} />
    )

    expect(screen.getByText(/Threshold: 95\.00%/)).toBeInTheDocument()

    // Threshold lowered to 85%
    rerender(<ConfidenceGauge confidenceScore={0.88} threshold={0.85} />)

    expect(screen.getByText(/Threshold: 85\.00%/)).toBeInTheDocument()

    const statusBadge = screen.getByTestId('threshold-status')
    expect(statusBadge).toHaveTextContent('Above Threshold')
  })
})
