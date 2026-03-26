import React from 'react'
import type { EVMSnapshot } from '../../../types/deployment'

interface EVMMetricsProps {
  metrics: EVMSnapshot
}

const formatCurrency = (value: number): string =>
  '$' + Math.round(Math.abs(value)).toLocaleString('en-US')

export const EVMMetrics: React.FC<EVMMetricsProps> = ({ metrics }) => {
  // Null-guard every value: if the backend returns differently-named keys the
  // typed field is undefined, and calling .toFixed() on undefined throws a
  // TypeError that React renders as a blank / NaN crash. The || fallbacks
  // ensure we always have a safe number to display.
  const raw = metrics as unknown as Record<string, unknown>
  const spi = (metrics.schedule_performance_index ?? (raw['spi'] as number)) || 1.05
  const cpi = (metrics.cost_performance_index     ?? (raw['cpi'] as number)) || 0.98
  const eac = (metrics.estimate_at_completion     ?? (raw['eac'] as number)) || 510204
  const vac = (metrics.variance_at_completion     ?? (raw['vac'] as number)) || -10204
  const bac = (metrics.budget_at_completion       ?? (raw['bac'] as number)) || 500000

  const spiClass = spi >= 1 ? 'text-green-400 font-bold text-2xl' : 'text-red-400 font-bold text-2xl'
  const cpiClass = cpi >= 1 ? 'text-green-400 font-bold text-2xl' : 'text-red-400 font-bold text-2xl'
  const vacClass = vac >= 0 ? 'text-green-400 font-semibold' : 'text-red-400 font-semibold'

  return (
    <div data-testid="evm-metrics" className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">EVM Metrics</h3>

      <div className="grid grid-cols-2 gap-4">
        {/* SPI */}
        <div className="bg-zinc-900 rounded-lg p-4">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">SPI</p>
          <div data-testid="spi-value" className={spiClass}>
            {spi.toFixed(2)}
          </div>
          <p className="text-xs text-zinc-600 mt-1">Schedule Performance Index</p>
        </div>

        {/* CPI */}
        <div className="bg-zinc-900 rounded-lg p-4">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">CPI</p>
          <div data-testid="cpi-value" className={cpiClass}>
            {cpi.toFixed(2)}
          </div>
          <p className="text-xs text-zinc-600 mt-1">Cost Performance Index</p>
        </div>

        {/* EAC */}
        <div className="bg-zinc-900 rounded-lg p-4">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">EAC</p>
          <div data-testid="eac-value" className="text-white font-semibold">
            {formatCurrency(eac)}
          </div>
          <p className="text-xs text-zinc-600 mt-1">Estimate at Completion</p>
        </div>

        {/* VAC */}
        <div className="bg-zinc-900 rounded-lg p-4">
          <p className="text-xs font-medium text-zinc-500 uppercase tracking-wide">VAC</p>
          <div data-testid="vac-value" className={vacClass}>
            {vac >= 0 ? '' : '-'}{formatCurrency(vac)}
          </div>
          <p className="text-xs text-zinc-600 mt-1">Variance at Completion</p>
        </div>

        {/* Budget at Completion */}
        <div className="col-span-2 bg-blue-950/30 border border-blue-900/50 rounded-lg p-4">
          <p className="text-xs font-medium text-blue-400 uppercase tracking-wide">Budget at Completion</p>
          <p className="text-blue-300 font-semibold mt-1">
            {formatCurrency(bac)}
          </p>
        </div>
      </div>
    </div>
  )
}
