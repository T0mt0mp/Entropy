/**
 * @file Entropy/ComponentManager.inl
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#include "ComponentManager.h"

/// Main Entropy namespace
namespace ent
{
    // ComponentManager implementation.
    template <typename UT>
    u64 ComponentManager<UT>::sComponentIdCounter{0};

    template <typename UT>
    ComponentManager<UT>::ComponentManager()
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
        for (BaseComponentHolderBase *h: mRefreshHolders)
        { // Refresh all the Component holders.
            h->refresh();
        }
    }

    template <typename UT>
    void ComponentManager<UT>::reset()
    {
        for (auto &h : mDestructOnReset)
        {
            h->destruct();
        }
        mDestructOnReset.clear();

        mRefreshHolders.clear();

        resetComponentIdCounter();
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT,
        typename... CArgTs>
    u64 ComponentManager<UT>::registerComponent(CArgTs... args)
    {
        static_assert(std::is_base_of<ent::BaseComponentHolder<ComponentT>, HolderT>::value,
                      "Component holder has to inherit from ent::BaseComponentHolder!");
        static_assert(sizeof(HolderT(std::declval<CArgTs>()...)), "Component holder has to be constructible!");

        if (registered<ComponentT>())
        { // Prevent double register.
            return id<ComponentT>();
        }

        // Create register for the new Component.
        componentInfo<ComponentT>().construct();
        ENT_ASSERT_FAST(componentInfo<ComponentT>().constructed());
        auto &regInfo = componentInfo<ComponentT>()();

        // Check, if there is enough space for one more Component.
        ENT_ASSERT_FAST(sComponentIdCounter < ENT_MAX_COMPONENTS);
        // Assign unique ID to the new Component.
        regInfo.id = compIdInc();
        const u64 cId{id<ComponentT>()};
        ENT_ASSERT_FAST(regInfo.id == cId);

        // Initialize the Component storage.
        regInfo.holder.construct(std::forward<CArgTs>(args)...);

        // Initialize Component mask.
        ENT_ASSERT_FAST(regInfo.mask.none());
        regInfo.mask.set(id<ComponentT>());
        ENT_ASSERT_FAST(regInfo.mask == mask<ComponentT>());

        // Register the Component storage for refresh.
        mRefreshHolders.emplace_back(regInfo.holder.ptr());

        // Register the registered component to destruct on system reset.
        mDestructOnReset.emplace_back(&(componentInfo<ComponentT>()));

        return cId;
    }

    template <typename UT>
    template <typename ComponentT>
    u64 ComponentManager<UT>::id() const
    {
        return componentInfo<ComponentT>()().id;
    }

    template <typename UT>
    template <typename ComponentT>
    const ComponentBitset &ComponentManager<UT>::mask() const
    {
        return componentInfo<ComponentT>()().mask;
    }

    template <typename UT>
    template <typename ComponentT>
    ComponentT *ComponentManager<UT>::add(EntityId id)
    {
        if (!registered<ComponentT>())
        {
            ENT_WARNING("Unknown Component type!");
            return nullptr;
        }

        return getHolder<ComponentT>().add(id);
    }

    template <typename UT>
    template <typename ComponentT>
    ComponentT *ComponentManager<UT>::get(EntityId id)
    {
        if (!registered<ComponentT>())
        {
            ENT_WARNING("Unknown Component type!");
            return nullptr;
        }

        return getHolder<ComponentT>().get(id);
    }

    template <typename UT>
    template <typename ComponentT>
    const ComponentT *ComponentManager<UT>::get(EntityId id) const
    {
        if (!registered<ComponentT>())
        {
            ENT_WARNING("Unknown Component type!");
            return nullptr;
        }

        return getHolder<ComponentT>().get(id);
    }

    template <typename UT>
    template <typename ComponentT>
    bool ComponentManager<UT>::remove(EntityId id)
    {
        if (!registered<ComponentT>())
        {
            ENT_WARNING("Unknown Component type!");
            return false;
        }

        return getHolder<ComponentT>().remove(id);
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    HolderT &ComponentManager<UT>::getHolder()
    {
        return componentInfo<ComponentT>()().holder();
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    const HolderT &ComponentManager<UT>::getHolder() const
    {
        return componentInfo<ComponentT>()().holder();
    }
    // ComponentManager implementation end.
} // namespace ent
