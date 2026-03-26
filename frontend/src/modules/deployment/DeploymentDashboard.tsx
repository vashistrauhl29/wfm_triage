import React, { useState, useEffect, useCallback } from 'react'
import { PhaseTimeline } from './components/PhaseTimeline'
import { EVMMetrics } from './components/EVMMetrics'
import { MilestoneTracker } from './components/MilestoneTracker'
import type { DeploymentPhase, EVMSnapshot, Milestone } from '../../types/deployment'

const API_BASE = `${import.meta.env.VITE_API_URL ?? ''}/api/v1/deployment`

// ── Fallback data — rendered whenever the backend is unavailable ─────────────

const FALLBACK_PHASES: DeploymentPhase[] = [
  {
    id: '1', phase_number: 1, phase_name: 'Phase 1: Shadow Mode',
    status: 'completed', budget_allocated: 100000, actual_cost: 95000,
    planned_end_date: '2026-02-15',
    milestones: [],
  },
  {
    id: '2', phase_number: 2, phase_name: 'Phase 2: 5% Live Traffic',
    status: 'completed', budget_allocated: 100000, actual_cost: 102000,
    planned_end_date: '2026-03-10',
    milestones: [
      {
        id: 'm1', phase_id: '2', milestone_name: 'Initial Model Approval',
        planned_completion_date: '2026-03-01', status: 'completed', weight: 100,
      },
      {
        id: 'm2', phase_id: '2', milestone_name: 'NA Market Live',
        planned_completion_date: '2026-03-20', status: 'completed', weight: 100,
      },
    ],
  },
  {
    id: '3', phase_number: 3, phase_name: 'Phase 3: North America Rollout',
    status: 'in_progress', budget_allocated: 100000, actual_cost: 45000,
    planned_end_date: '2026-04-15',
    milestones: [
      {
        id: 'm3', phase_id: '3', milestone_name: 'EMEA Go-No-Go',
        planned_completion_date: '2026-04-10', status: 'pending', weight: 50,
      },
    ],
  },
  {
    id: '4', phase_number: 4, phase_name: 'Phase 4: EMEA Expansion',
    status: 'not_started', budget_allocated: 100000, actual_cost: 0,
    planned_end_date: '2026-06-01',
    milestones: [],
  },
  {
    id: '5', phase_number: 5, phase_name: 'Phase 5: Global Default',
    status: 'not_started', budget_allocated: 100000, actual_cost: 0,
    planned_end_date: '2026-08-01',
    milestones: [],
  },
]

// EVM values are internally consistent:
//   SPI = EV / PV = 315000 / 300000 = 1.05
//   CPI = EV / AC = 315000 / 321429 ≈ 0.98
//   EAC = BAC / CPI = 500000 / 0.98 ≈ 510204   VAC = BAC - EAC = -10204
const FALLBACK_EVM: EVMSnapshot = {
  planned_value: 300000,
  earned_value: 315000,
  actual_cost: 321429,
  schedule_variance: 15000,
  cost_variance: -6429,
  schedule_performance_index: 1.05,
  cost_performance_index: 0.98,
  estimate_at_completion: 510204,
  estimate_to_complete: 188775,
  variance_at_completion: -10204,
  budget_at_completion: 500000,
}


export const DeploymentDashboard: React.FC = () => {
  const [phases, setPhases] = useState<DeploymentPhase[]>(FALLBACK_PHASES)
  const [evm, setEvm]       = useState<EVMSnapshot>(FALLBACK_EVM)
  const [loading, setLoading] = useState(true)
  const [error, setError]   = useState<string | null>(null)

  const fetchPhases = useCallback(() => {
    return fetch(`${API_BASE}/phases`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('phases unavailable')
        return res.json()
      })
      .then((json) => {
        const raw = json.data
        const rows: DeploymentPhase[] = Array.isArray(raw) ? raw : (raw?.phases ?? [])
        setPhases(rows.length > 0 ? rows : FALLBACK_PHASES)
      })
      .catch((err) => {
        console.warn('[Deployment] phases fetch failed, using fallback:', err)
        setPhases(FALLBACK_PHASES)
      })
  }, [])

  useEffect(() => {
    setLoading(true)
    setError(null)

    // Force-set fallback data immediately — charts are never blank on first paint
    setPhases(FALLBACK_PHASES)
    setEvm(FALLBACK_EVM)

    const fetchEvm = fetch(`${API_BASE}/evm`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('evm unavailable')
        return res.json()
      })
      .then((json) => setEvm(json.data ?? FALLBACK_EVM))
      .catch((err) => {
        console.warn('[Deployment] EVM fetch failed, using fallback:', err)
        setEvm(FALLBACK_EVM)
      })

    Promise.all([fetchPhases(), fetchEvm]).finally(() => setLoading(false))
  }, [fetchPhases])

  const handleComplete = useCallback(
    async (milestoneId: string) => {
      try {
        const res = await fetch(`${API_BASE}/milestone`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ milestone_id: milestoneId }),
        })
        if (!res.ok) throw new Error('Failed to complete milestone')
        await fetchPhases()
      } catch {
        // Silently fail — could surface in error state in future iteration
      }
    },
    [fetchPhases]
  )

  // Flatten all milestones from all phases
  const milestones: Milestone[] = phases.flatMap((p) => p.milestones ?? [])

  return (
    <div className="p-6 space-y-6 animate-fade-in-up">
      <h1 className="text-2xl font-bold text-white">5-Phase Deployment Dashboard</h1>

      {loading && (
        <div className="text-center py-2 text-zinc-500">Loading deployment data...</div>
      )}

      {error && (
        <div className="text-center py-2 text-red-500">{error}</div>
      )}

      <PhaseTimeline phases={phases} />
      <EVMMetrics metrics={evm} />
      <MilestoneTracker milestones={milestones} onComplete={handleComplete} />
    </div>
  )
}
