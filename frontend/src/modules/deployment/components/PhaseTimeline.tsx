import React from 'react'
import type { DeploymentPhase, PhaseStatus } from '../../../types/deployment'

interface PhaseTimelineProps {
  phases: DeploymentPhase[]
}

const STATUS_CLASSES: Record<PhaseStatus, string> = {
  completed:   'bg-green-950/40 border-green-900/50 text-green-300',
  in_progress: 'bg-blue-950/40 border-blue-900/50 text-blue-300',
  not_started: 'bg-gray-900 border-gray-700 text-gray-400',
  blocked:     'bg-red-950/40 border-red-900/50 text-red-300',
}

const STATUS_LABELS: Record<PhaseStatus, string> = {
  completed:   'Completed',
  in_progress: 'In Progress',
  not_started: 'Not Started',
  blocked:     'Blocked',
}

export const PhaseTimeline: React.FC<PhaseTimelineProps> = ({ phases }) => {
  const sorted = [...phases].sort((a, b) => a.phase_number - b.phase_number)

  return (
    <div data-testid="phase-timeline" className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Phase Timeline</h3>

      {sorted.length === 0 ? (
        <p className="text-sm text-zinc-500">No phases configured</p>
      ) : (
        <div className="flex flex-col gap-3">
          {sorted.map((phase) => (
            <div
              key={phase.id}
              data-testid="phase-item"
              data-phase-number={phase.phase_number}
              className={`flex items-center gap-4 rounded-lg border px-4 py-3 ${STATUS_CLASSES[phase.status]}`}
            >
              {/* Phase number badge */}
              <span className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full bg-white/10 text-sm font-bold">
                {phase.phase_number}
              </span>

              {/* Name + status */}
              <div className="flex-1 min-w-0">
                <p className="text-sm font-semibold truncate">{phase.phase_name}</p>
                <p className="text-xs mt-0.5">{STATUS_LABELS[phase.status]}</p>
              </div>

              {/* Dates */}
              {phase.planned_end_date && (
                <p className="text-xs shrink-0">
                  {new Date(phase.planned_end_date).toLocaleDateString()}
                </p>
              )}
            </div>
          ))}
        </div>
      )}
    </div>
  )
}
