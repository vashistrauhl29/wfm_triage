import { describe, it, expect, vi } from 'vitest'
import { render, screen, fireEvent } from '@testing-library/react'
import { ActionButtons } from '../ActionButtons'

describe('ActionButtons', () => {
  // ── Rendering ────────────────────────────────────────────────────────────
  it('renders the Approve button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /approve/i })).toBeInTheDocument()
  })

  it('renders the Reject button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /reject/i })).toBeInTheDocument()
  })

  it('renders the Escalate button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /escalate/i })).toBeInTheDocument()
  })

  // ── Callbacks ────────────────────────────────────────────────────────────
  it('calls onApprove when the Approve button is clicked', () => {
    const onApprove = vi.fn()
    render(
      <ActionButtons onApprove={onApprove} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    fireEvent.click(screen.getByRole('button', { name: /approve/i }))
    expect(onApprove).toHaveBeenCalledTimes(1)
  })

  it('calls onReject when the Reject button is clicked', () => {
    const onReject = vi.fn()
    render(
      <ActionButtons onApprove={vi.fn()} onReject={onReject} onEscalate={vi.fn()} />
    )
    fireEvent.click(screen.getByRole('button', { name: /reject/i }))
    expect(onReject).toHaveBeenCalledTimes(1)
  })

  it('calls onEscalate when the Escalate button is clicked', () => {
    const onEscalate = vi.fn()
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={onEscalate} />
    )
    fireEvent.click(screen.getByRole('button', { name: /escalate/i }))
    expect(onEscalate).toHaveBeenCalledTimes(1)
  })

  it('does not call onReject or onEscalate when Approve is clicked', () => {
    const onApprove = vi.fn()
    const onReject = vi.fn()
    const onEscalate = vi.fn()
    render(
      <ActionButtons onApprove={onApprove} onReject={onReject} onEscalate={onEscalate} />
    )
    fireEvent.click(screen.getByRole('button', { name: /approve/i }))
    expect(onReject).not.toHaveBeenCalled()
    expect(onEscalate).not.toHaveBeenCalled()
  })

  it('calls each handler only once per click', () => {
    const onApprove = vi.fn()
    const onReject = vi.fn()
    const onEscalate = vi.fn()
    render(
      <ActionButtons onApprove={onApprove} onReject={onReject} onEscalate={onEscalate} />
    )
    fireEvent.click(screen.getByRole('button', { name: /approve/i }))
    fireEvent.click(screen.getByRole('button', { name: /reject/i }))
    fireEvent.click(screen.getByRole('button', { name: /escalate/i }))
    expect(onApprove).toHaveBeenCalledTimes(1)
    expect(onReject).toHaveBeenCalledTimes(1)
    expect(onEscalate).toHaveBeenCalledTimes(1)
  })

  // ── Disabled state ───────────────────────────────────────────────────────
  it('disables all buttons when disabled prop is true', () => {
    render(
      <ActionButtons
        onApprove={vi.fn()}
        onReject={vi.fn()}
        onEscalate={vi.fn()}
        disabled
      />
    )
    expect(screen.getByRole('button', { name: /approve/i })).toBeDisabled()
    expect(screen.getByRole('button', { name: /reject/i })).toBeDisabled()
    expect(screen.getByRole('button', { name: /escalate/i })).toBeDisabled()
  })

  it('does not call onApprove when disabled and Approve is clicked', () => {
    const onApprove = vi.fn()
    render(
      <ActionButtons
        onApprove={onApprove}
        onReject={vi.fn()}
        onEscalate={vi.fn()}
        disabled
      />
    )
    fireEvent.click(screen.getByRole('button', { name: /approve/i }))
    expect(onApprove).not.toHaveBeenCalled()
  })

  it('enables all buttons when disabled prop is false', () => {
    render(
      <ActionButtons
        onApprove={vi.fn()}
        onReject={vi.fn()}
        onEscalate={vi.fn()}
        disabled={false}
      />
    )
    expect(screen.getByRole('button', { name: /approve/i })).not.toBeDisabled()
    expect(screen.getByRole('button', { name: /reject/i })).not.toBeDisabled()
    expect(screen.getByRole('button', { name: /escalate/i })).not.toBeDisabled()
  })

  it('enables all buttons by default when disabled prop is omitted', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /approve/i })).not.toBeDisabled()
    expect(screen.getByRole('button', { name: /reject/i })).not.toBeDisabled()
    expect(screen.getByRole('button', { name: /escalate/i })).not.toBeDisabled()
  })

  // ── Color coding ─────────────────────────────────────────────────────────
  it('applies a green color class to the Approve button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /approve/i })).toHaveClass('bg-green-600')
  })

  it('applies a red color class to the Reject button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /reject/i })).toHaveClass('bg-red-600')
  })

  it('applies a yellow color class to the Escalate button', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('button', { name: /escalate/i })).toHaveClass('bg-yellow-500')
  })

  // ── Accessibility ────────────────────────────────────────────────────────
  it('groups the action buttons in a toolbar or group container', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByRole('group')).toBeInTheDocument()
  })

  it('renders an "Actions" label for the button group', () => {
    render(
      <ActionButtons onApprove={vi.fn()} onReject={vi.fn()} onEscalate={vi.fn()} />
    )
    expect(screen.getByText(/actions/i)).toBeInTheDocument()
  })
})
