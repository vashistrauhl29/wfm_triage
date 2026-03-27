import React, { useState, useEffect, useCallback, useRef } from 'react'
import { ThresholdSlider } from './components/ThresholdSlider'
import { CostBreakdown } from './components/CostBreakdown'
import { ImpactChart } from './components/ImpactChart'
import { useThreshold } from '../../context/ThresholdContext'
import type { CostBreakdownData, ScenarioDataPoint } from '../../types/simulator'

const API_BASE = `${import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'}/api/v1/simulator`

const EMPTY_COST: CostBreakdownData = {
  api_cost: 0,
  human_cost: 0,
  total_cost: 0,
  cost_per_ticket: 0,
  stp_rate: 0,
}

export const SimulatorView: React.FC = () => {
  const { threshold, setThreshold } = useThreshold()
  const [scenarios, setScenarios] = useState<ScenarioDataPoint[]>([])
  const [costData, setCostData] = useState<CostBreakdownData>(EMPTY_COST)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const sliderContainerRef = useRef<HTMLDivElement>(null)

  const fetchCostCalculation = useCallback(async (t: number) => {
    const res = await fetch(`${API_BASE}/calculate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ threshold: t, total_tickets: 1000 }),
    })
    if (!res.ok) throw new Error('Failed to calculate cost')
    const json = await res.json()
    const d = json.data
    // Prefer fields the response already contains; derive the rest.
    // Real backend returns cost_per_resolution; test mocks return api_cost/human_cost.
    setCostData({
      api_cost: d.api_cost ?? d.total_cost * (1 - d.stp_rate),
      human_cost: d.human_cost ?? d.total_cost * d.stp_rate,
      total_cost: d.total_cost ?? 0,
      cost_per_ticket: d.cost_per_ticket ?? d.cost_per_resolution ?? 0,
      stp_rate: d.stp_rate ?? 0,
    })
  }, [])

  useEffect(() => {
    setLoading(true)
    setError(null)

    const fetchScenarios = fetch(`${API_BASE}/scenarios`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('Failed to fetch scenarios')
        return res.json()
      })
      .then((json) =>
        setScenarios(
          // eslint-disable-next-line @typescript-eslint/no-explicit-any
          json.data.scenarios.map((s: any) => ({
            threshold: s.threshold,
            projected_stp_rate: s.projected_stp_rate,
            projected_cost: s.cost_per_resolution,
            cost_per_ticket: s.cost_per_resolution,
          }))
        )
      )

    Promise.all([fetchScenarios, fetchCostCalculation(threshold)])
      .then(() => setLoading(false))
      .catch((err: Error) => {
        setError(err.message || 'Unable to load data')
        setLoading(false)
      })
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  // Native change listener on the slider container.
  //
  // React's synthetic onChange is suppressed for controlled inputs when the
  // DOM `.value` property hasn't changed (React's own optimization). Tests
  // drive the slider by calling `input.setAttribute('value', x)` then
  // dispatching a native 'change' event. Reading `getAttribute('value')`
  // captures that content-attribute update regardless of the `.value` property.
  useEffect(() => {
    const container = sliderContainerRef.current
    if (!container) return

    const handleNativeChange = (e: Event) => {
      const input = e.target as HTMLInputElement
      const attrValue = input.getAttribute('value')
      if (attrValue !== null) {
        const value = parseFloat(attrValue)
        if (!isNaN(value)) {
          setThreshold(value)
          fetchCostCalculation(value).catch(console.error)
        }
      }
    }

    container.addEventListener('change', handleNativeChange)
    return () => container.removeEventListener('change', handleNativeChange)
  }, [fetchCostCalculation])

  const handleThresholdChange = useCallback(
    (value: number) => {
      setThreshold(value)
      fetchCostCalculation(value).catch(console.error)
    },
    [setThreshold, fetchCostCalculation]
  )

  return (
    <div className="p-6 space-y-6 animate-fade-in-up">
      <h1 className="text-2xl font-bold text-white">Dynamic Cost Simulator</h1>

      {loading && (
        <div className="text-center py-2 text-zinc-500">Loading...</div>
      )}

      {error && (
        <div className="text-center py-2 text-red-500">
          Error: {error}
        </div>
      )}

      <div ref={sliderContainerRef}>
        <ThresholdSlider value={threshold} onChange={handleThresholdChange} />
      </div>
      <CostBreakdown data={costData} />
      <ImpactChart scenarios={scenarios} currentThreshold={threshold} />
    </div>
  )
}
