/**
 * @file Entropy/Group.h
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#ifndef ECS_FIT_GROUP_H
#define ECS_FIT_GROUP_H

#include "Types.h"
#include "Util.h"
#include "Component.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Helper structure for specifying required Component types
     * for a System.
     * @tparam RequireComponentTs List of required Component types.
     */
    template <typename... RequireComponentTs>
    struct Require
    { };

    /**
     * Helper structure for specifying rejected Component types
     * for a System.
     * @tparam RejectComponentTs List of rejected Component types.
     */
    template <typename... RejectComponentTs>
    struct Reject
    { };

    /**
     * EntityGroup class contains a list of Entities, which pass
     * a group specific filter. They are used for caching the Entities for
     * use in Systems.
     */
    class EntityGroup final : NonCopyable
    {
    public:
        /**
         * Construct a EntityGroup, specifying which
         * Entities belong into it, using the filter.
         * @param filter Specifies which Entities belong
         *   into this Group.
         * @param groupId ID of the group, starting at 0.
         */
        EntityGroup(const ComponentFilter &filter, u64 groupId);

        /// Filter getter.
        const ComponentFilter &filter() const
        { return mFilter; }

        /// Group ID getter.
        u64 id() const
        { return mId; }
    private:
        /// Filter specifying which Entities belong into this group.
        ComponentFilter mFilter;
        /// ID of this Group.
        u64 mId;
    protected:
    }; // EntityGroup

    /**
     * Extract Require and Reject type lists from given System type.
     * Default value (if either list is not present) is empty type list.
     * @tparam SystemT Type of the System.
     */
    template <typename SystemT>
    struct RequireRejectExtractor
    {
        struct RequireGetter
        {
            template <typename ST>
            static typename ST::Require getRequire(void *unused);
            template <typename ST>
            static Require<> getRequire(...);
            using RequireT = decltype(getRequire<SystemT>(nullptr));
        };

        struct RejectGetter
        {
            template <typename ST>
            static typename ST::Reject getReject(void *unused);
            template <typename ST>
            static Reject<> getReject(...);
            using RejectT = decltype(getReject<SystemT>(nullptr));
        };

        using RequireT = typename RequireGetter::RequireT;
        using RejectT = typename RejectGetter::RejectT;
    };

    /**
     * GroupManager base class containing code which does not need to be templated.
     */
    class GroupManagerBase : NonCopyable
    {
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
    template <typename UniverseT>
    class GroupManager final : public GroupManagerBase
    {
    public:
        /**
         * Basic constructor.
         * @param universe Universe containing this manager.
         */
        GroupManager(UniverseT *uni);

        /**
         * TODO - refresh GroupManager.
         */
        void refresh();

        /**
         * Add or get already created Entity group.
         * The pointer is guaranteed to be valid as long as the
         * GroupManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns ptr to the requested Entity Group.
         */
        template <typename RequireT,
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
        template <typename RequireT,
                  typename RejectT>
        ComponentFilter buildFilter() const;
    private:
        /// Group ID generator.
        class GroupIdGenerator : public StaticClassIdGenerator<GroupIdGenerator> {};

        /**
         * Build a filter bitset using template magic.
         * Takes a type list of Component types - ContainerT
         * can be any type. Each of the Component types is
         * used as template parameter to get the componentMask.
         * Final result is all masks ORed.
         * @tparam ContainerT Container containing type list
         *   of Component types.
         */
        template <typename ContainerT>
        struct FilterBuilder;

        template <template <typename...> typename ContainerT,
                  typename FirstT,
                  typename... RestTs>
        struct FilterBuilder<ContainerT<FirstT, RestTs...>>
        {
            static auto value(UniverseT *uni)
            {
                FilterBuilder<ContainerT<RestTs...>> next;
                return uni->template componentMask<FirstT>() | next.value(uni);
            }
        };

        template <template <typename...> typename ContainerT,
                  typename LastT>
        struct FilterBuilder<ContainerT<LastT>>
        {
            static auto value(UniverseT *uni)
            {
                return uni->template componentMask<LastT>();
            }
        };

        template <template <typename...> typename ContainerT>
        struct FilterBuilder<ContainerT<>>
        {
            static auto value(UniverseT *uni)
            {
                return ComponentBitset();
            }
        };

        /**
         * Initialize group.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template <typename RequireT,
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
        std::vector<EntityGroup*> mGroupId;

        /**
         * Static instance of EntityGroup.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template <typename RequireT,
                  typename RejectT>
        static ConstructionHandler<EntityGroup> mGroup;

        /// Universe containing this manager.
        UniverseT *mUniverse;
    protected:
    }; // GroupManager

    // GroupManager implementation.
    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    ConstructionHandler<EntityGroup> GroupManager<UT>::mGroup;

    template <typename UT>
    GroupManager<UT>::GroupManager(UT *uni) :
        mUniverse{uni}
    { }

    template <typename UT>
    void GroupManager<UT>::refresh()
    {
        ENT_WARNING("GroupManager::refresh() is not finished yet!");
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    EntityGroup *GroupManager<UT>::addGetGroup()
    {
        if (!mGroup<RequireT, RejectT>.constructed())
        {
            initGroup<RequireT, RejectT>();
        }

        return &mGroup<RequireT, RejectT>();
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    ComponentFilter GroupManager<UT>::buildFilter() const
    {
        FilterBuilder<RequireT> fbRequire;
        FilterBuilder<RejectT> fbReject;
        return ComponentFilter(fbRequire.value(mUniverse), fbReject.value(mUniverse));
    }

    template <typename UT>
    template <typename RequireT,
              typename RejectT>
    void GroupManager<UT>::initGroup()
    {
        static ComponentFilter groupFilter{buildFilter<RequireT, RejectT>()};

#ifdef ENT_DEBUG
        if (checkRedundancy(groupFilter))
        {
            ENT_WARNING("Multiple EntityGroups with the same filter, are you sure you want to do this?"
                        "(This can happen, when you specify the same Component types in require and "
                        "reject, but in different order)");
        }
#endif

        mGroup<RequireT, RejectT>.construct(groupFilter, mGroupId.size());

        mGroupId.push_back(mGroup<RequireT, RejectT>.ptr());
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
