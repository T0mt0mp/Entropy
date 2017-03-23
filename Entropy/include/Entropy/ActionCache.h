/**
 * @file Entropy/ActionCache.h
 * @author Tomas Polasek
 * @brief Cache for holding information about actions performed on the ECS.
 */

#ifndef ECS_FIT_ACTIONCACHE_H
#define ECS_FIT_ACTIONCACHE_H

#include "Types.h"
#include "Util.h"
#include "System.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Cache is used for holding information about actions performed
     * on the ECS.
     * TODO - refactor into Universe.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ActionCache : NonCopyable
    {
    public:
        /**
         * Construct the cache, passing containing Universe.
          * @param entityMgr Entity manager from the same Universe.
          * @param compMgr Component manager from the same Universe.
          * @param groupMgr Gropu manager from the same Universe.
          * @param systemMgr System manager from the same Universe.
          */
        ActionCache(EntityManager<UniverseT> &entityMgr, ComponentManager<UniverseT> &compMgr,
                    GroupManager<UniverseT> &groupMgr, SystemManager<UniverseT> &systemMgr);

        /**
         * Destruct and reset the Action cache.
         */
        ~ActionCache();

        /**
         * Refresh cache and perform actions.
         */
        void refresh();

        /**
         * Reset the actions.
         */
        void reset();

        /**
         * Add Component to the given Entity.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         */
        template <typename ComponentT>
        inline ComponentT *addComponent(EntityId id);

        /**
         * Remove Component from given Entity.
         * If there is no Component associated with the Entity, nothing happens.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         */
        template <typename ComponentT>
        inline void removeComponent(EntityId id);
    private:
        /// List of changed Entities since the last refresh.
        SortedList<EntityId> mChanged;
    protected:
    }; // class ActionCache

    // ActionCache implementation.
    template <typename UT>
    ActionCache<UT>::ActionCache(EntityManager <UT> &entityMgr, ComponentManager <UT> &compMgr,
                                 GroupManager <UT> &groupMgr, SystemManager <UT> &systemMgr) :
        mEM(entityMgr), mCM(compMgr), mGM(groupMgr), mSM(systemMgr)
    { }

    template <typename UT>
    ActionCache<UT>::~ActionCache()
    { }

    template <typename UT>
    void ActionCache<UT>::refresh()
    {
        for (const EntityId &id : mChanged)
        {
            mGM.checkEntity(id);
        }

        mGM.finalizeGroups();
        mChanged.clear();
    }

    template <typename UT>
    void ActionCache<UT>::reset()
    {
        mChanged.reclaim();
    }

    template <typename UT>
    template <typename ComponentT>
    inline ComponentT *ActionCache<UT>::addComponent(EntityId id)
    {
        // Is the component registered?
        ENT_ASSERT_SLOW(mCM.template registered<ComponentT>());

        ComponentT *res{mCM.template add<ComponentT>(id)};

        mChanged.insertUnique(id);

        return res;
    }

    template <typename UT>
    template <typename ComponentT>
    inline void ActionCache<UT>::removeComponent(EntityId id)
    {
        // Is the component registered?
        ENT_ASSERT_SLOW(mCM.template registered<ComponentT>());

        mChanged.insertUnique(id);

        mCM.template remove<ComponentT>(id);
    }
    // ActionCache implementation end.
} // namespace ent

#endif //ECS_FIT_ACTIONCACHE_H
