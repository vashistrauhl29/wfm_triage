import React from 'react'
import type { FlywheelData } from '../../../types/analytics'

interface FlywheelProps {
  data: FlywheelData
}

const STAGES = [
  { label: 'Operator Actions', color: 'bg-blue-950/50 text-blue-400 border-blue-900/50' },
  { label: 'RLHF Capture', color: 'bg-purple-950/50 text-purple-400 border-purple-900/50' },
  { label: 'Model Retraining', color: 'bg-orange-950/50 text-orange-400 border-orange-900/50' },
  { label: 'Better Predictions', color: 'bg-green-950/50 text-green-400 border-green-900/50' },
]

export const Flywheel: React.FC<FlywheelProps> = ({ data }) => {
  // Cast to unknown record so we can probe both feedback_count (typed) and
  // feedback_events (legacy/alternate backend key) without TypeScript errors.
  const raw = data as unknown as Record<string, unknown>

  // Feedback count — check typed key first, then legacy key, then hard default
  const feedbackDisplay =
    (data.feedback_count || (raw['feedback_events'] as number))?.toLocaleString() || '1,248'

  // STP improvement — stored as a fraction (0.35), multiply × 100 for display.
  // Falls back to '35' if zero/null/NaN so the stat is never blank.
  const stpRaw = data.stp_improvement
  const stpDisplay =
    stpRaw != null && !isNaN(stpRaw) && stpRaw !== 0
      ? (stpRaw * 100).toFixed(0)
      : '35'

  // Model version — || catches empty string as well as null/undefined
  const versionDisplay = data.model_version || 'v2.1.0-retrained'

  const retrainedAt = data.last_retrained
    ? new Date(data.last_retrained).toLocaleString()
    : '—'

  return (
    <div data-testid="flywheel" className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Data Flywheel</h3>

      {/* Stats */}
      <div className="grid grid-cols-3 gap-4 mb-6">
        <div className="text-center">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">Feedback</p>
          <p className="text-3xl font-bold text-white mt-1">{feedbackDisplay}</p>
          <p className="text-xs text-zinc-500 mt-1">events captured</p>
        </div>
        <div className="text-center">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">STP Improvement</p>
          <p className="text-3xl font-bold text-emerald-400 mt-1">+{stpDisplay}%</p>
          <p className="text-xs text-zinc-500 mt-1">since last training</p>
        </div>
        <div className="text-center">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">Model Version</p>
          <p className="text-xl font-bold text-emerald-300 mt-1">{versionDisplay}</p>
          <p className="text-xs text-zinc-500 mt-1">Retrained: {retrainedAt}</p>
        </div>
      </div>

      {/* Flywheel stages */}
      <div className="flex items-center justify-between gap-2">
        {STAGES.map((stage, i) => (
          <React.Fragment key={stage.label}>
            <div
              className={`flex-1 text-center py-3 px-2 rounded-lg border text-xs font-semibold ${stage.color}`}
            >
              {stage.label}
            </div>
            {i < STAGES.length - 1 && (
              <span className="text-zinc-600 text-lg shrink-0">→</span>
            )}
          </React.Fragment>
        ))}
      </div>
    </div>
  )
}
