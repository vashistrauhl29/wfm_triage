import React from 'react'

interface RecommendationCardProps {
  summary: string[]
  recommendation: string
}

export const RecommendationCard: React.FC<RecommendationCardProps> = ({
  summary,
  recommendation,
}) => {
  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <h3 className="text-lg font-semibold text-white mb-4">Recommendation</h3>

      <div className="mb-4">
        <p className="text-sm font-medium text-zinc-300 mb-2">Context Summary</p>
        <ul className="space-y-2 list-none">
          {summary.map((bullet, i) => (
            <li key={i} className="flex items-start gap-2 text-sm text-zinc-400">
              <span className="text-zinc-500 shrink-0 mt-0.5">•</span>
              <span>{bullet}</span>
            </li>
          ))}
        </ul>
      </div>

      <div>
        <p className="text-sm font-medium text-zinc-300 mb-2">Recommended Action</p>
        <div
          data-testid="recommendation-action"
          className="bg-blue-950/30 border border-blue-900/50 rounded-md p-3 text-sm font-medium text-blue-300"
        >
          {recommendation}
        </div>
      </div>
    </div>
  )
}
