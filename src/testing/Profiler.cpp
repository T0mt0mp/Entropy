/**
 * @file testing/Profiler.cpp
 * @author Tomas Polasek
 * @brief Contains profiling code.
 */

#include "testing/Profiler.h"

namespace prof
{
	u64 ProfilingManager::sConstructions{0};
    thread_local ThreadStatus ProfilingManager::mThreadStatus;
    ProfilingManager &ProfilingManager::instance()
    {
        static ProfilingManager mgr;
        return mgr;
    }
    ProfilingManager &mgr{ProfilingManager::instance()};

    ForeachPrint::ForeachPrint(u32 indent, f64 parentMegacycles) :
        mIndentLevel(indent),
        mParentMegacycles(parentMegacycles)
    {
    }

    void ForeachPrint::operator()(CallNode *node)
    {
        CallData &data = node->data();
        f64 including = data.getActualMegacycles();

        ForeachSumMegacycles adder;
        node->foreachnn(adder);
        f64 inner = adder.sum();

        f64 self = including - inner;
        f64 selfPercentage = including ?
                             (self / including) * 100.0 :
                             (100.0);

        f64 avg = data.getActualAvg();
        f64 parentPercentage = mParentMegacycles ?
                               (including / mParentMegacycles) * 100.0 :
                               (100.0);
        printf("%10.3f %9.3f %12.0llu %9.3f %8.3f %7.3f%% %*s%s\n",
               including, parentPercentage, data.getActualNumSamples(), avg, self,
               selfPercentage, mIndentLevel, "", node->name());

        ForeachPrint printer(mIndentLevel + 1, including);
        node->foreachnn(printer);
    }

    void ForeachSumMegacycles::operator()(CallNode *node)
    {
        mSum += node->data().getActualMegacycles();
        mNumSamples += node->data().getActualNumSamples();
    }

    void ProfilingManager::reset()
    {
        delete mRoot;
        mRoot = new ThreadNode("/", nullptr);

        mNumScopeEnter = 0;
        mNumScopeExit = 0;
        mNumThreadEnter = 0;
        mNumThreadExit = 0;

        mThreadStatus.mCurNode = nullptr;
        mThreadStatus.mRoot = nullptr;
        mThreadStatus.mThreadNode = nullptr;
    }

    ProfilingManager::ProfilingManager() :
        mRoot(new ThreadNode("/", nullptr))
    {
		// Only one construction should happen.
		ASSERT_FAST(sConstructions == 0);
		sConstructions++;

        //mRoot->data().timer().start();

        // Measure amount of time per enterScope->exitScope.
        enterThread("main");

        f64 minOverhead{std::numeric_limits<f32>::max()};

        for (u64 jjj = 0; jjj < 100; ++jjj)
        {
            prof::Timer timer;

            for (u64 iii = 0; iii < 1000; ++iii)
            {
                timer.start();

                enterScope("Testing");
                exitScope();

                timer.stop();
            }

            f64 newAvg{timer.avgTicks()};

            if (newAvg < minOverhead)
                minOverhead = newAvg;
        }

        mScopeOverhead = minOverhead;

        exitThread();

        reset();

        // Create the main thread node.
        enterThread("main");
    }

    ProfilingManager::~ProfilingManager()
    {
        //mRoot->data().timer().stop();

        /*
        ForeachSumMegacycles adder;
        mRoot->foreachnn(adder);
        mRoot->data().timer().setMegacycles(adder.sum());

        ForeachPrint printer;
        printer(mRoot);
         */

        delete mRoot;
    }

    inline CallNode *ProfilingManager::enterScope(const char *name)
    {
        CallNode *retValue{nullptr};

#ifdef PROFILE_LOCK
        ThreadData &thData = mThreadStatus.mThreadNode->data();
        thData.lock();
#endif
        {
            ASSERT_SLOW(mThreadStatus.mCurNode);
            mThreadStatus.mCurNode = mThreadStatus.mCurNode->getCreateChild(name);
            ASSERT_SLOW(mThreadStatus.mCurNode);
            retValue = mThreadStatus.mCurNode;

            mNumScopeEnter++;

            mThreadStatus.mCurNode->data().startTimer();
        }
#ifdef PROFILE_LOCK
        thData.unlock();
#endif

        return retValue;
    }

    inline CallNode *ProfilingManager::exitScope()
    {
        CallNode *retValue{nullptr};

#ifdef PROFILE_LOCK
        ThreadData &thData = mThreadStatus.mThreadNode->data();
        thData.lock();
#endif
        {
            ASSERT_SLOW(mThreadStatus.mCurNode);
            mThreadStatus.mCurNode->data().stopTimer();
            mThreadStatus.mCurNode = mThreadStatus.mCurNode->parent();
            ASSERT_SLOW(mThreadStatus.mCurNode);
            retValue = mThreadStatus.mCurNode;

            mNumScopeExit++;
        }
#ifdef PROFILE_LOCK
        thData.unlock();
#endif

        return retValue;
    }

    inline ThreadNode *ProfilingManager::enterThread(const char* name)
    {
        ThreadNode *threadNode{nullptr};

        mGlobalLock.lock();
        {
            threadNode = mRoot->getCreateChild(name);

            i8 tryOccupy = threadNode->data().occupy();
            ASSERT_FATAL(tryOccupy);

            mThreadStatus.mThreadNode = threadNode;
            mThreadStatus.mCurNode = threadNode->data().getRoot();
            mThreadStatus.mRoot = mThreadStatus.mCurNode;

            mThreadStatus.mRoot->data().startTimer();

            mNumThreadEnter++;
        }
        mGlobalLock.unlock();

        return threadNode;
    }

    void ProfilingManager::exitThread()
    {
        ThreadData &thData = mThreadStatus.mThreadNode->data();
#ifdef PROFILE_LOCK
        thData.lock();
#endif
        {
            mThreadStatus.mRoot->data().stopTimer();

            mNumThreadExit++;
        }
#ifdef PROFILE_LOCK
        thData.unlock();
#endif

        thData.freeOccupation();

        mThreadStatus.mThreadNode = nullptr;
        mThreadStatus.mCurNode = nullptr;
        mThreadStatus.mRoot = nullptr;
    }

    inline CallNode *ProfilingManager::getCurrentScope()
    {
        ASSERT_SLOW(mThreadStatus.mCurNode);
        return mThreadStatus.mCurNode;
    }

    void ProfilingManager::useCrawler(CallStackCrawler &crawler)
    {
        mGlobalLock.lock();
        {
            crawler.crawl(mRoot);
        }
        mGlobalLock.unlock();
    }

    ScopeProfiler::ScopeProfiler(const char *desc)
    {
        mgr.enterScope(desc);
    }

    ScopeProfiler::~ScopeProfiler()
    {
        mgr.exitScope();
    }

    BlockProfiler::BlockProfiler(const char *desc)
    {
        mgr.enterScope(desc);
    }

    void BlockProfiler::end()
    {
        mgr.exitScope();
    }

    ThreadProfiler::ThreadProfiler(const char *desc)
    {
        mgr.enterThread(desc);
    }

    ThreadProfiler::~ThreadProfiler()
    {
        mgr.exitThread();
    }
}
