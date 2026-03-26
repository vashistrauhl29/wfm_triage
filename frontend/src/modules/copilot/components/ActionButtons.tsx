import React from 'react'

interface ActionButtonsProps {
  onApprove: () => void
  onReject: () => void
  onEscalate: () => void
  disabled?: boolean
}

export const ActionButtons: React.FC<ActionButtonsProps> = ({
  onApprove,
  onReject,
  onEscalate,
  disabled = false,
}) => {
  return (
    <div className="bg-[#0A0A0A] rounded-lg border border-zinc-800 p-6">
      <p className="text-sm font-medium text-zinc-300 mb-3">Actions</p>
      <div role="group" className="flex gap-3">
        <button
          onClick={!disabled ? onApprove : undefined}
          disabled={disabled}
          className="bg-green-600 bg-opacity-10 border border-emerald-500/40 text-emerald-400 px-4 py-2 rounded-md text-sm font-medium hover:bg-opacity-20 transition-all duration-200 disabled:opacity-40 disabled:cursor-not-allowed"
        >
          Approve
        </button>
        <button
          onClick={!disabled ? onReject : undefined}
          disabled={disabled}
          className="bg-red-600 bg-opacity-10 border border-rose-500/40 text-rose-400 px-4 py-2 rounded-md text-sm font-medium hover:bg-opacity-20 transition-all duration-200 disabled:opacity-40 disabled:cursor-not-allowed"
        >
          Reject
        </button>
        <button
          onClick={!disabled ? onEscalate : undefined}
          disabled={disabled}
          className="bg-yellow-500 bg-opacity-10 border border-amber-500/40 text-amber-400 px-4 py-2 rounded-md text-sm font-medium hover:bg-opacity-20 transition-all duration-200 disabled:opacity-40 disabled:cursor-not-allowed"
        >
          Escalate
        </button>
      </div>
    </div>
  )
}
