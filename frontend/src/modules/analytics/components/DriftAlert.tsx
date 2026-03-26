import React from 'react'
import type { DriftReport } from '../../../types/analytics'

interface DriftAlertProps {
  report: DriftReport
}

export const DriftAlert: React.FC<DriftAlertProps> = ({ report }) => {
  const driftPct = (report.drift_magnitude * 100).toFixed(2) + '%'
  const currentPct = (report.current_accuracy * 100).toFixed(2) + '%'
  const baselinePct = (report.baseline_accuracy * 100).toFixed(2) + '%'
  const checkedAt = report.last_checked
    ? new Date(report.last_checked).toLocaleString()
    : '—'

  if (report.detected) {
    return (
      <div
        data-testid="drift-alert"
        role="alert"
        className="bg-red-950/30 border border-red-900/50 rounded-lg p-6"
      >
        <h3 className="text-lg font-semibold text-red-400 mb-3">Model Drift Detected</h3>
        <div className="grid grid-cols-2 gap-4 text-sm">
          <div>
            <span className="font-medium text-red-400">Drift Magnitude</span>
            <p className="text-red-300 font-bold mt-1">{driftPct}</p>
          </div>
          <div>
            <span className="font-medium text-red-400">Current Accuracy</span>
            <p className="text-red-300 mt-1">{currentPct}</p>
          </div>
          <div>
            <span className="font-medium text-red-400">Baseline Accuracy</span>
            <p className="text-red-300 mt-1">{baselinePct}</p>
          </div>
          <div>
            <span className="font-medium text-red-400">Model Version</span>
            <p className="text-red-300 mt-1">{report.model_version}</p>
          </div>
        </div>
        <p className="text-xs text-red-500 mt-3">Last checked: {checkedAt}</p>
      </div>
    )
  }

  return (
    <div
      data-testid="drift-alert"
      className="bg-green-950/30 border border-green-900/50 rounded-lg p-6"
    >
      <p className="text-green-400 font-medium">No Drift Detected</p>
      <div className="mt-2 text-sm text-green-600 space-y-1">
        <p>Model Version: {report.model_version}</p>
        <p>Last checked: {checkedAt}</p>
      </div>
    </div>
  )
}
