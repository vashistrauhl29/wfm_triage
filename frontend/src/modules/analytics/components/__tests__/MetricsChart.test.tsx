import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { MetricsChart } from '../MetricsChart'
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

const makeSnapshot = (date: string, overrides: Partial<MetricsSnapshot> = {}): MetricsSnapshot => ({
  date,
  accuracy: 0.91,
  stp_rate: 0.72,
  override_rate: 0.08,
  volume: 1200,
  total_cost: 5400,
  api_cost: 1200,
  human_cost: 4200,
  ...overrides,
})

const snapshots: MetricsSnapshot[] = [
  makeSnapshot('2026-03-18', { accuracy: 0.88, stp_rate: 0.65, override_rate: 0.12 }),
  makeSnapshot('2026-03-19', { accuracy: 0.89, stp_rate: 0.67, override_rate: 0.11 }),
  makeSnapshot('2026-03-20', { accuracy: 0.90, stp_rate: 0.69, override_rate: 0.10 }),
  makeSnapshot('2026-03-21', { accuracy: 0.91, stp_rate: 0.71, override_rate: 0.09 }),
  makeSnapshot('2026-03-22', { accuracy: 0.92, stp_rate: 0.73, override_rate: 0.08 }),
]

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('MetricsChart', () => {
  // ── Heading & container ───────────────────────────────────────────────────

  it('renders the "Model Performance Metrics" heading', () => {
    render(<MetricsChart data={snapshots} />)
    expect(
      screen.getByRole('heading', { name: /model performance metrics/i })
    ).toBeInTheDocument()
  })

  it('renders with data-testid="metrics-chart"', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('metrics-chart')).toBeInTheDocument()
  })

  it('renders a ResponsiveContainer', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('responsive-container')).toBeInTheDocument()
  })

  // ── Chart structure ───────────────────────────────────────────────────────

  it('renders a LineChart with the correct number of data points', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '5')
  })

  it('renders an accuracy line', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-accuracy')).toBeInTheDocument()
  })

  it('renders an stp_rate line', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-stp_rate')).toBeInTheDocument()
  })

  it('renders an override_rate line', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-override_rate')).toBeInTheDocument()
  })

  it('uses "date" as the X-axis data key', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('x-axis')).toHaveAttribute('data-key', 'date')
  })

  it('renders a Y-axis', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('y-axis')).toBeInTheDocument()
  })

  it('renders a Tooltip', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('tooltip')).toBeInTheDocument()
  })

  it('renders a Legend', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('legend')).toBeInTheDocument()
  })

  it('renders a CartesianGrid', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('grid')).toBeInTheDocument()
  })

  // ── Empty state ───────────────────────────────────────────────────────────

  it('shows an empty state message when data is empty', () => {
    render(<MetricsChart data={[]} />)
    expect(screen.getByText(/no metrics data/i)).toBeInTheDocument()
  })

  it('does not render a chart when data is empty', () => {
    render(<MetricsChart data={[]} />)
    expect(screen.queryByTestId('line-chart')).not.toBeInTheDocument()
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('updates data-points when data prop changes', () => {
    const { rerender } = render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '5')

    rerender(<MetricsChart data={snapshots.slice(0, 3)} />)
    expect(screen.getByTestId('line-chart')).toHaveAttribute('data-points', '3')
  })

  // ── Line colors ───────────────────────────────────────────────────────────

  it('accuracy line has a defined stroke color', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-accuracy').getAttribute('data-stroke')).toBeTruthy()
  })

  it('stp_rate line has a defined stroke color', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-stp_rate').getAttribute('data-stroke')).toBeTruthy()
  })

  it('override_rate line has a defined stroke color', () => {
    render(<MetricsChart data={snapshots} />)
    expect(screen.getByTestId('line-override_rate').getAttribute('data-stroke')).toBeTruthy()
  })

  it('all three metric lines use distinct stroke colors', () => {
    render(<MetricsChart data={snapshots} />)
    const accuracyStroke = screen.getByTestId('line-accuracy').getAttribute('data-stroke')
    const stpStroke = screen.getByTestId('line-stp_rate').getAttribute('data-stroke')
    const overrideStroke = screen.getByTestId('line-override_rate').getAttribute('data-stroke')
    expect(accuracyStroke).not.toBe(stpStroke)
    expect(accuracyStroke).not.toBe(overrideStroke)
    expect(stpStroke).not.toBe(overrideStroke)
  })
})
