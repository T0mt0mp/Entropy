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

        BaseComponentHolder() {};
        virtual ~BaseComponentHolder() {};

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
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual void remove(EntityId id) noexcept = 0;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept = 0;
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
        virtual inline CompPtr add(EntityId id) noexcept override;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline CompPtr get(EntityId id) noexcept override;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept override;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept override;
    private:
        /// Mapping from EntityId to Component.
        std::map<EntityId, CompT> mMap;
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
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

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
        virtual inline CompPtr add(EntityId id) noexcept override;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline CompPtr get(EntityId id) noexcept override;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept override;

        /**
         * Refresh the Component holder.
         * Called during the Universe refresh.
         */
        virtual inline void refresh() noexcept override;
    private:
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
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept;

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
    void ComponentHolder<ComponentT>::refresh() noexcept
    {
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

    // ComponentHolderMapList implementation.
    template <typename CT>
    ComponentHolderMapList<CT>::ComponentHolderMapList()
    { }

    template <typename CT>
    ComponentHolderMapList<CT>::~ComponentHolderMapList()
    { }

    template <typename CT>
    CT* ComponentHolderMapList<CT>::add(EntityId id) noexcept
    {
        try {
            u64 &index{mMapping[id]};

            if (index == 0)
            {
                if (mFreeIds.size() != 0)
                { // There is a free ID.
                    index = mFreeIds.back();
                    mFreeIds.popBack();
                }
                else
                { // There are no free IDs.
                    index = mList.size();
                    mList.pushBack();
                }
            }

            return &mList[index];
        } catch (...) {
            return nullptr;
        }
    }

    template <typename CT>
    CT* ComponentHolderMapList<CT>::get(EntityId id) noexcept
    {
        try {
            return &mList.at(mMapping.at(id.index()));
        } catch (...) {
            return nullptr;
        }
    }

    template <typename CT>
    void ComponentHolderMapList<CT>::refresh() noexcept
    {
    }

    template <typename CT>
    void ComponentHolderMapList<CT>::remove(EntityId id) noexcept
    {
        try {
            u64 &index = mMapping.at(id.index());

            mFreeIds.pushBack(index);

            index = 0;
        } catch (...) {
            return;
        }
    }
    // ComponentHolderMapList implementation end.

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
        if (id.index() >= mList.size())
        {
            try {
                mList.resize(id.index() + 1);
            } catch(...) {
                return nullptr;
            }
        }

        return &mList[id.index()];
    }

    template <typename CT>
    CT* ComponentHolderList<CT>::get(EntityId id) noexcept
    {
        if (id.index() < mList.size())
        {
            return &mList[id.index()];
        }
        else
        {
            return nullptr;
        }
    }

    template <typename CT>
    void ComponentHolderList<CT>::refresh() noexcept
    {
    }

    template <typename CT>
    void ComponentHolderList<CT>::remove(EntityId id) noexcept
    {

    }
    // ComponentHolderList implementation end.
} // namespace ent

#endif //ECS_FIT_COMPONENTSTORAGE_H
