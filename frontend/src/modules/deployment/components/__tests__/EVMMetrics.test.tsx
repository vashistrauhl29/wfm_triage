import { describe, it, expect, vi, afterEach } from 'vitest'
import { render, screen } from '@testing-library/react'
import { EVMMetrics } from '../EVMMetrics'
import type { EVMSnapshot } from '../../../../types/deployment'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

// Healthy project: ahead of schedule, under budget
const healthyMetrics: EVMSnapshot = {
  planned_value: 120_000,
  earned_value: 130_000,
  actual_cost: 115_000,
  schedule_variance: 10_000,       // SV = EV - PV > 0 (ahead)
  cost_variance: 15_000,           // CV = EV - AC > 0 (under budget)
  schedule_performance_index: 1.08, // SPI > 1 (ahead of schedule)
  cost_performance_index: 1.13,    // CPI > 1 (under budget)
  estimate_at_completion: 176_991,
  estimate_to_complete: 61_991,
  variance_at_completion: 23_009,  // VAC > 0 (projected savings)
  budget_at_completion: 200_000,
}

// Unhealthy project: behind schedule, over budget
const unhealthyMetrics: EVMSnapshot = {
  planned_value: 150_000,
  earned_value: 120_000,
  actual_cost: 140_000,
  schedule_variance: -30_000,      // SV < 0 (behind)
  cost_variance: -20_000,          // CV < 0 (over budget)
  schedule_performance_index: 0.80, // SPI < 1 (behind schedule)
  cost_performance_index: 0.86,    // CPI < 1 (over budget)
  estimate_at_completion: 232_558,
  estimate_to_complete: 92_558,
  variance_at_completion: -32_558, // VAC < 0 (projected overrun)
  budget_at_completion: 200_000,
}

// Boundary: exactly on schedule and budget
const exactlyOnTarget: EVMSnapshot = {
  ...healthyMetrics,
  schedule_performance_index: 1.0,
  cost_performance_index: 1.0,
  schedule_variance: 0,
  cost_variance: 0,
  variance_at_completion: 0,
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('EVMMetrics', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Container & heading ───────────────────────────────────────────────────

  it('renders with data-testid="evm-metrics"', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('evm-metrics')).toBeInTheDocument()
  })

  it('renders an "EVM Metrics" heading', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(
      screen.getByRole('heading', { name: /evm metrics/i })
    ).toBeInTheDocument()
  })

  // ── SPI ───────────────────────────────────────────────────────────────────

  it('renders an element with data-testid="spi-value"', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('spi-value')).toBeInTheDocument()
  })

  it('displays the SPI value', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('spi-value').textContent).toMatch(/1\.08/)
  })

  it('applies a green class to spi-value when SPI >= 1', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('spi-value').className).toMatch(/green/)
  })

  it('applies a red class to spi-value when SPI < 1', () => {
    render(<EVMMetrics metrics={unhealthyMetrics} />)
    expect(screen.getByTestId('spi-value').className).toMatch(/red/)
  })

  it('applies a green class to spi-value when SPI is exactly 1.0', () => {
    render(<EVMMetrics metrics={exactlyOnTarget} />)
    expect(screen.getByTestId('spi-value').className).toMatch(/green/)
  })

  it('shows a "SPI" label', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByText(/\bspi\b/i)).toBeInTheDocument()
  })

  // ── CPI ───────────────────────────────────────────────────────────────────

  it('renders an element with data-testid="cpi-value"', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('cpi-value')).toBeInTheDocument()
  })

  it('displays the CPI value', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('cpi-value').textContent).toMatch(/1\.13/)
  })

  it('applies a green class to cpi-value when CPI >= 1', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('cpi-value').className).toMatch(/green/)
  })

  it('applies a red class to cpi-value when CPI < 1', () => {
    render(<EVMMetrics metrics={unhealthyMetrics} />)
    expect(screen.getByTestId('cpi-value').className).toMatch(/red/)
  })

  it('applies a green class to cpi-value when CPI is exactly 1.0', () => {
    render(<EVMMetrics metrics={exactlyOnTarget} />)
    expect(screen.getByTestId('cpi-value').className).toMatch(/green/)
  })

  it('shows a "CPI" label', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByText(/\bcpi\b/i)).toBeInTheDocument()
  })

  // ── EAC ───────────────────────────────────────────────────────────────────

  it('renders an element with data-testid="eac-value"', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('eac-value')).toBeInTheDocument()
  })

  it('displays EAC formatted as currency with a $ sign', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('eac-value').textContent).toMatch(/\$/)
  })

  it('shows an "EAC" label', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByText(/\beac\b/i)).toBeInTheDocument()
  })

  // ── VAC ───────────────────────────────────────────────────────────────────

  it('renders an element with data-testid="vac-value"', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('vac-value')).toBeInTheDocument()
  })

  it('displays VAC formatted as currency with a $ sign', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('vac-value').textContent).toMatch(/\$/)
  })

  it('applies a green class to vac-value when VAC > 0 (projected savings)', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('vac-value').className).toMatch(/green/)
  })

  it('applies a red class to vac-value when VAC < 0 (projected overrun)', () => {
    render(<EVMMetrics metrics={unhealthyMetrics} />)
    expect(screen.getByTestId('vac-value').className).toMatch(/red/)
  })

  it('shows a "VAC" label', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByText(/\bvac\b/i)).toBeInTheDocument()
  })

  // ── Budget at completion ───────────────────────────────────────────────────

  it('shows the budget_at_completion value', () => {
    render(<EVMMetrics metrics={healthyMetrics} />)
    // $200,000 formatted with $ sign
    expect(screen.getByTestId('evm-metrics').textContent).toMatch(/200,000|200000/)
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('switches SPI indicator from green to red when metrics change to unhealthy', () => {
    const { rerender } = render(<EVMMetrics metrics={healthyMetrics} />)
    expect(screen.getByTestId('spi-value').className).toMatch(/green/)

    rerender(<EVMMetrics metrics={unhealthyMetrics} />)
    expect(screen.getByTestId('spi-value').className).toMatch(/red/)
  })
})
