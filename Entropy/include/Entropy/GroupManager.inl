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

        for (auto &h : mDestructOnReset)
        {
            h();
        }
        mDestructOnReset.clear();
    }

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    EntityGroup *GroupManager<UT>::addGroup(const ComponentManager<UT> &cm,
                                            EntityManager &em)
    {
        if (!hasGroup<RequireT, RejectT>())
        {
            initGroup<RequireT, RejectT>(buildFilter<RequireT, RejectT>(cm), em);
        }

        EntityGroup *result{getGroup<RequireT, RejectT>()};
        result->incUsage();
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

#ifdef ENT_NOT_USED
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
#endif

    template <typename UT>
    template <template<typename...> typename ContainerT,
                                    typename FirstT,
                                    typename... RestTs>
    struct GroupManager<UT>::FilterBuilder<ContainerT<FirstT, RestTs...>>
    {
        static void process(const ComponentManager<UT> &cm, EntityFilter &f, bool required)
        {
            FilterBuilder<ContainerT<RestTs...>> next;
            if (cm.template registered<FirstT>())
            {
                CIdType typeId{cm.template id<FirstT>()};
                f.addComponent(typeId, required);
            }
            next.process(cm, f, required);
        }
    };

    template <typename UT>
    template <template<typename...> typename ContainerT,
                                    typename LastT>
    struct GroupManager<UT>::FilterBuilder<ContainerT<LastT>>
    {
        static void process(const ComponentManager<UT> &cm, EntityFilter &f, bool required)
        {
            if (cm.template registered<LastT>())
            {
                CIdType typeId{cm.template id<LastT>()};
                f.addComponent(typeId, required);
            }
        }
    };

    template <typename UT>
    template<template<typename...> typename ContainerT>
    struct GroupManager<UT>::FilterBuilder<ContainerT<>>
    {
        static void process(const ComponentManager<UT> &cm, EntityFilter &f, bool required)
        { }
    };

    template <typename UT>
    template <typename RequireT,
        typename RejectT>
    EntityFilter GroupManager<UT>::buildFilter(const ComponentManager<UT> &cm) const
    {
        EntityFilter result;
        FilterBuilder<RequireT>{}.process(cm, result, true);
        FilterBuilder<RejectT>{}.process(cm, result, false);
        // TODO - make this flag changeable for user.
        result.setRequiredActivity(true);
        return result;
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

        /*
         * Look for Groups which are not in use and remove then.
         */
        for (u64 index = 0; index <= lastActive; )
        {
            EntityGroup *grp{mActiveGroups[index]};
            if (!grp->inUse())
            { // Group needs to be removed.
                em.removeGroup(grp->id());
                // Swap-remove and decrement the number of active Groups.
                std::swap(mActiveGroups[index], mActiveGroups[lastActive--]);
                mActiveGroups.pop_back();
            }
            else
            {
                index++;
            }
        }
    }

    template <typename UT>
    void GroupManager<UT>::checkEntities(const ent::SortedList<EntityId> &changed,
                              EntityManager &em)
    {
        // TODO - Parallelize.

        // Every Group has to check for changes in Entities.
        for (EntityGroup *grp : mActiveGroups)
        {
            const EntityFilter &filter(grp->filter());
            const u64 groupId{grp->id()};
            for (EntityId id : changed)
            {
                // Tests, if the Entity has been destroyed
                bool exists{em.valid(id)};
                // Tests, if the Entity is currently in the Group.
                bool inGroup{em.inGroup(id, groupId)};

                if (!exists && inGroup)
                { // Entity is within the Group, but has been destroyed.
                    grp->remove(id);
                    em.resetGroup(id, groupId);
                }
                else if (exists)
                {
                    FilterBitset entInfo{em.compressInfo(filter, id.index())};
                    bool passed{filter.match(entInfo)};

                    if (passed && !inGroup)
                    { // Not in Group, but should be.
                        grp->add(id);
                        em.setGroup(id, groupId);
                    }
                    else if (!passed && inGroup)
                    { // In Group, but shouldn't be.
                        grp->remove(id);
                        em.resetGroup(id, groupId);
                    }
                }
            }
        }

        // Some of the new Groups may not be in used any more...
        removeInactive(mNewGroups);

        // TODO - maybe use block-wise parallelism?
        /*
         * New groups need to go through all active Entities and
         * test, if they belong into the new Group.
         */
        for (EntityGroup *grp : mNewGroups)
        {
            const EntityFilter &filter(grp->filter());
            const u64 groupId{grp->id()};
            ValidEntityIterator it{em.validEntities()};

            while (it.valid())
            {
                FilterBitset entInfo{em.compressInfo(filter, it.index())};
                bool passed{filter.match(entInfo)};

                if (passed)
                {
                    EntityId id(it.index(), em.currentGen(it.index()));

                    grp->add(id);
                    em.setGroup(id, groupId);
                }

                it.increment();
            }
        }

        // Move the new Groups to the active list.
        mActiveGroups.insert(mActiveGroups.end(), mNewGroups.begin(), mNewGroups.end());
        mNewGroups.clear();
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
    void GroupManager<UT>::initGroup(const EntityFilter &f, EntityManager &em)
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
        u64 grpId{em.addGroup()};

        // Create the EntityGroup.
        group<RequireT, RejectT>().construct(f, grpId);

        // Activate the EntityGroup.
        mNewGroups.emplace_back(group<RequireT, RejectT>().ptr());

        // Register reset handle.
        mDestructOnReset.emplace_back(group<RequireT, RejectT>().destructLater());
    }

    template <typename UT>
    bool GroupManager<UT>::checkGrpRedundancy(const EntityFilter &filter)
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
