/**
 * @file Entropy/GroupManager.inl
 * @author Tomas Polasek
 * @brief Manager class allowing the use of EntityGroups.
 */

#include "GroupManager.h"

/// Main Entropy namespace
namespace ent
{
    template <typename UT>
    GroupManager<UT>::GroupManager()
    { reset(); }

    template <typename UT>
    GroupManager<UT>::~GroupManager()
    { reset(); }

    template <typename UT>
    void GroupManager<UT>::refresh(const ent::SortedList<EntityId> &changed, EntityManager &em)
    {
        refreshGroups();
        checkGroups(em);
        checkEntities(changed, em);
        finalizeGroups();
    }

    template <typename UT>
    void GroupManager<UT>::reset()
    {
        mActiveGroups.clear();
        mFreeIds.clear();
        mLastUsedId = 0u;

        for (auto &h : mDestructOnReset)
        {
            h->destruct();
        }
        mDestructOnReset.clear();
    }

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    EntityGroup *GroupManager<UT>::addGroup(const ComponentFilter &f)
    {
        if (!hasGroup<RequireT, RejectT>())
        {
            initGroup<RequireT, RejectT>(f);
        }

        EntityGroup *result{getGroup<RequireT, RejectT>()};
        result->incUsage();
        // Check for correctly set filter on the Group.
        ENT_ASSERT_FAST(result->filter() == f);
        return getGroup<RequireT, RejectT>();
    }

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    bool GroupManager<UT>::hasGroup()
    {
        return group<RequireT, RejectT>().constructed();
    }

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    EntityGroup *GroupManager<UT>::getGroup()
    {
        return group<RequireT, RejectT>().ptr();
    }

    template <typename UT>
    template<typename RequireT,
        typename RejectT>
    bool GroupManager<UT>::abandonGroup()
    {
        return getGroup<RequireT, RejectT>()->abandon() == 0u;
    }

    template <typename UT>
    template <template<typename...> typename ContainerT,
        typename FirstT,
        typename... RestTs>
    struct GroupManager<UT>::FilterBuilder<ContainerT<FirstT, RestTs...>>
    {
        static auto value(const ComponentManager<UT> &compMgr)
        {
            FilterBuilder<ContainerT<RestTs...>> next;
            ComponentBitset thisValue{
                compMgr.template registered<FirstT>() ?
                compMgr.template mask<FirstT>() :
                0
            };
            return thisValue | next.value(compMgr);
        }
    };

    template <typename UT>
    template <template<typename...> typename ContainerT,
        typename LastT>
    struct GroupManager<UT>::FilterBuilder<ContainerT<LastT>>
    {
        static auto value(const ComponentManager<UT> &compMgr)
        {
            ComponentBitset thisValue{
                compMgr.template registered<LastT>() ?
                compMgr.template mask<LastT>() :
                0
            };
            return thisValue;
        }
    };

    template <typename UT>
    template<template<typename...> typename ContainerT>
    struct GroupManager<UT>::FilterBuilder<ContainerT<>>
    {
        static auto value(const ComponentManager<UT> &compMgr)
        {
            return ComponentBitset();
        }
    };

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    ComponentFilter GroupManager<UT>::buildFilter(const ComponentManager<UT> &cm) const
    {
        FilterBuilder<RequireT> fbRequire;
        FilterBuilder<RejectT> fbReject;
        return ComponentFilter(fbRequire.value(cm), fbReject.value(cm));
    }

    template <typename UT>
    void GroupManager<UT>::refreshGroups()
    {
        for (EntityGroup *grp : mActiveGroups)
        {
            grp->refresh();
        }
    }

    template <typename UT>
    void GroupManager<UT>::checkGroups(EntityManager &em)
    {
        // TODO - Unit test.
        if (mActiveGroups.empty())
        { // No Groups to check.
            return;
        }

        u64 lastActive{mActiveGroups.size() - 1u};

        // AND reset mask.
        GroupBitset resetMask;
        resetMask.set();

        /*
         * Look for Groups which are not in use and remove then.
         * At the same time build AND reset mask, for resetting
         * Entity metadata.
         */
        for (u64 index = 0; index <= lastActive; )
        {
            EntityGroup *grp{mActiveGroups[index]};
            if (!grp->inUse())
            { // Group needs to be removed.
                resetMask.reset(grp->id());
                // Swap-remove and decrement the number of active Groups.
                std::swap(mActiveGroups[index], mActiveGroups[lastActive--]);
                mActiveGroups.pop_back();
            }
            else
            {
                index++;
            }
        }

        // Give Entity Manager the reset mask to reset Group metadata.
        em.resetGroups(resetMask);
    }

    template <typename UT>
    void GroupManager<UT>::checkEntities(const ent::SortedList<EntityId> &changed,
                              EntityManager &em)
    {
        if (!mActiveGroups.empty())
        {
            for (const EntityId &id : changed)
            {
                // TODO - Rewrite, parallelize.
                if (!em.valid(id))
                { // Destroyed Entity.
                    const GroupBitset &groups(em.groups(id));
                    for (EntityGroup *grp : mActiveGroups)
                    {
                        if (groups.test(grp->id()))
                        { // Entity is leaving the Group.
                            em.resetGroup(id, grp->id());
                            grp->remove(id);
                        }
                    }
                }
                else
                { // Entity with changed Components/activity.
                    const ComponentBitset &components(em.components(id));
                    const GroupBitset &groups(em.groups(id));

                    for (EntityGroup *grp : mActiveGroups)
                    {
                        // Test, if the Entity matches Group filter.
                        bool entityMatch{grp->filter().match(components)};
                        // Test, if the Entity is withing this Group already.
                        bool groupTest{groups.test(grp->id())};
                        if (!groupTest && entityMatch)
                        { // Entity is entering the Group.
                            em.setGroup(id, grp->id());
                            grp->add(id);
                        }
                        else if (groupTest && !entityMatch)
                        { // Entity is leaving the Group.
                            em.resetGroup(id, grp->id());
                            grp->remove(id);
                        }
                        else
                        { /* Nothing needs to be done. */ }
                    }
                }
            }
        }

        // Some of the new Groups may not be in used any more...
        removeInactive(mNewGroups);

        /*
         * New groups need to go through all active Entities and
         * test, if they belong into the new Group.
         */
        if (!mNewGroups.empty())
        {
            for (ActiveEntityIterator it = em.activeEntities();
                 it.active(); ++it)
            { // Over all active Entities.
                for (EntityGroup *grp : mNewGroups)
                { // Over all new Groups.
                    bool entityMatch{grp->filter().match(it.record().comp())};

                    // Check, if the last Group has been cleaned.
                    ENT_ASSERT_SLOW(!it.record().testGrp(grp->id()));

                    if (entityMatch)
                    { // Entity is entering the Group.
                        EntityId id{it.id()};
                        em.setGroup(id, grp->id());
                        grp->add(id);
                    }
                }
            }

            // Move the new Groups to the active list.
            mActiveGroups.insert(mActiveGroups.end(), mNewGroups.begin(), mNewGroups.end());
            mNewGroups.clear();
        }
    }

    template <typename UT>
    void GroupManager<UT>::finalizeGroups()
    {
        for (EntityGroup *grp : mActiveGroups)
        {
            grp->finalize();
        }
    }

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    void GroupManager<UT>::initGroup(const ComponentFilter &f)
    {
        if (group<RequireT, RejectT>().constructed())
        { // Prevent double initialized Entity Groups.
            return;
        }

#ifdef ENT_DEBUG
        if (checkGrpRedundancy(f))
        {
            ENT_WARNING("Multiple EntityGroups with the same filter, are you sure you want to do this?"
                            "(This can happen, when you specify the same Component types in require and "
                            "reject, but in different order)");
        }
#endif
        u64 grpId{nextGroupId()};
        if (grpId >= ENT_MAX_GROUPS)
        { // No more Groups can be created.
            ENT_WARNING("No more Entity Groups can be created, increase the "
                            "MAX_GROUPS number in \"Types.h\".");
            mLastUsedId--;
            return;
        }

        // Create the EntityGroup.
        group<RequireT, RejectT>().construct(f, grpId);

        // Activate the EntityGroup.
        mNewGroups.emplace_back(group<RequireT, RejectT>().ptr());

        // Register reset handle.
        mDestructOnReset.emplace_back(&group<RequireT, RejectT>());
    }

    template <typename UT>
    bool GroupManager<UT>::checkGrpRedundancy(const ComponentFilter &filter)
    {
        for (const EntityGroup *grp : mActiveGroups)
        {
            if (grp->filter() == filter)
            {
                return true;
            }
        }

        return false;
    }

    template <typename UT>
    u64 GroupManager<UT>::nextGroupId()
    {
        u64 result;

        if (mFreeIds.empty())
        {
            result = mLastUsedId++;
        }
        else
        {
            result = mFreeIds.back();
            mFreeIds.pop_back();
        }

        return result;
    }

    template <typename UT>
    void GroupManager<UT>::removeInactive(std::vector<EntityGroup*> &groups)
    {
        if (!groups.empty())
        {
            groups.erase(std::remove_if(groups.begin(), groups.end(),
                                        [] (EntityGroup *grp) {
                                            return !grp->inUse();
                                        }), groups.end());
        }
    }
} // namespace ent
