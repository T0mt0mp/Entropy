/**
 * @file Entropy/ComponentManager.h
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#ifndef ECS_FIT_COMPONENTMANAGER_H
#define ECS_FIT_COMPONENTMANAGER_H

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
        ComponentManager();

        /// Destructor.
        ~ComponentManager();

        /**
         * Refresh all Component Holders.
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
        template <typename ComponentT>
        inline ComponentT *add(EntityId id);

        /**
         * Get Component of given Entity.
         * Returns pointer to read-write Component.
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         * @return Returns pointer to the added Component.
         */
        template <typename ComponentT>
        inline ComponentT *get(EntityId id);

        /**
         * Get Component of given Entity.
         * Returns pointer to read-only Component!
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         * @param id Id of the Entity.
         * @return Returns pointer to the added Component.
         */
        template <typename ComponentT>
        inline const ComponentT *get(EntityId id) const;

        /**
         * Remove Component of given Entity.
         * @tparam ComponentT Component type.
         * @tparam HolderT Type of the holder, deduced from Component type.
         */
        template <typename ComponentT>
        inline bool remove(EntityId id);

        /**
         * Get ID for given Component type.
         * @tparam ComponentT Component type.
         * @return ID of the Component type.
         */
        template <typename ComponentT>
        inline u64 id() const;

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
        { return componentInfo<ComponentT>().constructed(); }
    private:
        /**
         * Information about registered Component.
         * @tparam HolderT Type of the Component holder.
         */
        template <typename HolderT>
        struct ComponentRegister
        {
            ComponentRegister() :
                mask{0u}, id{0u}
            { }

            /// Mask of the Component.
            ComponentBitset mask;
            /// ID of the Component.
            u64 id;

            /// Used for storing Component data.
            ConstructionHandler<HolderT> holder;
        };

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
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline HolderT &getHolder();
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
        inline const HolderT &getHolder() const;

        /**
         * Component information register.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type>
		static ConstructionHandler<ComponentRegister<HolderT>> &componentInfo()
		{
			static ConstructionHandler<ComponentRegister<HolderT>> reg;
			return reg;
		}

        /// List of holders to refresh.
        std::vector<BaseComponentHolderBase*> mRefreshHolders;

        /// List of things to destruct on reset.
        std::vector<ConstructionHandlerBase*> mDestructOnReset;

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
} // namespace ent

#include "ComponentManager.inl"

#endif //ECS_FIT_COMPONENTMANAGER_H
