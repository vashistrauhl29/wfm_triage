import React from 'react'
import { motion } from 'framer-motion'
import { TrendingUp, TrendingDown, Target } from 'lucide-react'

interface ConfidenceGaugeProps {
  confidenceScore: number  // 0.0 to 1.0
  threshold: number        // 0.0 to 1.0
}

export const ConfidenceGauge: React.FC<ConfidenceGaugeProps> = ({
  confidenceScore,
  threshold
}) => {
  const confidencePercentage = confidenceScore * 100
  const thresholdPercentage = threshold * 100

  // Determine color based on confidence level: red <70%, yellow 70-90%, green >90%
  const getGaugeColors = () => {
    if (confidencePercentage >= 90) {
      return {
        bg: 'bg-emerald-500',
        glow: 'shadow-glow-primary',
        text: 'text-emerald-400',
        gradient: 'from-emerald-500 to-emerald-600',
      }
    }
    if (confidencePercentage >= 70) {
      return {
        bg: 'bg-yellow-500',
        glow: 'shadow-glow-accent',
        text: 'text-yellow-400',
        gradient: 'from-yellow-500 to-yellow-600',
      }
    }
    return {
      bg: 'bg-red-500',
      glow: '',
      text: 'text-red-400',
      gradient: 'from-red-500 to-red-600',
    }
  }

  const colors = getGaugeColors()
  const isAboveThreshold = confidenceScore >= threshold
  const differenceFromThreshold = ((confidenceScore - threshold) * 100).toFixed(1)

  return (
    <motion.div
      initial={{ opacity: 0, scale: 0.95 }}
      animate={{ opacity: 1, scale: 1 }}
      transition={{ duration: 0.4, delay: 0.3 }}
      className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6 hover:border-zinc-700 transition-colors"
    >
      {/* Header */}
      <div className="flex items-center justify-between mb-6">
        <div>
          <h3 className="text-sm font-medium text-zinc-400 mb-1">
            Confidence Score
          </h3>
          <motion.div
            key={confidencePercentage}
            initial={{ scale: 1.1 }}
            animate={{ scale: 1 }}
            transition={{ type: 'spring', stiffness: 200 }}
            className="flex items-baseline gap-2"
          >
            <span className={`text-3xl font-bold ${colors.text}`}>
              {confidencePercentage.toFixed(1)}
            </span>
            <span className="text-lg text-zinc-500">%</span>
          </motion.div>
        </div>

        {/* Status Badge */}
        <motion.div
          initial={{ scale: 0 }}
          animate={{ scale: 1 }}
          transition={{ type: 'spring', stiffness: 200, delay: 0.1 }}
          className={`flex items-center gap-2 px-3 py-1.5 rounded-full ${
            isAboveThreshold
              ? 'bg-emerald-500/10 border border-emerald-500/20'
              : 'bg-orange-500/10 border border-orange-500/20'
          }`}
        >
          {isAboveThreshold ? (
            <TrendingUp className="w-4 h-4 text-emerald-400" />
          ) : (
            <TrendingDown className="w-4 h-4 text-orange-400" />
          )}
          <span
            className={`text-xs font-medium ${
              isAboveThreshold ? 'text-emerald-400' : 'text-orange-400'
            }`}
          >
            {isAboveThreshold ? 'STP' : 'Human'}
          </span>
        </motion.div>
      </div>

      {/* Enhanced Progress Bar */}
      <div className="relative mb-6">
        {/* Track */}
        <div className="relative h-3 bg-zinc-800/50 rounded-full overflow-hidden backdrop-blur-sm">
          {/* Gradient Background */}
          <div className="absolute inset-0 bg-gradient-to-r from-transparent via-zinc-700/20 to-transparent" />

          {/* Progress Fill with Gradient */}
          <motion.div
            data-testid="gauge-fill"
            initial={{ width: 0 }}
            animate={{ width: `${confidencePercentage}%` }}
            transition={{ duration: 0.8, ease: 'easeOut' }}
            className={`relative h-full bg-gradient-to-r ${colors.gradient} ${colors.glow}`}
            role="progressbar"
            aria-valuenow={Math.round(confidencePercentage)}
            aria-valuemin={0}
            aria-valuemax={100}
            aria-label={`Confidence score: ${confidencePercentage.toFixed(2)}%`}
          >
            {/* Shimmer effect */}
            <div className="absolute inset-0 bg-gradient-to-r from-transparent via-white/20 to-transparent animate-pulse" />
          </motion.div>

          {/* Threshold Indicator */}
          <motion.div
            data-testid="threshold-line"
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            transition={{ delay: 0.5 }}
            className="absolute top-0 bottom-0 w-0.5 bg-accent-500 z-10"
            style={{ left: `${thresholdPercentage}%` }}
          >
            {/* Threshold Marker */}
            <div className="absolute -top-1.5 left-1/2 transform -translate-x-1/2">
              <motion.div
                initial={{ scale: 0 }}
                animate={{ scale: 1 }}
                transition={{ type: 'spring', stiffness: 300, delay: 0.6 }}
                className="relative"
              >
                <div className="w-4 h-4 bg-accent-500 rounded-full border-2 border-black shadow-glow-accent" />
                <div className="absolute inset-0 bg-accent-500 rounded-full animate-ping opacity-50" />
              </motion.div>
            </div>
          </motion.div>
        </div>

        {/* Threshold Label */}
        <div
          className="absolute -bottom-5 text-xs text-zinc-500 flex items-center gap-1"
          style={{ left: `${thresholdPercentage}%`, transform: 'translateX(-50%)' }}
        >
          <Target className="w-3 h-3" />
          <span>{thresholdPercentage.toFixed(0)}%</span>
        </div>
      </div>

      {/* Statistics */}
      <div className="grid grid-cols-2 gap-4 pt-4 mt-6 border-t border-zinc-800">
        <div>
          <p className="text-xs text-zinc-500 mb-1">From Threshold</p>
          <p className={`text-sm font-bold ${
            differenceFromThreshold.startsWith('-') ? 'text-red-400' : 'text-emerald-400'
          }`}>
            {differenceFromThreshold.startsWith('-') ? '' : '+'}{differenceFromThreshold}%
          </p>
        </div>
        <div>
          <p className="text-xs text-zinc-500 mb-1">Status</p>
          <p
            data-testid="threshold-status"
            className={`text-sm font-bold ${
              isAboveThreshold ? 'text-emerald-400' : 'text-orange-400'
            }`}
          >
            {isAboveThreshold ? 'Auto-Process' : 'Manual Review'}
          </p>
        </div>
      </div>

      {/* Hidden element for testing */}
      <div
        data-testid="confidence-gauge"
        className={colors.bg}
        style={{ display: 'none' }}
      />
    </motion.div>
  )
}
