/**
 * @file Entropy/Universe.h
 * @author Tomas Polasek
 * @brief Universe is the wrapper around all other parts of the Entropy ECS.
 */

#ifndef ECS_FIT_UNIVERSE_H
#define ECS_FIT_UNIVERSE_H

#include "EntityManager.h"
#include "ComponentManager.h"
#include "GroupManager.h"
#include "SystemManager.h"
#include "ActionsCache.h"

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
        using TempEntityT = void; // TODO - TemporaryEntity
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
         * @remarks Not thread-safe!
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
         * @remarks Not thread-safe!
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
         * @remarks Each time this method is called the corresponding
         *   Group usage counter is incremented, which can result
         *   in Group existing even after destruction of its System.
         *   To remedy this, after each "addGetGroup" call there should
         *   be also "abandon" call on the returned Group.
         */
        template <typename RequireT,
            typename RejectT>
        EntityGroup *addGetGroup();

        /**
         * Decrease the usage counter for given EntityGroup.
         * If the counter reached zero, the Group will be
         * removed on next refresh.
         * @tparam RequireT List of required Component types.
         * @tparam RejectT List of rejected Component types.
         * @return Returns true, if the group reached zero, on
         *   then usage counter. Returns true, only the first
         *   time zero is reached, any following calls have no effect.
         * @remarks Is only thread-safe for EntityGroups created
         *   using addGetGroup, which are also not in used
         *   by any Systems.
         */
        template <typename RequireT,
            typename RejectT>
        bool abandonGroup();

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
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe! If thread-safety is required, use addComponentD.
         * @remarks Changes Entity metadata!
         * @remarks All pointers to Components of the same type may be invalidated!
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addComponent(EntityId id, CArgTs... cargs);

        /**
         * Add Component to the given Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addComponentD(EntityId id);

        /**
         * Add Component to the given Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Component constructor argument types.
         * @param id Id of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
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
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe in case of write access! If thread-safety
         *   is required, use getComponentD.
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline ComponentT *getComponent(EntityId id);

        /**
         * Get Component associated with the given Entity.
         * Returns read-only ptr to the Component.
         * Returns Component with current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         * @remarks Is thread-safe for cases, when on other thread
         *   has write access to the same Component.
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline const ComponentT *getComponent(EntityId id) const;

        /**
         * Get temporary Component, which can be safely
         * used for write access. The operation will be
         * finished on refresh.
         * Temporary Component has to be added first.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Returned pointer is valid until a new Component
         *   of the same type is added/removed, or the ChangeSet is
         *   committed.
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
         * Check, if there is a temporary Component prepared.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
         * @return Returns true, if there is a temporary
         *   Component of given type.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline bool hasComponentD(EntityId id) const;

        /**
         * Remove Component from given Entity.
         * If there is no Component associated with the Entity, nothing happens.
         * Operation is performed immediately.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
         * @return Returns true, if the Component has been successfully removed.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata!
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        template <typename ComponentT>
        inline bool removeComponent(EntityId id);

        /**
         * Remove Component from given Entity.
         * Operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
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
         * @remarks Default behavior, if Entity could not be created, is
         *   returning invalid Entity (Entity.valid() == false). Method can
         *   also throw exception of type std::runtime_error, if such
         *   result is require, macro ENT_ENTITY_EXCEPT should be defined.
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

#ifdef ENT_NOT_USED

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

#endif

        /**
         * Activate given Entity.
         * Action is performed immediately.
         * @param id ID of the Entity.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
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
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
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
         * @return Returns false, if the Entity could not
         *   be destroyed.
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

        /**
         * Commit actions stored in the ChangeSet of the
         * current thread.
         * @remarks Is thread-safe, using a mutex.
         */
        inline void commitChangeSet();
    private:
        /**
         * Reset parts of this Universe.
         * @remarks Not thread-safe!
         */
        void resetSelf();

        /**
         * Called, when an Entity has changed and its membership
         * in groups should be rechecked.
         * @param id ID of the Entity.
         * @remarks Not thread-safe!
         */
        void entityChanged(EntityId id);

        /// Statistics for this Universe.
        UniverseStats mStats;

        /// Used for managing Entities and metadata.
        EntityManager mEM;
        /// Used for managing Components and their Holders.
        ComponentManager<UniverseT> mCM;
        /// Used for Entity Group management.
        GroupManager<UniverseT> mGM;
        /// Used for managing Systems and Groups.
        SystemManager<UniverseT> mSM;
        /// Actions cache for storing actions to be performed at a later time.
        ActionsCache<UniverseT> mAC;

        /// List of changed Entities since the last refresh.
        SortedList<EntityId> mChanged;
    protected:
    }; // Universe
} // namespace ent

#include "Universe.inl"

#endif //ECS_FIT_UNIVERSE_H
