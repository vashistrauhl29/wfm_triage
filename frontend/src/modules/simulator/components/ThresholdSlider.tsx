import React from 'react'

interface ThresholdSliderProps {
  value: number
  onChange: (value: number) => void
}

export const ThresholdSlider: React.FC<ThresholdSliderProps> = ({
  value,
  onChange,
}) => {
  const percentage = (value * 100).toFixed(2) + '%'

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <div className="flex items-center justify-between mb-4">
        <label className="text-sm font-medium text-zinc-300">Threshold</label>
        <span className="text-2xl font-bold text-white">{percentage}</span>
      </div>

      <input
        type="range"
        role="slider"
        min="0.80"
        max="1.00"
        step="0.01"
        value={value}
        onChange={(e) => onChange(parseFloat(e.target.value))}
        aria-label="Confidence threshold"
        aria-valuetext={percentage}
        className="w-full h-2 bg-zinc-800 rounded-lg appearance-none cursor-pointer accent-white"
      />

      <div className="flex justify-between mt-1 text-xs text-zinc-500">
        <span>80%</span>
        <span>100%</span>
      </div>
    </div>
  )
}
