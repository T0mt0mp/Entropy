/**
 * @file Entropy/System.h
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#ifndef ECS_FIT_SYSTEM_H
#define ECS_FIT_SYSTEM_H

#include "Types.h"
#include "Util.h"
#include "EntityGroup.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * SystemManager base class containing code which does not need to be templated.
     */
    class SystemManagerBase : NonCopyable
    {
    public:
    private:
    protected:
    }; // SystemManagerBase

    /**
     * SystemManager is a part of Entropy ECS Universe.
     * Its main purpose is to manage Systems within a one Universe.
     * It contains methods for adding/removing systems, their
     * initialization and refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class SystemManager final : public SystemManagerBase
    {
    public:
        /**
         * Construct the System manager for given Universe.
         * @param groupMgr Group manager from the same Universe.
         */
        SystemManager();

        /**
         * Destruct the System manager and all System within.
         */
        ~SystemManager();

        /**
         * TODO - Refresh the Systems.
         */
        void refresh();

        /**
         * Destruct all systems.
         */
        void reset();

        /**
         * Add System of given type to the manager. System is constructed
         * with provided constructor parameters.
         * If there already exists System of given type, the old one
         * is destructed and new one constructed in its place.
         * @tparam SystemT Type of the System.
         * @tparam CArgTs Constructor argument types.
         * @param uni Universe ptr.
         * @param cArgs Construct arguments.
         * @return Returns ptr to the constructed System.
         */
        template <typename SystemT,
                  typename... CArgTs>
        SystemT *addSystem(UniverseT *uni, CArgTs... cArgs);

        /**
         * Get System with given type.
         * @tparam SystemT Type of the System.
         * @return Returns ptr to the System object, or nullptr
         *   if the System has not been added.
         */
        template <typename SystemT>
        SystemT *getSystem() const;

        /**
         * Remove System with given type and decrement the
         * usage counter within its group.
         * @tparam SystemT Type of the System.
         * @return Returns true, if the System has been
         *   removed successfully.
         */
        template <typename SystemT>
        bool removeSystem();

        /**
         * Add Entity group with Required and Rejected Components.
         * The pointer is guaranteed to be valid as long as the
         * SystemManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param f Filter corresponding to the template parameters.
         * @return Returns ptr to the requested Entity Group.
         */
        template<typename RequireT,
            typename RejectT>
        EntityGroup *addGroup(ComponentFilter &f);

        /**
         * Is there an EntityGroup with given filter?
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param f Filter corresponding to the template parameters.
         * @return Returns true, if such Group exists.
         */
        template<typename RequireT,
            typename RejectT>
        bool hasGroup(ComponentFilter &f);

        /**
         * Get already existing EntityGroup.
         * The pointer is guaranteed to be valid as long as the
         * SystemManager is instantiated - the Group will not be
         * moved around.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param f Filter corresponding to the template parameters.
         * @return Returns ptr to the requested EntityGroup, or
         *   nullptr, if such group does not exist.
         */
        template<typename RequireT,
            typename RejectT>
        EntityGroup *getGroup(ComponentFilter &f);

        /**
         * Decrement the usage counter for given EntityGroup.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @param f Filter corresponding to the template parameters.
         * @return Returns true for the first time the counter
         *   reaches zero.
         */
        template<typename RequireT,
            typename RejectT>
        bool abandonGroup(ComponentFilter &f);

        /**
         * Test all Entities on the changed list, if they should
         * be added/removed from any groups.
         * @param changed List of changed Entities since last refresh.
         * @param em EntityManager used for getting information about
         *   the Entities and write back Group changes.
         */
        void checkEntities(const ent::SortedList<EntityId> &changed,
                           EntityManager &em);
    private:
        /**
         * Container for the System.
         * @tparam SystemT Type of the System.
         */
        template <typename SystemT>
		static ConstructionHandler<SystemT> &systemGetter()
		{
			static ConstructionHandler<SystemT> sys;
			return sys;
		}

        /// List of reset lambdas.
        std::vector<std::function<void()>> mSystemResets;
    protected:
    }; // SystemManager

    /**
     * System in Entropy ECS contains the means to iterate
     * over its EntityGroup and other utility methods.
     * It is meant to be inherited by the System implementation
     * which can contain the logic.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class System : NonCopyable
    {
    public:
        friend class SystemManager<UniverseT>;

        ~System()
        { mInitialized = false; }

        /// Check, if the System is ready for use.
        bool isInitialized() const
        { return mInitialized; }

        /**
         * Iterator for iterating trough Entities within the group.
         * @return Returns iterator for iterating through Entities withing the group.
         */
        EntityList<UniverseT, EntityGroup::EntityListT> foreach()
        { return mGroup->foreach(mUniverse); }

        /**
         * Iterator for iterating trough Entities which were added since the last refresh.
         * @return Returns iterator for iterating through Entities which were added since the last refresh.
         */
        EntityList<UniverseT, EntityGroup::AddedListT> foreachAdded()
        { return mGroup->foreachAdded(mUniverse); }

        /**
         * Iterator for iterating trough Entities which were removed since the last refresh.
         * !!Warning: The Entities withing this Group may not exist anymore!!
         * @return Returns iterator for iterating through Entities which were removed since the last refresh.
         */
        EntityList<UniverseT, EntityGroup::RemovedListT> foreachRemoved()
        { return mGroup->foreachRemoved(mUniverse); }

        /// Filter getter.
        const ComponentFilter &filter() const
        { ENT_ASSERT_FAST(isInitialized()); return mGroup->filter(); }

        /// Group ID getter.
        u64 groupId() const
        { ENT_ASSERT_FAST(isInitialized()); return mGroup->id(); }
    private:
        /**
         * Set Entity Group containing Entities which are
         * of interest to this System.
         * @param grp Pointer to the Group.
         */
        void setGroup(EntityGroup *grp)
        { mGroup = grp; }

        /**
         * Set Universe, for this System.
         * @param uni Universe ptr.
         */
        void setUniverse(UniverseT *uni)
        {
            mInitialized = true;
            mUniverse = uni;
        }

        /// Flag used for signifying, that this System is ready for use.
        bool mInitialized;
        /// Entity group containing Entities, which are of interest to this System.
        EntityGroup *mGroup;
        /// Universe this System works for.
        UniverseT *mUniverse;
    protected:
    }; // class System

    // SystemManager implementation.
	/*
    template <typename UT>
    template <typename SystemT>
    ConstructionHandler<SystemT> SystemManager<UT>::mSystem;
	*/

    template <typename UT>
    SystemManager<UT>::SystemManager(GroupManager<UT> &groupMgr) :
        mGM{groupMgr}
    { }

    template <typename UT>
    SystemManager<UT>::~SystemManager()
    { reset(); }

    template <typename UT>
    void SystemManager<UT>::refresh()
    {
        for (auto *grp : mGroupId)
        {
            grp->refresh();
        }
    }

    template <typename UT>
    void SystemManager<UT>::reset()
    {
        for (auto &l : mSystemResets)
        {
            l();
        }
        mSystemResets.clear();

        for (auto &r : mGroupResets)
        {
            r();
        }
        mGroupResets.clear();

        mGroupId.clear();
    }

    template <typename UT>
    template <typename SystemT,
              typename... CArgTs>
    SystemT *SystemManager<UT>::addSystem(UT *uni, CArgTs... cArgs)
    {
        static_assert(std::is_base_of<System<UT>, SystemT>::value,
                      "System has to inherit from ent::System !");
        static_assert(sizeof(SystemT(cArgs...)), "System has to be instantiable!");

        systemGetter<SystemT>().construct(std::forward<CArgTs>(cArgs)...);
        using Extract = RequireRejectExtractor<SystemT>;
        systemGetter<SystemT>()().setGroup(
            mGM.template addGetGroup<
                typename Extract::RequireT,
                typename Extract::RejectT
            >());

        SystemT *sys{systemGetter<SystemT>().ptr()};

        mSystemResets.emplace_back([&] () {
            systemGetter<SystemT>().destruct();
        });

        sys->setUniverse(uni);
        return sys;
    }

    template <typename UT>
    template <typename SystemT>
    SystemT *SystemManager<UT>::getSystem() const
    {
        return systemGetter<SystemT>().ptr();
    }
    // SystemManager implementation end.
} // namespace ent

#endif //ECS_FIT_SYSTEM_H
