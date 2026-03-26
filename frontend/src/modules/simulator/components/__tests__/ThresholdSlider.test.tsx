import { describe, it, expect, vi } from 'vitest'
import { render, screen, fireEvent } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { ThresholdSlider } from '../ThresholdSlider'

describe('ThresholdSlider', () => {
  // ── Rendering ───────────────────────────────────────────────────────────
  it('renders a range input', () => {
    render(<ThresholdSlider value={0.95} onChange={vi.fn()} />)
    expect(screen.getByRole('slider')).toBeInTheDocument()
  })

  it('displays the current threshold value as a percentage', () => {
    render(<ThresholdSlider value={0.95} onChange={vi.fn()} />)
    expect(screen.getByText(/95\.00%/)).toBeInTheDocument()
  })

  it('renders the "Threshold" label', () => {
    render(<ThresholdSlider value={0.90} onChange={vi.fn()} />)
    expect(screen.getByText(/threshold/i)).toBeInTheDocument()
  })

  it('sets the slider range between 0.80 and 1.00', () => {
    render(<ThresholdSlider value={0.95} onChange={vi.fn()} />)
    const slider = screen.getByRole('slider')
    expect(slider).toHaveAttribute('min', '0.80')
    expect(slider).toHaveAttribute('max', '1.00')
  })

  it('sets the slider step to 0.01', () => {
    render(<ThresholdSlider value={0.95} onChange={vi.fn()} />)
    const slider = screen.getByRole('slider')
    expect(slider).toHaveAttribute('step', '0.01')
  })

  it('reflects the provided value prop on the slider', () => {
    render(<ThresholdSlider value={0.92} onChange={vi.fn()} />)
    const slider = screen.getByRole('slider') as HTMLInputElement
    expect(slider.value).toBe('0.92')
  })

  // ── Interaction ─────────────────────────────────────────────────────────
  it('calls onChange when the slider value changes', () => {
    const onChange = vi.fn()
    render(<ThresholdSlider value={0.95} onChange={onChange} />)

    const slider = screen.getByRole('slider')
    fireEvent.change(slider, { target: { value: '0.90' } })

    expect(onChange).toHaveBeenCalledWith(0.90)
  })

  it('calls onChange with the correct numeric value on each adjustment', () => {
    const onChange = vi.fn()
    render(<ThresholdSlider value={0.95} onChange={onChange} />)

    const slider = screen.getByRole('slider')

    fireEvent.change(slider, { target: { value: '0.88' } })
    expect(onChange).toHaveBeenLastCalledWith(0.88)

    fireEvent.change(slider, { target: { value: '0.99' } })
    expect(onChange).toHaveBeenLastCalledWith(0.99)

    expect(onChange).toHaveBeenCalledTimes(2)
  })

  it('updates the displayed percentage when value prop changes', () => {
    const { rerender } = render(
      <ThresholdSlider value={0.90} onChange={vi.fn()} />
    )
    expect(screen.getByText(/90\.00%/)).toBeInTheDocument()

    rerender(<ThresholdSlider value={0.97} onChange={vi.fn()} />)
    expect(screen.getByText(/97\.00%/)).toBeInTheDocument()
  })

  // ── Boundary values ─────────────────────────────────────────────────────
  it('displays the minimum boundary value correctly', () => {
    render(<ThresholdSlider value={0.80} onChange={vi.fn()} />)
    expect(screen.getByText(/80\.00%/)).toBeInTheDocument()
  })

  it('displays the maximum boundary value correctly', () => {
    render(<ThresholdSlider value={1.0} onChange={vi.fn()} />)
    expect(screen.getByText(/100\.00%/)).toBeInTheDocument()
  })

  // ── Accessibility ───────────────────────────────────────────────────────
  it('has an accessible aria-label on the slider', () => {
    render(<ThresholdSlider value={0.95} onChange={vi.fn()} />)
    const slider = screen.getByRole('slider')
    expect(slider).toHaveAttribute('aria-label', 'Confidence threshold')
  })

  it('exposes aria-valuetext with the formatted percentage', () => {
    render(<ThresholdSlider value={0.93} onChange={vi.fn()} />)
    const slider = screen.getByRole('slider')
    expect(slider).toHaveAttribute('aria-valuetext', '93.00%')
  })
})
