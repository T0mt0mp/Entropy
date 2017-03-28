/**
 * @file Entropy/GroupManager.h
 * @author Tomas Polasek
 * @brief Manager class allowing the use of EntityGroups.
 */

#ifndef ECS_FIT_GROUPMANAGER_H
#define ECS_FIT_GROUPMANAGER_H

#include "Util.h"
#include "List.h"
#include "EntityGroup.h"
#include "EntityManager.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * GroupManager is a part of Entropy ECS Universe.
     * GroupManager keeps a catalogue of EntityGroups for
     * Systems to use.
     * It has method which take care about creating/removing
     * EntityGroups, and their refreshing.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class GroupManager final
    {
    public:
        /// Default constructor.
        GroupManager();

        /// Destruct Entity Groups and this manager.
        ~GroupManager();

        /**
         * Refresh active groups with changed Entities. Groups with
         * usage counter which reached zero will be removed and their
         * IDs recycled.
         * @param changed List of changed Entity IDs.
         * @param em Used for checking present Components for Entities and
         *   changing their Group metadata.
         */
        void refresh(const ent::SortedList<EntityId> &changed, EntityManager &em);

        /**
         * Reset the Manager and all of the Entity Groups.
         */
        void reset();

        /**
         * Add Entity group with Required and Rejected Components.
         * The pointer is guaranteed to be valid as long as the
         * SystemManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param f Filter corresponding to the template parameters.
         * @return Returns ptr to the requested Entity Group. If the
         *   Group already exists, no new groups are created.
         * @remark Each time this method is called the corresponding usage
         *   counter is incremented!
         */
        template<typename RequireT,
            typename RejectT>
        inline EntityGroup *addGroup(const ComponentFilter &f);

        /**
         * Is there an EntityGroup with given filter?
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns true, if such Group exists.
         */
        template<typename RequireT,
            typename RejectT>
        inline bool hasGroup();

        /**
         * Get already existing EntityGroup.
         * The pointer is guaranteed to be valid as long as the
         * SystemManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns ptr to the requested EntityGroup, or
         *   nullptr, if such group does not exist.
         */
        template<typename RequireT,
            typename RejectT>
        inline EntityGroup *getGroup();

        /**
         * Decrement the usage counter for given EntityGroup.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns true for the first time the counter
         *   reaches zero.
         */
        template<typename RequireT,
            typename RejectT>
        inline bool abandonGroup();

        /**
         * Generate Component filter for given Require and Reject lists.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param cm Active Component manager, used for getting
         *   Component masks.
         * @return Returns Component filter for given Require
         *   and Reject lists.
         */
        template <typename RequireT,
                  typename RejectT>
        inline ComponentFilter buildFilter(const ComponentManager<UniverseT> &cm) const;
    private:
        /**
         * Build a filter bitset using template magic.
         * Takes a type list of Component types - ContainerT
         * can be any type. Each of the Component types is
         * used as template parameter to get the componentMask.
         * Final result is all masks ORed.
         * @tparam CompMgrT Type of the Component Manager.
         * @tparam ContainerT Container containing type list
         *   of Component types.
         */
        template<typename ContainerT>
        struct FilterBuilder;

        /**
         * Call refresh on all active Groups.
         */
        inline void refreshGroups();

        /**
         * Check EntityGroups, if they are still in use. If there is any
         * Group, which is not in use, it will be removed.
         * @param em Used for changing Group metadata of Entities.
         */
        inline void checkGroups(EntityManager &em);

        /**
         * Test all Entities on the changed list, if they should
         * be added/removed from any groups.
         * @param changed List of changed Entities since last refresh.
         * @param em EntityManager used for getting information about
         *   the Entities and write back Group changes.
         */
        inline void checkEntities(const ent::SortedList<EntityId> &changed,
                                  EntityManager &em);

        /**
         * Finish the operation of adding/removing Entities from all
         * affected Groups.
         */
        inline void finalizeGroups();

        /**
         * Initialize group.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template<typename RequireT,
            typename RejectT>
        inline void initGroup(const ComponentFilter &f);

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
        inline bool checkGrpRedundancy(const ComponentFilter &f);

        /**
         * Get next available Group ID and remove it from
         * the List of available IDs.
         * @return Next Entity Group ID not in use.
         */
        inline u64 nextGroupId();

        /**
         * Remove inactive Groups from given list.
         * @param groups List of Groups.
         */
        inline static void removeInactive(std::vector<EntityGroup*> &groups);

        /**
         * Static instance of EntityGroup.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         */
        template<typename RequireT,
            typename RejectT>
        static ConstructionHandler<EntityGroup> &group()
        {
            static ConstructionHandler<EntityGroup> group;
            return group;
        }

        /// List of recycled EntityGroup IDs.
        std::vector<u64> mFreeIds;
        /// Last Entity Group ID in use.
        u64 mLastUsedId;
        /// List of active EntityGroups, which are being refreshed.
        std::vector<EntityGroup*> mActiveGroups;
        /// List of newly created EntityGroups.
        std::vector<EntityGroup*> mNewGroups;

        /// List of things to destruct on reset.
        std::vector<ConstructionHandlerBase*> mDestructOnReset;
    protected:
    }; // class GroupManager
} // namespace ent

#include "GroupManager.inl"

#endif //ECS_FIT_GROUPMANAGER_H
