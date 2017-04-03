/**
 * @file Entropy/Entity.h
 * @author Tomas Polasek
 * @brief Entity is an object within the Entropy ECS Universe.
 */

#ifndef ECS_FIT_ENTITY_H
#define ECS_FIT_ENTITY_H

#include "Types.h"
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
        inline void copy(const Entity<UT> &rhs);
        inline void copy(const Entity &rhs);

        /**
         * Comparison operator, compares only ID!
         * @param rhs Second operand.
         * @return Returns true, if the IDs are the same.
         */
        inline bool operator==(const Entity &rhs) const;

        /**
         * Does this Entity have Component of given type
         * associated with it?
         * @tparam ComponentT Type of Component.
         * @return Returns true, if such Component is associated.
         */
        template <typename ComponentT>
        inline bool has() const;

        /**
         * Get the Component of given type associated with
         * this Entity.
         * @tparam ComponentT Type of the Component.
         * @return Returns ptr to the Component, or nullptr if this
         *   Entity does not have such Component.
         */
        template <typename ComponentT>
        inline const ComponentT *get() const;
        template <typename ComponentT>
        inline ComponentT *get();

        /**
         * Add Component of given type to this Entity. If there already
         * is Component of this type associated with this Entity, nothing
         * happens.
         * @tparam ComponentT Type of the Component.
         * @return Returns ptr to new Component, or already existing one.
         */
        template <typename ComponentT>
        inline ComponentT *add();

        /**
         * Remove Component of given type from this Entity.
         * If there is no Component of this type associated with
         * the Entity, nothing happens.
         * @tparam ComponentT Type of the Component.
         * @return Returns true, if the Component has been successfully removed.
         */
        template <typename ComponentT>
        inline bool remove();

        /**
         * Check if this Entity contains valid EntityId.
         * !!Does not check for validity withing the Universe - use valid() method for that!!
         * @return Returns true, if the EntityId has been set to valid (not null) value.
         */
        inline bool created() const;

        /**
         * Check if this Entity is valid within its Universe.
         * @return Returns true, if it exists within its universe.
         */
        inline bool valid() const;

        /**
         * Activate this Entity, if it's already active, nothing happens.
         */
        inline void activate();

        /**
         * Deactivate this Entity, if it's already inactive, nothing happens.
         */
        inline void deactivate();

        /**
         * Is this Entity active?
         * @return Returns true, if this Entity is active.
         */
        inline bool active() const;

        /**
         * Destroy this Entity. Resets ID of the Entity to 0.
         * @return Returns false, if this Entity did not exist.
         */
        inline bool destroy();

        /// Universe ptr getter.
        inline const UniverseT *universe() const;

        /// EntityId getter.
        inline const EntityId &id() const;

        /// EntityId setter.
        inline void setId(EntityId id);
    private:
        /// Universe this Entity lives in.
        UniverseT *mUniverse;
        /// ID this Entity represents.
        EntityId mId;
    protected:
    }; // class Entity

    /**
     * Temporary Entity handle used for performing operations
     * on Entities which are not yet created.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class TemporaryEntity : NonCopyable
    {
    public:
        /**
         * Check, if there is a temporary Component prepared.
         * @tparam ComponentT Type of the Component
         * @return Returns true, if there is a temporary
         *   Component of given type.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline bool has() const;

        /**
         * Get temporary Component, which can be safely
         * used for write access. The operation will be
         * finished on refresh.
         * Temporary Component has to be added first.
         * @tparam ComponentT Type of the Component
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Returned pointer is valid until a new Component
         *   of the same type is added/removed, or the ChangeSet is
         *   committed.
         */
        template <typename ComponentT>
        inline ComponentT *get();

        /**
         * Add Component to the given Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
         */
        template <typename ComponentT>
        inline ComponentT *add();

        /**
         * Add Component to the given Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Component constructor argument types.
         * @param id Id of the Component
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *add(CArgTs... cArgs);

        /**
         * Remove Component from given Entity.
         * Operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline void remove();

        /**
         * Activate given Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe
         */
        inline void activate();

        /**
         * Deactivate given Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe.
         */
        inline void deactivate();

        /**
         * Destroy given Entity.
         * Action is finished on refresh.
         * @return Returns false, if the Entity does not exist.
         * @remarks Is thread-safe.
         */
        inline void destroy();
    private:
    protected:
    }; // class TemporaryEntity
} // namespace ent

#include "Entity.inl"

#endif //ECS_FIT_ENTITY_H
