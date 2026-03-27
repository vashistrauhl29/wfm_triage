import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import { ThresholdProvider } from './context/ThresholdContext'
import './index.css'

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <ThresholdProvider>
      <App />
    </ThresholdProvider>
  </React.StrictMode>
)
