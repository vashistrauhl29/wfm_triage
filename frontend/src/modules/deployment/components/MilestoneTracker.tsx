import React from 'react'
import type { Milestone, MilestoneStatus } from '../../../types/deployment'

interface MilestoneTrackerProps {
  milestones: Milestone[]
  onComplete?: (milestoneId: string) => void
}

const STATUS_CLASSES: Record<MilestoneStatus, string> = {
  pending:     'bg-gray-900 border-gray-700 text-gray-300',
  in_progress: 'bg-blue-950/40 border-blue-900/50 text-blue-300',
  completed:   'bg-green-950/40 border-green-900/50 text-green-300',
  at_risk:     'bg-red-950/40 border-red-900/50 text-red-300',
}

const STATUS_LABELS: Record<MilestoneStatus, string> = {
  pending:     'Pending',
  in_progress: 'In Progress',
  completed:   'Completed',
  at_risk:     'At Risk',
}

function isMilestoneBlocked(milestone: Milestone, allMilestones: Milestone[]): boolean {
  if (!milestone.dependencies || milestone.dependencies.length === 0) return false
  return milestone.dependencies.some((depId) => {
    const dep = allMilestones.find((m) => m.id === depId)
    return !dep || dep.status !== 'completed'
  })
}

export const MilestoneTracker: React.FC<MilestoneTrackerProps> = ({
  milestones,
  onComplete,
}) => {
  return (
    <div data-testid="milestone-tracker" className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Milestone Tracker</h3>

      {milestones.length === 0 ? (
        <p className="text-sm text-zinc-500">No milestones configured</p>
      ) : (
        <div className="flex flex-col gap-3">
          {milestones.map((milestone) => {
            const blocked = isMilestoneBlocked(milestone, milestones)
            const showComplete = milestone.status !== 'completed'

            return (
              <div
                key={milestone.id}
                data-testid="milestone-item"
                className={`rounded-lg border px-4 py-3 ${STATUS_CLASSES[milestone.status]}`}
              >
                <div className="flex items-center justify-between gap-4">
                  <div className="flex-1 min-w-0">
                    <p className="text-sm font-semibold truncate">{milestone.milestone_name}</p>
                    <div className="flex items-center gap-3 mt-1">
                      <span className="text-xs">{STATUS_LABELS[milestone.status]}</span>
                      {milestone.owner && (
                        <span className="text-xs text-zinc-500">{milestone.owner}</span>
                      )}
                      <span className="text-xs text-zinc-600">
                        {new Date(milestone.planned_completion_date).toLocaleDateString()}
                      </span>
                    </div>
                  </div>

                  {showComplete && (
                    <button
                      disabled={blocked}
                      onClick={blocked ? undefined : () => onComplete?.(milestone.id)}
                      className="shrink-0 px-3 py-1.5 text-xs font-medium rounded-md bg-blue-600 text-white hover:bg-blue-700 disabled:opacity-40 disabled:cursor-not-allowed"
                    >
                      Complete
                    </button>
                  )}
                </div>
              </div>
            )
          })}
        </div>
      )}
    </div>
  )
}
