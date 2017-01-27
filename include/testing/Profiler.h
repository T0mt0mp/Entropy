/**
 * @file testing/Profiler.h
 * @author Tomas Polasek
 * @brief Contains profiling code.
 */

#ifndef TESTING_PROFILER_H
#define TESTING_PROFILER_H

#include <atomic>
#include <thread>

#include "math/Math.h"
#include "util/Threading.h"
#include "testing/HashNode.h"

// Profiling macros:
#ifndef NPROFILE
#   define PROF_CONCAT(FIRST, SECOND) \
        FIRST ## SECOND
#   define PROF_NAME(FIRST, SECOND) \
        PROF_CONCAT(FIRST, SECOND)
#   define PROF_BLOCK(NAME) \
        prof::BlockProfiler PROF_NAME(_bp, __LINE__)(NAME)
#   define PROF_BLOCK_END() \
        prof::BlockProfiler::end()
#   define PROF_SCOPE(NAME) \
        prof::ScopeProfiler PROF_NAME(_sp, __LINE__)(NAME)
#   define PROF_DUMP(CRAWLER) \
        prof::ProfilingManager::instance().useCrawler(CRAWLER)
#   define PROF_THREAD(NAME) \
        prof::ThreadProfiler PROF_NAME(_tp, __LINE__)(NAME)
#else
#   define PROF_CONCAT(F, S)
#   define PROF_NAME(F, S)
#   define PROF_BLOCK(_)
#   define PROF_BLOCK_END(_)
#   define PROF_SCOPE(_)
#   define PROF_DUMP(_)
#   define PROF_THREAD(_)
#endif

namespace prof
{
    /**
     * Timer class, using rdtsc instruction.
     */
    class Timer
    {
    public:
        static constexpr f64 TICKS_TO_MEGACYCLES{1000000.0};

        Timer()
        {
            reset();
        }

        /**
         * Reset this timer.
         */
        void reset()
        {
            mNumSamples = 0;
            mRunning = false;
            mStart = 0;
            mTicks = 0;
        }

        /**
         * Get the current number of ticks taken by active timer.
         * Returns 0, if the timer is not active.
         * @return Returns the actual number of ticks from active timer.
         */
        u64 runningTicks()
        {
            if (mRunning)
            {
                return getTicks() - mStart;
            }

            return 0;
        }

        /**
         * Get the current number of megacycles from running timer.
         * Returns 0.0, if the timer is not running.
         * @return Returns the number of megacycles, from the start of this timer.
         */
        f64 runningMegacycles()
        {
            return ticksToMegacycles(runningTicks());
        }

        /**
         * Convert ticks to megacycles.
         * @param ticks Number of ticks.
         * @return Number of megacycles.
         */
        static f64 ticksToMegacycles(u64 ticks)
        {
            return ticks / TICKS_TO_MEGACYCLES;
        }

        /**
         * Convert megacycles to ticks.
         * @param megacycles Amount of megacycles.
         * @return Amount of ticks.
         */
        static u64 megacyclesToTicks(f64 megacycles)
        {
            return static_cast<u64>(megacycles * TICKS_TO_MEGACYCLES);
        }

        /**
         * Add given number of ticks to the internal counter.
         * @param ticks Number of ticks to add.
         */
        void addTicks(u64 ticks)
        {
            mTicks += ticks;
        }

        /**
         * Start the timer.
         */
        void start()
        {
            mNumSamples++;
            mRunning = true;
            mStart = getTicks();
        }

        /**
         * Stop the timer and calculate the number of ticks
         * passed between now and start.
         */
        void stop()
        {
            mTicks += (getTicks() - mStart);
            mRunning = false;
        }

        /**
         * Stop the timer, only if it is running.
         */
        void condStop()
        {
            if (mRunning)
            {
                stop();
            }
        }

        /**
         * Get the number of ticks counted by this timer.
         * @return The number of ticks.
         */
        u64 ticks()
        {
            return mTicks;
        }

        /**
         * Get the actual number of ticks.
         * If the timer is currently running, count the ticks
         * from the start.
         * @return The actual number of ticks.
         */
        u64 actualTicks()
        {
            if (mRunning)
            {
                return mTicks + (getTicks() - mStart);
            }
            else
                return mTicks;
        }

        /**
         * Set the number of megacycles to given number.
         * @param megacycles Number of megacycles.
         */
        void setMegacycles(f64 megacycles)
        {
            mTicks = megacyclesToTicks(megacycles);
        }

        /**
         * Get the number of megacycles counted by this timer.
         * @return Returns the number of ticks divided by 10^6
         */
        f64 megacycles()
        {
            return ticksToMegacycles(mTicks);
        }

        /**
         * Get the actual number of megacycles counted by this timer.
         * @return Returns the actual number of ticks divided by 10^6
         */
        f64 actualMegacycles()
        {
            return ticksToMegacycles(actualTicks());
        }

        /**
         * Get the average number of megacycles.
         * @return Get the average number of megacycles per sample
         */
        f64 avg()
        {
            return math::avg(megacycles(), mNumSamples);
        }

        /**
         * Get the average number of megacycles.
         * @return Get the average number of megacycles per sample
         */
        f64 actualAvg()
        {
            return math::avg(actualMegacycles(), mNumSamples);
        }

        /**
         * Get the average number of ticks.
         * @return Get the average number of ticks per sample
         */
        f64 avgTicks()
        {
            return math::avg(mTicks, mNumSamples);
        }

        /**
         * Get the number of samples taken by this timer.
         */
        u64 numSamples()
        {
            return mNumSamples;
        }

        /**
         * Get the actual number of samples, including the running one.
         */
        u64 actualNumSamples()
        {
            return mNumSamples;
        }

        /**
         * Get current number of ticks passed since reset.
         * Uses rdtsc instruction.
         * @return The number of ticks passed.
         */
        static u64 getTicks()
        {
#ifdef _WIN32
            return __rdtsc();
#else
#if 1
            u32 lo;
            u32 hi;
            __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
            return ((u64)hi << 32) | lo;
#elif 0
            timespec t;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
            return static_cast<u64>(t.tv_nsec);
#else
            return
            std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
#endif
        }
    private:
    protected:
        /// Start time in ticks.
        u64 mStart{0};
        /// Current time passed in ticks.
        u64 mTicks{0};
        /// Number of samples taken with this timer.
        u64 mNumSamples{0};
        /// Is the timer currently running?
        i8 mRunning{false};
    };

    /**
     * Scope timer. Uses already existing Timer to measure ticks taken
     * for a single scope.
     */
    class ScopeTimer
    {
    public:
        ScopeTimer(Timer &timer) :
            mTimer(timer)
        {
            mTimer.start();
        }

        ~ScopeTimer()
        {
            mTimer.stop();
        }
    private:
    protected:
        /// Holds reference to the used timer object.
        Timer &mTimer;
    };

    /**
     * Measurement data for each node.
     */
    struct CallData
    {
    public:
        /// Start the timer.
        void startTimer()
        {
            mTimer.start();
        }

        /// Stop the timer.
        void stopTimer()
        {
            mTimer.stop();
        }

        /// Get the current megacycles from the timer.
        f64 getActualMegacycles()
        {
            return mTimer.actualMegacycles();
        }

        /// Get the current average megacycles per run.
        f64 getActualAvg()
        {
            return mTimer.actualAvg();
        }

        /// Get the number of samples taken by the timer.
        u64 getActualNumSamples()
        {
            return mTimer.actualNumSamples();
        }

        /// Add given number of ticks to the timer.
        void addTicks(u64 ticks)
        {
            mTimer.addTicks(ticks);
        }
    private:
    protected:
        /// Timer used for timing this node.
        Timer mTimer;
    };

    using CallNode = util::HashNode<CallData>;

    /**
     * Data for each thread name.
     */
    struct ThreadData
    {
    public:
        ThreadData() :
            mRoot(new CallNode("/", nullptr))
        {
        }

        ~ThreadData()
        {
            delete mRoot;
        }

        /**
         * Try to occupy this thread call stack.
         * @return Returns true, if the occupation succeeded, else
         *  returns false.
         */
        i8 occupy()
        {
            return !mOccupied.tryLock();
        }

        /**
         * Free this call stack.
         */
        void freeOccupation()
        {
            mOccupied.unlock();
        }

#ifdef PROFILE_LOCK

        /// Lock the thread structures.
        void lock()
        {
            mLock.lock();
        }

        /// Unlock the thread structures.
        void unlock()
        {
            mLock.unlock();
        }
#endif

        /// Get the root of threads call stack.
        CallNode *getRoot()
        {
            return mRoot;
        }
    private:
    protected:
#ifdef PROFILE_LOCK
        /// Used for locking the call stack.
        thr::SpinLock mLock;
#endif

        /// Root node of the call stack.
        CallNode *mRoot;

        /// Only one thread is allowed in each thread call stack.
        thr::SpinLock mOccupied;
    };

    using ThreadNode = util::HashNode<ThreadData>;

    /**
     * Print information about each node.
     */
    class ForeachPrint
    {
    public:
        ForeachPrint(u32 indent = 0, f64 parentMegacycles = 0);
        void operator()(CallNode *node);
    protected:
        /// Level of indentation.
        u32 mIndentLevel{0};
        /// Megacycles of parent node.
        f64 mParentMegacycles{0.0};
    };

    /**
     * Sum megacycles of all children
     */
    class ForeachSumMegacycles
    {
    public:
        void operator()(CallNode *node);

        /**
         * Sum getter.
         * @return Returns the sum.
         */
        f64 sum()
        {
            return mSum;
        }

        /**
         * Number of samples getter.
         * @return Number of samples.
         */
        u64 numSamples()
        {
            return mNumSamples;
        }
    protected:
        /// Sum of megacycles.
        f64 mSum{0.0};
        /// Number of samples taken.
        u64 mNumSamples{0};
    };

    /**
     * Contains information about threads call stack.
     */
    class ThreadStatus
    {
    public:
        /// Ptr to the current node in the call stack.
        CallNode *mCurNode{nullptr};
        /// Ptr to the root node of the call stack.
        CallNode *mRoot{nullptr};
        /// Ptr to the current thread node.
        ThreadNode *mThreadNode{nullptr};
    private:
    protected:
    };

    /**
     * Abstract call stack crawler.
     */
    class CallStackCrawler
    {
    public:
        /**
         * Function is called by the ProfilingManager.
         * @param root The root node of the profiling manager.
         */
        virtual void crawl(ThreadNode *root) = 0;
    private:
    protected:
    };

    /**
     * Profiling manager keeps information about each thread
     * and its location in the call stack.
     * It also contains the interface to all profiling functions.
     */
    class ProfilingManager
    {
    public:
        /**
         * Enter a scope with given name.
         * @param name Identifier of the scope.
         * @return Returns ptr to the scope node.
         */
        CallNode *enterScope(const char* name);

        /**
         * Exit the current scope and go to the
         * parent scope.
         * If there is no parent scope, returns nullptr.
         * @return Returns ptr to the parent, or nullptr.
         */
        CallNode *exitScope();

        /**
         * Enter a thread with given name.
         * @param name Identifier of the thread.
         * @return Returns ptr to the thread node.
         */
        ThreadNode *enterThread(const char* name);

        /**
         * Exit the current thread.
         */
        void exitThread();

        /**
         * Get the current scope node.
         * @return Returns ptr to the current node,
         * should never return nullptr.
         */
        CallNode *getCurrentScope();

        /**
         * Use given crawler object to crawl through
         * the call stack.
         * @param crawler Reference to the crawler object.
         */
        void useCrawler(CallStackCrawler &crawler);

        /**
         * Get the overhead of entering and exiting scope.
         * @return The overhead.
         */
        f64 getScopeOverhead()
        {
            return mScopeOverhead;
        }

        // Statistics getters
        u64 getNumScopeEnter()
        {
            return mNumScopeEnter;
        }

        u64 getNumScopeExit()
        {
            return mNumScopeExit;
        }

        u64 getNumThreadEnter()
        {
            return mNumThreadEnter;
        }

        u64 getNumThreadExit()
        {
            return mNumThreadExit;
        }

        /**
         * Get instance of the profiling manager.
         * @return Singleton instance of the profiling manager.
         */
        static ProfilingManager &instance();
    private:
        ProfilingManager();
        ~ProfilingManager();

        void reset();
    protected:
        /// Root node of the profiling tree.
        ThreadNode *mRoot{nullptr};
        /// Global lock for adding threads to the ProfilingManager.
        thr::SpinLock mGlobalLock;
        /// Thread local storage containing call stack information.
        thread_local static ThreadStatus mThreadStatus;
        /// Statistics
        u64 mNumScopeEnter{0};
        u64 mNumScopeExit{0};
        u64 mNumThreadEnter{0};
        u64 mNumThreadExit{0};
        /// Overhead per enter/exit scope.
        f64 mScopeOverhead{0.0};
    };

    /**
     * Scope profiler.
     */
    class ScopeProfiler
    {
    public:
        /**
         * Create a scope profiler, add it to the global structure
         * (if necessary) and start the timer.
         * @param desc Description for this profiler.
         */
        ScopeProfiler(const char *desc);

        /**
         * Exit the scope and stop the timer.
         */
        ~ScopeProfiler();
    private:
    protected:
    };

    /**
     * Scope profiler.
     */
    class BlockProfiler
    {
    public:
        /**
         * Create a block profiler, add it to the global structure
         * (if necessary) and start the timer.
         * @param desc Description for this profiler.
         */
        BlockProfiler(const char *desc);

        /**
         * Exit the scope and stop the timer.
         */
        static void end();
    private:
    protected:
    };

    /**
     * Thread profiler.
     */
    class ThreadProfiler
    {
    public:
        /**
         * Create a scope profiler, add it to the global structure
         * (if necessary) and start the timer.
         * @param desc Description for this profiler.
         */
        ThreadProfiler(const char *desc);

        /**
         * Exit the scope and stop the timer.
         */
        ~ThreadProfiler();
    private:
    protected:
    };
}

#endif //TESTING_PROFILER_H
