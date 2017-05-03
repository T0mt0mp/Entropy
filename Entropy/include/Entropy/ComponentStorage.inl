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
        }
        return result;
    }

    template <typename ComponentT>
    ComponentT *ComponentHolder<ComponentT>::replace(EntityId id, const ComponentT &comp) noexcept
    {
        return add(id, std::forward<const ComponentT&>(comp));
    }

    template <typename ComponentT>
    template <typename... CArgTs>
    ComponentT *ComponentHolder<ComponentT>::add(EntityId id, CArgTs... cArgs) noexcept
    {
        ComponentT* result{nullptr};
        try {
            //result = &(mMap.try_emplace(id, std::forward<CArgTs>(cArgs)...).first->second);
            result = &(mMap.emplace(id, std::move(ComponentT(std::forward<CArgTs>(cArgs)...))).first->second);
        } catch(...) {
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
            u64 index{getCreateIndex(id)};

            return &mList[index];
        } catch (...) {
            return nullptr;
        }
    }

    template <typename CT>
    inline CT *ComponentHolderMapList<CT>::replace(EntityId id, const CT &comp) noexcept
    {
        return add(id, std::forward<const CT&>(comp));
    }

    template <typename CT>
    template <typename... CArgTs>
    CT *ComponentHolderMapList<CT>::add(EntityId id, CArgTs... cArgs) noexcept
    {
        try {
            u64 index{getCreateIndex(id)};

            mList.emplace(mList.begin() + index, std::forward<CArgTs>(cArgs)...);

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
    u64 ComponentHolderMapList<CT>::getCreateIndex(EntityId id)
    {
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

        return index;
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
    inline CT *ComponentHolderList<CT>::replace(EntityId id, const CT &comp) noexcept
    {
        if (id.index() >= mList.size())
        {
            try {
                mList.resize(id.index() + 1);
            } catch(...) {
                return nullptr;
            }
        }

        CT *result{&mList[id.index()]};
        *result = comp;

        return result;
        //return add(id, std::forward<const CT&>(comp));
    }

    template <typename CT>
    template <typename... CArgTs>
    CT *ComponentHolderList<CT>::add(EntityId id, CArgTs... cArgs) noexcept
    {
        if (id.index() >= mList.size())
        {
            try {
                mList.resize(id.index() + 1);
            } catch(...) {
                return nullptr;
            }
        }

        mList.emplace(mList.begin() + id.index(), std::forward<CArgTs>(cArgs)...);

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
