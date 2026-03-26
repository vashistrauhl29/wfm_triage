import { describe, it, expect, vi, afterEach } from 'vitest'
import { render, screen, fireEvent } from '@testing-library/react'
import { MilestoneTracker } from '../MilestoneTracker'
import type { Milestone } from '../../../../types/deployment'

// ─── Fixtures ─────────────────────────────────────────────────────────────────

const makeMilestone = (overrides: Partial<Milestone> & { id: string }): Milestone => ({
  phase_id: 'phase-1',
  milestone_name: `Milestone ${overrides.id}`,
  planned_completion_date: '2026-02-15',
  status: 'pending',
  weight: 20,
  owner: 'Team Alpha',
  dependencies: [],
  ...overrides,
})

const completedA = makeMilestone({ id: 'ms-A', milestone_name: 'Establish Baseline', status: 'completed', owner: 'Team Alpha' })
const pendingB   = makeMilestone({ id: 'ms-B', milestone_name: 'Finalize SOPs', status: 'pending', owner: 'Team Beta', dependencies: [] })
const inProgressC = makeMilestone({ id: 'ms-C', milestone_name: 'Deploy Copilot Sandbox', status: 'in_progress', owner: 'Team Gamma' })
const atRiskD    = makeMilestone({ id: 'ms-D', milestone_name: 'Calibrate Thresholds', status: 'at_risk', owner: 'Team Delta' })

// Blocked by unmet dependency: ms-E depends on ms-F which is pending (not completed)
const pendingF   = makeMilestone({ id: 'ms-F', milestone_name: 'Precondition Task', status: 'pending' })
const blockedByF = makeMilestone({ id: 'ms-E', milestone_name: 'Dependent Task', status: 'pending', dependencies: ['ms-F'] })

// Unblocked: ms-G depends on ms-A which is completed
const unblockedByA = makeMilestone({ id: 'ms-G', milestone_name: 'Post-Baseline Task', status: 'pending', dependencies: ['ms-A'] })

const allMilestones: Milestone[] = [completedA, pendingB, inProgressC, atRiskD]

// ─── Tests ────────────────────────────────────────────────────────────────────

describe('MilestoneTracker', () => {
  afterEach(() => {
    vi.clearAllMocks()
  })

  // ── Container & heading ───────────────────────────────────────────────────

  it('renders with data-testid="milestone-tracker"', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getByTestId('milestone-tracker')).toBeInTheDocument()
  })

  it('renders a "Milestone Tracker" heading', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(
      screen.getByRole('heading', { name: /milestone tracker/i })
    ).toBeInTheDocument()
  })

  // ── Milestone items ───────────────────────────────────────────────────────

  it('renders one item per milestone with data-testid="milestone-item"', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getAllByTestId('milestone-item')).toHaveLength(4)
  })

  it('displays the milestone name', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getByText('Establish Baseline')).toBeInTheDocument()
    expect(screen.getByText('Finalize SOPs')).toBeInTheDocument()
  })

  it('displays the milestone status', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getByText(/completed/i)).toBeInTheDocument()
    expect(screen.getByText(/pending/i)).toBeInTheDocument()
    expect(screen.getByText(/in.progress|in_progress/i)).toBeInTheDocument()
    expect(screen.getByText(/at.risk|at_risk/i)).toBeInTheDocument()
  })

  it('displays the owner of each milestone', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getByText('Team Alpha')).toBeInTheDocument()
    expect(screen.getByText('Team Beta')).toBeInTheDocument()
  })

  it('displays the planned_completion_date formatted (not raw ISO)', () => {
    render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.queryByText('2026-02-15')).not.toBeInTheDocument()
  })

  // ── Complete button ───────────────────────────────────────────────────────

  it('does NOT show a Complete button for completed milestones', () => {
    render(<MilestoneTracker milestones={[completedA]} onComplete={vi.fn()} />)
    expect(screen.queryByRole('button', { name: /complete/i })).not.toBeInTheDocument()
  })

  it('shows a Complete button for pending milestones', () => {
    render(<MilestoneTracker milestones={[pendingB]} onComplete={vi.fn()} />)
    expect(screen.getByRole('button', { name: /complete/i })).toBeInTheDocument()
  })

  it('shows a Complete button for in_progress milestones', () => {
    render(<MilestoneTracker milestones={[inProgressC]} onComplete={vi.fn()} />)
    expect(screen.getByRole('button', { name: /complete/i })).toBeInTheDocument()
  })

  it('shows a Complete button for at_risk milestones', () => {
    render(<MilestoneTracker milestones={[atRiskD]} onComplete={vi.fn()} />)
    expect(screen.getByRole('button', { name: /complete/i })).toBeInTheDocument()
  })

  it('calls onComplete with the milestone id when Complete is clicked', () => {
    const onComplete = vi.fn()
    render(<MilestoneTracker milestones={[pendingB]} onComplete={onComplete} />)
    fireEvent.click(screen.getByRole('button', { name: /complete/i }))
    expect(onComplete).toHaveBeenCalledWith('ms-B')
  })

  it('calls onComplete only once per click', () => {
    const onComplete = vi.fn()
    render(<MilestoneTracker milestones={[pendingB]} onComplete={onComplete} />)
    fireEvent.click(screen.getByRole('button', { name: /complete/i }))
    expect(onComplete).toHaveBeenCalledTimes(1)
  })

  // ── Dependency resolution ─────────────────────────────────────────────────

  it('disables the Complete button when a dependency is not yet completed', () => {
    render(
      <MilestoneTracker milestones={[pendingF, blockedByF]} onComplete={vi.fn()} />
    )
    // ms-E depends on ms-F (pending) → its button should be disabled
    const items = screen.getAllByTestId('milestone-item')
    const blockedItem = items.find((el) => el.textContent?.includes('Dependent Task'))!
    const btn = blockedItem.querySelector('button')
    expect(btn).toBeDisabled()
  })

  it('enables the Complete button when all dependencies are completed', () => {
    render(
      <MilestoneTracker milestones={[completedA, unblockedByA]} onComplete={vi.fn()} />
    )
    // ms-G depends on ms-A (completed) → button should be enabled
    const items = screen.getAllByTestId('milestone-item')
    const unblockedItem = items.find((el) => el.textContent?.includes('Post-Baseline Task'))!
    const btn = unblockedItem.querySelector('button')
    expect(btn).not.toBeDisabled()
  })

  it('does not call onComplete when a blocked dependency button is clicked', () => {
    const onComplete = vi.fn()
    render(
      <MilestoneTracker milestones={[pendingF, blockedByF]} onComplete={onComplete} />
    )
    const items = screen.getAllByTestId('milestone-item')
    const blockedItem = items.find((el) => el.textContent?.includes('Dependent Task'))!
    const btn = blockedItem.querySelector('button')
    if (btn) fireEvent.click(btn)
    expect(onComplete).not.toHaveBeenCalled()
  })

  // ── Status color coding ───────────────────────────────────────────────────

  it('applies a green class to a completed milestone item', () => {
    render(<MilestoneTracker milestones={[completedA]} />)
    const item = screen.getByTestId('milestone-item')
    expect(item.className).toMatch(/green/)
  })

  it('applies a red or yellow class to an at_risk milestone item', () => {
    render(<MilestoneTracker milestones={[atRiskD]} />)
    const item = screen.getByTestId('milestone-item')
    expect(item.className).toMatch(/red|yellow|orange/)
  })

  // ── Empty state ───────────────────────────────────────────────────────────

  it('shows an empty state message when milestones array is empty', () => {
    render(<MilestoneTracker milestones={[]} />)
    expect(screen.getByText(/no milestones/i)).toBeInTheDocument()
  })

  it('renders no milestone items when milestones is empty', () => {
    render(<MilestoneTracker milestones={[]} />)
    expect(screen.queryAllByTestId('milestone-item')).toHaveLength(0)
  })

  // ── Re-render ─────────────────────────────────────────────────────────────

  it('updates item count when milestones prop changes', () => {
    const { rerender } = render(<MilestoneTracker milestones={allMilestones} />)
    expect(screen.getAllByTestId('milestone-item')).toHaveLength(4)

    rerender(<MilestoneTracker milestones={[completedA, pendingB]} />)
    expect(screen.getAllByTestId('milestone-item')).toHaveLength(2)
  })
})
