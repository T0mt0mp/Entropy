/**
 * @file Entropy/System.inl
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#include "SystemManager.h"

/// Main Entropy namespace
namespace ent
{
    // SystemManager implementation.
    template <typename UT>
    SystemManager<UT>::SystemManager()
    { }

    template <typename UT>
    SystemManager<UT>::~SystemManager()
    { reset(); }

    template <typename UT>
    void SystemManager<UT>::reset()
    {
        for (auto &h : mDestructOnReset)
        {
            h->destruct();
        }
        mDestructOnReset.clear();
    }

    template <typename UT>
    template <typename SystemT,
        typename... CArgTs>
    SystemT *SystemManager<UT>::addSystem(UT *uni, const ComponentManager<UT> &cm,
                                          GroupManager<UT> &gm, CArgTs... cArgs)
    {
        static_assert(std::is_base_of<System<UT>, SystemT>::value,
                      "System has to inherit from ent::System !");
        static_assert(sizeof(SystemT(cArgs...)), "System has to be instantiable!");

        if (hasSystem<SystemT>())
        { // Prevent double-initialization of Systems.
            return getSystem<SystemT>();
        }

        // Construct the System.
        system<SystemT>().construct(std::forward<CArgTs>(cArgs)...);

        // Get the Require and Reject lists.
        using Extract = RequireRejectExtractor<SystemT>;

        /*
         * Create, if necessary, and set the correct EntityGroup.
         * Part of creating a Group is the creation of the
         * Component filter.
         */
        system<SystemT>()().setGroup(
            gm.template addGroup<
                typename Extract::RequireT,
                typename Extract::RejectT
            >(gm.template buildFilter<
                  typename Extract::RequireT,
                  typename Extract::RejectT
              >(cm)
            )
        );

        mDestructOnReset.emplace_back(&system<SystemT>());

        system<SystemT>()().setUniverse(uni);
        return system<SystemT>().ptr();
    }

    template <typename UT>
    template <typename SystemT>
    bool SystemManager<UT>::hasSystem() const
    {
        return system<SystemT>().constructed();
    }

    template <typename UT>
    template <typename SystemT>
    SystemT *SystemManager<UT>::getSystem() const
    {
        return system<SystemT>().ptr();
    }

    template <typename UT>
    template <typename SystemT>
    bool SystemManager<UT>::removeSystem()
    {
        if (hasSystem<SystemT>())
        {
            system<SystemT>().destruct();

            return true;
        }

        // System not found.
        return false;
    }

    // SystemManager implementation end.

    // System implementation.
    template <typename UT>
    System<UT>::~System()
    {
        if (mGroup && mInitialized)
        { // Notify the Group.
            mGroup->abandon();
        }

        mInitialized = false;
    }

    template <typename UT>
    bool System<UT>::isInitialized() const
    { return mInitialized; }

    template <typename UT>
    EntityList<UT, EntityGroup::EntityListT> System<UT>::foreach()
    { return mGroup->foreach(mUniverse); }

    template <typename UT>
    EntityList<UT, EntityGroup::AddedListT> System<UT>::foreachAdded()
    { return mGroup->foreachAdded(mUniverse); }

    template <typename UT>
    EntityList<UT, EntityGroup::RemovedListT> System<UT>::foreachRemoved()
    { return mGroup->foreachRemoved(mUniverse); }

    template <typename UT>
    const ComponentFilter &System<UT>::filter() const
    { ENT_ASSERT_FAST(isInitialized()); return mGroup->filter(); }

    template <typename UT>
    u64 System<UT>::groupId() const
    { ENT_ASSERT_FAST(isInitialized()); return mGroup->id(); }

    template <typename UT>
    void System<UT>::setGroup(EntityGroup *grp)
    { mGroup = grp; }

    template <typename UT>
    void System<UT>::setUniverse(UT *uni)
    {
        mInitialized = true;
        mUniverse = uni;
    }
    // System implementation end.
} // namespace ent
