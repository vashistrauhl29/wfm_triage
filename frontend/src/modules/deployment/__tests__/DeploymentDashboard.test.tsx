import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor, act } from '@testing-library/react'
import { DeploymentDashboard } from '../DeploymentDashboard'
import type { DeploymentPhase, EVMSnapshot, Milestone } from '../../../types/deployment'

// ─── Mock child components ─────────────────────────────────────────────────────

vi.mock('../components/PhaseTimeline', () => ({
  PhaseTimeline: ({ phases }: { phases: DeploymentPhase[] }) => (
    <div data-testid="phase-timeline" data-phase-count={phases.length}>
      {phases.map((p) => (
        <div key={p.id} data-testid="mock-phase" data-phase-number={p.phase_number}>
          {p.phase_name}
        </div>
      ))}
    </div>
  ),
}))

vi.mock('../components/EVMMetrics', () => ({
  EVMMetrics: ({ metrics }: { metrics: EVMSnapshot }) => (
    <div
      data-testid="evm-metrics"
      data-spi={metrics.schedule_performance_index}
      data-cpi={metrics.cost_performance_index}
      data-bac={metrics.budget_at_completion}
    >
      EVMMetrics
    </div>
  ),
}))

vi.mock('../components/MilestoneTracker', () => ({
  MilestoneTracker: ({
    milestones,
    onComplete,
  }: {
    milestones: Milestone[]
    onComplete: (id: string) => void
  }) => (
    <div data-testid="milestone-tracker" data-milestone-count={milestones.length}>
      <button onClick={() => onComplete('ms-1')}>CompleteMilestone</button>
    </div>
  ),
}))

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const mockMilestone: Milestone = {
  id: 'ms-1',
  phase_id: 'phase-1',
  milestone_name: 'Establish Baseline',
  planned_completion_date: '2026-02-15',
  status: 'pending',
  weight: 20,
  owner: 'Team Alpha',
  dependencies: [],
}

const mockPhases: DeploymentPhase[] = [
  {
    id: 'phase-1',
    phase_number: 1,
    phase_name: 'Manual Baseline & Context Capture',
    status: 'in_progress',
    budget_allocated: 50_000,
    actual_cost: 30_000,
    milestones: [mockMilestone],
  },
  {
    id: 'phase-2',
    phase_number: 2,
    phase_name: 'Shadow AI & Tooling Integration',
    status: 'not_started',
    budget_allocated: 50_000,
    actual_cost: 0,
    milestones: [],
  },
]

const mockEVM: EVMSnapshot = {
  planned_value: 120_000,
  earned_value: 130_000,
  actual_cost: 115_000,
  schedule_variance: 10_000,
  cost_variance: 15_000,
  schedule_performance_index: 1.08,
  cost_performance_index: 1.13,
  estimate_at_completion: 176_991,
  estimate_to_complete: 61_991,
  variance_at_completion: 23_009,
  budget_at_completion: 200_000,
}

const mockMilestoneCompletionResponse = {
  data: { id: 'ms-1', status: 'completed' },
}

function createFetchMock() {
  return vi.fn((url: string, init?: RequestInit) => {
    if (url.includes('/deployment/phases')) {
      return Promise.resolve({ ok: true, json: () => Promise.resolve({ data: mockPhases }) })
    }
    if (url.includes('/deployment/evm')) {
      return Promise.resolve({ ok: true, json: () => Promise.resolve({ data: mockEVM }) })
    }
    if (url.includes('/deployment/milestone') && init?.method === 'POST') {
      return Promise.resolve({ ok: true, json: () => Promise.resolve(mockMilestoneCompletionResponse) })
    }
    return Promise.resolve({ ok: false, json: () => Promise.resolve({}) })
  }) as unknown as typeof globalThis.fetch
}

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('DeploymentDashboard', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', createFetchMock())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.clearAllMocks()
  })

  // ── Heading ───────────────────────────────────────────────────────────────

  it('renders a "5-Phase Deployment Dashboard" h1 heading', () => {
    render(<DeploymentDashboard />)
    expect(
      screen.getByRole('heading', { name: /5-phase deployment dashboard/i, level: 1 })
    ).toBeInTheDocument()
  })

  // ── Initial render ─────────────────────────────────────────────────────────

  it('renders PhaseTimeline from the first render', () => {
    render(<DeploymentDashboard />)
    expect(screen.getByTestId('phase-timeline')).toBeInTheDocument()
  })

  it('renders EVMMetrics from the first render', () => {
    render(<DeploymentDashboard />)
    expect(screen.getByTestId('evm-metrics')).toBeInTheDocument()
  })

  it('renders MilestoneTracker from the first render', () => {
    render(<DeploymentDashboard />)
    expect(screen.getByTestId('milestone-tracker')).toBeInTheDocument()
  })

  // ── API calls on mount ─────────────────────────────────────────────────────

  it('fetches GET /api/v1/deployment/phases on mount', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/deployment/phases'),
        expect.anything()
      )
    })
  })

  it('fetches GET /api/v1/deployment/evm on mount', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/deployment/evm'),
        expect.anything()
      )
    })
  })

  // ── Data propagation ───────────────────────────────────────────────────────

  it('passes fetched phases to PhaseTimeline', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('phase-timeline')).toHaveAttribute('data-phase-count', '2')
    })
  })

  it('passes correct phase names to PhaseTimeline', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByText('Manual Baseline & Context Capture')).toBeInTheDocument()
    })
  })

  it('passes fetched EVM metrics SPI to EVMMetrics', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('evm-metrics')).toHaveAttribute('data-spi', '1.08')
    })
  })

  it('passes fetched EVM metrics CPI to EVMMetrics', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('evm-metrics')).toHaveAttribute('data-cpi', '1.13')
    })
  })

  it('passes milestones extracted from phases to MilestoneTracker', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      // Phase 1 has 1 milestone, phase 2 has 0 → total 1
      expect(screen.getByTestId('milestone-tracker')).toHaveAttribute('data-milestone-count', '1')
    })
  })

  // ── Loading state ──────────────────────────────────────────────────────────

  it('shows "Loading deployment data..." while fetches are in-flight', () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(() => new Promise(() => {})) as unknown as typeof globalThis.fetch
    )
    render(<DeploymentDashboard />)
    expect(screen.getByText(/loading deployment data/i)).toBeInTheDocument()
  })

  it('hides "Loading deployment data..." after all fetches resolve', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.queryByText(/loading deployment data/i)).not.toBeInTheDocument()
    })
  })

  // ── Error state ────────────────────────────────────────────────────────────

  it('shows an error message when a fetch fails', async () => {
    vi.stubGlobal(
      'fetch',
      vi.fn(() =>
        Promise.resolve({ ok: false, json: () => Promise.resolve({}) })
      ) as unknown as typeof globalThis.fetch
    )
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(
        screen.getByText(/failed to load deployment data|error/i)
      ).toBeInTheDocument()
    })
  })

  // ── Milestone completion ───────────────────────────────────────────────────

  it('POSTs to /api/v1/deployment/milestone when onComplete is triggered', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('milestone-tracker')).toHaveAttribute('data-milestone-count', '1')
    })

    await act(async () => {
      screen.getByText('CompleteMilestone').click()
    })

    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        expect.stringContaining('/api/v1/deployment/milestone'),
        expect.objectContaining({ method: 'POST' })
      )
    })
  })

  it('includes the milestone id in the POST body', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('milestone-tracker')).toHaveAttribute('data-milestone-count', '1')
    })

    await act(async () => {
      screen.getByText('CompleteMilestone').click()
    })

    await waitFor(() => {
      const postCall = (fetch as ReturnType<typeof vi.fn>).mock.calls.find(
        ([url, init]) =>
          typeof url === 'string' &&
          url.includes('/deployment/milestone') &&
          init?.method === 'POST'
      )
      expect(postCall).toBeDefined()
      const body = JSON.parse(postCall![1].body as string)
      expect(body.milestone_id).toBe('ms-1')
    })
  })

  it('re-fetches phases after a milestone is completed', async () => {
    render(<DeploymentDashboard />)
    await waitFor(() => {
      expect(screen.getByTestId('milestone-tracker')).toHaveAttribute('data-milestone-count', '1')
    })

    const fetchCallCountBefore = (fetch as ReturnType<typeof vi.fn>).mock.calls.filter(
      ([url]) => typeof url === 'string' && url.includes('/deployment/phases')
    ).length

    await act(async () => {
      screen.getByText('CompleteMilestone').click()
    })

    await waitFor(() => {
      const fetchCallCountAfter = (fetch as ReturnType<typeof vi.fn>).mock.calls.filter(
        ([url]) => typeof url === 'string' && url.includes('/deployment/phases')
      ).length
      expect(fetchCallCountAfter).toBeGreaterThan(fetchCallCountBefore)
    })
  })
})
