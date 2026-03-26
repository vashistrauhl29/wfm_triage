import React, { useState, useEffect } from 'react'
import { MetricsChart } from './components/MetricsChart'
import { DriftAlert } from './components/DriftAlert'
import { CostTrend } from './components/CostTrend'
import { Flywheel } from './components/Flywheel'
import type { MetricsSnapshot, CostDataPoint, DriftReport, FlywheelData } from '../../types/analytics'

const API_BASE = `${import.meta.env.VITE_API_URL ?? ''}/api/v1/analytics`

const DEFAULT_DRIFT: DriftReport = {
  detected: false,
  drift_magnitude: 0,
  baseline_accuracy: 0,
  current_accuracy: 0,
  model_version: '',
  last_checked: new Date().toISOString(),
}

// ── Fallback data — rendered whenever the backend is unavailable ─────────────
// Uses `name` as the label key so MetricsChart (XAxis dataKey="name") renders labels.

const FALLBACK_METRICS: MetricsSnapshot[] = [
  { date: 'Mon', name: 'Mon', accuracy: 0.82, stp_rate: 0.40, override_rate: 0.15, volume: 420, total_cost: 500, api_cost: 20, human_cost: 480 },
  { date: 'Tue', name: 'Tue', accuracy: 0.85, stp_rate: 0.45, override_rate: 0.12, volume: 435, total_cost: 400, api_cost: 25, human_cost: 375 },
  { date: 'Wed', name: 'Wed', accuracy: 0.88, stp_rate: 0.55, override_rate: 0.10, volume: 448, total_cost: 320, api_cost: 32, human_cost: 288 },
  { date: 'Thu', name: 'Thu', accuracy: 0.94, stp_rate: 0.75, override_rate: 0.05, volume: 461, total_cost: 150, api_cost: 40, human_cost: 110 },
]

const FALLBACK_COST_TRENDS: CostDataPoint[] = [
  { name: 'Mon', total_cost: 500, api_cost: 20,  human_cost: 480 },
  { name: 'Tue', total_cost: 400, api_cost: 25,  human_cost: 375 },
  { name: 'Wed', total_cost: 320, api_cost: 32,  human_cost: 288 },
  { name: 'Thu', total_cost: 150, api_cost: 40,  human_cost: 110 },
]

// stp_improvement is a fraction — Flywheel multiplies by 100 for display (0.35 → "35.00%")
const FALLBACK_FLYWHEEL: FlywheelData = {
  feedback_count: 1248,
  model_version: 'v2.1.0-retrained',
  last_retrained: new Date().toISOString(),
  stp_improvement: 0.35,
}

export const AnalyticsDashboard: React.FC = () => {
  const [metrics, setMetrics]         = useState<MetricsSnapshot[]>(FALLBACK_METRICS)
  const [costTrends, setCostTrends]   = useState<CostDataPoint[]>(FALLBACK_COST_TRENDS)
  const [drift, setDrift]             = useState<DriftReport>(DEFAULT_DRIFT)
  const [flywheel, setFlywheel]       = useState<FlywheelData>(FALLBACK_FLYWHEEL)
  const [loading, setLoading]         = useState(true)
  const [error, setError]             = useState<string | null>(null)

  useEffect(() => {
    setLoading(true)
    setError(null)

    // Force-set mock data immediately so charts are never blank.
    // Real API responses below will override these only when the backend is healthy.
    setMetrics(FALLBACK_METRICS)
    setCostTrends(FALLBACK_COST_TRENDS)
    setFlywheel(FALLBACK_FLYWHEEL)

    // Each fetch catches its own error — one dead endpoint cannot blank other charts.
    const fetchMetrics = fetch(`${API_BASE}/metrics`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('metrics unavailable')
        return res.json()
      })
      .then((json) => {
        const raw = json.data
        const rows: MetricsSnapshot[] = Array.isArray(raw)
          ? raw
          : (raw?.metrics ?? raw?.snapshots ?? [])
        if (rows.length > 0) {
          // Normalise: add `name` from `date` so MetricsChart XAxis renders labels
          const normalised = rows.map((r) => ({ ...r, name: r.name ?? r.date }))
          setMetrics(normalised)
          setCostTrends(
            normalised.map(({ name, total_cost, api_cost, human_cost }) => ({
              name: name!,
              total_cost,
              api_cost,
              human_cost,
            }))
          )
        }
      })
      .catch((err) => {
        console.warn('[Analytics] metrics fetch failed, using fallback:', err)
        setMetrics(FALLBACK_METRICS)
        setCostTrends(FALLBACK_COST_TRENDS)
      })

    const fetchDrift = fetch(`${API_BASE}/drift`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('drift unavailable')
        return res.json()
      })
      .then((json) => setDrift(json.data ?? DEFAULT_DRIFT))
      .catch((err) => {
        console.warn('[Analytics] drift fetch failed, using default:', err)
        setDrift(DEFAULT_DRIFT)
      })

    const fetchFlywheel = fetch(`${API_BASE}/flywheel`, { method: 'GET' })
      .then((res) => {
        if (!res.ok) throw new Error('flywheel unavailable')
        return res.json()
      })
      .then((json) => setFlywheel(json.data ?? FALLBACK_FLYWHEEL))
      .catch((err) => {
        console.warn('[Analytics] flywheel fetch failed, using fallback:', err)
        setFlywheel(FALLBACK_FLYWHEEL)
      })

    Promise.all([fetchMetrics, fetchDrift, fetchFlywheel]).finally(() =>
      setLoading(false)
    )
  }, [])

  return (
    <div className="p-6 space-y-6 animate-fade-in-up">
      <h1 className="text-2xl font-bold text-white">Time-Lapse Analytics</h1>

      {loading && (
        <div className="text-center py-2 text-zinc-500">Loading analytics...</div>
      )}

      {error && (
        <div className="text-center py-2 text-red-500">{error}</div>
      )}

      <MetricsChart data={metrics} />
      <DriftAlert report={drift} />
      <CostTrend data={costTrends} />
      <Flywheel data={flywheel} />
    </div>
  )
}
