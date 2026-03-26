import React, { useState, useEffect, Suspense, Component } from 'react'
import {
  Activity,
  Calculator,
  User,
  ClipboardList,
  LineChart,
  Rocket,
  Settings,
  Wifi,
  WifiOff,
  Loader2,
} from 'lucide-react'

// Lazy-load all module views for code splitting
const RouterDashboard = React.lazy(() =>
  import('./modules/router/RouterDashboard').then(m => ({ default: m.RouterDashboard }))
)
const SimulatorView = React.lazy(() =>
  import('./modules/simulator/SimulatorView').then(m => ({ default: m.SimulatorView }))
)
const CopilotView = React.lazy(() =>
  import('./modules/copilot/CopilotView').then(m => ({ default: m.CopilotView }))
)
const RLHFView = React.lazy(() =>
  import('./modules/rlhf/RLHFView').then(m => ({ default: m.RLHFView }))
)
const AnalyticsDashboard = React.lazy(() =>
  import('./modules/analytics/AnalyticsDashboard').then(m => ({ default: m.AnalyticsDashboard }))
)
const DeploymentDashboard = React.lazy(() =>
  import('./modules/deployment/DeploymentDashboard').then(m => ({
    default: m.DeploymentDashboard,
  }))
)

type ViewId = 'router' | 'simulator' | 'copilot' | 'rlhf' | 'analytics' | 'deployment'

interface NavItem {
  id: ViewId
  label: string
  Icon: React.ElementType
  description: string
}

const NAV_ITEMS: NavItem[] = [
  { id: 'router',     label: 'HITL Router',      Icon: Activity,      description: 'Live ticket routing' },
  { id: 'simulator',  label: 'Cost Simulator',   Icon: Calculator,    description: 'Threshold optimization' },
  { id: 'copilot',    label: 'Operator Copilot', Icon: User,          description: 'AI-assisted review' },
  { id: 'rlhf',       label: 'RLHF Capture',     Icon: ClipboardList, description: 'Feedback loop' },
  { id: 'analytics',  label: 'Analytics',        Icon: LineChart,     description: 'Time-lapse metrics' },
  { id: 'deployment', label: 'Deployment',       Icon: Rocket,        description: '5-phase tracker' },
]

// Default props for components that require ticket context
const DEFAULT_TICKET_ID           = 'demo-001'
const DEFAULT_OPERATOR_ID         = 'operator-001'
const DEFAULT_RECOMMENDED_ACTION  = 'approve' as const

interface ErrorBoundaryState { hasError: boolean; message: string }

class ErrorBoundary extends Component<
  { children: React.ReactNode },
  ErrorBoundaryState
> {
  state: ErrorBoundaryState = { hasError: false, message: '' }

  static getDerivedStateFromError(error: Error): ErrorBoundaryState {
    return { hasError: true, message: error.message }
  }

  render() {
    if (this.state.hasError) {
      return (
        <div className="flex items-center justify-center h-full bg-black">
          <div className="text-center space-y-2">
            <p className="text-red-500 text-sm font-medium">Failed to load module</p>
            <p className="text-zinc-600 text-xs">{this.state.message}</p>
            <button
              className="mt-3 text-xs text-zinc-400 underline"
              onClick={() => this.setState({ hasError: false, message: '' })}
            >
              Retry
            </button>
          </div>
        </div>
      )
    }
    return this.props.children
  }
}

function LoadingFallback() {
  return (
    <div className="flex items-center justify-center h-full bg-black">
      <Loader2 className="w-5 h-5 text-zinc-500 animate-spin" />
    </div>
  )
}

export default function App() {
  const [activeView, setActiveView]   = useState<ViewId>('router')
  const [apiHealthy, setApiHealthy]   = useState<boolean | null>(null)

  // Poll backend health endpoint
  useEffect(() => {
    const check = () => {
      fetch('/health')
        .then(r => setApiHealthy(r.ok))
        .catch(() => setApiHealthy(false))
    }
    check()
    const id = setInterval(check, 30_000)
    return () => clearInterval(id)
  }, [])

  function renderView() {
    switch (activeView) {
      case 'router':     return <RouterDashboard />
      case 'simulator':  return <SimulatorView />
      case 'copilot':    return <CopilotView ticketId={DEFAULT_TICKET_ID} />
      case 'rlhf':
        return (
          <RLHFView
            ticketId={DEFAULT_TICKET_ID}
            recommendedAction={DEFAULT_RECOMMENDED_ACTION}
            operatorId={DEFAULT_OPERATOR_ID}
          />
        )
      case 'analytics':  return <AnalyticsDashboard />
      case 'deployment': return <DeploymentDashboard />
    }
  }

  const activeItem = NAV_ITEMS.find(n => n.id === activeView)!

  return (
    <div className="flex h-screen overflow-hidden" style={{ background: '#000000' }}>
      {/* ── Sidebar ─────────────────────────────────────────────── */}
      <aside
        className="w-60 flex-shrink-0 flex flex-col border-r border-zinc-800"
        style={{ background: '#0A0A0A' }}
      >
        {/* Brand */}
        <div className="px-5 py-5 border-b border-zinc-800">
          <div className="flex items-center gap-3">
            <div className="w-7 h-7 rounded flex items-center justify-center bg-white">
              <Settings className="w-4 h-4 text-black" />
            </div>
            <div>
              <h1 className="text-white font-semibold text-sm tracking-tight leading-tight">
                WFM Triage Engine
              </h1>
              <p className="text-zinc-500 text-xs mt-0.5">Unit Economics Platform</p>
            </div>
          </div>
        </div>

        {/* Navigation */}
        <nav className="flex-1 py-3 px-2 space-y-0.5 overflow-y-auto">
          {NAV_ITEMS.map(item => {
            const isActive = item.id === activeView
            return (
              <button
                key={item.id}
                onClick={() => setActiveView(item.id)}
                className={[
                  'w-full text-left px-3 py-2.5 rounded-md flex items-center gap-3',
                  'transition-all duration-200 ease-in-out group',
                  isActive
                    ? 'bg-white text-black'
                    : 'text-zinc-400 hover:bg-zinc-800 hover:text-white',
                ].join(' ')}
              >
                <item.Icon
                  className={[
                    'w-4 h-4 flex-shrink-0 transition-colors duration-200',
                    isActive ? 'text-black' : 'text-zinc-500 group-hover:text-white',
                  ].join(' ')}
                  strokeWidth={1.75}
                />
                <div className="min-w-0">
                  <div className="text-sm font-medium truncate">{item.label}</div>
                  {!isActive && (
                    <div className="text-xs text-zinc-600 truncate group-hover:text-zinc-400 transition-colors duration-200">
                      {item.description}
                    </div>
                  )}
                </div>
              </button>
            )
          })}
        </nav>

        {/* API health indicator */}
        <div className="px-5 py-4 border-t border-zinc-800">
          <div className="flex items-center gap-2">
            {apiHealthy === null ? (
              <Loader2 className="w-3 h-3 text-yellow-500 animate-spin flex-shrink-0" />
            ) : apiHealthy ? (
              <Wifi className="w-3 h-3 text-emerald-500 flex-shrink-0" />
            ) : (
              <WifiOff className="w-3 h-3 text-red-500 flex-shrink-0" />
            )}
            <span className="text-xs text-zinc-500">
              {apiHealthy === null
                ? 'Checking…'
                : apiHealthy
                ? 'Backend connected'
                : 'API offline'}
            </span>
          </div>
        </div>
      </aside>

      {/* ── Main content ─────────────────────────────────────────── */}
      <main className="flex-1 flex flex-col overflow-hidden" style={{ background: '#000000' }}>
        {/* Top bar */}
        <header
          className="border-b border-zinc-800 px-6 py-3 flex items-center gap-3 flex-shrink-0"
          style={{ background: '#0A0A0A' }}
        >
          <activeItem.Icon className="w-4 h-4 text-zinc-400" strokeWidth={1.75} />
          <div>
            <h2 className="text-white font-semibold text-sm tracking-tight">
              {activeItem.label}
            </h2>
            <p className="text-zinc-500 text-xs">{activeItem.description}</p>
          </div>
        </header>

        {/* View */}
        <div className="flex-1 overflow-auto">
          <ErrorBoundary>
            <Suspense fallback={<LoadingFallback />}>
              {renderView()}
            </Suspense>
          </ErrorBoundary>
        </div>
      </main>
    </div>
  )
}
