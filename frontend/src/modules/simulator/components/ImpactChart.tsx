import React from 'react'
import {
  ResponsiveContainer,
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  Legend,
  CartesianGrid,
  ReferenceLine,
} from 'recharts'
import type { ScenarioDataPoint } from '../../../types/simulator'

interface ImpactChartProps {
  scenarios: ScenarioDataPoint[]
  currentThreshold: number
}

export const ImpactChart: React.FC<ImpactChartProps> = ({
  scenarios,
  currentThreshold,
}) => {
  if (scenarios.length === 0) {
    return (
      <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
        <h3 className="text-lg font-semibold text-white mb-4">Cost Projection</h3>
        <p className="text-zinc-500 text-center py-8">No scenario data available</p>
      </div>
    )
  }

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Cost Projection</h3>
      <ResponsiveContainer width="100%" height={300}>
        <LineChart data={scenarios}>
          <CartesianGrid strokeDasharray="3 3" stroke="#27272a" />
          <XAxis dataKey="threshold" type="number" domain={[0.8, 1.0]} stroke="#71717a" tick={{ fill: '#71717a' }} />
          <YAxis stroke="#71717a" tick={{ fill: '#71717a' }} />
          <Tooltip
            contentStyle={{ background: '#0A0A0A', border: '1px solid #27272a', color: '#fff' }}
            formatter={(value: number) => value.toFixed(2)}
            labelFormatter={(label: number) => `Threshold: ${Number(label).toFixed(2)}`}
          />
          <Legend />
          <Line type="monotone" dataKey="projected_cost" name="Projected Cost ($/ticket)" stroke="#3b82f6" dot={false} />
          <Line type="monotone" dataKey="projected_stp_rate" name="Automated Resolution (%)" stroke="#f97316" dot={false} />
          <ReferenceLine x={currentThreshold} stroke="#ef4444" strokeDasharray="3 3" />
        </LineChart>
      </ResponsiveContainer>
    </div>
  )
}
