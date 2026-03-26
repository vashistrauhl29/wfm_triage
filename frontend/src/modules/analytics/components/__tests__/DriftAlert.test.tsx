import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { DriftAlert } from '../DriftAlert'
import type { DriftReport } from '../../../../types/analytics'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const driftDetectedReport: DriftReport = {
  detected: true,
  drift_magnitude: 0.08,
  baseline_accuracy: 0.93,
  current_accuracy: 0.85,
  model_version: 'v2.3.1',
  last_checked: '2026-03-24T10:00:00Z',
}

const noDriftReport: DriftReport = {
  detected: false,
  drift_magnitude: 0.02,
  baseline_accuracy: 0.93,
  current_accuracy: 0.92,
  model_version: 'v2.3.1',
  last_checked: '2026-03-24T10:00:00Z',
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('DriftAlert', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Container ────────────────────────────────────────────────────────────

  it('renders with data-testid="drift-alert"', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByTestId('drift-alert')).toBeInTheDocument()
  })

  // ── Drift detected state ──────────────────────────────────────────────────

  it('renders role="alert" when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByRole('alert')).toBeInTheDocument()
  })

  it('shows the "Model Drift Detected" heading when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(
      screen.getByRole('heading', { name: /model drift detected/i })
    ).toBeInTheDocument()
  })

  it('displays the drift magnitude as a percentage', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    // 0.08 → "8.00%" or "8%"
    expect(screen.getByText(/8(\.\d+)?%/)).toBeInTheDocument()
  })

  it('displays the current_accuracy value when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByText(/0\.85|85(\.\d+)?%/)).toBeInTheDocument()
  })

  it('displays the baseline_accuracy value when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByText(/0\.93|93(\.\d+)?%/)).toBeInTheDocument()
  })

  it('applies a red background class when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    const alert = screen.getByTestId('drift-alert')
    expect(alert.className).toMatch(/bg-red/)
  })

  it('shows the model_version when drift is detected', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByText(/v2\.3\.1/)).toBeInTheDocument()
  })

  it('shows the last_checked date formatted as a locale string (not raw ISO)', () => {
    render(<DriftAlert report={driftDetectedReport} />)
    expect(screen.queryByText('2026-03-24T10:00:00Z')).not.toBeInTheDocument()
  })

  // ── No drift state ────────────────────────────────────────────────────────

  it('shows "No Drift Detected" when detected is false', () => {
    render(<DriftAlert report={noDriftReport} />)
    expect(screen.getByText(/no drift detected/i)).toBeInTheDocument()
  })

  it('does NOT render role="alert" when drift is not detected', () => {
    render(<DriftAlert report={noDriftReport} />)
    expect(screen.queryByRole('alert')).not.toBeInTheDocument()
  })

  it('does NOT render "Model Drift Detected" heading when not detected', () => {
    render(<DriftAlert report={noDriftReport} />)
    expect(
      screen.queryByRole('heading', { name: /model drift detected/i })
    ).not.toBeInTheDocument()
  })

  it('applies a green or neutral background class when no drift', () => {
    render(<DriftAlert report={noDriftReport} />)
    const container = screen.getByTestId('drift-alert')
    expect(container.className).toMatch(/bg-green|bg-gray/)
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('switches from no-drift to drift state when report prop changes', () => {
    const { rerender } = render(<DriftAlert report={noDriftReport} />)
    expect(screen.queryByRole('alert')).not.toBeInTheDocument()

    rerender(<DriftAlert report={driftDetectedReport} />)
    expect(screen.getByRole('alert')).toBeInTheDocument()
  })
})
