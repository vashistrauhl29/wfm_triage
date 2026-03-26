import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { CostBreakdown } from '../CostBreakdown'
import type { CostBreakdownData } from '../../../../types/simulator'

const baseCost: CostBreakdownData = {
  api_cost: 100.0,
  human_cost: 400.0,
  total_cost: 500.0,
  cost_per_ticket: 0.05,
  stp_rate: 0.80,
}

describe('CostBreakdown', () => {
  // ── Section rendering ───────────────────────────────────────────────────
  it('renders the "Cost Breakdown" heading', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(
      screen.getByRole('heading', { name: /cost breakdown/i })
    ).toBeInTheDocument()
  })

  it('displays the API cost value formatted as currency', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/\$100\.00/)).toBeInTheDocument()
  })

  it('displays the human cost value formatted as currency', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/\$400\.00/)).toBeInTheDocument()
  })

  it('displays the total cost value formatted as currency', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/\$500\.00/)).toBeInTheDocument()
  })

  it('displays the cost per ticket', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/\$0\.05/)).toBeInTheDocument()
  })

  // ── Labels ──────────────────────────────────────────────────────────────
  it('labels the API cost row', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/api cost/i)).toBeInTheDocument()
  })

  it('labels the human cost row', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/human cost/i)).toBeInTheDocument()
  })

  it('labels the total cost row', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/total cost/i)).toBeInTheDocument()
  })

  it('labels the cost-per-ticket row', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/cost per ticket/i)).toBeInTheDocument()
  })

  // ── Visual proportion bar ───────────────────────────────────────────────
  it('renders an API cost proportion bar', () => {
    render(<CostBreakdown data={baseCost} />)
    const bar = screen.getByTestId('api-cost-bar')
    expect(bar).toBeInTheDocument()
  })

  it('renders a human cost proportion bar', () => {
    render(<CostBreakdown data={baseCost} />)
    const bar = screen.getByTestId('human-cost-bar')
    expect(bar).toBeInTheDocument()
  })

  it('sets the API cost bar width proportional to total cost', () => {
    // api_cost = 100, total = 500 → 20%
    render(<CostBreakdown data={baseCost} />)
    const bar = screen.getByTestId('api-cost-bar')
    expect(bar).toHaveStyle({ width: '20%' })
  })

  it('sets the human cost bar width proportional to total cost', () => {
    // human_cost = 400, total = 500 → 80%
    render(<CostBreakdown data={baseCost} />)
    const bar = screen.getByTestId('human-cost-bar')
    expect(bar).toHaveStyle({ width: '80%' })
  })

  // ── STP rate display ────────────────────────────────────────────────────
  it('displays the STP rate as a percentage', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/80\.00%/)).toBeInTheDocument()
  })

  it('labels the STP rate section', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByText(/stp rate/i)).toBeInTheDocument()
  })

  // ── Re-render with new data ─────────────────────────────────────────────
  it('updates all values when the data prop changes', () => {
    const { rerender } = render(<CostBreakdown data={baseCost} />)

    const updatedCost: CostBreakdownData = {
      api_cost: 200.0,
      human_cost: 300.0,
      total_cost: 500.0,
      cost_per_ticket: 0.05,
      stp_rate: 0.60,
    }

    rerender(<CostBreakdown data={updatedCost} />)

    expect(screen.getByText(/\$200\.00/)).toBeInTheDocument()
    expect(screen.getByText(/\$300\.00/)).toBeInTheDocument()
    expect(screen.getByText(/60\.00%/)).toBeInTheDocument()
  })

  // ── Edge: zero total cost ───────────────────────────────────────────────
  it('handles zero total cost without crashing', () => {
    const zeroCost: CostBreakdownData = {
      api_cost: 0,
      human_cost: 0,
      total_cost: 0,
      cost_per_ticket: 0,
      stp_rate: 0,
    }
    render(<CostBreakdown data={zeroCost} />)
    expect(screen.getByText(/\$0\.00/)).toBeInTheDocument()
  })

  // ── Color coding ────────────────────────────────────────────────────────
  it('uses blue for the API cost bar', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByTestId('api-cost-bar')).toHaveClass('bg-blue-500')
  })

  it('uses orange for the human cost bar', () => {
    render(<CostBreakdown data={baseCost} />)
    expect(screen.getByTestId('human-cost-bar')).toHaveClass('bg-orange-500')
  })
})
