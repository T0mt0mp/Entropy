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
    { }

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
    // Entity implementation end.
} // namespace ent
