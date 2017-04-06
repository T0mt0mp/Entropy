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
        inline Entity(UniverseT *uni, EntityId id = {});

        /**
         * Copy constructor.
         * @param rhs Entity to copy.
         */
        inline Entity(const Entity &rhs);

        /**
         * Move constructor.
         * @param rhs Entity to move.
         */
        inline Entity(Entity &&rhs);

        /**
         * Copy-assignment operator.
         * @tparam UT If the UniverseT of the copied Entity is the same the
         *   Universe pointer is copied too, else only ID is copied.
         * @param rhs Entity to copy.
         */
        template <typename UT>
        inline Entity &operator=(const Entity<UT> &rhs);

        /**
         * Move-assignment operator.
         * @tparam UT If the UniverseT of the moved Entity is the same the
         *   Universe pointer is moved too, else only ID is moved.
         * @param rhs Entity to move.
         */
        template <typename UT>
        inline Entity &operator=(Entity<UT> &&rhs);

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
         * Does this Entity have Component associated with it?
         * Checks the current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @return Returns true, if there is such a Component.
         * @remarks Is thread-safe, if no other thread is
         *   directly writing to entity metadata.
         */
        template <typename ComponentT>
        inline bool has() const;

        /**
         * Check, if there is a temporary Component prepared.
         * @tparam ComponentT Type of the Component
         * @return Returns true, if there is a temporary
         *   Component of given type.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline bool hasD() const;

        /**
         * Get Component associated with this Entity.
         * Returns read-only ptr to the Component.
         * Returns Component with current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @return Returns pointer to the Component.
         * @remarks Is thread-safe for cases, when on other thread
         *   has write access to the same Component.
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline const ComponentT *get() const;

        /**
         * Get Component associated with this Entity.
         * Returns read-write ptr to the Component.
         * For read-only access, const version should
         * be called.
         * Returns Component with current state, excluding any thread-local changes.
         * @tparam ComponentT Type of the Component
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe in case of write access! If thread-safety
         *   is required, use add/getComponentD.
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline ComponentT *get();

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
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline ComponentT *getD();

        /**
         * Add Component to this Entity.
         * Immediate version, all actions are performed
         * immediately, including editing Entity metadata.
         * @tparam ComponentT Type of the Component
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe! If thread-safety is required, use addD.
         * @remarks Changes Entity metadata!
         * @remarks All pointers to Components of the same type may be invalidated!
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        template <typename ComponentT>
        inline ComponentT *add();

        /**
         * Add Component to this Entity.
         * Immediate version, all actions are performed
         * immediately, including editing Entity metadata.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Component constructor argument types.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the Component.
         * @remarks Not thread-safe! If thread-safety is required, use addD.
         * @remarks Changes Entity metadata!
         * @remarks All pointers to Components of the same type may be invalidated!
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *add(CArgTs... cArgs);

        /**
         * Add Component to this Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
         */
        template <typename ComponentT>
        inline ComponentT *addD();

        /**
         * Add Component to this Entity.
         * Deferred version, temporary Component is
         * returned, operation is finished on refresh.
         * @tparam ComponentT Type of the Component
         * @tparam CArgTs Component constructor argument types.
         * @param cArgs Component constructor arguments.
         * @return Returns pointer to the temporary Component.
         * @remarks Is thread-safe.
         * @remarks Each call invalidates previously returned
         *   temporary Component pointers of the same type.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addD(CArgTs... cArgs);

        /**
         * Remove Component from this Entity.
         * If there is no Component associated with the Entity, nothing happens.
         * Operation is performed immediately.
         * @tparam ComponentT Type of the Component
         * @return Returns true, if the Component has been successfully removed.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata!
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        template <typename ComponentT>
        inline bool remove();

        /**
         * Remove Component from this Entity.
         * Operation is finished on refresh.
         * @tparam ComponentT Type of the Component.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline void removeD();

        /**
         * Remove temporary Component from this Entity.
         * Operation is performed immediately, on the
         * current ChangeSet.
         * @tparam ComponentT Type of the Component.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline void removeDC();

        /**
         * Check if this Entity contains valid EntityId.
         * @return Returns true, if the EntityId has been set to valid (not null) value.
         */
        inline bool validId() const;

        /**
         * Checks validity of this Entity.
         * Checks the current state, excluding any thread-local changes.
         * @return Returns true, if the Entity exists.
         * @remarks Is thread-safe, if no other thread is modifying
         *   entity metadata.
         */
        inline bool valid() const;

        /**
         * Activate this Entity.
         * Action is performed immediately.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        inline void activate();

        /**
         * Activate this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        inline void activateD();

        /**
         * Deactivate this Entity.
         * Action is performed immediately.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        inline void deactivate();

        /**
         * Deactivate this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe.
         * @remarks Method does NOT check, if the
         *   Entity is valid. If such behavior is
         *   required, ENT_ENTITY_VALID should be
         *   defined. If the macro is defined and
         *   Entity is not valid, exception of type
         *   std::runtime_exception will be thrown.
         */
        inline void deactivateD();

        /**
         * Checks if this Entity is active.
         * Checks the current state, excluding any thread-local changes.
         * @return Returns true, if the Entity is active.
         * @remarks Does NOT check index bounds!
         * @remarks Is thread-safe, if no other thread is modifying
         *   entity metadata.
         */
        inline bool active() const;

        /**
         * Destroy this Entity.
         * Action is performed immediately.
         * @return Returns false, if the Entity could not
         *   be destroyed.
         * @remarks Not thread-safe!
         * @remarks Changes Entity metadata.
         */
        inline bool destroy();

        /**
         * Destroy this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe.
         */
        inline void destroyD();

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
    class TemporaryEntity
    {
    public:
        /**
         * Create Temporary Entity.
         * @param uni Universe instance.
         * @param id ID of the temporary Entity.
         */
        TemporaryEntity(UniverseT *uni, EntityId id);

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
         * @remarks Default behavior, if Component doesn't exist, is returning
         *   nullptr. Method can throw exception instead, if ENT_COMP_EXCEPT
         *   macro is defined. Exception of type std::runtime_error is thrown in
         *   that case.
         */
        template <typename ComponentT>
        inline ComponentT *get();

        /**
         * Add Component to the this Entity.
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
         * Add Component to this Entity.
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
         * Remove temporary Component from this Entity.
         * Operation is performed immediately, on the
         * current ChangeSet.
         * @tparam ComponentT Type of the Component.
         * @remarks Is thread-safe.
         */
        template <typename ComponentT>
        inline void remove();

        /**
         * Activate this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe
         */
        inline void activate();

        /**
         * Deactivate this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe.
         */
        inline void deactivate();

        /**
         * Destroy this Entity.
         * Action is finished on refresh.
         * @remarks Is thread-safe.
         */
        inline void destroy();
    private:
        /// Universe this Entity lives in.
        UniverseT *mUniverse;
        /// ID this Entity represents.
        EntityId mId;
    protected:
    }; // class TemporaryEntity
} // namespace ent

#include "Entity.inl"

#endif //ECS_FIT_ENTITY_H
