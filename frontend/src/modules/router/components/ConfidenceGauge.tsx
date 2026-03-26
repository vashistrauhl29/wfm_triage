import React from 'react'

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
  const getGaugeColorClass = () => {
    if (confidencePercentage >= 90) return 'bg-green-500'
    if (confidencePercentage >= 70) return 'bg-yellow-500'
    return 'bg-red-500'
  }

  // Check if confidence is above or below threshold
  const isAboveThreshold = confidenceScore >= threshold

  // Calculate difference from threshold
  const differenceFromThreshold = ((confidenceScore - threshold) * 100).toFixed(2)
  const differenceSign = confidenceScore >= threshold ? '+' : ''

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <div className="mb-4">
        <div className="flex items-center justify-between mb-2">
          <h3 className="text-sm font-medium text-zinc-300">
            Confidence Score
          </h3>
          <span className="text-2xl font-bold text-white">
            {confidencePercentage.toFixed(2)}%
          </span>
        </div>

        {/* Progress Bar Container */}
        <div className="relative h-8 bg-zinc-800 rounded-full overflow-hidden">
          {/* Progress Fill */}
          <div
            data-testid="gauge-fill"
            className={`h-full ${getGaugeColorClass()} transition-all duration-300 ease-in-out`}
            style={{ width: `${confidencePercentage}%`, transition: 'width 0.3s ease-in-out' }}
            role="progressbar"
            aria-valuenow={Math.round(confidencePercentage)}
            aria-valuemin={0}
            aria-valuemax={100}
            aria-label={`Confidence score: ${confidencePercentage.toFixed(2)}%`}
          />

          {/* Threshold Line */}
          <div
            data-testid="threshold-line"
            className="absolute top-0 bottom-0 w-0.5 bg-red-600 z-10"
            style={{ left: `${thresholdPercentage}%` }}
          >
            <div className="absolute -top-1 left-1/2 transform -translate-x-1/2">
              <div className="w-3 h-3 bg-red-600 rounded-full border-2 border-white" />
            </div>
          </div>
        </div>

        {/* Hidden element for testing with data-testid */}
        <div
          data-testid="confidence-gauge"
          className={getGaugeColorClass()}
          style={{ display: 'none' }}
        />
      </div>

      {/* Threshold Info */}
      <div className="space-y-2">
        <div className="flex items-center justify-between text-sm">
          <span className="text-zinc-400">Threshold: {thresholdPercentage.toFixed(2)}%</span>
          <span
            data-testid="threshold-status"
            className={`font-medium ${
              isAboveThreshold ? 'text-green-600' : 'text-orange-600'
            }`}
          >
            {isAboveThreshold ? 'Above Threshold' : 'Below Threshold'}
          </span>
        </div>

        <div className="text-sm text-zinc-400">
          <span className={differenceFromThreshold.startsWith('-') ? 'text-red-600' : 'text-green-600'}>
            {differenceSign}{differenceFromThreshold}% from threshold
          </span>
        </div>
      </div>
    </div>
  )
}
