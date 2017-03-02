/**
 * @file Entropy/Component.h
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#ifndef ECS_FIT_COMPONENT_H
#define ECS_FIT_COMPONENT_H

#include "Types.h"
#include "Util.h"
#include "EntityId.h"
#include "EntityManager.h"
#include "Memory.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Base Component holder.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class BaseComponentHolder
    {
    public:
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual CompPtr add(EntityId id) noexcept = 0;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual CompPtr get(EntityId id) noexcept = 0;

        /**
         * Does given Entity have a Component associated with it?
         * @param id Id of the Entity.
         * @return Returns true, if there IS a Component associated.
         */
        virtual bool has(EntityId id) const noexcept = 0;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual void remove(EntityId id) noexcept = 0;
    private:
    protected:
    };

    /**
     * Default ComponentHolder with all required functionality.
     * Basic implementation.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolder final : public BaseComponentHolder<ComponentT>
    {
    public:
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

        /**
         * Default constructor.
         */
        ComponentHolder();

        /// Destructor
        ~ComponentHolder();

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual inline CompPtr add(EntityId id) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline CompPtr get(EntityId id) noexcept;

        /**
         * Does given Entity have an Component associated with it?
         * @param id Id of the Entity.
         * @return Returns true, if there IS an Component associated.
         */
        virtual inline bool has(EntityId id) const noexcept;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept;
    private:
        /// Mapping from EntityId to Component.
        std::map<EntityId, CompT> mMap;
    protected:
    };

    /**
     * ComponentHolder with std::map and a List.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolderList final : public BaseComponentHolder<ComponentT>
    {
    public:
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

        /**
         * Default constructor.
         */
        ComponentHolderList();

        /// Destructor
        ~ComponentHolderList();

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual inline CompPtr add(EntityId id) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline CompPtr get(EntityId id) noexcept;

        /**
         * Does given Entity have an Component associated with it?
         * @param id Id of the Entity.
         * @return Returns true, if there IS an Component associated.
         */
        virtual inline bool has(EntityId id) const noexcept;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept;
    private:
        /// Mapping from EntityId to index in the List.
        std::map<EntityId, u64> mMap;
        /// List containing the components.
        List<ComponentT> mList;
    protected:
    };

    /**
     * ComponentManager base class containing code which does not need to be templated.
     */
    class ComponentManagerBase : NonCopyable
    {
    public:
    private:
    protected:
    }; // ComponentManagerBase

    /**
     * ComponentManager is a part of Entropy ECS Universe.
     * Used for handling list of ComponentHolders.
     * Contains methods for adding new ComponentHolders and
     * their refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class ComponentManager final : public ComponentManagerBase
    {
    private:
        /**
         * Extract Component holder type from given Component.
         * Default value is ent::ComponentHolder.
         * This is the base case, where Component has no specified Holder type.
         * @tparam ComponentT Type of the Component.
         * @tparam Check SFINAE check.
         */
        template <typename ComponentT,
                  typename = void>
        struct HolderExtractor
        {
            using type = ent::ComponentHolder<ComponentT>;
        };

        /**
         * Extract Component holder type from given Component.
         * Default value is ent::ComponentHolder.
         * This is the case, when Component does have a Holder type specified.
         * @tparam ComponentT Type of the Component.
         * @tparam Check SFINAE check.
         */
        template <typename ComponentT>
        struct HolderExtractor<ComponentT,
            typename std::enable_if<
                std::is_base_of<ent::BaseComponentHolder<ComponentT>, typename ComponentT::HolderT>::value
            >::type>
        {
            using type = typename ComponentT::HolderT;
        };
    public:
        /**
         * Default constructor.
         * @param entityMgr Entity manager withing the same Universe.
         */
        ComponentManager(EntityManager<UniverseT> &entityMgr);

        /// Destructor.
        ~ComponentManager();

        /**
         * TODO - ComponentManager refresh
         */
        void refresh();

        /**
         * Register given Component with its ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam HolderT Type of the Holder, deduced from Component type.
         * @tparam CArgTs Constructor argument types.
         * @param args Constructor arguments passed to the Holder constructor.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type,
                  typename... CArgTs>
        inline u64 registerComponent(CArgTs... args);

        /**
         * Add Component for given Entity.
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         * @return Returns pointer to the added Component.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline ComponentT *add(EntityId id);

        /**
         * Get Component of given Entity.
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         * @return Returns pointer to the added Component.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline ComponentT *get(EntityId id);

        /**
         * Does given Entity have a Component?
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         * @return Returns true, if the Component is present.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline bool has(EntityId id);

        /**
         * Remove Component of given Entity.
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline void remove(EntityId id);

        /**
         * Get ID for given Component type.
         * @tparam ComponentT Component type.
         * @return ID of the Component type.
         */
        template <typename ComponentT>
        inline u64 id();

        /**
         * Get bitset mask for given Component type.
         * Mask contains at most one bit set to '1'.
         * If there are no bits set to '1', then the Component has not been registered.
         * @tparam ComponentT Type of the Component
         * @return Bitset mask for given Component type.
         */
        template <typename ComponentT>
        inline const ComponentBitset &mask() const;
    private:
        /// Component ID generator.
        class ComponentIdGenerator : public StaticClassIdGenerator<ComponentIdGenerator> {};

        /**
         * Get ComponentHolder.
         * @tparam HolderT Type of the holder.
         * @return Returns reference to the holder.
         */
        template <typename HolderT>
        inline HolderT &getHolder();
        template <typename HolderT>
        inline const HolderT &getHolder() const;

        /**
         * Construct Holder with given constructor parameters.
         * @tparam HolderT Type of the Holder.
         * @tparam CArgTs Constructor argument types.
         * @param args Constructor arguments.
         */
        template <typename HolderT,
                  typename... CArgTs>
        inline void initHolder(CArgTs... args);

        /**
         * Initialize bitset mask for given Component.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT>
        inline void initMask();

        /// Entity manager from the same Universe.
        EntityManager<UniverseT> &mEM;

        /**
         * Holder instance.
         * @tparam HolderT Type of the Holder.
         */
        template <typename HolderT>
        static ConstructionHandler<HolderT> mHolder;

        /**
         * Mask for Component type.
         * Only one bit is set.
         * If there are no bits set, the Component is not registered.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT>
        static ComponentBitset mMask;
    protected:
    }; // ComponentManager

    /**
     * ComponentFilter is used for filtering Entities by their present/missing Components.
     */
    class ComponentFilter final
    {
    public:
        /**
         * Create a filter from require and reject masks.
         * @param require Which bits need to be true.
         * @param reject Which bits must not be true, unless they are in the require mask.
         */
        ComponentFilter(const ComponentBitset &require,
                        const ComponentBitset &reject) :
            mRequire(require), mMask(require | reject) { }

        /**
         * Create a filter from require mask.
         * @param require Which bits need to be true.
         */
        ComponentFilter(const ComponentBitset &require) :
            mRequire(require), mMask(require) { }

        /**
         * Create a filter from require and reject masks.
         * @param require Which bits need to be true.
         * @param reject Which bits must not be true, unless they are in the require mask.
         */
        ComponentFilter(ComponentBitset&& require,
                        ComponentBitset&& reject) :
            mRequire(require), mMask(require | reject) { }

        ComponentFilter(const ComponentFilter &rhs) :
            mRequire{rhs.mRequire}, mMask{rhs.mMask} { }

        ComponentFilter &operator=(const ComponentFilter &rhs)
        {
            mRequire = rhs.mRequire;
            mMask = rhs.mMask;
            return *this;
        }

        /**
         * Check if the given bitset passes this filter.
         * @param bitset Bitset to check.
         * @return Returns true, if the bitset passes through this filter.
         */
        bool match(const ComponentBitset &bitset) const
        { return (bitset & mMask) == mRequire; }

        /// Comparison operator.
        bool operator==(const ComponentFilter &rhs) const
        { return (mRequire == rhs.mRequire) && (mMask == rhs.mMask); }

        /// Print operator.
        friend std::ostream &operator<<(std::ostream &out, const ComponentFilter &rhs);
    private:
        /// Bitset signifying which Component types need to be present.
        ComponentBitset mRequire;
        /// Bitset mask representing which bits are of interest.
        ComponentBitset mMask;
    protected:
    };

    // ComponentManager implementation.
    template <typename UT>
    ComponentManager<UT>::ComponentManager(EntityManager<UT> &entityMgr) :
        mEM(entityMgr)
    {
        static bool instantiated{false};
        if (instantiated)
        {
            ENT_WARNING("Class instantiated multiple times, correct functionality is compromised!");
        }
        instantiated = true;
    }

    template <typename UT>
    ComponentManager<UT>::~ComponentManager()
    { }

    template <typename UT>
    void ComponentManager<UT>::refresh()
    {
        ENT_WARNING("ComponentManager::refresh() is not finished yet!");
    }

    template <typename UT>
    template <typename ComponentT,
              typename HolderT,
              typename... CArgTs>
    u64 ComponentManager<UT>::registerComponent(CArgTs... args)
    {
        ENT_ASSERT_FAST(!mHolder<HolderT>.constructed());
        const u64 cId{id<ComponentT>()};

        ENT_ASSERT_FAST(cId < MAX_COMPONENTS); // Unable to register more Component types
        static_assert(std::is_base_of<ent::BaseComponentHolder<ComponentT>, HolderT>::value,
                      "Component holder has to inherit from ent::BaseComponentHolder!");
        static_assert(sizeof(HolderT(args...)), "Component holder has to be instantiable!");

        initHolder<HolderT>(std::forward<CArgTs>(args)...);
        initMask<ComponentT>();

        return cId;
    }

    template <typename UT>
    template <typename ComponentT>
    u64 ComponentManager<UT>::id()
    {
        static const u64 cId{ComponentIdGenerator::template getId<ComponentT>()};
        return cId;
    }

    template <typename UT>
    template <typename ComponentT>
    const ComponentBitset &ComponentManager<UT>::mask() const
    {
        return mMask<ComponentT>;
    }

    template <typename UT>
    template <typename ComponentT,
              typename HolderT>
    ComponentT *ComponentManager<UT>::add(EntityId id)
    {
        ComponentT *result{getHolder<HolderT>().add(id)};

        if (result)
        {
            mEM.template addComponent(id, ComponentManager<UT>::id<ComponentT>());
        }

        return result;
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    inline ComponentT *ComponentManager<UT>::get(EntityId id)
    {
        return getHolder<HolderT>().get(id);
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    inline bool ComponentManager<UT>::has(EntityId id)
    {
        return mEM.template hasComponent(id, ComponentManager<UT>::id<ComponentT>());
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    inline void ComponentManager<UT>::remove(EntityId id)
    {
        mEM.template removeComponent(id, ComponentManager<UT>::id<ComponentT>());

        return getHolder<HolderT>().remove(id);
    }

    template <typename UT>
    template <typename HolderT>
    inline HolderT &ComponentManager<UT>::getHolder()
    {
        return mHolder<HolderT>();
    }

    template <typename UT>
    template <typename HolderT>
    inline const HolderT &ComponentManager<UT>::getHolder() const
    {
        return mHolder<HolderT>();
    }

    template <typename UT>
    template <typename HolderT,
              typename... CArgTs>
    void ComponentManager<UT>::initHolder(CArgTs... args)
    {
        mHolder<HolderT>.construct(std::forward<CArgTs>(args)...);
    }

    template <typename UT>
    template <typename ComponentT>
    inline void ComponentManager<UT>::initMask()
    {
        ENT_ASSERT_FAST(mMask<ComponentT>.none());
        mMask<ComponentT>.set(id<ComponentT>());
    }

    template <typename UT>
    template <typename HolderT>
    ConstructionHandler<HolderT> ComponentManager<UT>::mHolder;

    template <typename UT>
    template <typename Component>
    ComponentBitset ComponentManager<UT>::mMask{0};
    // ComponentManager implementation end.

    // ComponentHolder implementation.
    template <typename ComponentT>
    ComponentHolder<ComponentT>::ComponentHolder()
    { }

    template <typename ComponentT>
    ComponentHolder<ComponentT>::~ComponentHolder()
    { }

    template <typename ComponentT>
    ComponentT* ComponentHolder<ComponentT>::add(EntityId id) noexcept
    {
        ComponentT* result{nullptr};
        try {
            result = &mMap[id];
        } catch(...) {
            //ENT_WARNING("Get has thrown an exception!");
        }
        return result;
    }

    template <typename ComponentT>
    ComponentT* ComponentHolder<ComponentT>::get(EntityId id) noexcept
    {
        ComponentT* result{nullptr};
        try {
            result = &mMap.at(id);
        } catch(...) {
            //ENT_WARNING("Get has thrown an exception!");
        }
        return result;
    }

    template <typename ComponentT>
    bool ComponentHolder<ComponentT>::has(EntityId id) const noexcept
    {
        return (mMap.find(id) != mMap.end());
    }

    template <typename ComponentT>
    void ComponentHolder<ComponentT>::remove(EntityId id) noexcept
    {
        try {
            mMap.erase(id);
        } catch(...) {
            //ENT_WARNING("Remove has thrown an exception!");
        }
    }
    // ComponentHolder implementation end.

    // ComponentHolderList implementation.
    template <typename CT>
    ComponentHolderList<CT>::ComponentHolderList()
    { }

    template <typename CT>
    ComponentHolderList<CT>::~ComponentHolderList()
    { }

    template <typename CT>
    CT* ComponentHolderList<CT>::add(EntityId id) noexcept
    {
        decltype(mMap.begin()) findIt{mMap.find(id)};
        if (findIt != mMap.end())
        {
            return &mList[findIt->second];
        }
        else
        {
            mMap[id] = mList.size();
            mList.pushBack();
            return &mList.back();
        }
    }

    template <typename CT>
    CT* ComponentHolderList<CT>::get(EntityId id) noexcept
    {
        decltype(mMap.begin()) findIt{mMap.find(id)};
        if (findIt != mMap.end())
        {
            return &mList[findIt->second];
        }

        return nullptr;
    }

    template <typename CT>
    bool ComponentHolderList<CT>::has(EntityId id) const noexcept
    {
        decltype(mMap.begin()) findIt{mMap.find(id)};
        if (findIt != mMap.end())
        {
            return true;
        }

        return false;
    }

    template <typename CT>
    void ComponentHolderList<CT>::remove(EntityId id) noexcept
    {
        decltype(mMap.begin()) findIt{mMap.find(id)};
        if (findIt != mMap.end())
        {
            mMap.erase(findIt);
        }
    }
    // ComponentHolderList implementation end.
} // namespace ent

#endif //ECS_FIT_COMPONENT_H
