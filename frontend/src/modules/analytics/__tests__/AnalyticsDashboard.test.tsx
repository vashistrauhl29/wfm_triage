import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import { AnalyticsDashboard } from '../AnalyticsDashboard'
import type { MetricsSnapshot, DriftReport, FlywheelData } from '../../../types/analytics'

// ─── Mock child components ─────────────────────────────────────────────────────

vi.mock('../components/MetricsChart', () => ({
  MetricsChart: ({ data }: { data: MetricsSnapshot[] }) => (
    <div data-testid="metrics-chart" data-point-count={data.length}>
      MetricsChart
    </div>
  ),
}))

vi.mock('../components/DriftAlert', () => ({
  DriftAlert: ({ report }: { report: DriftReport }) => (
    <div
      data-testid="drift-alert"
      data-detected={report.detected ? 'true' : 'false'}
      data-model-version={report.model_version}
    >
      DriftAlert
    </div>
  ),
}))

vi.mock('../components/CostTrend', () => ({
  CostTrend: ({ data }: { data: MetricsSnapshot[] }) => (
    <div data-testid="cost-trend" data-point-count={data.length}>
      CostTrend
    </div>
  ),
}))

vi.mock('../components/Flywheel', () => ({
  Flywheel: ({ data }: { data: FlywheelData }) => (
    <div
      data-testid="flywheel"
      data-feedback-count={data.feedback_count}
      data-model-version={data.model_version}
    >
      Flywheel
    </div>
  ),
}))

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const mockSnapshot: MetricsSnapshot = {
  date: '2026-03-24',
  accuracy: 0.91,
  stp_rate: 0.72,
  override_rate: 0.08,
  volume: 1200,
  total_cost: 5400,
  api_cost: 1200,
  human_cost: 4200,
}

const mockMetricsResponse = { data: [mockSnapshot, { ...mockSnapshot, date: '2026-03-23' }] }

const mockDriftReport: DriftReport = {
  detected: true,
  drift_magnitude: 0.08,
  baseline_accuracy: 0.93,
  current_accuracy: 0.85,
  model_version: 'v2.3.1',
  last_checked: '2026-03-24T10:00:00Z',
}

const mockDriftResponse = { data: mockDriftReport }

const mockFlywheelData: FlywheelData = {
  feedback_count: 342,
  model_version: 'v2.3.1',
  last_retrained: '2026-03-20T08:00:00Z',
  stp_improvement: 0.05,
}

const mockFlywheelResponse = { data: mockFlywheelData }

function createFetchMock() {
  return vi.fn((url: string) => {
    if (url.includes('/analytics/metrics')) {
      return Promise.resolve({ ok: true, json: () => Promise.resolve(mockMetricsResponse) })
    }
    if (url.includes('/analytics/drift')) {
      return Promise.resolve({ ok: true, json: () => Promise.resolve(mockDriftResponse) })
    }
    if (url.includes('/analytics/flywheel')) {
      return Promise.resolve({ ok: true, json: () => Promise.resolve(mockFlywheelResponse) })
    }
    return Promise.resolve({ ok: false, json: () => Promise.resolve({}) })
  }) as unknown as typeof globalThis.fetch
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('AnalyticsDashboard', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Heading ───────────────────────────────────────────────────────────────

  it('renders a "Time-Lapse Analytics" h1 heading', () => {
    render(<AnalyticsDashboard />)
    expect(
      screen.getByRole('heading', { name: /time-lapse analytics/i, level: 1 })
    ).toBeInTheDocument()
  })

  // ── Initial render ─────────────────────────────────────────────────────────

  it('renders MetricsChart from the first render', () => {
    render(<AnalyticsDashboard />)
    expect(screen.getByTestId('metrics-chart')).toBeInTheDocument()
  })

  it('renders DriftAlert from the first render', () => {
    render(<AnalyticsDashboard />)
    expect(screen.getByTestId('drift-alert')).toBeInTheDocument()
  })

  it('renders CostTrend from the first render', () => {
    render(<AnalyticsDashboard />)
    expect(screen.getByTestId('cost-trend')).toBeInTheDocument()
  })

  it('renders Flywheel from the first render', () => {
    render(<AnalyticsDashboard />)
    expect(screen.getByTestId('flywheel')).toBeInTheDocument()
  })

  // ── API calls ──────────────────────────────────────────────────────────────

  it('fetches /api/v1/analytics/metrics on mount', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/analytics/metrics'),
        expect.anything()
      )
    })
  })

  it('fetches /api/v1/analytics/drift on mount', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/analytics/drift'),
        expect.anything()
      )
    })
  })

  it('fetches /api/v1/analytics/flywheel on mount', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/analytics/flywheel'),
        expect.anything()
      )
    })
  })

  // ── Data propagation ───────────────────────────────────────────────────────

  it('passes fetched metrics snapshots to MetricsChart', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('metrics-chart')).toHaveAttribute('data-point-count', '2')
    })
  })

  it('passes fetched metrics snapshots to CostTrend', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('cost-trend')).toHaveAttribute('data-point-count', '2')
    })
  })

  it('passes the drift report detected flag to DriftAlert', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('drift-alert')).toHaveAttribute('data-detected', 'true')
    })
  })

  it('passes the drift report model_version to DriftAlert', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('drift-alert')).toHaveAttribute('data-model-version', 'v2.3.1')
    })
  })

  it('passes flywheel feedback_count to Flywheel', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('flywheel')).toHaveAttribute('data-feedback-count', '342')
    })
  })

  // ── Loading state ──────────────────────────────────────────────────────────

  it('shows "Loading analytics..." while fetches are in-flight', () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(() => new Promise(() => {})) as unknown as typeof globalThis.fetch
    )
    render(<AnalyticsDashboard />)
    expect(screen.getByText(/loading analytics/i)).toBeInTheDocument()
  })

  it('hides "Loading analytics..." after all fetches resolve', async () => {
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.queryByText(/loading analytics/i)).not.toBeInTheDocument()
    })
  })

  // ── Error state ────────────────────────────────────────────────────────────

  it('shows an error message when a fetch fails', async () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(() => Promise.resolve({ ok: false, json: () => Promise.resolve({}) })) as unknown as typeof globalThis.fetch
    )
    render(<AnalyticsDashboard />)
    await waitFor(() => {
      expect(screen.getByText(/failed to load analytics|error/i)).toBeInTheDocument()
    })
  })
})
