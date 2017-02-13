/**
 * @file Entropy/Universe.h
 * @author Tomas Polasek
 * @brief Universe is the wrapper around all other parts of the Entropy ECS.
 */

#ifndef ECS_FIT_UNIVERSE_H
#define ECS_FIT_UNIVERSE_H

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
    class Universe : OnceInstantiable<Universe<T>>
    {
    public:
        using UniverseT = Universe<T>;
        using EntityT = Entity<UniverseT>;
        friend class Entity<UniverseT>;

        /**
         * Default constructor for Universe.
         * !!Only one instantiation is allowed per Universe type!!
         */
        Universe();

        /**
         * Register given System for this universe.
         * @tparam SystemT Type of the system, has to inherit from base ent::System.
         * @tparam CArgTs System constructor argument types.
         * @param args System constructor arguments, passed to the System on construction.
         * @return Returns reference to the newly registered System.
         */
        template <typename SystemT,
                  typename... CArgTs>
        SystemT& addSystem(CArgTs... args);

        /**
         * Remove a registered System.
         * @tparam SystemT Type of the System.
         */
        template <typename SystemT>
        void removeSystem();

        /**
         * Register given Component and its ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs ComponentHolder constructor argument types.
         * @param args ComponentHolder constructor arguments, passed to the ComponentHolder on construction.
         * @return Returns ID of the Component.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        u64 registerComponent(CArgTs... args);

        // System manager proxy methods.
    public:
    private:

        // Group manager proxy methods.
    public:
    private:

        // Component manager proxy methods.
    public:
        /**
         * Add Component to the given Entity.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         */
        template <typename ComponentT>
        inline ComponentT *addComponent(EntityId id);

        /**
         * Get Component associated with the given Entity.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns pointer to the Component.
         */
        template <typename ComponentT>
        inline ComponentT *getComponent(EntityId id);

        /**
         * Does the given Entity have Component associated with it?
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         * @return Returns true, if there is such Component.
         */
        template <typename ComponentT>
        inline bool hasComponent(EntityId id);

        /**
         * Remove Component from given Entity.
         * If there is no Component associated with the Entity, nothing happens.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Component
         */
        template <typename ComponentT>
        inline void removeComponent(EntityId id);

        /**
         * Get bitset mask for given Component type.
         * @tparam ComponentT Type of the Component.
         * @return Mask containing one set bit. If the Component has not been registered the mask will contain
         *   zero set bits.
         */
        template <typename ComponentT>
        inline const ComponentBitset &componentMask();
    private:

        // Entity manager proxy methods.
    public:
        /**
         * Create a new Entity and return a handle to it.
         * @return Handle to newly created Entity.
         */
        inline EntityT createEntity();

        /**
         * Try to create Entity with given ID (without generation), if
         * the ID is free, the Entity will be created and the same Entity ID
         * will be returned. If the requested ID is invalid or unavailable, invalid
         * Entity ID will be returned instead.
         * @param id Requested ID.
         * @return Returns Entity with the same ID, or if the operation fails, returns
         *   Entity with "entity.created == false".
         */
        inline EntityT createEntity(EIdType id);

        /**
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * If the operation fails the returned sequence record will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        inline EntityHolder::SequenceRecord createSequentialEntities(u64 size);

        /**
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * Attempts to create sequence starting with startId, if that fails, returned sequence record
         * will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        inline EntityHolder::SequenceRecord createSequentialEntities(EIdType startId, u64 size);

    private:
        /**
         * Activate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        inline void activateEntity(EntityId id);

        /**
         * Deactivate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        inline void deactivateEntity(EntityId id);

        /**
         * Destroy given Entity.
         * TODO - Should we actually check validity of Entity?
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         */
        inline bool destroyEntity(EntityId id);

        /**
         * Checks validity of given Entity.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity exists.
         */
        inline bool entityValid(EntityId id) const;

        /**
         * Checks if the given Entity is active.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         * @return Returns true, if the Entity is active.
         */
        inline bool entityActive(EntityId id) const;
    private:
#ifndef NDEBUG
        /**
         * Statistics about Universe.
         */
        struct UniverseStats
        {
            //friend std::ostream &operator<<(std::ostream &out, const UniverseStats &stats);
            bool working() const
            { return IS_DEBUG_BOOL; }
            u64 entActive;
            u64 entTotal;
            u64 entCreated;
            u64 entDestroyed;
        };
        /// Statistics instance.
        UniverseStats mStats;
#endif

        /// Flag for instantiation, only one is allowed.
        static bool mInstantiated;
        /// Used for managing Systems.
        SystemManager<UniverseT> mSM;
        /// Used for managing EntityGroups.
        GroupManager<UniverseT> mGM;
        /// Used for managing ComponentHolders.
        ComponentManager<UniverseT> mCM;
        /// Used for managing Entities.
        EntityManager<UniverseT> mEM;
    protected:
    }; // Universe

    template <typename T>
    bool Universe<T>::mInstantiated{false};

    template <typename T>
    template <typename SystemT,
              typename... CArgTs>
    SystemT &Universe<T>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<System, SystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<SystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");
        ENT_WARNING("Called unfinished method!");

        static SystemT sys(args...);
        return sys;
    }

    template <typename T>
    template<typename SystemT>
    void Universe<T>::removeSystem()
    {
        static_assert(std::is_base_of<System, SystemT>::value, "The System has to inherit from ent::System!");
        ENT_WARNING("Called unfinished method!");
    }

    template <typename T>
    template <typename ComponentT,
              typename... CArgTs>
    u64 Universe<T>::registerComponent(CArgTs... args)
    {
        static bool registered{false};
        // Check for multiple calls for single Component.
        if (registered)
        {
            ENT_WARNING("registerComponent called multiple times!");
        }

        static const u64 cId{mCM.registerComponent<ComponentT>(std::forward<CArgTs>(args)...)};

        registered = true;

        return cId;
    }

    template <typename T>
    Universe<T>::Universe()
    {
    }

    template <typename T>
    template <typename ComponentT>
    ComponentT *Universe<T>::addComponent(EntityId id)
    { return mCM.template add<ComponentT>(id); }

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
    { mCM.template remove<ComponentT>(id); }

    template <typename T>
    template <typename ComponentT>
    const ComponentBitset &Universe<T>::componentMask()
    { return mCM.template mask<ComponentT>(); }

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

} // namespace ent

#endif //ECS_FIT_UNIVERSE_H
