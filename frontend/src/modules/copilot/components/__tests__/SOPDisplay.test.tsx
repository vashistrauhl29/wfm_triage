import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { SOPDisplay } from '../SOPDisplay'
import type { SOPSearchResult } from '../../../../types/ticket'

const highRelevanceSOP: SOPSearchResult = {
  id: 'SOP-01',
  title: 'Hate Speech Removal Policy',
  summary:
    'When a post contains hate speech targeting protected characteristics, operators must remove the content immediately and issue a formal account warning.',
  relevance_score: 0.94,
}

const mediumRelevanceSOP: SOPSearchResult = {
  id: 'SOP-02',
  title: 'Account Warning Procedure',
  summary:
    'Operators should issue a warning via the account management panel. First offenses receive a written notice; repeat violations trigger suspension.',
  relevance_score: 0.72,
}

const lowRelevanceSOP: SOPSearchResult = {
  id: 'SOP-03',
  title: 'Generic Moderation Fallback',
  summary: 'When no specific SOP matches, apply the general moderation guidelines.',
  relevance_score: 0.41,
}

describe('SOPDisplay', () => {
  // ── Basic content ────────────────────────────────────────────────────────
  it('renders the SOP title', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByText(/Hate Speech Removal Policy/)).toBeInTheDocument()
  })

  it('renders the SOP summary text', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(
      screen.getByText(/remove the content immediately/i)
    ).toBeInTheDocument()
  })

  it('renders the relevance score as a percentage', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByText(/94\.00%/)).toBeInTheDocument()
  })

  it('renders a "Relevance" label', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByText(/relevance/i)).toBeInTheDocument()
  })

  // ── Relevance color coding ───────────────────────────────────────────────
  it('applies a green color indicator for high relevance (≥ 0.85)', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByTestId('relevance-indicator')).toHaveClass('bg-green-500')
  })

  it('applies a yellow color indicator for medium relevance (0.60–0.84)', () => {
    render(<SOPDisplay sop={mediumRelevanceSOP} />)
    expect(screen.getByTestId('relevance-indicator')).toHaveClass('bg-yellow-500')
  })

  it('applies a red color indicator for low relevance (< 0.60)', () => {
    render(<SOPDisplay sop={lowRelevanceSOP} />)
    expect(screen.getByTestId('relevance-indicator')).toHaveClass('bg-red-500')
  })

  // ── Highlighting ─────────────────────────────────────────────────────────
  it('renders without highlight terms by default', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    // Should not throw; summary rendered as plain text
    expect(
      screen.getByText(/remove the content immediately/i)
    ).toBeInTheDocument()
  })

  it('wraps matched highlight terms in a mark element', () => {
    render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={['hate speech', 'account warning']} />
    )
    const marks = document.querySelectorAll('mark')
    expect(marks.length).toBeGreaterThan(0)
  })

  it('highlights the first search term in the summary', () => {
    render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={['hate speech']} />
    )
    const marks = document.querySelectorAll('mark')
    const markedTexts = Array.from(marks).map((m) => m.textContent?.toLowerCase())
    expect(markedTexts.some((t) => t?.includes('hate speech'))).toBe(true)
  })

  it('highlights multiple distinct search terms', () => {
    render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={['hate speech', 'remove']} />
    )
    const marks = document.querySelectorAll('mark')
    expect(marks.length).toBeGreaterThanOrEqual(2)
  })

  it('ignores search terms that do not appear in the summary', () => {
    render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={['unrelated_term_xyz']} />
    )
    const marks = document.querySelectorAll('mark')
    expect(marks.length).toBe(0)
  })

  it('performs case-insensitive matching for highlight terms', () => {
    render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={['HATE SPEECH']} />
    )
    const marks = document.querySelectorAll('mark')
    expect(marks.length).toBeGreaterThan(0)
  })

  // ── SOP ID ───────────────────────────────────────────────────────────────
  it('exposes the sop id via data attribute', () => {
    render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByTestId('sop-display')).toHaveAttribute('data-sop-id', 'SOP-01')
  })

  // ── Re-render ────────────────────────────────────────────────────────────
  it('updates content when the sop prop changes', () => {
    const { rerender } = render(<SOPDisplay sop={highRelevanceSOP} />)
    expect(screen.getByText(/Hate Speech Removal Policy/)).toBeInTheDocument()

    rerender(<SOPDisplay sop={mediumRelevanceSOP} />)
    expect(screen.getByText(/Account Warning Procedure/)).toBeInTheDocument()
    expect(screen.getByText(/72\.00%/)).toBeInTheDocument()
  })

  it('updates highlights when searchTerms prop changes', () => {
    const { rerender } = render(
      <SOPDisplay sop={highRelevanceSOP} searchTerms={[]} />
    )
    expect(document.querySelectorAll('mark').length).toBe(0)

    rerender(<SOPDisplay sop={highRelevanceSOP} searchTerms={['hate speech']} />)
    expect(document.querySelectorAll('mark').length).toBeGreaterThan(0)
  })
})
