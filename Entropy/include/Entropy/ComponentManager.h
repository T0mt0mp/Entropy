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

    /**
     * ComponentManager is a part of Entropy ECS Universe.
     * Used for handling list of ComponentHolders.
     * Contains methods for adding new ComponentHolders and
     * their refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class ComponentManager final : NonCopyable
    {
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
         * @param cArgs Constructor arguments passed to the Holder constructor.
         */
        template <typename ComponentT,
                  typename HolderT = typename HolderExtractor<ComponentT>::type,
                  typename... CArgTs>
        inline CIdType registerComponent(CArgTs... cArgs);

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
         * Add Component for given Entity.
         * Pass constructor parameters to the holder.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the added Component.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs);

        /**
         * Add or replace a Component for given Entity.
         * Pass constructor parameters to the holder.
         * @tparam ComponentT Type of the Component.
         * @param id ID of the Entity.
         * @param comp Component data.
         * @return Returns pointer to the changed Component.
         */
        template <typename ComponentT>
        inline ComponentT *replace(EntityId id, const ComponentT &comp);

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
        inline CIdType id() const;

        /**
         * Check, if given Component type is registered in this manager.
         * @tparam ComponentT Type of the Component.
         * @return Returns true, if the Component has been registered.
         */
        template <typename ComponentT>
        inline bool registered() const;
    private:
        /**
         * Information about registered Component.
         * @tparam HolderT Type of the Component holder.
         */
        template <typename HolderT>
        struct ComponentRegister
        {
            ComponentRegister() :
                id{0u}
            { }

            /// ID of the Component.
            CIdType id;

            /// Used for storing Component data.
            ConstructionHandler<HolderT> holder;
        };

        /// Reset the Component ID counter to 0.
        static void resetComponentIdCounter()
        { sComponentIdCounter = 0; }

        /// Get new unique Component ID and increment the counter.
        static CIdType compIdInc()
        {
            ENT_ASSERT_FAST(sComponentIdCounter < ENT_MAX_COMPONENTS);
            return sComponentIdCounter++;
        }

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
        std::vector<std::function<void()>> mDestructOnReset;

        /// Counter for Component IDs.
        static CIdType sComponentIdCounter;
    protected:
    }; // ComponentManager
} // namespace ent

#include "ComponentManager.inl"

#endif //ECS_FIT_COMPONENTMANAGER_H
