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
} from 'recharts'
import type { CostDataPoint } from '../../../types/analytics'

interface CostTrendProps {
  data: CostDataPoint[]
}

export const CostTrend: React.FC<CostTrendProps> = ({ data }) => {
  return (
    <div data-testid="cost-trend" className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Daily WFM Cost Trend</h3>

      {data.length === 0 ? (
        <p className="text-sm text-zinc-500 text-center py-8">No cost data available</p>
      ) : (
        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" stroke="#27272a" />
            <XAxis dataKey="name" stroke="#71717a" tick={{ fill: '#71717a' }} />
            <YAxis stroke="#71717a" tick={{ fill: '#71717a' }} />
            <Tooltip contentStyle={{ background: '#0A0A0A', border: '1px solid #27272a', color: '#fff' }} />
            <Legend />
            <Line type="monotone" dataKey="total_cost" stroke="#6366f1" dot={false} />
            <Line type="monotone" dataKey="api_cost" stroke="#3b82f6" dot={false} />
            <Line type="monotone" dataKey="human_cost" stroke="#f97316" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      )}
    </div>
  )
}
