import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, act } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { SimulatorView } from '../SimulatorView'

// ─── Mock child components ───────────────────────────────────────────────────
vi.mock('../components/ThresholdSlider', () => ({
  ThresholdSlider: ({
    value,
    onChange,
  }: {
    value: number
    onChange: (v: number) => void
  }) => (
    <div data-testid="threshold-slider" data-value={value}>
      <input
        type="range"
        role="slider"
        value={value}
        onChange={(e) => onChange(parseFloat(e.target.value))}
        data-testid="threshold-input"
      />
    </div>
  ),
}))

vi.mock('../components/CostBreakdown', () => ({
  CostBreakdown: ({ data }: { data: unknown }) => (
    <div data-testid="cost-breakdown" data-payload={JSON.stringify(data)}>
      CostBreakdown
    </div>
  ),
}))

vi.mock('../components/ImpactChart', () => ({
  ImpactChart: ({
    scenarios,
    currentThreshold,
  }: {
    scenarios: unknown[]
    currentThreshold: number
  }) => (
    <div
      data-testid="impact-chart"
      data-scenarios={scenarios.length}
      data-threshold={currentThreshold}
    >
      ImpactChart
    </div>
  ),
}))

// ─── fetch mock ──────────────────────────────────────────────────────────────
const mockCalculateResponse = {
  data: {
    api_cost: 100.0,
    human_cost: 400.0,
    total_cost: 500.0,
    cost_per_ticket: 0.05,
    stp_rate: 0.80,
  },
}

const mockScenariosResponse = {
  data: {
    scenarios: [
      { threshold: 0.90, projected_stp_rate: 0.82, projected_cost: 4200, cost_per_ticket: 0.42 },
      { threshold: 0.92, projected_stp_rate: 0.78, projected_cost: 4500, cost_per_ticket: 0.45 },
      { threshold: 0.95, projected_stp_rate: 0.70, projected_cost: 5000, cost_per_ticket: 0.50 },
      { threshold: 0.97, projected_stp_rate: 0.60, projected_cost: 5800, cost_per_ticket: 0.58 },
      { threshold: 0.99, projected_stp_rate: 0.45, projected_cost: 7000, cost_per_ticket: 0.70 },
    ],
  },
}

function createFetchMock() {
  return vi.fn((url: string, init?: RequestInit) => {
    if (url.includes('/scenarios')) {
      return Promise.resolve({
        ok: true,
        json: () => Promise.resolve(mockScenariosResponse),
      })
    }
    if (url.includes('/calculate')) {
      return Promise.resolve({
        ok: true,
        json: () => Promise.resolve(mockCalculateResponse),
      })
    }
    return Promise.resolve({
      ok: false,
      json: () => Promise.resolve({ error: { code: 'not_found', message: 'Not found' } }),
    })
  }) as unknown as typeof globalThis.fetch
}

describe('SimulatorView', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Static rendering ────────────────────────────────────────────────────
  describe('initial render', () => {
    it('renders the "Dynamic Cost Simulator" heading', () => {
      render(<SimulatorView />)
      expect(
        screen.getByRole('heading', { name: /dynamic cost simulator/i })
      ).toBeInTheDocument()
    })

    it('renders the ThresholdSlider child component', () => {
      render(<SimulatorView />)
      expect(screen.getByTestId('threshold-slider')).toBeInTheDocument()
    })

    it('renders the CostBreakdown child component', () => {
      render(<SimulatorView />)
      expect(screen.getByTestId('cost-breakdown')).toBeInTheDocument()
    })

    it('renders the ImpactChart child component', () => {
      render(<SimulatorView />)
      expect(screen.getByTestId('impact-chart')).toBeInTheDocument()
    })

    it('initialises the threshold slider to 0.95', () => {
      render(<SimulatorView />)
      expect(screen.getByTestId('threshold-slider')).toHaveAttribute(
        'data-value',
        '0.95'
      )
    })
  })

  // ── API integration ─────────────────────────────────────────────────────
  describe('API integration', () => {
    it('fetches scenarios on mount', async () => {
      render(<SimulatorView />)

      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/api/v1/simulator/scenarios'),
          expect.anything()
        )
      })
    })

    it('populates the ImpactChart with fetched scenarios', async () => {
      render(<SimulatorView />)

      await waitFor(() => {
        expect(screen.getByTestId('impact-chart')).toHaveAttribute(
          'data-scenarios',
          '5'
        )
      })
    })

    it('fetches cost calculation on mount', async () => {
      render(<SimulatorView />)

      await waitFor(() => {
        expect(fetch).toHaveBeenCalledWith(
          expect.stringContaining('/api/v1/simulator/calculate'),
          expect.objectContaining({ method: 'POST' })
        )
      })
    })

    it('populates CostBreakdown with the calculated cost data', async () => {
      render(<SimulatorView />)

      await waitFor(() => {
        const el = screen.getByTestId('cost-breakdown')
        const payload = JSON.parse(el.getAttribute('data-payload') ?? '{}')
        expect(payload.total_cost).toBe(500.0)
        expect(payload.api_cost).toBe(100.0)
        expect(payload.human_cost).toBe(400.0)
      })
    })
  })

  // ── Threshold interaction ───────────────────────────────────────────────
  describe('threshold interaction', () => {
    it('updates the ImpactChart current threshold when slider changes', async () => {
      render(<SimulatorView />)

      const input = screen.getByTestId('threshold-input')
      await act(async () => {
        input.setAttribute('value', '0.90')
        input.dispatchEvent(new Event('change', { bubbles: true }))
      })

      await waitFor(() => {
        expect(screen.getByTestId('impact-chart')).toHaveAttribute(
          'data-threshold',
          '0.9'
        )
      })
    })

    it('triggers a new cost calculation when the threshold changes', async () => {
      render(<SimulatorView />)

      // Wait for initial fetches to settle
      await waitFor(() => {
        expect(fetch).toHaveBeenCalled()
      })

      const callsBefore = (fetch as ReturnType<typeof vi.fn>).mock.calls.length

      const input = screen.getByTestId('threshold-input')
      await act(async () => {
        input.setAttribute('value', '0.92')
        input.dispatchEvent(new Event('change', { bubbles: true }))
      })

      await waitFor(() => {
        const callsAfter = (fetch as ReturnType<typeof vi.fn>).mock.calls.length
        expect(callsAfter).toBeGreaterThan(callsBefore)
      })
    })
  })

  // ── Loading / error states ──────────────────────────────────────────────
  describe('loading and error states', () => {
    it('shows a loading indicator while fetching data', () => {
      // Use a fetch that never resolves
      vi.stubGlobal(
        'fetch',
        vi.fn(() => new Promise(() => {})) as unknown as typeof globalThis.fetch
      )

      render(<SimulatorView />)
      expect(screen.getByText(/loading/i)).toBeInTheDocument()
    })

    it('shows an error message when fetch fails', async () => {
      vi.stubGlobal(
        'fetch',
        vi.fn(() => Promise.reject(new Error('Network error'))) as unknown as typeof globalThis.fetch
      )

      render(<SimulatorView />)

      await waitFor(() => {
        expect(
          screen.getByText(/error|failed|unable/i)
        ).toBeInTheDocument()
      })
    })

    it('hides the loading indicator once data arrives', async () => {
      render(<SimulatorView />)

      await waitFor(() => {
        expect(screen.getByTestId('impact-chart')).toHaveAttribute(
          'data-scenarios',
          '5'
        )
      })

      expect(screen.queryByText(/loading/i)).not.toBeInTheDocument()
    })
  })
})
