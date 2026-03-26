import React from 'react'
import type { SOPSearchResult } from '../../../types/ticket'

interface SOPDisplayProps {
  sop: SOPSearchResult
  searchTerms?: string[]
}

function highlightText(text: string, terms: string[]): React.ReactNode {
  if (terms.length === 0) return text

  const escaped = terms.map((t) => t.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'))
  const pattern = new RegExp(escaped.join('|'), 'gi')

  const result: React.ReactNode[] = []
  let lastIndex = 0
  let match: RegExpExecArray | null

  while ((match = pattern.exec(text)) !== null) {
    if (match.index > lastIndex) {
      result.push(text.slice(lastIndex, match.index))
    }
    result.push(<mark key={match.index}>{match[0]}</mark>)
    lastIndex = match.index + match[0].length
  }

  if (lastIndex < text.length) {
    result.push(text.slice(lastIndex))
  }

  return result.length > 0 ? <>{result}</> : text
}

export const SOPDisplay: React.FC<SOPDisplayProps> = ({ sop, searchTerms = [] }) => {
  const relevancePct = (sop.relevance_score * 100).toFixed(2)

  const indicatorColor =
    sop.relevance_score >= 0.85
      ? 'bg-green-500'
      : sop.relevance_score >= 0.6
      ? 'bg-yellow-500'
      : 'bg-red-500'

  return (
    <div
      data-testid="sop-display"
      data-sop-id={sop.id}
      className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-4"
    >
      <div className="flex items-center justify-between mb-2">
        <h4 className="text-sm font-semibold text-white">{sop.title}</h4>
        <div className="flex items-center gap-2">
          <span className="text-xs text-zinc-500">Relevance</span>
          <div
            data-testid="relevance-indicator"
            className={`${indicatorColor} w-3 h-3 rounded-full`}
          />
          <span className="text-xs font-medium text-zinc-300">{`${relevancePct}%`}</span>
        </div>
      </div>
      <p className="text-sm text-zinc-400">
        {highlightText(sop.summary, searchTerms)}
      </p>
    </div>
  )
}
