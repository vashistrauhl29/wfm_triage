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
import { motion, AnimatePresence } from 'framer-motion'

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
    <motion.div
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.2 }}
      className="flex items-center justify-center h-full bg-black"
    >
      <div className="text-center space-y-3">
        <Loader2 className="w-6 h-6 text-zinc-500 animate-spin mx-auto" />
        <p className="text-xs text-zinc-600">Loading module...</p>
      </div>
    </motion.div>
  )
}

export default function App() {
  const [activeView, setActiveView]   = useState<ViewId>('router')
  const [apiHealthy, setApiHealthy]   = useState<boolean | null>(null)
  const [showErrorIndicator, setShowErrorIndicator] = useState(false)

  // Poll backend health endpoint with debounced error display
  useEffect(() => {
    let errorTimeout: ReturnType<typeof setTimeout> | null = null

    const check = () => {
      fetch('/health')
        .then(r => {
          setApiHealthy(r.ok)
          setShowErrorIndicator(false)
          if (errorTimeout) {
            clearTimeout(errorTimeout)
            errorTimeout = null
          }
        })
        .catch(() => {
          // Only show error indicator after 2 seconds of failed connection
          // This prevents flash during page load/refresh
          if (!errorTimeout) {
            errorTimeout = setTimeout(() => {
              setApiHealthy(false)
              setShowErrorIndicator(true)
            }, 2000)
          }
        })
    }
    check()
    const id = setInterval(check, 30_000)
    return () => {
      clearInterval(id)
      if (errorTimeout) clearTimeout(errorTimeout)
    }
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
        <motion.div
          initial={{ opacity: 0, y: -10 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.4 }}
          className="px-5 py-5 border-b border-zinc-800"
        >
          <div className="flex items-center gap-3">
            <motion.div
              whileHover={{ rotate: 360 }}
              transition={{ duration: 0.6 }}
              className="w-9 h-9 rounded-lg flex items-center justify-center bg-gradient-to-br from-primary-600 to-primary-700 shadow-premium"
            >
              <Settings className="w-5 h-5 text-white" strokeWidth={2} />
            </motion.div>
            <div>
              <h1 className="text-white font-bold text-base tracking-tight leading-tight">
                WFM Triage Engine
              </h1>
              <p className="text-zinc-500 text-xs mt-0.5 font-medium">Unit Economics Platform</p>
            </div>
          </div>
        </motion.div>

        {/* Navigation */}
        <nav className="flex-1 py-3 px-2 space-y-0.5 overflow-y-auto">
          {NAV_ITEMS.map(item => {
            const isActive = item.id === activeView
            return (
              <motion.button
                key={item.id}
                onClick={() => setActiveView(item.id)}
                whileHover={{ scale: 1.02, x: 2 }}
                whileTap={{ scale: 0.98 }}
                transition={{ duration: 0.2, ease: 'easeOut' }}
                className={[
                  'w-full text-left px-3 py-2.5 rounded-md flex items-center gap-3',
                  'transition-all duration-200 ease-in-out group',
                  isActive
                    ? 'bg-white text-black shadow-lg'
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
              </motion.button>
            )
          })}
        </nav>

        {/* API health indicator */}
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ delay: 0.5 }}
          className="px-5 py-4 border-t border-zinc-800 bg-surface"
        >
          <div className="flex items-center gap-2.5">
            <div className="relative">
              {apiHealthy === null || (apiHealthy && !showErrorIndicator) ? (
                <>
                  <Wifi className="w-3.5 h-3.5 text-emerald-500 flex-shrink-0 transition-opacity duration-300" />
                  <div className="absolute -inset-1 bg-emerald-500/20 rounded-full blur animate-pulse" />
                </>
              ) : showErrorIndicator ? (
                <>
                  <WifiOff className="w-3.5 h-3.5 text-red-500 flex-shrink-0 animate-pulse" />
                  <div className="absolute -inset-1 bg-red-500/20 rounded-full blur animate-pulse" />
                </>
              ) : (
                <Loader2 className="w-3.5 h-3.5 text-yellow-500 animate-spin flex-shrink-0" />
              )}
            </div>
            <div className="flex flex-col">
              <span className="text-xs font-medium text-zinc-400 transition-colors duration-300">
                {showErrorIndicator ? 'Disconnected' : 'Connected'}
              </span>
              <span className="text-[10px] text-zinc-600">
                {showErrorIndicator ? 'Retrying...' : 'Live stream active'}
              </span>
            </div>
          </div>
        </motion.div>
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
              <AnimatePresence mode="wait">
                <motion.div
                  key={activeView}
                  initial={{ opacity: 0, y: 10 }}
                  animate={{ opacity: 1, y: 0 }}
                  exit={{ opacity: 0, y: -10 }}
                  transition={{ duration: 0.3, ease: 'easeInOut' }}
                  className="h-full"
                >
                  {renderView()}
                </motion.div>
              </AnimatePresence>
            </Suspense>
          </ErrorBoundary>
        </div>
      </main>
    </div>
  )
}
