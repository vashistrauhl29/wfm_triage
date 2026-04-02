import { motion } from 'framer-motion'

interface SkeletonLoaderProps {
  variant?: 'card' | 'text' | 'circle' | 'ticket'
  count?: number
  className?: string
}

export const SkeletonLoader: React.FC<SkeletonLoaderProps> = ({
  variant = 'card',
  count = 1,
  className = '',
}) => {
  const renderSkeleton = () => {
    switch (variant) {
      case 'ticket':
        return (
          <div className={`border border-zinc-800 rounded-lg p-4 bg-[#0A0A0A] ${className}`}>
            <div className="flex items-center justify-between">
              <div className="flex-1 space-y-3">
                <div className="flex items-center gap-3">
                  <div className="h-5 w-24 bg-zinc-800 rounded animate-pulse" />
                  <div className="h-4 w-20 bg-zinc-800 rounded animate-pulse" />
                </div>
                <div className="flex items-center gap-4">
                  <div className="h-4 w-16 bg-zinc-800 rounded animate-pulse" />
                  <div className="h-6 w-20 bg-zinc-800 rounded-full animate-pulse" />
                </div>
              </div>
              <div className="h-3 w-16 bg-zinc-800 rounded animate-pulse" />
            </div>
          </div>
        )

      case 'card':
        return (
          <div className={`bg-[#0A0A0A] rounded-lg p-6 border border-zinc-800 ${className}`}>
            <div className="space-y-3">
              <div className="h-6 w-32 bg-zinc-800 rounded animate-pulse" />
              <div className="h-4 w-full bg-zinc-800 rounded animate-pulse" />
              <div className="h-4 w-3/4 bg-zinc-800 rounded animate-pulse" />
            </div>
          </div>
        )

      case 'text':
        return <div className={`h-4 bg-zinc-800 rounded animate-pulse ${className}`} />

      case 'circle':
        return <div className={`rounded-full bg-zinc-800 animate-pulse ${className}`} />

      default:
        return null
    }
  }

  return (
    <motion.div
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.2 }}
    >
      {Array.from({ length: count }).map((_, index) => (
        <div key={index} className="mb-2">
          {renderSkeleton()}
        </div>
      ))}
    </motion.div>
  )
}
