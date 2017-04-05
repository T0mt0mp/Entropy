/**
 * @file Entropy/Entity.inl
 * @author Tomas Polasek
 * @brief Entity is an object within the Entropy ECS Universe.
 */

#include "Entity.h"

/// Main Entropy namespace
namespace ent
{
    // Entity implementation.
    template <typename UniverseT>
    Entity<UniverseT>::Entity(UniverseT *uni, EntityId id) :
        mUniverse{uni}, mId{id}
    { ENT_ASSERT_SLOW(mId.generation() != EntityId::TEMP_ENTITY_GEN); }

    template <typename UniverseT>
    Entity<UniverseT>::Entity(const Entity &rhs)
    {
        copy(rhs);
    }

    template <typename UniverseT>
    Entity<UniverseT>::Entity(Entity &&rhs)
    {
        copy(rhs);
    }

    template <typename UniverseT>
    template <typename UniverseT2>
    Entity<UniverseT> &Entity<UniverseT>::operator=(const Entity<UniverseT2> &rhs)
    {
        copy(rhs);
        return *this;
    }

    template <typename UniverseT>
    template <typename UniverseT2>
    Entity<UniverseT> &Entity<UniverseT>::operator=(Entity<UniverseT2> &&rhs)
    {
        copy(rhs);
        return *this;
    }

    template <typename UniverseT>
    template <typename UniverseT2>
    void Entity<UniverseT>::copy(const Entity<UniverseT2> &rhs)
    {
        mId = rhs.mId;
    }

    template <typename UniverseT>
    void Entity<UniverseT>::copy(const Entity<UniverseT> &rhs)
    {
        mId = rhs.mId;
        mUniverse = rhs.mUniverse;
    }

    template <typename UniverseT>
    bool Entity<UniverseT>::operator==(const Entity &rhs) const
    {
        return mId == rhs.mId && mId.generation() == rhs.mId.generation();
    }

    template <typename UniverseT>
    template <typename ComponentT>
    bool Entity<UniverseT>::has() const
    { return mUniverse->template hasComponent<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    bool Entity<UniverseT>::hasD() const
    { return mUniverse->template hasComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    const ComponentT *Entity<UniverseT>::get() const
    { return mUniverse->template getComponent<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *Entity<UniverseT>::get()
    { return mUniverse->template getComponent<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *Entity<UniverseT>::getD()
    { return mUniverse->template getComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *Entity<UniverseT>::add()
    { return mUniverse->template addComponent<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *Entity<UniverseT>::add(CArgTs... cArgs)
    { return mUniverse->template addComponent<ComponentT>(mId, std::forward<CArgTs>(cArgs)...); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *Entity<UniverseT>::addD()
    { return mUniverse->template addComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *Entity<UniverseT>::addD(CArgTs... cArgs)
    { return mUniverse->template addComponentD<ComponentT>(mId, std::forward<CArgTs>(cArgs)...); }

    template <typename UniverseT>
    template <typename ComponentT>
    bool Entity<UniverseT>::remove()
    { return mUniverse->template removeComponent<ComponentT>(mId); }

    template <typename UniverseT>
    template <typename ComponentT>
    void Entity<UniverseT>::removeD()
    { return mUniverse->template removeComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    bool Entity<UniverseT>::validId() const
    { return mId.index() != 0 && mUniverse != nullptr; }

    template <typename UniverseT>
    bool Entity<UniverseT>::valid() const
    { return validId() && mUniverse->entityValid(mId); }

    template <typename UniverseT>
    void Entity<UniverseT>::activate()
    { mUniverse->activateEntity(mId); }

    template <typename UniverseT>
    void Entity<UniverseT>::activateD()
    { mUniverse->activateEntityD(mId); }

    template <typename UniverseT>
    void Entity<UniverseT>::deactivate()
    { mUniverse->deactivateEntity(mId); }

    template <typename UniverseT>
    void Entity<UniverseT>::deactivateD()
    { mUniverse->deactivateEntityD(mId); }

    template <typename UniverseT>
    bool Entity<UniverseT>::active() const
    { return mUniverse->entityActive(mId); }

    template <typename UniverseT>
    bool Entity<UniverseT>::destroy()
    {
        bool result{mUniverse->destroyEntity(mId)};
        mId = 0;
        return result;
    }

    template <typename UniverseT>
    inline void Entity<UniverseT>::destroyD()
    { mUniverse->destroyEntityD(mId); }

    template <typename UniverseT>
    const UniverseT *Entity<UniverseT>::universe() const
    { return mUniverse; }

    template <typename UniverseT>
    const EntityId &Entity<UniverseT>::id() const
    { return mId; }

    template <typename UniverseT>
    void Entity<UniverseT>::setId(EntityId id)
    { mId = id; }
    // Entity implementation end.

    // TemporaryEntity implementation.
    template <typename UniverseT>
    TemporaryEntity<UniverseT>::TemporaryEntity(UniverseT *uni, EntityId id) :
        mUniverse{uni}, mId{id}
    { ENT_ASSERT_SLOW(mId.generation() == EntityId::TEMP_ENTITY_GEN); }

    template <typename UniverseT>
    template <typename ComponentT>
    bool TemporaryEntity<UniverseT>::has() const
    { return mUniverse->template hasComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template<typename ComponentT>
    ComponentT *TemporaryEntity<UniverseT>::get()
    { return mUniverse->template getComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template<typename ComponentT>
    ComponentT *TemporaryEntity<UniverseT>::add()
    { return mUniverse->template addComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    template<typename ComponentT,
             typename... CArgTs>
    ComponentT *TemporaryEntity<UniverseT>::add(CArgTs... cArgs)
    { return mUniverse->template addComponentD<ComponentT>(mId, std::forward<CArgTs>(cArgs)...); }

    template <typename UniverseT>
    template<typename ComponentT>
    void TemporaryEntity<UniverseT>::remove()
    { return mUniverse->template removeComponentD<ComponentT>(mId); }

    template <typename UniverseT>
    void TemporaryEntity<UniverseT>::activate()
    { mUniverse->activateEntityD(mId); }

    template <typename UniverseT>
    void TemporaryEntity<UniverseT>::deactivate()
    { mUniverse->deactivateEntityD(mId); }

    template <typename UniverseT>
    void TemporaryEntity<UniverseT>::destroy()
    { mUniverse->destroyEntityD(mId); }
    // TemporaryEntity implementation end.
} // namespace ent
