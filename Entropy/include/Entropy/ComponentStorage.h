/**
 * @file Entropy/ComponentStorage.h
 * @author Tomas Polasek
 * @brief Component storage implementations.
 */

#ifndef ECS_FIT_COMPONENTSTORAGE_H
#define ECS_FIT_COMPONENTSTORAGE_H

#include "Types.h"
#include "Util.h"
#include "EntityId.h"
#include "Memory.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Base of teh BaseComponentHolder.
     * Contains virtual refresh method which will be called by the
     * ComponentManager.
     */
    class BaseComponentHolderBase
    {
    public:
        virtual ~BaseComponentHolderBase() { }

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual void refresh() noexcept = 0;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         * @return Returns true, if there are no more Components of this type
         *   for given Entity.
         */
        virtual bool remove(EntityId id) noexcept = 0;
    private:
    protected:
    }; // class BaseComponentHolderBase

    /**
     * Base Component holder, contains all the necessary methods to be implemented.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class BaseComponentHolder : public BaseComponentHolderBase
    {
    public:
        BaseComponentHolder() {};
        virtual ~BaseComponentHolder() {};

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual ComponentT *add(EntityId id) noexcept = 0;

        /**
         * Add/replace Component of given Entity with a copy of given Component.
         * @param id ID of the Entity.
         * @param comp Component to copy.
         * @return Return ptr to the Component.
         */
        virtual ComponentT *replace(EntityId id, const ComponentT &comp) noexcept = 0;

        /**
         * Optional operation for holders.
         *
         * Add Component for given EntityId, if the Component
         * already exists, It will be overwritten with element
         * constructed with given constructor parameters..
         * Pass constructor parameters to the Component on
         * construction.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the Component.
         */
        //template <typename... CArgTs>
        //inline ComponentT *add(EntityId id, CArgTs... cArgs) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * Returns a pointer o read-write Component.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual ComponentT *get(EntityId id) noexcept = 0;

        /**
         * Get Component belonging to given EntityId.
         * Returns a pointer o read-only Component!
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual const ComponentT *get(EntityId id) const noexcept = 0;
    private:
    protected:
    }; // class BaseComponentHolder

    /**
     * Default ComponentHolder with all required functionality.
     * Basic implementation.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolder final : public BaseComponentHolder<ComponentT>
    {
    public:
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
        virtual inline ComponentT *add(EntityId id) noexcept override;

        /**
         * Add/replace Component of given Entity with a copy of given Component.
         * @param id ID of the Entity.
         * @param comp Component to copy.
         * @return Return ptr to the Component.
         */
        virtual inline ComponentT *replace(EntityId id, const ComponentT &comp) noexcept override;

        /**
         * Add Component for given EntityId, if the Component
         * already exists, It will be overwritten with element
         * constructed with given constructor parameters.
         * Pass constructor parameters to the Component on
         * construction.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the Component.
         */
        template <typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline ComponentT *get(EntityId id) noexcept override;
        virtual inline const ComponentT *get(EntityId id) const noexcept override;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         * @return Returns true, if there are no more Components of this type
         *   for given Entity.
         */
        virtual inline bool remove(EntityId id) noexcept override;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept override;
    private:
        /// Mapping from EntityId to Component.
        std::map<EntityId, ComponentT> mMap;
    protected:
    }; // class ComponentHolder

    /**
     * ComponentHolder with std::map and a List.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolderMapList final : public BaseComponentHolder<ComponentT>
    {
    public:
        /**
         * Default constructor.
         */
        ComponentHolderMapList();

        /// Destructor
        ~ComponentHolderMapList();

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual inline ComponentT *add(EntityId id) noexcept override;

        /**
         * Add/replace Component of given Entity with a copy of given Component.
         * @param id ID of the Entity.
         * @param comp Component to copy.
         * @return Return ptr to the Component.
         */
        virtual inline ComponentT *replace(EntityId id, const ComponentT &comp) noexcept override;

        /**
         * Add Component for given EntityId, if the Component
         * already exists, It will be overwritten with element
         * constructed with given constructor parameters.
         * Pass constructor parameters to the Component on
         * construction.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the Component.
         */
        template <typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline ComponentT *get(EntityId id) noexcept override;
        virtual inline const ComponentT *get(EntityId id) const noexcept override;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         * @return Returns true, if there are no more Components of this type
         *   for given Entity.
         */
        virtual inline bool remove(EntityId id) noexcept override;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept override;
    private:
        /**
         * Get already existing index, or create a new element.
         * @param id ID of the Entity.
         * @return Returns index of the element in
         *   mList for given Entity.
         */
        u64 getCreateIndex(EntityId id);

        /// Mapping from EntityId to index in the List.
        std::map<EntityId, u64> mMapping;
        /// List of free IDs.
        List<u64> mFreeIds;
        /// List containing the components.
        List<ComponentT> mList;
    protected:
    }; // ComponentHolderMapList

    /**
     * ComponentHolder with a List.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolderList final : public BaseComponentHolder<ComponentT>
    {
    public:
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
        virtual inline ComponentT *add(EntityId id) noexcept override;

        /**
         * Add/replace Component of given Entity with a copy of given Component.
         * @param id ID of the Entity.
         * @param comp Component to copy.
         * @return Return ptr to the Component.
         */
        virtual inline ComponentT *replace(EntityId id, const ComponentT &comp) noexcept override;

        /**
         * Add Component for given EntityId, if the Component
         * already exists, It will be overwritten with element
         * constructed with given constructor parameters.
         * Pass constructor parameters to the Component on
         * construction.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the Component.
         */
        template <typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline ComponentT *get(EntityId id) noexcept override;
        virtual inline const ComponentT *get(EntityId id) const noexcept override;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         * @return Returns true, if there are no more Components of this type
         *   for given Entity.
         */
        virtual inline bool remove(EntityId id) noexcept override;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept override;
    private:
        /// List containing the components.
        List<ComponentT> mList;
    protected:
    }; // ComponentHolderMapList
} // namespace ent

#include "ComponentStorage.inl"

#endif //ECS_FIT_COMPONENTSTORAGE_H
