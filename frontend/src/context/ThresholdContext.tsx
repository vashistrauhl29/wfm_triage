import React, { createContext, useContext, useState, useCallback, useEffect } from 'react'

const BACKEND = import.meta.env.VITE_API_URL || 'https://wfm-backend-645460010450.us-central1.run.app'
const STORAGE_KEY = 'wfm_threshold'
const DEFAULT_THRESHOLD = 0.95

interface ThresholdContextValue {
  threshold: number
  setThreshold: (t: number) => void
}

const ThresholdContext = createContext<ThresholdContextValue>({
  threshold: DEFAULT_THRESHOLD,
  setThreshold: () => {},
})

export function ThresholdProvider({ children }: { children: React.ReactNode }) {
  const [threshold, setThresholdState] = useState<number>(() => {
    const stored = parseFloat(localStorage.getItem(STORAGE_KEY) ?? '')
    return isNaN(stored) ? DEFAULT_THRESHOLD : stored
  })

  const setThreshold = useCallback((t: number) => {
    setThresholdState(t)
    localStorage.setItem(STORAGE_KEY, t.toString())
    fetch(`${BACKEND}/api/v1/router/threshold`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ threshold: t }),
    }).catch(() => {})
  }, [])

  // Sync to backend once on mount so the SSE stream starts with the right value
  useEffect(() => {
    fetch(`${BACKEND}/api/v1/router/threshold`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ threshold }),
    }).catch(() => {})
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  return (
    <ThresholdContext.Provider value={{ threshold, setThreshold }}>
      {children}
    </ThresholdContext.Provider>
  )
}

export const useThreshold = () => useContext(ThresholdContext)
