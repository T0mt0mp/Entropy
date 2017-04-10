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
    CIdType ComponentManager<UT>::sComponentIdCounter{0};

    template <typename UT>
    ComponentManager<UT>::ComponentManager()
    { reset(); }

    template <typename UT>
    ComponentManager<UT>::~ComponentManager()
    { reset(); }

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
            h();
        }
        mDestructOnReset.clear();

        mRefreshHolders.clear();

        resetComponentIdCounter();
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT,
        typename... CArgTs>
    CIdType ComponentManager<UT>::registerComponent(CArgTs... cArgs)
    {
        static_assert(std::is_base_of<ent::BaseComponentHolder<ComponentT>, HolderT>::value,
                      "Component holder has to inherit from ent::BaseComponentHolder!");

        // Check that all required Holder operations are present.
        /*
        static_assert(std::is_same<decltype(std::declval<HolderT>().
                refresh()), void>::value,
            "Component holder has to have refresh method!");
        static_assert(std::is_same<decltype(std::declval<HolderT>().
                remove(std::declval<EntityId>())), bool>::value,
            "Component holder has to have remove method!");
        static_assert(std::is_same<decltype(std::declval<HolderT>().
                add(std::declval<EntityId>())), ComponentT*>::value,
            "Component holder has to have add method!");
        static_assert(std::is_same<decltype(std::declval<HolderT>().
                replace(std::declval<EntityId>(), std::declval<const ComponentT&>())), ComponentT*>::value,
            "Component holder has to have add/replace method method!");
        static_assert(std::is_same<decltype(std::declval<HolderT>().
                get(std::declval<EntityId>())), ComponentT*>::value,
            "Component holder has to have r/w get method method!");
        static_assert(std::is_same<decltype(std::declval<const HolderT>().
                get(std::declval<EntityId>())), const ComponentT*>::value,
            "Component holder has to have read only get method method!");
            */
        static_assert(sizeof(HolderT(std::declval<CArgTs>()...)),
            "Component holder has to be constructible!");

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
        const CIdType cId{id<ComponentT>()};
        ENT_ASSERT_FAST(regInfo.id == cId);

        // Initialize the Component storage.
        regInfo.holder.construct(std::forward<CArgTs>(cArgs)...);

        // Register the Component storage for refresh.
        mRefreshHolders.emplace_back(regInfo.holder.ptr());

        // Register the registered component to destruct on system reset.
        mDestructOnReset.emplace_back(regInfo.holder.destructLater());
        mDestructOnReset.emplace_back(componentInfo<ComponentT>().destructLater());

        return cId;
    }

    template <typename UT>
    template <typename ComponentT>
    CIdType ComponentManager<UT>::id() const
    {
        ENT_ASSERT_SLOW(registered<ComponentT>());
        return componentInfo<ComponentT>()().id;
    }

    template <typename UT>
    CIdType ComponentManager<UT>::numRegistered() const
    {
        return sComponentIdCounter;
    }

    template <typename UT>
    template <typename ComponentT>
    bool ComponentManager<UT>::registered() const
    { return componentInfo<ComponentT>().constructed(); }

    template <typename UT>
    template <typename ComponentT>
    ComponentT *ComponentManager<UT>::add(EntityId id)
    {
        return getHolder<ComponentT>().add(id);
    }

    template <typename UT>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ComponentManager<UT>::add(EntityId id, CArgTs... cArgs)
    {
        return getHolder<ComponentT>().add(id, std::forward<CArgTs>(cArgs)...);
    }

    template <typename UT>
    template <typename ComponentT>
    ComponentT *ComponentManager<UT>::replace(EntityId id, const ComponentT &comp)
    {
        return getHolder<ComponentT>().replace(id, std::forward<const ComponentT&>(comp));
    }

    template <typename UT>
    template <typename ComponentT>
    ComponentT *ComponentManager<UT>::get(EntityId id)
    {
        return getHolder<ComponentT>().get(id);
    }

    template <typename UT>
    template <typename ComponentT>
    const ComponentT *ComponentManager<UT>::get(EntityId id) const
    {
        return getHolder<ComponentT>().get(id);
    }

    template <typename UT>
    template <typename ComponentT>
    bool ComponentManager<UT>::remove(EntityId id)
    {
        return getHolder<ComponentT>().remove(id);
    }

    template <typename UT>
    template <typename ComponentT,
        typename HolderT>
    HolderT &ComponentManager<UT>::getHolder()
    {
        ENT_ASSERT_SLOW(registered<ComponentT>());
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
