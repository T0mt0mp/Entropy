/**
 * @file Entropy/System.h
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#ifndef ECS_FIT_SYSTEM_H
#define ECS_FIT_SYSTEM_H

#include "Types.h"
#include "Util.h"
#include "Group.h"

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
        SystemManager(GroupManager<UniverseT> &groupMgr);

        /**
         * TODO - Refresh the Systems.
         */
        void refresh();

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
         * !!System has to be added first!!
         * @tparam SystemT Type of the System.
         * @return Returns ptr to the System object, or nullptr
         *   if the System has not been added.
         */
        template <typename SystemT>
        SystemT *getSystem() const;
    private:
        /**
         * Container for the System.
         * @tparam SystemT Type of the System.
         */
        template <typename SystemT>
        static ConstructionHandler<SystemT> mSystem;

        /// Group manager from the same Universe.
        GroupManager<UniverseT> &mGM;
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
    }; // System

    // SystemManager implementation.
    template <typename UT>
    template <typename SystemT>
    ConstructionHandler<SystemT> SystemManager<UT>::mSystem;

    template <typename UT>
    SystemManager<UT>::SystemManager(GroupManager<UT> &groupMgr) :
        mGM{groupMgr}
    { }

    template <typename UT>
    void SystemManager<UT>::refresh()
    {
        ENT_WARNING("SystemManager::refresh() is not finished yet!");
    }

    template <typename UT>
    template <typename SystemT,
              typename... CArgTs>
    SystemT *SystemManager<UT>::addSystem(UT *uni, CArgTs... cArgs)
    {
        static_assert(std::is_base_of<System<UT>, SystemT>::value,
                      "System has to inherit from ent::System !");
        static_assert(sizeof(SystemT(cArgs...)), "System has to be instantiable!");

        mSystem<SystemT>.construct(std::forward<CArgTs>(cArgs)...);
        using Extract = RequireRejectExtractor<SystemT>;
        mSystem<SystemT>().setGroup(
            mGM.template addGetGroup<
                typename Extract::RequireT,
                typename Extract::RejectT
            >());

        SystemT *sys{mSystem<SystemT>.ptr()};

        sys->setUniverse(uni);
        return sys;
    }

    template <typename UT>
    template <typename SystemT>
    SystemT *SystemManager<UT>::getSystem() const
    {
        return mSystem<SystemT>.ptr();
    }
    // SystemManager implementation end.
} // namespace ent

#endif //ECS_FIT_SYSTEM_H
