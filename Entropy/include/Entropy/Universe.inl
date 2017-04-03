/**
 * @file Entropy/Universe.inl
 * @author Tomas Polasek
 * @brief Universe is the wrapper around all other parts of the Entropy ECS.
 */

#include "Universe.h"

/// Main Entropy namespace
namespace ent
{
    // Universe implementation.
    template <typename T>
    Universe<T>::Universe() :
        mEM(), mCM(), mGM(), mSM(), mAC()
    { }

    template <typename T>
    Universe<T>::~Universe()
    { }

    template <typename T>
    void Universe<T>::init()
    {
        refresh();
    }

    template <typename T>
    void Universe<T>::refresh()
    {
        /*
         * 1) Refresh ActionCache:
         *   a) Destroy E -> Create E.
         *   b1) Remove/Add <C>.
         *   b2) Change metadata.
         *   b3) Add to changed list.
         * 2) Refresh ComponentManager:
         *   a) Refresh ComponentHolders
         * 3) Refresh GroupManager:
         *   a) Prepare Groups.
         *   b) Check Groups for activity.
         *   c) Check change Entities, add/remove
         *     from Groups, change Entity metadata.
         *   d) Finalize Groups.
         */

        mCM.refresh();
        mGM.refresh(mChanged, mEM);
    }

    template <typename T>
    void Universe<T>::reset()
    {
        mAC.reset();
        mSM.reset();
        mGM.reset();
        mEM.reset();
        mCM.reset();
        resetSelf();
    }

    template <typename T>
    template <typename ASystemT,
        typename... CArgTs>
    ASystemT *Universe<T>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<SystemT, ASystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<ASystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");

        ASystemT *system{mSM.template addSystem<ASystemT>(this, mCM, mGM, std::forward<CArgTs>(args)...)};

        if (LOG_STATS)
        {
            mStats.sysActive++;
            mStats.sysAdded++;
            CHECK_STATS(mStats);
        }

        return system;
    }

    template <typename T>
    template <typename ASystemT>
    ASystemT *Universe<T>::getSystem()
    {
        return mSM.template getSystem<ASystemT>();
    }

    template <typename T>
    template<typename ASystemT>
    bool Universe<T>::removeSystem()
    {
        static_assert(std::is_base_of<SystemT, ASystemT>::value, "The System has to inherit from ent::System!");

        bool removed{mSM.template removeSystem<ASystemT>()};

        if (LOG_STATS && removed)
        {
            mStats.sysActive--;
            mStats.sysRemoved++;
            CHECK_STATS(mStats);
        }

        return removed;
    }

    template <typename T>
    template <typename RequireT,
        typename RejectT>
    EntityGroup *Universe<T>::addGetGroup()
    {
        if (!mGM.template hasGroup<RequireT, RejectT>())
        {
            mGM.template addGroup<RequireT, RejectT>(mGM.template buildFilter<RequireT, RejectT>(mCM));
            if (LOG_STATS)
            {
                mStats.grpActive++;
                mStats.grpAdded++;
                CHECK_STATS(mStats);
            }
        }

        return mGM.template getGroup<RequireT, RejectT>();
    }

    template <typename T>
    template <typename RequireT,
        typename RejectT>
    bool Universe<T>::abandonGroup()
    {
        bool result{mGM.template abandonGroup<RequireT, RejectT>()};

        if (LOG_STATS && result)
        {
            mStats.grpActive--;
            mStats.grpRemoved++;
            CHECK_STATS(mStats);
        }

        return result;
    }

    template <typename T>
    template <typename ComponentT,
        typename... CArgTs>
    u64 Universe<T>::registerComponent(CArgTs... args)
    {
        // Check for multiple calls for single Component.
        if (mCM.template registered<ComponentT>())
        {
            ENT_WARNING("registerComponent called multiple times!");
            return mCM.template id<ComponentT>();
        }

        u64 cId{mCM.template registerComponent<ComponentT>(std::forward<CArgTs>(args)...)};

        if (LOG_STATS)
        {
            mStats.compRegistered++;
            CHECK_STATS(mStats);
        }

        return cId;
    }

    template <typename T>
    template <typename ComponentT>
    const ComponentBitset &Universe<T>::componentMask() const
    { return mCM.template mask<ComponentT>(); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::componentRegistered() const
    { return mCM.template registered<ComponentT>(); }

    template <typename T>
    template <typename ComponentT,
        typename... CArgTs>
    ComponentT *Universe<T>::addComponent(EntityId id, CArgTs... cargs)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_exception("Unable to add Component to invalid Entity!");
        }
#endif

        ComponentT *result{mCM.template add<ComponentT>(id, std::forward(cargs)...)};

        if (result)
        { // Check, if the add operation returned success.
            bool alreadyPresent{mEM.hasComponent(id, mCM.template id<ComponentT>())};
            if (!alreadyPresent)
            { // Check, if the Component has been added previously.
                entityChanged(id);
                mEM.addComponent(id, mCM.template id<ComponentT>());
            }
        }

        return result;
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::getComponent(EntityId id)
    {
#ifdef ENT_COMP_EXCEPT
        ComponentT *result{mCM.template get<ComponentT>(id)};
        if (result == nullptr)
        {
            throw std::runtime_exception("Component for given Entity does not exist!");
        }
        return result;
#else
        return mCM.template get<ComponentT>(id);
#endif
    }

    template <typename T>
    template <typename ComponentT>
    const ComponentT *Universe<T>::getComponent(EntityId id) const
    {
#ifdef ENT_COMP_EXCEPT
        ComponentT *result{mCM.template get<ComponentT>(id)};
        if (result == nullptr)
        {
            throw std::runtime_exception("Component for given Entity does not exist!");
        }
        return result;
#else
        return mCM.template get<ComponentT>(id);
#endif
    }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::hasComponent(EntityId id) const
    { return mCM.template registered<ComponentT>() && mEM.hasComponent(id, mCM.template id<ComponentT>()); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::removeComponent(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_exception("Unable to add Component to invalid Entity!");
        }
#endif
        bool result{mCM.template remove<ComponentT>(id)};

        if (result)
        {
            entityChanged(id);
            mEM.removeComponent(id, mCM.template id<ComponentT>());
        }

        return result;
    }

    template <typename T>
    auto Universe<T>::createEntity() -> EntityT
    {
        EntityId newId{mEM.create()};

#ifdef ENT_ENTITY_EXCEPT
        if (newId.index() == 0)
        { // Unable to create the Entity.
            throw std::runtime_exception("Unable to create Entity!");
        }
#endif


        if (newId.index())
        { // If valid ID has been returned.
            entityChanged(newId);

            if (LOG_STATS)
            {
                mStats.entCreated++;
                mStats.entActive++;
                mStats.entTotal++;
            }
        }

        return EntityT(this, newId);
    }

    template <typename T>
    void Universe<T>::activateEntity(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_exception("Cannot activate invalid Entity!");
        }
#endif

        if (!mEM.active(id))
        {
            entityChanged(id);
            mEM.activate(id);
        }
    }

    template <typename T>
    void Universe<T>::deactivateEntity(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_exception("Cannot deactivate invalid Entity!");
        }
#endif

        if (mEM.active(id))
        {
            entityChanged(id);
            mEM.deactivate(id);
        }
    }

    template <typename T>
    bool Universe<T>::destroyEntity(EntityId id)
    {
        bool result{mEM.destroy(id)};

        if (result)
        {
            mCM.entityDestroyed(id, mEM.components(id));
            entityChanged(id);
        }

        return result;
    }

    template <typename T>
    bool Universe<T>::entityValid(EntityId id) const
    {
        return mEM.valid(id);
    }

    template <typename T>
    bool Universe<T>::entityActive(EntityId id) const
    {
        return mEM.active(id);
    }

    template <typename T>
    void Universe<T>::commitChangeSet()
    {
        mAC.commitChangeSet();
    }

    template <typename T>
    void Universe<T>::resetSelf()
    {
        mChanged.reclaim();
    }

    template <typename T>
    void Universe<T>::entityChanged(EntityId id)
    {
        mChanged.insertUnique(id);
    }
    // Universe implementation end.
} // namespace ent
