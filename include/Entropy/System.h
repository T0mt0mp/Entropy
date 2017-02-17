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
         * @param uni Universe, this manager is part of.
         */
        SystemManager(UniverseT *uni);

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
         * @param cArgs Construct arguments.
         * @return Returns ptr to the constructed System.
         */
        template <typename SystemT,
                  typename... CArgTs>
        SystemT *addSystem(CArgTs... cArgs);

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

        /// Universe, this manager is part of.
        UniverseT *mUniverse;
    protected:
    }; // SystemManager

    /**
     * System in Entropy ECS contains the means to iterate
     * over its EntityGroup and other utility methods.
     * It is meant to be inherited by the System implementation
     * which can contain the logic.
     */
    class System : NonCopyable
    {
    public:
        template <typename UniverseT>
        friend class SystemManager;
    private:
        /**
         * Set Entity Group containing Entities which are
         * of interest to this System.
         * @param grp Pointer to the Group.
         */
        void setGroup(EntityGroup *grp);

        /// Flag used for signifying, that this System is ready for use.
        bool mInitialized;
        /// Entity group containing Entities, which are of interest to this System.
        EntityGroup *mGroup;
    protected:
        /// Check, if the System is ready for use.
        bool isInitialized() const
        { return mInitialized; }

        /// Filter getter.
        const ComponentFilter &filter() const
        { ENT_ASSERT_FAST(isInitialized()); return mGroup->filter(); }

        u64 groupId() const
        { ENT_ASSERT_FAST(isInitialized()); return mGroup->id(); }
    }; // System

    // SystemManager implementation.
    template <typename UT>
    template <typename SystemT>
    ConstructionHandler<SystemT> SystemManager<UT>::mSystem;

    template <typename UT>
    SystemManager<UT>::SystemManager(UT *uni) :
        mUniverse{uni}
    { }

    template <typename UT>
    void SystemManager<UT>::refresh()
    {
        ENT_WARNING("SystemManager::refresh() is not finished yet!");
    }

    template <typename UT>
    template <typename SystemT,
              typename... CArgTs>
    SystemT *SystemManager<UT>::addSystem(CArgTs... cArgs)
    {
        static_assert(std::is_base_of<System, SystemT>::value,
                      "System has to inherit from ent::System !");
        static_assert(sizeof(SystemT(cArgs...)), "System has to be instantiable!");

        mSystem<SystemT>.construct(std::forward<CArgTs>(cArgs)...);
        using Extract = RequireRejectExtractor<SystemT>;
        mSystem<SystemT>().setGroup(
            mUniverse->template addGetGroup<
                typename Extract::RequireT,
                typename Extract::RejectT
            >());

        return mSystem<SystemT>.ptr();
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
