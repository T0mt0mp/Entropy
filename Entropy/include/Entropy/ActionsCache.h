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

        /**
         * Called when new Component is registered.
         * @tparam ComponentT Type of the Component.
         * @param cId Component ID.
         */
        template <typename ComponentT>
        void registerComponent(u64 cId);

        /**
         * Apply committed ChangeSets from ActionsCache.
         * @param uni Universe instance.
         */
        void applyChangeSets(UniverseT *uni);
    private:
        class ComponentExtractor
        {
        public:
            virtual void addRemoveComponents(ComponentActions *ca,
                                             const ent::List<EntityId> &tempMapping,
                                             UniverseT *uni) = 0;
        private:
        protected:
        }; // class ComponentExtractor

        template <typename ComponentT>
        class ComponentExtractorSpec : public ComponentExtractor
        {
        public:
            virtual void addRemoveComponents(ComponentActions *ca,
                                             const ent::List<EntityId> &tempMapping,
                                             UniverseT *uni) override final;
        private:
        protected:
        }; // class ComponentExtractorSpec

        template <typename ComponentT>
        ComponentExtractorSpec<ComponentT> &extractorGetter()
        {
            static ComponentExtractorSpec<ComponentT> extr;
            return extr;
        }

        /**
         * Actions storage specific to each thread
         * and Universe type.
         */
        static thread_local ActionsContainer tActions;

        /// Mutex used for locking access to mCommittedChanges.
        std::mutex mCommitMutex;
        /// List of ChangeSets which were committed for the next refresh.
        std::vector<std::unique_ptr<ChangeSet>> mCommittedChanges;
        /// List of registered Component extractors.
        std::vector<ComponentExtractor*> mRegisteredExtractors;
    protected:
    }; // class ActionsCache

    /// List of changed Entity IDs for each thread.
    template <typename UniverseT>
    class ChangedEntitiesHolder : NonCopyable
    {
    public:
        /// Create the list and register it.
        ChangedEntitiesHolder();

        /// Mark the list pointer for removal.
        ~ChangedEntitiesHolder();

        /// Remove all lists marked for removal.
        inline void refresh();

        /// Reset the list of change lists.
        inline void reset();

        /// Add the entity ID to the thread list.
        inline void entityChanged(EntityId id);

        /// Create the resulting list and return it.
        inline SortedList<EntityId> &createResultList();
    private:
        /// Register given list.
        inline void registerList(SortedList<EntityId> *list);

        /// Unregister given list.
        inline void unregisterList(SortedList<EntityId> *list);

        /// List of changed Entities since the last refresh.
        static thread_local SortedList<EntityId> *mChangedEntities;

        /// Mutex for the list of change lists.
        static std::mutex sChangesMutex;
        /// Holding the list of all active change lists.
        static std::vector<std::pair<std::unique_ptr<SortedList<EntityId>>, bool>> sChanges;
        /// Result of merging of the individual change lists.
        static SortedList<EntityId> sResultList;
    protected:
    }; // class ChangeEntitiesHolder
} // namespace ent

#include "ActionsCache.inl"

#endif //ECS_FIT_ACTIONSCACHE_H
