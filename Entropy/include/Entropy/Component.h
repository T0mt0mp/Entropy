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
#include "ComponentStorage.h"
#include "EntityId.h"
#include "EntityManager.h"

/// Main Entropy namespace
namespace ent
{
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
         * Reset Component holders.
         */
        void reset();

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

        /**
         * Check, if given Component type is registered in this manager.
         * @tparam ComponentT Type of the Component.
         * @return Returns true, if the Component has been registered.
         */
        template <typename ComponentT>
        bool registered() const
        //{ return mRegistered<ComponentT>; }
        { return registeredGetter<ComponentT>(); }
    private:
        /// Reset the Component ID counter to 0.
        static void resetComponentIdCounter()
        { sComponentIdCounter = 0; }

        /// Get new unique Component ID and increment the counter.
        static u64 compIdInc()
        { return sComponentIdCounter++; }

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

        /**
         * Initialize the ID for given Component type.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT>
        inline void initId();

        /// Entity manager from the same Universe.
        EntityManager<UniverseT> &mEM;

        /**
         * Holder instance.
         * @tparam HolderT Type of the Holder.
         */
		/*
        template <typename HolderT>
        static ConstructionHandler<HolderT> mHolder;
		*/
        template <typename HolderT>
        static ConstructionHandler<HolderT> &holderGetter()
		{
			static ConstructionHandler<HolderT> handler;
			return handler;
		}

        /**
         * Mask for Component type.
         * Only one bit is set.
         * If there are no bits set, the Component is not registered.
         * @tparam ComponentT Type of the Component.
         */
		/*
        template <typename ComponentT>
        static ComponentBitset mMask;
		*/
        template <typename ComponentT>
		static ComponentBitset &maskGetter()
		{
			static ComponentBitset bitset{0u};
			return bitset;
		}

        /**
         * Flag for checking, if given Component type is registered.
         * @tparam ComponentT Type of the Component.
         */
		/*
        template <typename ComponentT>
        static bool mRegistered;
		*/
        template <typename ComponentT>
		static bool &registeredGetter()
		{
			static bool reg{false};
			return reg;
		}

        /**
         * Get ID for given Component type.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT>
        static u64 &idGetter()
        {
            static u64 innerId{0};
            return innerId;
        }

        /// List of reset functions for Component registration.
        std::vector<std::function<void()>> mComponentResets;

        /// Counter for Component IDs.
        static u64 sComponentIdCounter;
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
    u64 ComponentManager<UT>::sComponentIdCounter{0};

    template <typename UT>
    ComponentManager<UT>::ComponentManager(EntityManager<UT> &entityMgr) :
        mEM(entityMgr)
    {
    }

    template <typename UT>
    ComponentManager<UT>::~ComponentManager()
    {
        reset();
    }

    template <typename UT>
    void ComponentManager<UT>::refresh()
    {
        ENT_WARNING("ComponentManager::refresh() is not finished yet!");
    }

    template <typename UT>
    void ComponentManager<UT>::reset()
    {
        for (auto &l : mComponentResets)
        {
            l();
        }
        mComponentResets.clear();

        resetComponentIdCounter();
    }

    template <typename UT>
    template <typename ComponentT,
              typename HolderT,
              typename... CArgTs>
    u64 ComponentManager<UT>::registerComponent(CArgTs... args)
    {
        //ENT_ASSERT_FAST(!mHolder<HolderT>.constructed());
        ENT_ASSERT_FAST(!holderGetter<HolderT>().constructed());

        initId<ComponentT>();

        const u64 cId{id<ComponentT>()};

        ENT_ASSERT_FAST(cId < MAX_COMPONENTS); // Unable to register more Component types
        static_assert(std::is_base_of<ent::BaseComponentHolder<ComponentT>, HolderT>::value,
                      "Component holder has to inherit from ent::BaseComponentHolder!");
        static_assert(sizeof(HolderT(args...)), "Component holder has to be instantiable!");

        initHolder<HolderT>(std::forward<CArgTs>(args)...);
        initMask<ComponentT>();

        mComponentResets.emplace_back([] () {
            holderGetter<HolderT>().destruct();
            maskGetter<ComponentT>().reset();
            idGetter<ComponentT>() = 0;
            registeredGetter<ComponentT>() = false;
        });

        return cId;
    }

    template <typename UT>
    template <typename ComponentT>
    u64 ComponentManager<UT>::id()
    {
        return idGetter<ComponentT>();
    }

    template <typename UT>
    template <typename ComponentT>
    const ComponentBitset &ComponentManager<UT>::mask() const
    {
        //return mMask<ComponentT>;
        return maskGetter<ComponentT>();
    }

    template <typename UT>
    template <typename ComponentT,
              typename HolderT>
    ComponentT *ComponentManager<UT>::add(EntityId id)
    {
        ComponentT *result{getHolder<HolderT>().add(id)};

        if (result)
        {
            mEM.addComponent(id, ComponentManager<UT>::id<ComponentT>());
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
        return mEM.hasComponent(id, ComponentManager<UT>::id<ComponentT>());
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    inline void ComponentManager<UT>::remove(EntityId id)
    {
        mEM.removeComponent(id, ComponentManager<UT>::id<ComponentT>());

        return getHolder<HolderT>().remove(id);
    }

    template <typename UT>
    template <typename HolderT>
    inline HolderT &ComponentManager<UT>::getHolder()
    {
        //return mHolder<HolderT>();
        return holderGetter<HolderT>()();
    }

    template <typename UT>
    template <typename HolderT>
    inline const HolderT &ComponentManager<UT>::getHolder() const
    {
        //return mHolder<HolderT>();
        return holderGetter<HolderT>()();
    }

    template <typename UT>
    template <typename HolderT,
              typename... CArgTs>
    void ComponentManager<UT>::initHolder(CArgTs... args)
    {
        //mHolder<HolderT>.construct(std::forward<CArgTs>(args)...);
        holderGetter<HolderT>().construct(std::forward<CArgTs>(args)...);
    }

    template <typename UT>
    template <typename ComponentT>
    inline void ComponentManager<UT>::initMask()
    {
        // Check, that the Component has not been already registered.
        //ENT_ASSERT_FAST(mMask<ComponentT>.none() && mRegistered<ComponentT> == false);
        ENT_ASSERT_FAST(maskGetter<ComponentT>().none() && registeredGetter<ComponentT>() == false);
        //mMask<ComponentT>.set(id<ComponentT>());
        maskGetter<ComponentT>().set(id<ComponentT>());
        //mRegistered<ComponentT> = true;
        registeredGetter<ComponentT>() = true;
    }

    template <typename UT>
    template <typename ComponentT>
    inline void ComponentManager<UT>::initId()
    {
        idGetter<ComponentT>() = compIdInc();
    }

	/*
    template <typename UT>
    template <typename HolderT>
    ConstructionHandler<HolderT> ComponentManager<UT>::mHolder;

    template <typename UT>
    template <typename Component>
    ComponentBitset ComponentManager<UT>::mMask{0};

    template <typename UT>
    template <typename Component>
    bool ComponentManager<UT>::mRegistered{false};
	*/
    // ComponentManager implementation end.
} // namespace ent

#endif //ECS_FIT_COMPONENT_H
