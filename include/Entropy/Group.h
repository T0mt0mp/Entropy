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
    private:
    protected:
    }; // EntityGroup

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
        GroupManager(UniverseT *universe) :
            mUniverse{universe}
        { }

        /**
         * Add or get already created Entity group.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
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

        /**
         * Initialize group.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template <typename RequireT,
                  typename RejectT>
        void initGroup();

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
        ComponentFilter groupFilter{buildFilter<RequireT, RejectT>()};

        mGroup<RequireT, RejectT>.construct(groupFilter);

        mGroupId.push_back(&mGroup<RequireT, RejectT>());
    }
    // GroupManager implementation end.
} // namespace ent

#endif //ECS_FIT_GROUP_H
