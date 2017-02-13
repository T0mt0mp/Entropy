/**
 * @file Entropy/Entity.h
 * @author Tomas Polasek
 * @brief Entity is an object within the Entropy ECS Universe.
 */

#ifndef ECS_FIT_ENTITY_H
#define ECS_FIT_ENTITY_H

#include "Types.h"
#include "Component.h"
#include "EntityId.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * @brief Entity is composed of ID and Universe ptr, where it resides.
     * Entity class should not be used as the primary way to access
     * Components. EntityGroup + System combination should be used.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class Entity
    {
    public:
        template <typename UT>
        friend class Entity;

        /**
         * Create Entity which resides in given Universe and
         * has specified ID.
         * @param uni Universe pointer.
         * @param id ID of the Entity.
         */
        Entity(UniverseT *uni, EntityId id = {});

        /**
         * Copy constructor.
         * @param rhs Entity to copy.
         */
        Entity(const Entity &rhs);

        /**
         * Move constructor.
         * @param rhs Entity to move.
         */
        Entity(Entity &&rhs);

        /**
         * Copy-assignment operator.
         * @tparam UT If the UniverseT of the copied Entity is the same the
         *   Universe pointer is copied too, else only ID is copied.
         * @param rhs Entity to copy.
         */
        template <typename UT>
        Entity &operator=(const Entity<UT> &rhs);

        /**
         * Move-assignment operator.
         * @tparam UT If the UniverseT of the moved Entity is the same the
         *   Universe pointer is moved too, else only ID is moved.
         * @param rhs Entity to move.
         */
        template <typename UT>
        Entity &operator=(Entity<UT> &&rhs);

        /**
         * Copy helper method. Copy Given Entity ID, if the
         * Entity is from the same Universe, copy the Universe ptr too.
         * @tparam UT Universe type.
         * @param rhs Copy this Entity.
         */
        template <typename UT>
        void copy(const Entity<UT> &rhs);
        void copy(const Entity &rhs);

        /**
         * Comparison operator, compares only ID!
         * @param rhs Second operand.
         * @return Returns true, if the IDs are the same.
         */
        bool operator==(const Entity &rhs) const;

        /**
         * Does this Entity have Component of given type
         * associated with it?
         * @tparam ComponentT Type of Component.
         * @return Returns true, if such Component is associated.
         */
        template <typename ComponentT>
        bool has() const
        { return mUniverse->template hasComponent<ComponentT>(mId); }

        /**
         * Get the Component of given type associated with
         * this Entity.
         * @tparam ComponentT Type of the Component.
         * @return Returns ptr to the Component, or nullptr if this
         *   Entity does not have such Component.
         */
        template <typename ComponentT>
        const ComponentT *get() const
        { return mUniverse->template getComponent<ComponentT>(mId); }
        template <typename ComponentT>
        ComponentT *get()
        { return mUniverse->template getComponent<ComponentT>(mId); }

        /**
         * Add Component of given type to this Entity. If there already
         * is Component of this type associated with this Entity, nothing
         * happens.
         * @tparam ComponentT Type of the Component.
         * @return Returns ptr to new Component, or already existing one.
         */
        template <typename ComponentT>
        const ComponentT *add() const
        { return mUniverse->template addComponent<ComponentT>(mId); }
        template <typename ComponentT>
        ComponentT *add()
        { return mUniverse->template addComponent<ComponentT>(mId); }

        /**
         * Remove Component of given type from this Entity.
         * If there is no Component of this type associated with
         * the Entity, nothing happens.
         * @tparam ComponentT Type of the Component.
         */
        template <typename ComponentT>
        void remove()
        { return mUniverse->template removeComponent<ComponentT>(mId); }

        /**
         * Check if this Entity contains valid EntityId.
         * !!Does not check for validity withing the Universe - use valid() method for that!!
         * @return Returns true, if the EntityId has been set to valid (not null) value.
         */
        bool created() const
        { return mId.index() != 0; }

        /**
         * Check if this Entity is valid within its Universe.
         * @return Returns true, if it exists within its universe.
         */
        bool valid() const
        { return mUniverse->entityValid(mId); }

        /**
         * Activate this Entity, if it's already active, nothing happens.
         */
        void activate()
        { mUniverse->activateEntity(mId); }

        /**
         * Deactivate this Entity, if it's already inactive, nothing happens.
         */
        void deactivate()
        { mUniverse->deactivateEntity(mId); }

        /**
         * Is this Entity active?
         * @return Returns true, if this Entity is active.
         */
        bool active() const
        { return mUniverse->entityActive(mId); }

        /**
         * Destroy this Entity. Resets ID of the Entity to 0.
         * @return Returns false, if this Entity did not exist.
         */
        bool destroy()
        {
            bool result{mUniverse->destroyEntity(mId)};
            mId = 0;
            return result;
        }

        /// Universe ptr getter.
        const UniverseT *universe() const
        { return mUniverse; }

        /// EntityId getter.
        const EntityId &id() const
        { return mId; }
    private:
        /// Universe this Entity lives in.
        UniverseT *mUniverse;
        /// ID this Entity represents.
        EntityId mId;
    protected:
    }; // Entity

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
        return mId == rhs.mId;
    }
    // Entity implementation end.
} // namespace ent

#endif //ECS_FIT_ENTITY_H
