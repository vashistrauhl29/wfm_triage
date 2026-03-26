import React from 'react'
import type { QueueStatus } from '../../../types/router'

interface RouteStatusProps {
  queueStatus: QueueStatus
}

export const RouteStatus: React.FC<RouteStatusProps> = ({ queueStatus }) => {
  const stpRatePercentage = queueStatus.stp_rate * 100

  // Determine STP rate color: green >80%, yellow 60-80%, red <60%
  const getStpRateColorClass = () => {
    if (stpRatePercentage >= 80) return 'text-green-600'
    if (stpRatePercentage >= 60) return 'text-yellow-600'
    return 'text-red-600'
  }

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h2 className="text-lg font-semibold text-white mb-4">
        Queue Status
      </h2>

      <div className="grid grid-cols-2 gap-4 mb-6">
        {/* STP Queue */}
        <div
          data-testid="stp-indicator"
          className="bg-green-500 bg-opacity-10 border border-green-500/20 rounded-lg p-4 text-emerald-400 transition-all duration-300 hover:-translate-y-0.5"
        >
          <div className="text-xs font-medium mb-1 text-emerald-400/70 uppercase tracking-wide">STP</div>
          <div className="text-3xl font-bold">{queueStatus.stp_queue_size}</div>
          <div className="text-xs mt-1 text-emerald-400/60">Automated</div>
        </div>

        {/* Human Queue */}
        <div
          data-testid="human-indicator"
          className="bg-orange-500 bg-opacity-10 border border-amber-500/20 rounded-lg p-4 text-amber-400 transition-all duration-300 hover:-translate-y-0.5"
        >
          <div className="text-xs font-medium mb-1 text-amber-400/70 uppercase tracking-wide">Human Queue</div>
          <div className="text-3xl font-bold">{queueStatus.human_queue_size}</div>
          <div className="text-xs mt-1 text-amber-400/60">Manual Review</div>
        </div>
      </div>

      {/* STP Rate */}
      <div className="border-t border-zinc-800 pt-4">
        <div className="flex items-center justify-between mb-2">
          <span className="text-sm font-medium text-zinc-300">STP Rate</span>
          <span
            data-testid="stp-rate"
            className={`text-lg font-bold ${getStpRateColorClass()}`}
          >
            {stpRatePercentage.toFixed(2)}%
          </span>
        </div>

        {/* Total Processed */}
        <div className="flex items-center justify-between text-sm">
          <span className="text-zinc-400">Total Processed</span>
          <span className="font-semibold text-white">
            {queueStatus.total_processed}
          </span>
        </div>
      </div>
    </div>
  )
}
