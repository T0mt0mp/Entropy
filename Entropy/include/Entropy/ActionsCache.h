/**
 * @file Entropy/ActionsCache.h
 * @author Tomas Polasek
 * @brief Used for managing Actions in threaded environment.
 */

#ifndef ECS_FIT_ACTIONSCACHE_H
#define ECS_FIT_ACTIONSCACHE_H

#include <mutex>

#include "ChangeSet.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Used as manager of change actions in threaded environment.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ActionsCache : NonCopyable
    {
    public:
        /**
         * Get ChangeSet specific to current thread.
         * ChangeSets allow to store actions to be
         * performed at a later time.
         * @return Returns reference to the ChangeSet
         *   currently in use.
         */
        ChangeSet &changeSet();

        /**
         * Commit actions of the active thread.
         */
        void commitChangeSet();

        /**
         * Reset actions of the active thread.
         */
        void resetChangeSet();

        /**
         * Reset the ActionsCache.
         */
        void reset();
    private:
        /**
         * Actions storage specific to each thread
         * and Universe type.
         */
        static thread_local ActionsContainer tActions;

        /// Mutex used for locking access to mCommittedChanges.
        std::mutex mCommitMutex;
        /// List of ChangeSets which were committed for the next refresh.
        std::vector<std::unique_ptr<ChangeSet>> mCommittedChanges;
    protected:
    }; // class ActionsCache
} // namespace ent

#include "ActionsCache.inl"

#endif //ECS_FIT_ACTIONSCACHE_H
