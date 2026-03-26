import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { RecommendationCard } from '../RecommendationCard'

const threeBullets = [
  'User reported post violates community guidelines on hate speech.',
  'ML model confidence is 87%, below the 95% STP threshold.',
  'Similar cases in the past week have resulted in content removal.',
]

const recommendation = 'Remove content and issue account warning'

describe('RecommendationCard', () => {
  // ── Heading ──────────────────────────────────────────────────────────────
  it('renders a "Recommendation" heading', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(
      screen.getByRole('heading', { name: /recommendation/i })
    ).toBeInTheDocument()
  })

  // ── Summary bullets ──────────────────────────────────────────────────────
  it('renders all three summary bullet points', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    threeBullets.forEach((bullet) => {
      expect(screen.getByText(bullet)).toBeInTheDocument()
    })
  })

  it('renders summary items in a list element', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    const list = screen.getByRole('list')
    expect(list).toBeInTheDocument()
  })

  it('renders each summary bullet as a list item', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    const items = screen.getAllByRole('listitem')
    expect(items).toHaveLength(3)
  })

  it('renders bullets in order', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    const items = screen.getAllByRole('listitem')
    items.forEach((item, i) => {
      expect(item).toHaveTextContent(threeBullets[i])
    })
  })

  it('renders a single bullet when only one summary point is provided', () => {
    render(
      <RecommendationCard summary={['Single key insight.']} recommendation={recommendation} />
    )
    expect(screen.getAllByRole('listitem')).toHaveLength(1)
    expect(screen.getByText('Single key insight.')).toBeInTheDocument()
  })

  it('renders two bullets when two summary points are provided', () => {
    render(
      <RecommendationCard
        summary={['First point.', 'Second point.']}
        recommendation={recommendation}
      />
    )
    expect(screen.getAllByRole('listitem')).toHaveLength(2)
  })

  it('labels the summary section', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(screen.getByText(/context summary/i)).toBeInTheDocument()
  })

  // ── Recommended action ───────────────────────────────────────────────────
  it('renders the recommended action text', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(screen.getByText(recommendation)).toBeInTheDocument()
  })

  it('labels the recommended action section', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(screen.getByText(/recommended action/i)).toBeInTheDocument()
  })

  it('renders the recommendation in a visually distinct element', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(screen.getByTestId('recommendation-action')).toBeInTheDocument()
  })

  it('the recommendation action element contains the recommendation text', () => {
    render(<RecommendationCard summary={threeBullets} recommendation={recommendation} />)
    expect(screen.getByTestId('recommendation-action')).toHaveTextContent(recommendation)
  })

  // ── Re-render ────────────────────────────────────────────────────────────
  it('updates bullets when summary prop changes', () => {
    const { rerender } = render(
      <RecommendationCard summary={threeBullets} recommendation={recommendation} />
    )
    expect(screen.getAllByRole('listitem')).toHaveLength(3)

    const newSummary = ['Updated single insight.']
    rerender(<RecommendationCard summary={newSummary} recommendation={recommendation} />)
    expect(screen.getAllByRole('listitem')).toHaveLength(1)
    expect(screen.getByText('Updated single insight.')).toBeInTheDocument()
  })

  it('updates the recommendation when recommendation prop changes', () => {
    const { rerender } = render(
      <RecommendationCard summary={threeBullets} recommendation={recommendation} />
    )
    expect(screen.getByTestId('recommendation-action')).toHaveTextContent(recommendation)

    const newRec = 'Escalate to senior moderator'
    rerender(<RecommendationCard summary={threeBullets} recommendation={newRec} />)
    expect(screen.getByTestId('recommendation-action')).toHaveTextContent(newRec)
  })

  // ── Empty state ──────────────────────────────────────────────────────────
  it('handles an empty summary array without crashing', () => {
    render(<RecommendationCard summary={[]} recommendation={recommendation} />)
    expect(screen.getByRole('heading', { name: /recommendation/i })).toBeInTheDocument()
  })
})
