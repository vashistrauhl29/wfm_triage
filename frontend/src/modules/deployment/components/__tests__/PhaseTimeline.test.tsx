import { describe, it, expect, vi, afterEach } from 'vitest'
import { render, screen } from '@testing-library/react'
import { PhaseTimeline } from '../PhaseTimeline'
import type { DeploymentPhase } from '../../../../types/deployment'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const makePhase = (overrides: Partial<DeploymentPhase> & { phase_number: number }): DeploymentPhase => ({
  id: `phase-${overrides.phase_number}`,
  phase_name: `Phase ${overrides.phase_number} Name`,
  description: 'A deployment phase.',
  start_date: '2026-01-01',
  planned_end_date: '2026-02-01',
  status: 'not_started',
  budget_allocated: 100_000,
  actual_cost: 0,
  milestones: [],
  ...overrides,
})

const phases: DeploymentPhase[] = [
  makePhase({ phase_number: 1, phase_name: 'Manual Baseline & Context Capture', status: 'completed', planned_end_date: '2026-02-28', actual_end_date: '2026-02-25' }),
  makePhase({ phase_number: 2, phase_name: 'Shadow AI & Tooling Integration', status: 'in_progress', planned_end_date: '2026-03-31' }),
  makePhase({ phase_number: 3, phase_name: 'Hybrid Routing Activation', status: 'not_started', planned_end_date: '2026-04-30' }),
  makePhase({ phase_number: 4, phase_name: 'Dynamic Threshold Optimization', status: 'not_started', planned_end_date: '2026-05-31' }),
  makePhase({ phase_number: 5, phase_name: 'Automated Flywheel & Steady-State', status: 'blocked', planned_end_date: '2026-06-30' }),
]

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('PhaseTimeline', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Container & heading ───────────────────────────────────────────────────

  it('renders with data-testid="phase-timeline"', () => {
    render(<PhaseTimeline phases={phases} />)
    expect(screen.getByTestId('phase-timeline')).toBeInTheDocument()
  })

  it('renders a "Phase Timeline" heading', () => {
    render(<PhaseTimeline phases={phases} />)
    expect(
      screen.getByRole('heading', { name: /phase timeline/i })
    ).toBeInTheDocument()
  })

  // ── Phase items ───────────────────────────────────────────────────────────

  it('renders one phase item per phase with data-testid="phase-item"', () => {
    render(<PhaseTimeline phases={phases} />)
    expect(screen.getAllByTestId('phase-item')).toHaveLength(5)
  })

  it('displays the phase name for each phase', () => {
    render(<PhaseTimeline phases={phases} />)
    expect(screen.getByText('Manual Baseline & Context Capture')).toBeInTheDocument()
    expect(screen.getByText('Shadow AI & Tooling Integration')).toBeInTheDocument()
  })

  it('displays the phase number on each item', () => {
    render(<PhaseTimeline phases={phases} />)
    // Phase numbers 1–5 should all appear
    expect(screen.getByText('1')).toBeInTheDocument()
    expect(screen.getByText('2')).toBeInTheDocument()
    expect(screen.getByText('3')).toBeInTheDocument()
  })

  it('renders phases in ascending phase_number order', () => {
    // Provide phases in reversed order; timeline should still order 1→5
    const reversed = [...phases].reverse()
    render(<PhaseTimeline phases={reversed} />)
    const items = screen.getAllByTestId('phase-item')
    expect(items[0]).toHaveAttribute('data-phase-number', '1')
    expect(items[4]).toHaveAttribute('data-phase-number', '5')
  })

  it('sets data-phase-number attribute on each phase item', () => {
    render(<PhaseTimeline phases={phases} />)
    const items = screen.getAllByTestId('phase-item')
    const numbers = items.map((el) => el.getAttribute('data-phase-number'))
    expect(numbers).toEqual(['1', '2', '3', '4', '5'])
  })

  it('displays the planned_end_date formatted (not raw ISO)', () => {
    render(<PhaseTimeline phases={phases} />)
    // Raw ISO date strings should not appear
    expect(screen.queryByText('2026-02-28')).not.toBeInTheDocument()
  })

  it('displays the status of each phase', () => {
    render(<PhaseTimeline phases={phases} />)
    expect(screen.getByText(/completed/i)).toBeInTheDocument()
    expect(screen.getByText(/in.progress|in_progress/i)).toBeInTheDocument()
  })

  // ── Status color coding ───────────────────────────────────────────────────

  it('applies a green class to a completed phase item', () => {
    render(<PhaseTimeline phases={phases} />)
    const completedItem = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '1'
    )!
    expect(completedItem.className).toMatch(/green/)
  })

  it('applies a blue class to an in_progress phase item', () => {
    render(<PhaseTimeline phases={phases} />)
    const inProgressItem = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '2'
    )!
    expect(inProgressItem.className).toMatch(/blue/)
  })

  it('applies a gray class to a not_started phase item', () => {
    render(<PhaseTimeline phases={phases} />)
    const notStartedItem = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '3'
    )!
    expect(notStartedItem.className).toMatch(/gray/)
  })

  it('applies a red class to a blocked phase item', () => {
    render(<PhaseTimeline phases={phases} />)
    const blockedItem = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '5'
    )!
    expect(blockedItem.className).toMatch(/red/)
  })

  // ── Empty state ───────────────────────────────────────────────────────────

  it('renders no phase items when phases array is empty', () => {
    render(<PhaseTimeline phases={[]} />)
    expect(screen.queryAllByTestId('phase-item')).toHaveLength(0)
  })

  it('shows an empty state message when phases is empty', () => {
    render(<PhaseTimeline phases={[]} />)
    expect(screen.getByText(/no phases/i)).toBeInTheDocument()
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('updates phase count when phases prop changes', () => {
    const { rerender } = render(<PhaseTimeline phases={phases} />)
    expect(screen.getAllByTestId('phase-item')).toHaveLength(5)

    rerender(<PhaseTimeline phases={phases.slice(0, 2)} />)
    expect(screen.getAllByTestId('phase-item')).toHaveLength(2)
  })

  it('reflects status changes on re-render', () => {
    const { rerender } = render(<PhaseTimeline phases={phases} />)
    const item = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '3'
    )!
    expect(item.className).toMatch(/gray/)

    const updatedPhases = phases.map((p) =>
      p.phase_number === 3 ? { ...p, status: 'in_progress' as const } : p
    )
    rerender(<PhaseTimeline phases={updatedPhases} />)
    const updatedItem = screen.getAllByTestId('phase-item').find(
      (el) => el.getAttribute('data-phase-number') === '3'
    )!
    expect(updatedItem.className).toMatch(/blue/)
  })
})
