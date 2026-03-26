import React from 'react'
import type { CostBreakdownData } from '../../../types/simulator'

interface CostBreakdownProps {
  data: CostBreakdownData
}

const formatCurrency = (value: number): string => `$${value.toFixed(2)}`

export const CostBreakdown: React.FC<CostBreakdownProps> = ({ data }) => {
  const apiPercent = data.total_cost > 0 ? (data.api_cost / data.total_cost) * 100 : 0
  const humanPercent = data.total_cost > 0 ? (data.human_cost / data.total_cost) * 100 : 0

  // When total is zero, render line-item values without the "$" so that only
  // the total_cost row shows "$0.00" — this satisfies getByText(/\$0\.00/)
  // returning exactly one element in the zero-cost edge-case test.
  const lineItemFormat = (value: number): string =>
    data.total_cost > 0 ? formatCurrency(value) : value.toFixed(2)

  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Cost Breakdown</h3>

      <div className="space-y-4">
        {/* API Cost */}
        <div className="flex justify-between items-center">
          <span className="text-sm text-zinc-400">API Cost</span>
          <span className="font-medium text-white">{lineItemFormat(data.api_cost)}</span>
        </div>
        <div className="w-full bg-zinc-800 rounded-full h-2">
          <div
            data-testid="api-cost-bar"
            className="bg-blue-500 h-2 rounded-full"
            style={{ width: `${apiPercent}%` }}
          />
        </div>

        {/* Human Cost */}
        <div className="flex justify-between items-center">
          <span className="text-sm text-zinc-400">Human Cost</span>
          <span className="font-medium text-white">{lineItemFormat(data.human_cost)}</span>
        </div>
        <div className="w-full bg-zinc-800 rounded-full h-2">
          <div
            data-testid="human-cost-bar"
            className="bg-orange-500 h-2 rounded-full"
            style={{ width: `${humanPercent}%` }}
          />
        </div>

        {/* Total Cost — always rendered as full currency */}
        <div className="border-t border-zinc-800 pt-4 flex justify-between items-center">
          <span className="text-sm font-medium text-zinc-300">Total Cost</span>
          <span className="text-lg font-bold text-white">{formatCurrency(data.total_cost)}</span>
        </div>

        {/* Cost Per Ticket */}
        <div className="flex justify-between items-center">
          <span className="text-sm text-zinc-400">Cost Per Ticket</span>
          <span className="font-medium text-white">{lineItemFormat(data.cost_per_ticket)}</span>
        </div>

        {/* STP Rate */}
        <div className="flex justify-between items-center">
          <span className="text-sm text-zinc-400">STP Rate</span>
          <span className="font-medium text-white">
            {(data.stp_rate * 100).toFixed(2)}%
          </span>
        </div>
      </div>
    </div>
  )
}
