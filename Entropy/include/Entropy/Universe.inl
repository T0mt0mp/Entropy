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
#ifdef ENT_THREADED_CHANGES
    template <typename T>
    thread_local ChangedEntitiesHolder<Universe<T>> Universe<T>::tChanges;
#endif

    template <typename T>
    Universe<T>::Universe() :
        mEM(), mCM(), mGM(), mSM(), mAC()
    { }

    template <typename T>
    Universe<T>::~Universe()
    {
#ifdef ENT_THREADED_CHANGES
        tChanges.reset();
#endif
#ifdef ENT_STATS_ENABLED
        mStats.reset();
#endif
    }

    template <typename T>
    void Universe<T>::init()
    {
        if (LOG_STATS)
        {
            mStats.univInits++;
            ENT_CHECK_STATS(mStats);
            ENT_ASSERT_FAST(mStats.compRegistered == mCM.numRegistered());
        }

        mEM.init(mCM.numRegistered());

        refresh();
    }

    template <typename T>
    void Universe<T>::refresh()
    {
        /*
         * 1) Refresh EntityManager.
         * 2) Refresh ActionCache:
         *   a) Destroy E -> Create E.
         *   b1) Remove/Add <C>.
         *   b2) Change metadata.
         *   b3) Add to changed list.
         * 3) Refresh ComponentManager:
         *   a) Refresh ComponentHolders
         * 4) Refresh GroupManager:
         *   a) Prepare Groups.
         *   b) Check Groups for activity.
         *   c) Check change Entities, add/remove
         *     from Groups, change Entity metadata.
         *   d) Finalize Groups.
         */

        mEM.refresh();

        mAC.applyChangeSets(this);

        mCM.refresh();

#ifdef ENT_THREADED_CHANGES
        mGM.refresh(tChanges.createResultList(), mEM);

        tChanges.refresh();
#else
        mGM.refresh(mChanged, mEM);

        mChanged.clear();
#endif
    }

    template <typename T>
    void Universe<T>::reset()
    {
        mAC.reset();
        mSM.reset();
        mGM.reset();
        mEM.reset();
        mCM.reset();
#ifdef ENT_THREADED_CHANGES
        tChanges.refresh();
#endif
        resetSelf();
    }

    template <typename T>
    void Universe<T>::printStatus(std::ostream &out)
    {
#ifdef ENT_NOT_USED
        out << "Information about Universe: \n";

        mStats.print(out);
        /*
         * TODO - finish print methods.
        mEM.printStatus(out);
        mCM.printStatus(out);
        mGM.printStatus(out);
        mSM.printStatus(out);
        mAC.printStatus(out);
         */

        out << "Changed list contains " << mChanged.size() << " Entities.\n";
        if (mChanged.size() <= ENT_PRINT_LIMIT)
        {
            out << "\tContents: \n";
            for (EntityId id : mChanged)
            {
                out << "\t\t" << id << "\n";
            }
        }
        out << std::endl;
#endif
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

        ASystemT *system{mSM.template addSystem<ASystemT>(this, mCM, mEM, mGM, std::forward<CArgTs>(args)...)};

        if (LOG_STATS)
        {
            mStats.sysActive++;
            mStats.sysAdded++;
            ENT_CHECK_STATS(mStats);
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
            ENT_CHECK_STATS(mStats);
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
            mGM.template addGroup<RequireT, RejectT>(mCM, mEM);
            if (LOG_STATS)
            {
                mStats.grpActive++;
                mStats.grpAdded++;
                ENT_CHECK_STATS(mStats);
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
            ENT_CHECK_STATS(mStats);
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
        }

        mAC.template registerComponent<ComponentT>(cId);

        return cId;
    }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::componentRegistered() const
    { return mCM.template registered<ComponentT>(); }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::addComponent(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Unable to add Component to invalid Entity!");
        }
#endif

        ComponentT *result{mCM.template add<ComponentT>(id)};

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
    template <typename ComponentT,
        typename... CArgTs>
    ComponentT *Universe<T>::addComponent(EntityId id, CArgTs... cArgs)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Unable to add Component to invalid Entity!");
        }
#endif

        ComponentT *result{mCM.template add<ComponentT>(id, std::forward<CArgTs>(cArgs)...)};

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
    ComponentT *Universe<T>::replaceComponent(EntityId id, const ComponentT &comp)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Unable to add/replace Component to invalid Entity!");
        }
#endif

        ComponentT *result{mCM.template replace<ComponentT>(id, comp)};

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
    ComponentT *Universe<T>::addComponentD(EntityId id)
    {
        // TODO - check existence of the Entity?
        return mAC.changeSet().template addComponent<ComponentT>(mCM.template id<ComponentT>(), id);
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::addComponentT(EntityId id)
    {
        // TODO - check existence of the Entity?
        return mAC.changeSet().template addComponentT<ComponentT>(mCM.template id<ComponentT>(), id);
    }

    template <typename T>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *Universe<T>::addComponentD(EntityId id, CArgTs... cArgs)
    {
        // TODO - check existence of the Entity?
        return mAC.changeSet().template addComponent<ComponentT>(mCM.template id<ComponentT>(), id, std::forward<CArgTs>(cArgs)...);
    }

    template <typename T>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *Universe<T>::addComponentT(EntityId id, CArgTs... cArgs)
    {
        // TODO - check existence of the Entity?
        return mAC.changeSet().template addComponentT<ComponentT>(mCM.template id<ComponentT>(), id, std::forward<CArgTs>(cArgs)...);
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::getComponent(EntityId id)
    {
#ifdef ENT_COMP_EXCEPT
        ComponentT *result{mCM.template get<ComponentT>(id)};
        if (result == nullptr)
        {
            throw std::runtime_error("Component for given Entity does not exist!");
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
            throw std::runtime_error("Component for given Entity does not exist!");
        }
        return result;
#else
        return mCM.template get<ComponentT>(id);
#endif
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::getComponentD(EntityId id)
    {
#ifdef ENT_COMP_EXCEPT
        ComponentT *result{mAC.changeSet().template getComponent<ComponentT>(mCM.template id<ComponentT>(), id)};
        if (result == nullptr)
        {
            throw std::runtime_error("Component for given Entity does not exist!");
        }
        return result;
#else
        return mAC.changeSet().template getComponent<ComponentT>(mCM.template id<ComponentT>(), id);
#endif
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::getComponentT(EntityId id)
    {
#ifdef ENT_COMP_EXCEPT
        ComponentT *result{mAC.changeSet().template getComponentT<ComponentT>(mCM.template id<ComponentT>(), id)};
        if (result == nullptr)
        {
            throw std::runtime_error("Component for given Entity does not exist!");
        }
        return result;
#else
        return mAC.changeSet().template getComponentT<ComponentT>(mCM.template id<ComponentT>(), id);
#endif
    }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::hasComponent(EntityId id) const
    { return mCM.template registered<ComponentT>() && mEM.hasComponent(id, mCM.template id<ComponentT>()); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::hasComponentD(EntityId id)
    { return mAC.changeSet().template hasComponent<ComponentT>(mCM.template id<ComponentT>(), id); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::hasComponentT(EntityId id)
    { return mAC.changeSet().template hasComponentT<ComponentT>(mCM.template id<ComponentT>(), id); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::removeComponent(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Unable to add Component to invalid Entity!");
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
    template <typename ComponentT>
    void Universe<T>::removeComponentD(EntityId id)
    {
        mAC.changeSet().template removeComponent<ComponentT>(mCM.template id<ComponentT>(), id);
    }

    template <typename T>
    template <typename ComponentT>
    void Universe<T>::removeTempComponent(EntityId id)
    {
        mAC.changeSet().template removeTempComponent<ComponentT>(mCM.template id<ComponentT>(), id);
    }

    template <typename T>
    template <typename ComponentT>
    void Universe<T>::removeTempComponentT(EntityId id)
    {
        mAC.changeSet().template removeTempComponentT<ComponentT>(mCM.template id<ComponentT>(), id);
    }

    template <typename T>
    auto Universe<T>::createEntity() -> EntityT
    {
        return EntityT(this, createEntityId());
    }

    template <typename T>
    EntityId Universe<T>::createEntityId()
    {
        EntityId newId{mEM.create()};

#ifdef ENT_ENTITY_EXCEPT
        if (newId.index() == 0)
        { // Unable to create the Entity.
            throw std::runtime_error("Unable to create Entity!");
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

        return newId;
    }

    template <typename T>
    auto Universe<T>::createEntityD() -> TempEntityT
    {
        return TempEntityT(this, mAC.changeSet().createEntity());
    }

    template <typename T>
    void Universe<T>::setActivityEntity(EntityId id, bool activity)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Cannot activate invalid Entity!");
        }
#endif

        if (mEM.setActivity(id, activity))
        {
            entityChanged(id);
        }
    }

    template <typename T>
    void Universe<T>::activateEntity(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Cannot activate invalid Entity!");
        }
#endif

        if (!mEM.active(id))
        {
            entityChanged(id);
            mEM.activate(id);
        }
    }

    template <typename T>
    void Universe<T>::activateEntityD(EntityId id)
    { mAC.changeSet().activateEntity(id); }

    template <typename T>
    void Universe<T>::activateEntityT(EntityId id)
    { mAC.changeSet().activateTempEntity(id); }

    template <typename T>
    void Universe<T>::deactivateEntity(EntityId id)
    {
#ifdef ENT_ENTITY_VALID
        if (!mEM.valid(id))
        {
            throw std::runtime_error("Cannot deactivate invalid Entity!");
        }
#endif

        if (mEM.active(id))
        {
            entityChanged(id);
            mEM.deactivate(id);
        }
    }

    template <typename T>
    void Universe<T>::deactivateEntityD(EntityId id)
    { mAC.changeSet().deactivateEntity(id); }

    template <typename T>
    void Universe<T>::deactivateEntityT(EntityId id)
    { mAC.changeSet().deactivateTempEntity(id); }

    template <typename T>
    bool Universe<T>::destroyEntity(EntityId id)
    {
        bool result{mEM.destroy(id)};

        if (result)
        {
            entityChanged(id);
        }

        return result;
    }

    template <typename T>
    void Universe<T>::destroyEntityD(EntityId id)
    { return mAC.changeSet().destroyEntity(id); }

    template <typename T>
    void Universe<T>::destroyEntityT(EntityId id)
    { return mAC.changeSet().destroyTempEntity(id); }

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
#ifndef ENT_THREADED_CHANGES
        mChanged.reclaim();
#endif
#ifdef ENT_STATS_ENABLED
        mStats.reset();
#endif
    }

    template <typename T>
    void Universe<T>::entityChanged(EntityId id)
    {
        // TODO - Performance, sorted insert without moving immediately?
#ifdef ENT_THREADED_CHANGES
        tChanges.entityChanged(id);
#else
        mChanged.insertUnique(id);
#endif
    }
    // Universe implementation end.
} // namespace ent
