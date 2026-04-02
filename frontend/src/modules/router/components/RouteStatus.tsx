import React from 'react'
import { motion } from 'framer-motion'
import type { QueueStatus } from '../../../types/router'

interface RouteStatusProps {
  queueStatus: QueueStatus
}

const AnimatedNumber: React.FC<{ value: number; className?: string }> = ({ value, className }) => {
  return (
    <motion.div
      key={value}
      initial={{ opacity: 0, y: -10 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.3, ease: 'easeOut' }}
      className={className}
    >
      {value}
    </motion.div>
  )
}

export const RouteStatus: React.FC<RouteStatusProps> = ({ queueStatus }) => {
  const stpRatePercentage = queueStatus.stp_rate * 100

  // Determine STP rate color: green >80%, yellow 60-80%, red <60%
  const getStpRateColor = () => {
    if (stpRatePercentage >= 80) return { text: 'text-emerald-400', bg: 'bg-emerald-500/10' }
    if (stpRatePercentage >= 60) return { text: 'text-yellow-400', bg: 'bg-yellow-500/10' }
    return { text: 'text-red-400', bg: 'bg-red-500/10' }
  }

  const rateColors = getStpRateColor()

  return (
    <motion.div
      initial={{ opacity: 0, scale: 0.95 }}
      animate={{ opacity: 1, scale: 1 }}
      transition={{ duration: 0.4, delay: 0.2 }}
      className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6 hover:border-zinc-700 transition-colors"
    >
      <h2 className="text-lg font-semibold text-white mb-4">
        Queue Status
      </h2>

      <div className="grid grid-cols-2 gap-4 mb-6">
        {/* STP Queue */}
        <motion.div
          data-testid="stp-indicator"
          whileHover={{ scale: 1.05, y: -4 }}
          transition={{ duration: 0.2, ease: 'easeOut' }}
          className="bg-emerald-500/10 border border-emerald-500/20 rounded-lg p-4 text-emerald-400 cursor-pointer"
        >
          <div className="text-xs font-medium mb-1 text-emerald-400/70 uppercase tracking-wide">STP</div>
          <AnimatedNumber value={queueStatus.stp_queue_size} className="text-3xl font-bold" />
          <div className="text-xs mt-1 text-emerald-400/60">Automated</div>
        </motion.div>

        {/* Human Queue */}
        <motion.div
          data-testid="human-indicator"
          whileHover={{ scale: 1.05, y: -4 }}
          transition={{ duration: 0.2, ease: 'easeOut' }}
          className="bg-orange-500/10 border border-orange-500/20 rounded-lg p-4 text-orange-400 cursor-pointer"
        >
          <div className="text-xs font-medium mb-1 text-orange-400/70 uppercase tracking-wide">Human Queue</div>
          <AnimatedNumber value={queueStatus.human_queue_size} className="text-3xl font-bold" />
          <div className="text-xs mt-1 text-orange-400/60">Manual Review</div>
        </motion.div>
      </div>

      {/* STP Rate */}
      <div className="border-t border-zinc-800 pt-4">
        <div className="flex items-center justify-between mb-3">
          <span className="text-sm font-medium text-zinc-300">STP Rate</span>
          <motion.div
            className={`px-3 py-1 rounded-full ${rateColors.bg} ${rateColors.text} text-sm font-bold`}
            initial={{ scale: 0.8 }}
            animate={{ scale: 1 }}
            transition={{ type: 'spring', stiffness: 200 }}
          >
            <AnimatedNumber value={Number(stpRatePercentage.toFixed(1))} className="inline" />%
          </motion.div>
        </div>

        {/* Progress bar */}
        <div className="mb-3 h-1.5 bg-zinc-800 rounded-full overflow-hidden">
          <motion.div
            className={`h-full ${rateColors.bg.replace('/10', '')}`}
            initial={{ width: 0 }}
            animate={{ width: `${stpRatePercentage}%` }}
            transition={{ duration: 0.5, ease: 'easeOut' }}
          />
        </div>

        {/* Total Processed */}
        <div className="flex items-center justify-between text-sm">
          <span className="text-zinc-400">Total Processed</span>
          <AnimatedNumber
            value={queueStatus.total_processed}
            className="font-semibold text-white"
          />
        </div>
      </div>
    </motion.div>
  )
}
