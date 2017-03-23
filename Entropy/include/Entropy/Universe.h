/**
 * @file Entropy/Universe.h
 * @author Tomas Polasek
 * @brief Universe is the wrapper around all other parts of the Entropy ECS.
 */

#ifndef ECS_FIT_UNIVERSE_H
#define ECS_FIT_UNIVERSE_H

#include "ActionCache.h"
#include "System.h"
#include "Component.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Group.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Universe is the main wrapper class around all other
     * functionality of Entropy ECS.
     * It contains management classes for different
     * aspects of the system and methods to access them.
     * @tparam T Used for distinguishing between Universes, CRTP.
     */
    template <typename T>
    class Universe : NonCopyable
    {
    public:
        using UniverseT = Universe<T>;
        using EntityT = Entity<UniverseT>;
        using TempEntityT = TemporaryEntity;
        using SystemT = System<UniverseT>;

        friend class Entity<UniverseT>;

#ifdef ENT_STATS_ENABLED
        static constexpr bool LOG_STATS{true};
#else
        static constexpr bool LOG_STATS{false};
#endif

        /**
         * Default constructor for Universe.
         */
        Universe();

        /**
         * Universe destructs itself and all inner managers.
         */
        ~Universe();

        /**
         * Initialize all required structures AFTER adding all the
         * required Components.
         * @code
         * Universe<T> u;
         * // Register Components
         * // Optionally add Systems - only after Components!
         * u.init();
         * @endcode
         */
        void init();

        /**
         * Refresh inner structures and prepare for next run.
         * Actions that will be taken:
         *   Add/remove Entities.
         *   Add/remove Components.
         *   Refresh EntityGroups.
         */
        void refresh();

        /**
         * Reset the managers.
         * All Components/Systems will have to be added again.
         */
        void reset();

#ifdef ENT_STATS_ENABLED
        /// Get statistics for this Universe.
        const UniverseStats &statistics() const
        { return mStats; }
#endif
        /**
         * Register given System for this universe.
         * @tparam ASystemT Type of the system, has to inherit from base ent::System.
         * @tparam CArgTs System constructor argument types.
         * @param args System constructor arguments, passed to the System on construction.
         * @return Returns ptr to the newly registered System.
         * @remarks Not thread-safe!
         */
        template <typename ASystemT,
                  typename... CArgTs>
        ASystemT* addSystem(CArgTs... args);

        /**
         * Get already added System.
         * @tparam ASystemT Type of the System.
         * @return Returns ptr to the System, if the System has not
         *   been added yet, nullptr is returned instead.
         * @remarks Not thread-safe!
         */
        template <typename ASystemT>
        ASystemT* getSystem();

        /**
         * Remove a registered System.
         * @tparam ASystemT Type of the System.
         * @return Returns true, if the system has been removed.
         * @remarks Not thread-safe!
         */
        template <typename ASystemT>
        bool removeSystem();

        /**
         * Add or get already created Entity group.
         * The pointer is guaranteed to be valid as long as the
         * Universe is instantiated - the Group will not be
         * moved around.
         * If Require and Reject have one or more same types,
         * the Require list has higher priority.
         * Order DOES matter, if there already is a group with
         * same filter, but the order of types inside Require/Reject
         * is different a new Group WILL be created. If debug is
         * enabled a warning message will be displayed, informing
         * about this.
         * @code
         * struct Comp1 {};
         * struct Comp2 {};
         * ...
         * using Require = ent::Require<Comp1>;
         * using Reject = ent::Reject<Comp2>;
         * EntityGroup *myGroup = addGetGroup<Require, Reject>();
         * @endcode
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns ptr to the requested Entity Group.
         * @remarks Not thread-safe!
         */
        template <typename RequireT,
            typename RejectT>
        EntityGroup *addGetGroup();

        /**
         * Register given Component and its ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs ComponentHolder constructor argument types.
         * @param args ComponentHolder constructor arguments, passed to the ComponentHolder on construction.
         * @return Returns ID of the Component.
         * @remarks Not thread-safe!
         */
        template <typename ComponentT,
            typename... CArgTs>
        u64 registerComponent(CArgTs... args);

        /**
         * Get bitset mask for given Component type.
         * @tparam ComponentT Type of the Component.
         * @return Mask containing one set bit. If the Component has not been registered the mask will contain
         *   zero set bits.
         * @remarks Thread-safe, if no other thread is registering Components.
         */
        template <typename ComponentT>
        inline const ComponentBitset &componentMask() const;

        /**
         * Check, if the given Component type has been registered.
         * @tparam ComponentT Type of the Component.
         * @return Returns true, if the Component has been registered.
         * @remarks Not thread-safe!
         */
        template <typename ComponentT>
        inline bool componentRegistered() const;

        /**
         * Add Component to the given Entity.
         * Immediate version, all actions are performed
         * immediately, including Entity metadata.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Constructor argument types.
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe! If thread-safety is required, use addComponentD.
         * @remarks Changes Entity metadata!
         * @remarks All pointers to Components of the same type may be invalidated!
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addComponent(EntityId id, CArgTs... cargs);

        /**
         * Add Component to the given Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Constructor argument types.
         * @param id Id of the Component
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT,
            typename... CArgTs>
        inline ComponentT *addComponentD(EntityId id, CArgTs... cargs);

        /**
         * Get Component associated with the given Entity.
         * Returns read-write ptr to the Component.
         * For read-only access, const version should
         * be called.
         * Returns Component with current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe in case of write access! If threa-safety
         *   is required, use getComponentD.
         */
        template <typename ComponentT>
        inline ComponentT *getComponent(EntityId id);

        /**
         * Get Component associated with the given Entity.
         * Returns read-only ptr to the Component.
         * Returns Component with current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         * @remarks Is thread-safe for cases, when on other thread
         *   has write access to the same Component.
         */
        template <typename ComponentT>
        inline const ComponentT *getComponent(EntityId id) const;

        /**
         * Get temporary Component, which can be safely
         * used for write access. The operation will be
         * finished on refresh.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline ComponentT *getComponentD(EntityId id);

        /**
         * Does the given Entity have Component associated with it?
         * Checks the current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns true, if there is such Component.
         * @remarks Is thread-safe, if no other thread is
         *   directly accessing entity metadata.
         */
        template <typename ComponentT>
        inline bool hasComponent(EntityId id) const;

        /**
         * Remove Component from given Entity.
         * If there is no Component associated with the Entity, nothing happens.
         * Operation is performed immediately.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata!
         */
        template <typename ComponentT>
        inline void removeComponent(EntityId id);

        /**
         * Remove Component from given Entity.
         * Operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline void removeComponentD(EntityId id);

        /**
         * Create a new Entity and return a handle to it.
         * Operation is performed immediately.
         * @return Handle to newly created Entity.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline EntityT createEntity();

        /**
         * Create a new Entity and return a handle to it.
         * Returned Entity is a temporary placeholder.
         * Actual Entity creation and any operations performed
         * on the temporary Entity will be performed on
         * refresh.
         * @return Returns handle to a temporary Entity.
         * @remarks Is thread-safe.
         */
        inline TempEntityT createEntityD();

        /**
         * Try to create Entity with given ID (without generation), if
         * the ID is free, the Entity will be created and the same Entity ID
         * will be returned. If the requested ID is invalid or unavailable, invalid
         * Entity ID will be returned instead.
         * @param id Requested ID.
         * @return Returns Entity with the same ID, or if the operation fails, returns
         *   Entity with "entity.created == false".
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline EntityT createEntity(EIdType id);

        /**
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * If the operation fails the returned sequence record will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline EntityHolder::SequenceRecord createSequentialEntities(u64 size);

        /**
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * Attempts to create sequence starting with startId, if that fails, returned sequence record
         * will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline EntityHolder::SequenceRecord createSequentialEntities(EIdType startId, u64 size);

        /**
         * Activate given Entity.
         * Action is performed immediately.
         * @param id ID of the Entity.
         * @remarks Does NOT check index bounds!
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline void activateEntity(EntityId id);

        /**
         * Activate given Entity.
         * Action is finished on refresh.
         * @param id ID of the Entity.
         * @remarks Is thread-safe
         */
        inline void activateEntityD(EntityId id);

        /**
         * Deactivate given Entity.
         * Action is performed immediately.
         * @param id ID of the Entity.
         * @remarks Does NOT check index bounds!
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline void deactivateEntity(EntityId id);

        /**
         * Deactivate given Entity.
         * Action is finished on refresh.
         * @param id ID of the Entity.
         * @remarks Is thread-safe.
         */
        inline void deactivateEntityD(EntityId id);

        /**
         * Destroy given Entity.
         * Action is performed immediately.
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline bool destroyEntity(EntityId id);

        /**
         * Destroy given Entity.
         * Action is finished on refresh.
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         * @remarks Is thread-safe.
         */
        inline bool destroyEntityD(EntityId id);

        /**
         * Checks validity of given Entity.
         * Checks the current state, excluding any thread-local changes.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity exists.
         * @remarks Is thread-safe, if no other thread is modifying
         *   entity metadata.
         */
        inline bool entityValid(EntityId id) const;

        /**
         * Checks if the given Entity is active.
         * Checks the current state, excluding any thread-local changes.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity is active.
         * @remarks Does NOT check index bounds!
         * @remarks Is thread-safe, if no other thread is modifying
         *   entity metadata.
         */
        inline bool entityActive(EntityId id) const;
    private:

#ifdef ENT_STATS_ENABLED
        /// Statistics for this Universe.
        UniverseStats mStats;
#endif

        /// Used for managing Entities and metadata.
        EntityManager<UniverseT> mEM;
        /// Used for managing Components and their Holders.
        ComponentManager<UniverseT> mCM;
        /// Used for managing Systems and Groups.
        SystemManager<UniverseT> mSM;
    protected:
    }; // Universe

    template <typename T>
    template <typename ASystemT,
              typename... CArgTs>
    ASystemT *Universe<T>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<SystemT, ASystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<ASystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");

        ASystemT *system{mSM.template addSystem<ASystemT>(this, std::forward<CArgTs>(args)...)};

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
        if (!mSM.template hasGroup<RequireT, RejectT>())
        {
            mSM.template addGroup<RequireT, RejectT>();
            if (LOG_STATS)
            {
                mStats.grpActive--;
                mStats.grpRemoved++;
                CHECK_STATS(mStats);
            }
        }

        return mSM.template getGroup<RequireT, RejectT>();
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
    Universe<T>::Universe() :
        mEM(), mCM(), mSM()
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
         * GroupManager:
         *  Empty added/removed lists.
         * ActionCache:
         *  Execute operations.
         *  Notify Groups with changed Entity IDs.
         * TODO - Order, refresh?
         */
        mEM.refresh();
        mCM.refresh();
        mSM.refresh();
    }

    template <typename T>
    void Universe<T>::reset()
    {
        mSM.reset();
        mEM.reset();
        mCM.reset();
    }

#ifdef OLD_UNUSED
    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::addComponent(EntityId id)
    { return mAC.template addComponent<ComponentT>(id); }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::getComponent(EntityId id)
    { return mCM.template get<ComponentT>(id); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::hasComponent(EntityId id)
    { return mCM.template has<ComponentT>(id); }

    template <typename T>
    template <typename ComponentT>
    void Universe<T>::removeComponent(EntityId id)
    { mAC.template removeComponent<ComponentT>(id); }

    template <typename T>
    template <typename ComponentT>
    const ComponentBitset &Universe<T>::componentMask()
    { return mCM.template mask<ComponentT>(); }

    template <typename T>
    template <typename ComponentT>
    bool Universe<T>::componentRegistered()
    { return mCM.template registered<ComponentT>(); }

    template <typename T>
    auto Universe<T>::createEntity() -> EntityT
    { return EntityT(this, mEM.create()); }

    template <typename T>
    void Universe<T>::activateEntity(EntityId id)
    { mEM.activate(id); }

    template <typename T>
    void Universe<T>::deactivateEntity(EntityId id)
    { mEM.deactivate(id); }

    template <typename T>
    bool Universe<T>::destroyEntity(EntityId id)
    { return mEM.destroy(id); }

    template <typename T>
    bool Universe<T>::entityValid(EntityId id) const
    { return mEM.valid(id); }

    template <typename T>
    bool Universe<T>::entityActive(EntityId id) const
    { return mEM.active(id); }
#endif

} // namespace ent

#endif //ECS_FIT_UNIVERSE_H
