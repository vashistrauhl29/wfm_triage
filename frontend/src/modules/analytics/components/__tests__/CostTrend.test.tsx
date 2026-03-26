import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { CostTrend } from '../CostTrend'
import type { MetricsSnapshot } from '../../../../types/analytics'

// ─── Recharts mock ────────────────────────────────────────────────────────────

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
    YAxis: () => <div data-testid="y-axis" />,
    Tooltip: () => <div data-testid="tooltip" />,
    Legend: () => <div data-testid="legend" />,
    CartesianGrid: () => <div data-testid="grid" />,
  }
})

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const makeSnapshot = (date: string, costs: Pick<MetricsSnapshot, 'total_cost' | 'api_cost' | 'human_cost'>): MetricsSnapshot => ({
  date,
  accuracy: 0.91,
  stp_rate: 0.72,
  override_rate: 0.08,
  volume: 1200,
  ...costs,
})

const costData: MetricsSnapshot[] = [
  makeSnapshot('2026-03-18', { total_cost: 5200, api_cost: 1100, human_cost: 4100 }),
  makeSnapshot('2026-03-19', { total_cost: 5300, api_cost: 1150, human_cost: 4150 }),
  makeSnapshot('2026-03-20', { total_cost: 5100, api_cost: 1050, human_cost: 4050 }),
  makeSnapshot('2026-03-21', { total_cost: 4900, api_cost: 980, human_cost: 3920 }),
  makeSnapshot('2026-03-22', { total_cost: 4700, api_cost: 940, human_cost: 3760 }),
]

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('CostTrend', () => {
  // ── Heading & container ───────────────────────────────────────────────────

  it('renders the "Daily WFM Cost Trend" heading', () => {
    render(<CostTrend data={costData} />)
    expect(
      screen.getByRole('heading', { name: /daily wfm cost trend/i })
    ).toBeInTheDocument()
  })

  it('renders with data-testid="cost-trend"', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('cost-trend')).toBeInTheDocument()
  })

  it('renders a ResponsiveContainer', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('responsive-container')).toBeInTheDocument()
  })

  // ── Chart structure ───────────────────────────────────────────────────────

  it('renders a LineChart with the correct number of data points', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '5')
  })

  it('renders a total_cost line', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('line-total_cost')).toBeInTheDocument()
  })

  it('renders an api_cost line', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('line-api_cost')).toBeInTheDocument()
  })

  it('renders a human_cost line', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('line-human_cost')).toBeInTheDocument()
  })

  it('uses "date" as the X-axis data key', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('x-axis')).toHaveAttribute('data-key', 'date')
  })

  it('renders a Y-axis', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('y-axis')).toBeInTheDocument()
  })

  it('renders a Tooltip', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('tooltip')).toBeInTheDocument()
  })

  it('renders a Legend', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('legend')).toBeInTheDocument()
  })

  it('renders a CartesianGrid', () => {
    render(<CostTrend data={costData} />)
    expect(screen.getByTestId('grid')).toBeInTheDocument()
  })

  // ── Empty state ───────────────────────────────────────────────────────────

  it('shows an empty state message when data is empty', () => {
    render(<CostTrend data={[]} />)
    expect(screen.getByText(/no cost data/i)).toBeInTheDocument()
  })

  it('does not render a chart when data is empty', () => {
    render(<CostTrend data={[]} />)
    expect(screen.queryByTestId('line-chart')).not.toBeInTheDocument()
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('updates data-points when data prop changes', () => {
    const { rerender } = render(<CostTrend data={costData} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '5')

    rerender(<CostTrend data={costData.slice(0, 2)} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '2')
  })

  // ── Line colors ───────────────────────────────────────────────────────────

  it('all three cost lines use distinct stroke colors', () => {
    render(<CostTrend data={costData} />)
    const totalStroke = screen.getByTestId('line-total_cost').getAttribute('data-stroke')
    const apiStroke = screen.getByTestId('line-api_cost').getAttribute('data-stroke')
    const humanStroke = screen.getByTestId('line-human_cost').getAttribute('data-stroke')
    expect(totalStroke).toBeTruthy()
    expect(apiStroke).toBeTruthy()
    expect(humanStroke).toBeTruthy()
    expect(totalStroke).not.toBe(apiStroke)
    expect(totalStroke).not.toBe(humanStroke)
    expect(apiStroke).not.toBe(humanStroke)
  })
})
