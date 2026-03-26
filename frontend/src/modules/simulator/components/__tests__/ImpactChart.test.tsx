import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { ImpactChart } from '../ImpactChart'
import type { ScenarioDataPoint } from '../../../../types/simulator'

// Mock Recharts to avoid canvas/SVG rendering issues in jsdom
vi.mock('recharts', () => {
  const OriginalModule = vi.importActual('recharts')
  return {
    ...OriginalModule,
    ResponsiveContainer: ({ children }: { children: React.ReactNode }) => (
      <div data-testid="responsive-container" style={{ width: 500, height: 300 }}>
        {children}
      </div>
    ),
    LineChart: ({ children, data }: { children: React.ReactNode; data: unknown[] }) => (
      <div data-testid="line-chart" data-points={data.length}>
        {children}
      </div>
    ),
    Line: ({ dataKey, stroke }: { dataKey: string; stroke: string }) => (
      <div data-testid={`line-${dataKey}`} data-stroke={stroke} />
    ),
    XAxis: ({ dataKey }: { dataKey: string }) => (
      <div data-testid="x-axis" data-key={dataKey} />
    ),
    YAxis: ({ dataKey }: { dataKey?: string }) => (
      <div data-testid="y-axis" data-key={dataKey ?? ''} />
    ),
    Tooltip: () => <div data-testid="tooltip" />,
    Legend: () => <div data-testid="legend" />,
    CartesianGrid: () => <div data-testid="grid" />,
    ReferenceLine: ({ x }: { x?: number }) => (
      <div data-testid="reference-line" data-x={x} />
    ),
  }
})

const scenarios: ScenarioDataPoint[] = [
  { threshold: 0.90, projected_stp_rate: 0.82, projected_cost: 4200, cost_per_ticket: 0.42 },
  { threshold: 0.92, projected_stp_rate: 0.78, projected_cost: 4500, cost_per_ticket: 0.45 },
  { threshold: 0.95, projected_stp_rate: 0.70, projected_cost: 5000, cost_per_ticket: 0.50 },
  { threshold: 0.97, projected_stp_rate: 0.60, projected_cost: 5800, cost_per_ticket: 0.58 },
  { threshold: 0.99, projected_stp_rate: 0.45, projected_cost: 7000, cost_per_ticket: 0.70 },
]

describe('ImpactChart', () => {
  // ── Container & heading ─────────────────────────────────────────────────
  it('renders the "Cost Projection" heading', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(
      screen.getByRole('heading', { name: /cost projection/i })
    ).toBeInTheDocument()
  })

  it('renders a responsive chart container', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('responsive-container')).toBeInTheDocument()
  })

  // ── Chart structure ─────────────────────────────────────────────────────
  it('renders a LineChart with the correct number of data points', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    const chart = screen.getByTestId('line-chart')
    expect(chart).toHaveAttribute('data-points', '5')
  })

  it('renders a cost line', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('line-projected_cost')).toBeInTheDocument()
  })

  it('renders an STP rate line', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('line-projected_stp_rate')).toBeInTheDocument()
  })

  it('uses threshold as the X-axis data key', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('x-axis')).toHaveAttribute(
      'data-key',
      'threshold'
    )
  })

  it('renders a Y-axis', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('y-axis')).toBeInTheDocument()
  })

  it('renders a tooltip', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('tooltip')).toBeInTheDocument()
  })

  it('renders a legend', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('legend')).toBeInTheDocument()
  })

  it('renders a cartesian grid', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('grid')).toBeInTheDocument()
  })

  // ── Current-threshold reference line ────────────────────────────────────
  it('renders a reference line at the current threshold', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    const refLine = screen.getByTestId('reference-line')
    expect(refLine).toHaveAttribute('data-x', '0.95')
  })

  it('updates the reference line when currentThreshold changes', () => {
    const { rerender } = render(
      <ImpactChart scenarios={scenarios} currentThreshold={0.95} />
    )
    expect(screen.getByTestId('reference-line')).toHaveAttribute(
      'data-x',
      '0.95'
    )

    rerender(<ImpactChart scenarios={scenarios} currentThreshold={0.92} />)
    expect(screen.getByTestId('reference-line')).toHaveAttribute(
      'data-x',
      '0.92'
    )
  })

  // ── Empty state ─────────────────────────────────────────────────────────
  it('shows an empty state when no scenarios are provided', () => {
    render(<ImpactChart scenarios={[]} currentThreshold={0.95} />)
    expect(screen.getByText(/no scenario data/i)).toBeInTheDocument()
  })

  // ── Re-render with new data ─────────────────────────────────────────────
  it('updates chart data when scenarios prop changes', () => {
    const { rerender } = render(
      <ImpactChart scenarios={scenarios} currentThreshold={0.95} />
    )
    expect(screen.getByTestId('line-chart')).toHaveAttribute(
      'data-points',
      '5'
    )

    const twoScenarios = scenarios.slice(0, 2)
    rerender(<ImpactChart scenarios={twoScenarios} currentThreshold={0.95} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute(
      'data-points',
      '2'
    )
  })

  // ── Line colors ─────────────────────────────────────────────────────────
  it('uses a distinct color for the cost line', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    const costLine = screen.getByTestId('line-projected_cost')
    expect(costLine.getAttribute('data-stroke')).toBeTruthy()
  })

  it('uses a distinct color for the STP rate line', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    const stpLine = screen.getByTestId('line-projected_stp_rate')
    expect(stpLine.getAttribute('data-stroke')).toBeTruthy()
  })

  it('uses different colors for cost and STP rate lines', () => {
    render(<ImpactChart scenarios={scenarios} currentThreshold={0.95} />)
    const costStroke = screen
      .getByTestId('line-projected_cost')
      .getAttribute('data-stroke')
    const stpStroke = screen
      .getByTestId('line-projected_stp_rate')
      .getAttribute('data-stroke')
    expect(costStroke).not.toBe(stpStroke)
  })
})
