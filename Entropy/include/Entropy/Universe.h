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
    class Universe : OnceInstantiable<Universe<T>>
    {
    public:
        using UniverseT = Universe<T>;
        using EntityT = Entity<UniverseT>;
        using SystemT = System<UniverseT>;

        friend class Entity<UniverseT>;

        /**
         * Default constructor for Universe.
         * !!Only one instantiation is allowed per Universe type!!
         */
        Universe();

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

        // System manager proxy methods.
    public:
        /**
         * Register given System for this universe.
         * @tparam ASystemT Type of the system, has to inherit from base ent::System.
         * @tparam CArgTs System constructor argument types.
         * @param args System constructor arguments, passed to the System on construction.
         * @return Returns ptr to the newly registered System.
         */
        template <typename ASystemT,
                  typename... CArgTs>
        ASystemT* addSystem(CArgTs... args);

        /**
         * Get already added System.
         * @tparam ASystemT Type of the System.
         * @return Returns ptr to the System, if the System has not
         *   been added yet, nullptr is returned instead.
         */
        template <typename ASystemT>
        ASystemT* getSystem();

        /**
         * Remove a registered System.
         * @tparam ASystemT Type of the System.
         */
        template <typename ASystemT>
        void removeSystem();
    private:

        // Group manager proxy methods.
    public:
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
         */
        template <typename RequireT,
                  typename RejectT>
        EntityGroup *addGetGroup();
    private:

        // Component manager proxy methods.
    public:
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
        /// Flag representing the state of this Universe.
        static bool mInitialized;
        /// Used for managing Entities.
        EntityManager<UniverseT> mEM;
        /// Used for managing ComponentHolders.
        ComponentManager<UniverseT> mCM;
        /// Used for managing EntityGroups.
        GroupManager<UniverseT> mGM;
        /// Used for managing Systems.
        SystemManager<UniverseT> mSM;
        /// Used for managing actions.
        ActionCache<UniverseT> mAC;
    protected:
    }; // Universe

    template <typename T>
    bool Universe<T>::mInstantiated{false};

    template <typename T>
    bool Universe<T>::mInitialized{false};

    template <typename T>
    template <typename ASystemT,
              typename... CArgTs>
    ASystemT *Universe<T>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<SystemT, ASystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<ASystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");

        ASystemT *system{mSM.addSystem<ASystemT>(this, std::forward<CArgTs>(args)...)};

        return system;
    }

    template <typename T>
    template <typename ASystemT>
    ASystemT *Universe<T>::getSystem()
    {
        return mSM.getSystem<ASystemT>();
    }

    template <typename T>
    template<typename ASystemT>
    void Universe<T>::removeSystem()
    {
        static_assert(std::is_base_of<SystemT, ASystemT>::value, "The System has to inherit from ent::System!");
        ENT_WARNING("Called unfinished method!");
    }

    template <typename T>
    template <typename RequireT,
              typename RejectT>
    EntityGroup *Universe<T>::addGetGroup()
    {
        return mGM.addGetGroup<RequireT, RejectT>();
    }

    template <typename T>
    template <typename ComponentT,
              typename... CArgTs>
    u64 Universe<T>::registerComponent(CArgTs... args)
    {
        // Adding Component types is only allowed before initialization.
        ENT_ASSERT_FAST(!mInitialized);

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
    Universe<T>::Universe() :
        mEM(),
        mCM(mEM),
        mGM(mEM, mCM),
        mSM(mGM),
        mAC(mEM, mCM, mGM, mSM)
    {
    }

    template <typename T>
    void Universe<T>::init()
    {
        ENT_ASSERT_FAST(!mInitialized);



        refresh();
        mInitialized = true;
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
        mGM.refresh();
        mAC.refresh();
        mEM.refresh();
        mCM.refresh();
        mSM.refresh();
    }

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
