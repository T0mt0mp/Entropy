/**
 * @file Entropy/Group.h
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#ifndef ECS_FIT_GROUP_H
#define ECS_FIT_GROUP_H

#include "Types.h"
#include "Memory.h"
#include "Util.h"
#include "Component.h"
#include "Entity.h"

/// Main Entropy namespace
namespace ent {
    /**
     * Helper structure for specifying required Component types
     * for a System.
     * @tparam RequireComponentTs List of required Component types.
     */
    template<typename... RequireComponentTs>
    struct Require {
    };

    /**
     * Helper structure for specifying rejected Component types
     * for a System.
     * @tparam RejectComponentTs List of rejected Component types.
     */
    template<typename... RejectComponentTs>
    struct Reject {
    };

    /**
     * Iterator used for iterating over Entities within Entity group.
     * @tparam UniverseT Type of the Universe.
     * @tparam IterT Type of the Iterator.
     */
    template <typename UniverseT,
              typename IterT>
    class EntityGroupIterator final
    {
    public:
        using ValueT = typename UniverseT::EntityT;
        using RefT = ValueT&;
        using PtrT = ValueT*;

        /**
         * Construct Entity iterator.
         * @param uni Universe, where the Entities exist.
         * @param it Iterator.
         */
        EntityGroupIterator(UniverseT *uni, IterT it) :
            mIterator{it}, mEntity{uni}, mUniverse{uni} { }

        /// Prefix increment.
        EntityGroupIterator &operator++()
        {
            ++mIterator;
            return *this;
        }

        /// Postfix increment.
        EntityGroupIterator operator++(int)
        {
            EntityGroupIterator tmp{*this};
            ++(*this);
            return tmp;
        }

        /// Equality comparison operator.
        bool operator==(const EntityGroupIterator &rhs) const
        { return mIterator == rhs.mIterator; }

        /// Inequality comparison operator.
        bool operator!=(const EntityGroupIterator &rhs) const
        { return !(*this == rhs); }

        /// Access operator.
        RefT operator*()
        { updateEntity(); return mEntity; }

        /// Access operator.
        PtrT operator->()
        { updateEntity(); return &mEntity; }

        EntityGroupIterator operator+(u64 val)
        { return EntityGroupIterator(mUniverse, mIterator + val); }

        /**
         * Subscript operator proxy.
         * decltype(std::declval<IterT>()[std::declval<IndexT>()])
         */
        template <typename IndexT>
        RefT operator[](IndexT idx)
        { updateEntity(mIterator[idx]); return mEntity; }
    private:
        /**
         * Update the value of the innter Entity.
         */
        void updateEntity()
        { mEntity.setId(*mIterator); }

        /**
         * Update Entity to given ID.
         * @param id ID of the Entity.
         */
        void updateEntity(EntityId id)
        { mEntity.setId(id); }

        /// Inner iterator.
        IterT mIterator;
        /// Entity object, which will be returned.
        ValueT mEntity;
        /// Universe pointer;
        UniverseT *mUniverse;
    protected:
    }; // class EntityGroupIterator

    /**
     * Helper object, used in foreach loops.
     * @tparam UniverseT Type of the Universe.
     * @tparam IteratedT Over which type does it iterate?
     */
    template <typename UniverseT,
              typename IteratedT>
    class EntityList
    {
    public:
        //using IteratorT = EntityGroupIterator<UniverseT, decltype(std::begin(std::declval<IteratedT>()))>;
        //using IteratorT = EntityGroupIterator<UniverseT, decltype(std::declval<IteratedT>().begin())>;
        using IteratorT = EntityGroupIterator<UniverseT, typename IteratedT::iterator>;
        using ConstIteratorT = EntityGroupIterator<UniverseT, typename IteratedT::const_iterator>;

        /**
         * Create iterable object, iterating over given object.
         * @param it Iterate over this object.
         */
        EntityList(UniverseT *uni, IteratedT &it) :
            mUniverse{uni}, mIt{it}
        { }

        typename std::enable_if<true, decltype(std::declval<IteratedT>().size())>::type
        size() const
        { return mIt.size(); }

        /// Get begin iterator.
        IteratorT begin()
        { return IteratorT(mUniverse, mIt.begin()); }
        /// Get begin iterator.
        ConstIteratorT begin() const
        { return IteratorT(mUniverse, mIt.begin()); }

        /// Get end iterator.
        IteratorT end()
        { return IteratorT(mUniverse, mIt.end()); }
        /// Get end iterator.
        ConstIteratorT end() const
        { return IteratorT(mUniverse, mIt.end()); }

        template <typename UT, typename IT>
        friend IteratorT begin(EntityList<UT, IT> &entityList);
        template <typename UT, typename IT>
        friend IteratorT end(EntityList<UT, IT> &entityList);
    private:
        /// Universe object.
        UniverseT *mUniverse;
        /// Object to iterate over.
        IteratedT &mIt;
    protected:
    };


    /**
     * EntityGroup class contains a list of Entities, which pass
     * a group specific filter. They are used for caching the Entities for
     * use in Systems.
     */
    class EntityGroup final : NonCopyable
    {
    public:
        using EntityListT = SortedList<EntityId>;
        using AddedListT = List<EntityId>;
        using RemovedListT = List<EntityId>;

        /**
         * Construct a EntityGroup, specifying which
         * Entities belong into it, using the filter.
         * @param filter Specifies which Entities belong
         *   into this Group.
         * @param groupId ID of the group, starting at 0.
         */
        EntityGroup(const ComponentFilter &filter, u64 groupId);

        /**
         * Reset this group to default state.
         */
        inline void reset();

        /// Filter getter.
        const ComponentFilter &filter() const
        { return mFilter; }

        /// Group ID getter.
        u64 id() const
        { return mId; }

        /**
         * Add given Entity ID to this group.
         * @param id ID of the Entity.
         */
        inline void add(EntityId id);

        /**
         * Remove given Entity ID from this group.
         * @param id ID of the Entity.
         */
        inline void remove(EntityId id);

        /**
         * Refresh this Group - clear added/removed lists.
         */
        inline void refresh();

        /**
         * Finalize adding and removing of Entities.
         */
        inline void finalize();

        /**
         * Get foreach iterator object, iterating over Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, EntityListT> foreach(UT *uni)
        { return EntityList<UT, EntityListT>(uni, *mEntities); }

        /**
         * Get foreach iterator object, iterating over added Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, AddedListT> foreachAdded(UT *uni)
        { return EntityList<UT, AddedListT>(uni, mAdded); }

        /**
         * Get foreach iterator object, iterating over removed Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, RemovedListT> foreachRemoved(UT *uni)
        { return EntityList<UT, RemovedListT>(uni, mRemoved); }
    private:
        /// Get the front Entity buffer.
        EntityListT *entitiesFront()
        { return mEntities; }
        /// Get the back Entity buffer.
        EntityListT *entitiesBack()
        { return mEntitiesBack; }
        /// Swap the Entity buffers = front <-> back.
        void swapEntityBuffers()
        { std::swap(mEntities, mEntitiesBack); }

        /// Filter specifying which Entities belong into this group.
        ComponentFilter mFilter;
        /// ID of this Group.
        u64 mId;
        /// List of Entities, corresponding with the filter.
        EntityListT *mEntities;
        /// List of Entities, corresponding with the filter.
        EntityListT *mEntitiesBack;
        /// Front and back buffers.
        EntityListT mEntityBuffers[2];
        /// List of Entities, which were added since the last refresh.
        AddedListT mAdded;
        /// List of Entities, which were removed since the last refresh.
        RemovedListT mRemoved;
    protected:
    }; // EntityGroup

    /**
     * Extract Require and Reject type lists from given System type.
     * Default value (if either list is not present) is empty type list.
     * @tparam SystemT Type of the System.
     */
    template<typename SystemT>
    struct RequireRejectExtractor {
        struct RequireGetter {
            template<typename ST>
            static typename ST::Require getRequire(void *unused);

            template<typename ST>
            static Require<> getRequire(...);

            using RequireT = decltype(getRequire<SystemT>(nullptr));
        };

        struct RejectGetter {
            template<typename ST>
            static typename ST::Reject getReject(void *unused);

            template<typename ST>
            static Reject<> getReject(...);

            using RejectT = decltype(getReject<SystemT>(nullptr));
        };

        using RequireT = typename RequireGetter::RequireT;
        using RejectT = typename RejectGetter::RejectT;
    };

    /**
     * GroupManager base class containing code which does not need to be templated.
     */
    class GroupManagerBase : NonCopyable {
    public:
    private:
    protected:
    }; // GroupManagerBase

    /**
     * GroupManager is a part of Entropy ECS Universe.
     * GroupManager keeps a catalogue of EntityGroups for
     * Systems to use.
     * It has method which take care about creating/removing
     * EntityGroups, and their refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template<typename UniverseT>
    class GroupManager final : public GroupManagerBase {
    public:
        /**
         * Basic constructor.
         * @param entityMgr Entity manager from the same Universe.
         * @param compMgr Component manager from the same Universe.
         */
        GroupManager(EntityManager<UniverseT> &entityMgr, ComponentManager<UniverseT> &compMgr);

        /**
         * Resets the Group manager.
         */
        ~GroupManager();

        /**
         * TODO - refresh GroupManager.
         */
        void refresh();

        /**
         * Reset all the groups.
         */
        void reset();

        /**
         * Add or get already created Entity group.
         * The pointer is guaranteed to be valid as long as the
         * GroupManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns ptr to the requested Entity Group.
         */
        template<typename RequireT,
            typename RejectT>
        EntityGroup *addGetGroup();

        /**
         * Build a Component filter using the required and rejected
         * type lists.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns Component filter created from the
         *   required and rejected type lists.
         */
        template<typename RequireT,
            typename RejectT>
        ComponentFilter buildFilter() const;

        /**
         * Finalize refreshing the groups after adding/removing Entities.
         */
        void finalizeGroups();

        /**
         * Check given Entity and add/remove it from
         * @param id ID of the Entity.
         */
        void checkEntity(EntityId id);
    private:
        /// Group ID generator.
        class GroupIdGenerator : public StaticClassIdGenerator<GroupIdGenerator> {
        };

        /**
         * Build a filter bitset using template magic.
         * Takes a type list of Component types - ContainerT
         * can be any type. Each of the Component types is
         * used as template parameter to get the componentMask.
         * Final result is all masks ORed.
         * @tparam ContainerT Container containing type list
         *   of Component types.
         */
        template<typename ContainerT>
        struct FilterBuilder;

        template<template<typename...> typename ContainerT,
            typename FirstT,
            typename... RestTs>
        struct FilterBuilder<ContainerT<FirstT, RestTs...>> {
            static auto value(ComponentManager<UniverseT> &compMgr)
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

        template<template<typename...> typename ContainerT,
            typename LastT>
        struct FilterBuilder<ContainerT<LastT>> {
            static auto value(ComponentManager<UniverseT> &compMgr)
            {
                ComponentBitset thisValue{
                    compMgr.template registered<LastT>() ?
                    compMgr.template mask<LastT>() :
                    0
                };
                return thisValue;
            }
        };

        template<template<typename...> typename ContainerT>
        struct FilterBuilder<ContainerT<>> {
            static auto value(ComponentManager<UniverseT> &compMgr)
            {
                return ComponentBitset();
            }
        };

        /**
         * Initialize group.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template<typename RequireT,
            typename RejectT>
        void initGroup();

        /**
         * Check, if there is no EntityGroup already using this
         * filter.
         * This method is used for checking EntityGroup
         * redundancy, when the Component type list is in
         * different order.
         * @param filter Filter to search for.
         * @return Returns true, if there already is a EntityGroup
         *   with same filter.
         */
        bool checkRedundancy(const ComponentFilter &filter);

        /// Mapping of Entity groups to IDs.
        std::vector<EntityGroup *> mGroupId;

        /// Vector of destruction functors.
        std::vector<std::function<void()>> mGroupResets;

        /**
         * Static instance of EntityGroup.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
		/*
        template<typename RequireT,
            typename RejectT>
        static ConstructionHandler<EntityGroup> mGroup;
		*/
        template<typename RequireT,
            typename RejectT>
		static ConstructionHandler<EntityGroup> &groupGetter()
		{
			static ConstructionHandler<EntityGroup> group;
			return group;
		}

        /// Entity manager from the same Universe.
        EntityManager<UniverseT> &mEM;
        /// Component manager from the same Universe.
        ComponentManager<UniverseT> &mCM;
    protected:
    }; // GroupManager

    // EntityGroup implementation.
    void EntityGroup::reset()
    {
        mEntityBuffers[0].reclaim();
        mEntityBuffers[1].reclaim();
        mAdded.reclaim();
        mRemoved.reclaim();
    }

    void EntityGroup::add(EntityId id)
    {
        //mEntities.insertUnique(id);
        mAdded.pushBack(id);
    }

    void EntityGroup::remove(EntityId id)
    {
        //mEntities.erase(id);
        mRemoved.pushBack(id);
    }

    void EntityGroup::refresh()
    {
        mAdded.clear();
        mAdded.shrinkToFit();
        mRemoved.clear();
        mAdded.shrinkToFit();
    }

    void EntityGroup::finalize()
    {
        std::sort(mAdded.begin(), mAdded.end());
        std::sort(mRemoved.begin(), mRemoved.end());

        // Assure the size for the back buffer.
        entitiesBack()->resize(entitiesFront()->size() + mAdded.size() + mRemoved.size());
        // Sort the new list into the back buffer.
        auto iit1 = mAdded.cbegin();
        auto ieit1 = mAdded.cend();
        auto iit2 = mRemoved.cbegin();
        auto ieit2 = mRemoved.cend();
        auto iit3 = entitiesFront()->cbegin();
        auto ieit3 = entitiesFront()->cend();

        auto oit = entitiesBack()->begin();
        auto oeit = entitiesBack()->end();

        for (;
             oit != oeit && (iit1 != ieit1 || iit2 != ieit2 || iit3 != ieit3);
             ++oit)
        {
            /*
             *                   A < B
             *           T                   F
             *         A < C               B < C
             *    T            F    T               F
             *    |            |    |               |
             *    A            C    B               C
             *                 +    +               +
             *            ?C == A ?B == A         ?C == B
             *                                    ?B == A
             */

            /*
            // A < B
            if ((iit1 != ieit1) && ((iit2 != ieit2) || (*iit1 < *iit2)))
            { // A < B
                // A < C
                if ((iit3 != ieit3) || (*iit1 < *iit3))
                { // A < C
                    // -> A
                    *oit = *iit1;
                    ++iit1;
                }
                else
                { // C <= A
                    // -> C + ?C == A?
                    *oit = *iit3;
                    ++iit3;
                }
            }
            else
            { // B >= A
                // B < C
                if ((iit2 != ieit2) && ((iit3 != ieit3) || (*iit2 < *iit3)))
                { // B < C
                    // -> B + ?B == A?
                    *oit = *iit2;
                    ++iit2;
                }
                else
                { // C >= B >= A
                    // -> C + ?C == B? + ?B == A?
                    *oit = *iit3;
                    ++iit3;
                }
            }
             */

            *oit = (iit1 != ieit1) && ((iit2 == ieit2) || (*iit1 < *iit2)) ?
                   (iit3 == ieit3) || (*iit1 < *iit3) ?
                   (*iit1) : (*iit3)
                                                                           :
                   (iit2 != ieit2) && ((iit3 == ieit3) || (*iit2 < *iit3)) ?
                   (*iit2) : (*iit3);

            while ((iit1 != ieit1) && (*iit1 == *oit))
            {
                ++iit1;
            }
            while ((iit2 != ieit2) && (*iit2 == *oit))
            {
                ++iit2;
            }
            while ((iit3 != ieit3) && (*iit3 == *oit))
            {
                ++iit3;
            }
        }

        ENT_ASSERT_SLOW(iit1 == ieit1);
        ENT_ASSERT_SLOW(iit2 == ieit2);
        ENT_ASSERT_SLOW(iit3 == ieit3);

        u64 finalSize{static_cast<u64>(oit - entitiesBack()->begin())};
        entitiesBack()->resize(finalSize);

        swapEntityBuffers();
    }
    // EntityGroup implementation end.

    // GroupManager implementation.
	/*
    template<typename UT>
    template<typename RequireT,
        typename RejectT>
    ConstructionHandler<EntityGroup> GroupManager<UT>::mGroup;
	*/

    template<typename UT>
    GroupManager<UT>::GroupManager(EntityManager<UT> &entityMgr, ComponentManager<UT> &compMgr) :
        mEM(entityMgr), mCM(compMgr)
    { }

    template<typename UT>
    GroupManager<UT>::~GroupManager()
    {
        reset();
    }

    template <typename UT>
    void GroupManager<UT>::refresh()
    {
        for (auto *grp : mGroupId)
        {
            grp->refresh();
        }
    }

    template <typename UT>
    void GroupManager<UT>::reset()
    {
        for (auto &r : mGroupResets)
        {
            r();
        }
        mGroupResets.clear();

        mGroupId.clear();
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    EntityGroup *GroupManager<UT>::addGetGroup()
    {
        if (!groupGetter<RequireT, RejectT>().constructed())
        {
            initGroup<RequireT, RejectT>();
        }

        return groupGetter<RequireT, RejectT>().ptr();
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    ComponentFilter GroupManager<UT>::buildFilter() const
    {
        FilterBuilder<RequireT> fbRequire;
        FilterBuilder<RejectT> fbReject;
        return ComponentFilter(fbRequire.value(mCM), fbReject.value(mCM));
    }

    template <typename UT>
    void GroupManager<UT>::checkEntity(EntityId id)
    {
        const ComponentBitset &components(mEM.components(id));
        const GroupBitset &groups(mEM.groups(id));

        for (EntityGroup *grp : mGroupId)
        {
            bool matches{grp->filter().match(components)};
            bool groupTest{groups.test(grp->id() + 1)};
            if (!groupTest && matches)
            {
                mEM.setGroup(id, grp->id() + 1);
                grp->add(id);
            }
            else if (groupTest && !matches)
            {
                mEM.resetGroup(id, grp->id() + 1);
                grp->remove(id);
            }
        }
    }

    template <typename UT>
    void GroupManager<UT>::finalizeGroups()
    {
        for (EntityGroup *grp : mGroupId)
        {
            grp->finalize();
        }
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    void GroupManager<UT>::initGroup()
    {
        ComponentFilter groupFilter{buildFilter<RequireT, RejectT>()};

#ifdef ENT_DEBUG
        if (checkRedundancy(groupFilter))
        {
            ENT_WARNING("Multiple EntityGroups with the same filter, are you sure you want to do this?"
                        "(This can happen, when you specify the same Component types in require and "
                        "reject, but in different order)");
        }
#endif

        groupGetter<RequireT, RejectT>().construct(groupFilter, mGroupId.size());

        ENT_ASSERT_FAST(mGroupId.size() <= MAX_GROUPS);
        mGroupId.push_back(groupGetter<RequireT, RejectT>().ptr());
        mGroupResets.emplace_back([&] () {
            groupGetter<RequireT, RejectT>().destruct();
        });
    }

    template <typename UT>
    bool GroupManager<UT>::checkRedundancy(const ComponentFilter &filter)
    {
        for (const EntityGroup *grp : mGroupId)
        {
            if (grp->filter() == filter)
            {
                return true;
            }
        }

        return false;
    }
    // GroupManager implementation end.
} // namespace ent

#endif //ECS_FIT_GROUP_H
