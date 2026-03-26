import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { Flywheel } from '../Flywheel'
import type { FlywheelData } from '../../../../types/analytics'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const flywheelData: FlywheelData = {
  feedback_count: 342,
  model_version: 'v2.3.1',
  last_retrained: '2026-03-20T08:00:00Z',
  stp_improvement: 0.05,
}

const zeroImprovementData: FlywheelData = {
  feedback_count: 0,
  model_version: 'v1.0.0',
  last_retrained: '2026-01-01T00:00:00Z',
  stp_improvement: 0.0,
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('Flywheel', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Container & heading ───────────────────────────────────────────────────

  it('renders with data-testid="flywheel"', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByTestId('flywheel')).toBeInTheDocument()
  })

  it('renders the "Data Flywheel" heading', () => {
    render(<Flywheel data={flywheelData} />)
    expect(
      screen.getByRole('heading', { name: /data flywheel/i })
    ).toBeInTheDocument()
  })

  // ── Data display ──────────────────────────────────────────────────────────

  it('displays the feedback_count value', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText('342')).toBeInTheDocument()
  })

  it('displays the model_version', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/v2\.3\.1/)).toBeInTheDocument()
  })

  it('displays the stp_improvement as a percentage', () => {
    render(<Flywheel data={flywheelData} />)
    // 0.05 → "5.00%" or "5%"
    expect(screen.getByText(/5(\.\d+)?%/)).toBeInTheDocument()
  })

  it('displays the last_retrained date formatted (not raw ISO)', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.queryByText('2026-03-20T08:00:00Z')).not.toBeInTheDocument()
  })

  it('shows a label for the feedback count', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/feedback/i)).toBeInTheDocument()
  })

  it('shows a label for the model version', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/model version/i)).toBeInTheDocument()
  })

  it('shows a label for the STP improvement', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/stp improvement|improvement/i)).toBeInTheDocument()
  })

  // ── Flywheel stages ───────────────────────────────────────────────────────

  it('renders the "Operator Actions" stage', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/operator actions/i)).toBeInTheDocument()
  })

  it('renders the "RLHF Capture" stage', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/rlhf capture/i)).toBeInTheDocument()
  })

  it('renders the "Model Retraining" stage', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/model retraining/i)).toBeInTheDocument()
  })

  it('renders the "Better Predictions" stage', () => {
    render(<Flywheel data={flywheelData} />)
    expect(screen.getByText(/better predictions/i)).toBeInTheDocument()
  })

  // ── Edge cases ────────────────────────────────────────────────────────────

  it('displays zero feedback_count', () => {
    render(<Flywheel data={zeroImprovementData} />)
    expect(screen.getByText('0')).toBeInTheDocument()
  })

  it('displays 0% STP improvement', () => {
    render(<Flywheel data={zeroImprovementData} />)
    expect(screen.getByText(/0(\.\d+)?%/)).toBeInTheDocument()
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('updates feedback count when data prop changes', () => {
    const { rerender } = render(<Flywheel data={flywheelData} />)
    expect(screen.getByText('342')).toBeInTheDocument()

    rerender(<Flywheel data={{ ...flywheelData, feedback_count: 500 }} />)
    expect(screen.getByText('500')).toBeInTheDocument()
  })
})
