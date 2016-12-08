/**
 * @file testing/PrintCrawler.h
 * @author Tomas Polasek
 * @brief Printing class for the profiler.
 */

#ifndef TESTING_PRINTCRAWLER_H
#define TESTING_PRINTCRAWLER_H

#include "testing/Profiler.h"

namespace prof
{
    class PrintCrawler : public prof::CallStackCrawler
    {
    public:
        /**
         * Function is called by the ProfilingManager.
         * @param root The root node of the profiling manager.
         */
        virtual void crawl(prof::ThreadNode *root)
        {
            prof::ProfilingManager &mgr = prof::ProfilingManager::instance();

            // Print stats
            printf("Scopes entered : %llu\n",
                   mgr.getNumScopeEnter());
            printf("Scopes exited : %llu\n",
                   mgr.getNumScopeExit());
            printf("Overhead per scope [ticks] : %.3f\n",
                   mgr.getScopeOverhead());
            printf("Threads entered : %llu\n",
                   mgr.getNumThreadEnter());
            printf("Threads exited : %llu\n",
                   mgr.getNumThreadExit());
            printf("============================\n");

            ForeachPrintThread threadPrinter;
            root->foreachnn(threadPrinter);

            printf("============================\n");
        }
    private:
        class ForeachPrintThread
        {
        public:
            void operator()(prof::ThreadNode *node)
            {
                printf("%s:\n", node->name());

                prof::ForeachPrint printer;

#ifdef PROFILE_LOCK
                node->data().lock();
                {
#endif
                    printer(node->data().getRoot());
#ifdef PROFILE_LOCK
                }
                node->data().unlock();
#endif
            }
        private:
        protected:
        };
    protected:
    };
}

#endif //TESTING_PRINTCRAWLER_H
