/**
 * @file Entropy/ComponentStorage.inl
 * @author Tomas Polasek
 * @brief Component storage implementations.
 */

#include "ComponentStorage.h"

/// Main Entropy namespace
namespace ent
{
    // ComponentHolder implementation.
    template <typename ComponentT>
    ComponentHolder<ComponentT>::ComponentHolder()
    { }

    template <typename ComponentT>
    ComponentHolder<ComponentT>::~ComponentHolder()
    { }

    template <typename ComponentT>
    ComponentT *ComponentHolder<ComponentT>::add(EntityId id) noexcept
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
    ComponentT *ComponentHolder<ComponentT>::get(EntityId id) noexcept
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
    const ComponentT *ComponentHolder<ComponentT>::get(EntityId id) const noexcept
    { return const_cast<ComponentHolder<ComponentT>*>(this)->get(id); }

    template <typename ComponentT>
    void ComponentHolder<ComponentT>::refresh() noexcept
    {
    }

    template <typename ComponentT>
    bool ComponentHolder<ComponentT>::remove(EntityId id) noexcept
    {
        try {
            mMap.erase(id);
            return true;
        } catch(...) {
            //ENT_WARNING("Remove has thrown an exception!");
        }
        return false;
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
    CT *ComponentHolderMapList<CT>::add(EntityId id) noexcept
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
    CT *ComponentHolderMapList<CT>::get(EntityId id) noexcept
    {
        try {
            return &mList.at(mMapping.at(id.index()));
        } catch (...) {
            return nullptr;
        }
    }

    template <typename CT>
    const CT *ComponentHolderMapList<CT>::get(EntityId id) const noexcept
    { return const_cast<ComponentHolderMapList<CT>*>(this)->get(id); }

    template <typename CT>
    void ComponentHolderMapList<CT>::refresh() noexcept
    {
    }

    template <typename CT>
    bool ComponentHolderMapList<CT>::remove(EntityId id) noexcept
    {
        try {
            u64 &index = mMapping.at(id.index());

            mFreeIds.pushBack(index);

            index = 0;
            return true;

        } catch (...) {
        }
        return false;
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
    CT *ComponentHolderList<CT>::add(EntityId id) noexcept
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
    CT *ComponentHolderList<CT>::get(EntityId id) noexcept
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
    const CT *ComponentHolderList<CT>::get(EntityId id) const noexcept
    { return const_cast<ComponentHolderList<CT>*>(this)->get(id); }

    template <typename CT>
    void ComponentHolderList<CT>::refresh() noexcept
    {
    }

    template <typename CT>
    bool ComponentHolderList<CT>::remove(EntityId id) noexcept
    {
        return true;
    }
    // ComponentHolderList implementation end.
} // namespace ent
