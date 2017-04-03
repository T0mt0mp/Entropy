/**
 * @file Entropy/ChangeSet.h
 * @author Tomas Polasek
 * @brief Action cache used for aggregating actions from parallel threads.
 */

#ifndef ECS_FIT_CHANGESET_H
#define ECS_FIT_CHANGESET_H

#include "Types.h"
#include "Util.h"
#include "SortedList.h"
#include "EntityId.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Holds information about newly crated Component.
     * @tparam ComponentT Type of the Component
     */
    template <typename ComponentT>
    struct ComponentChange
    {
        /**
         * Used as comparation functor.
         */
        struct ComponentChangeCmp
        {
            /// Comparison operator.
            inline bool operator()(const EntityId &id, const ComponentChange<ComponentT> &cc);
            inline bool operator()(const ComponentChange<ComponentT> &cc, const EntityId &id);
            inline bool operator()(const ComponentChange<ComponentT> &cc1, const ComponentChange<ComponentT> &cc2);
        };

        /**
         * Construct ComponentChange.
         * @tparam CArgTs Component constructor argument types.
         * @param id ID of the Entity.
         * @param cArgs Component constructor arguments.
         */
        template <typename... CArgTs>
        ComponentChange(EntityId id, CArgTs... cArgs);

        /// ID of the Entity, to which will own this Component.
        EntityId id;
        /// Component instance.
        ComponentT comp;
    }; // class ComponentChange

    template <typename ComponentT>
    class ComponentActionsSpec;

    /**
     * Abstract base class for storing Component actions.
     */
    class ComponentActions
    {
    public:
        /// Cleanup.
        virtual inline ~ComponentActions();

        /**
         * Remove Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        template <typename ComponentT>
        inline void remove(EntityId id);

        /**
         * Get already added temporary Component.
         * @tparam ComponentT Type of the Component.
         * @param id ID of the Entity.
         * @return Returns ptr to the Component data, or
         *   nullptr, if such Component does not exist.
         */
        template <typename ComponentT>
        inline ComponentT *get(EntityId id);

        /**
         * Add a temporary Component to given Entity.
         * If such Component already exists, return ptr
         * to it and leave it in current state.
         * @tparam ComponentT Type of the Component.
         * @param id ID of the Entity.
         * @return Returns ptr to the new, or old, Component data.
         * @remarks Each call may invalidate pointers to
         *   previously returned Component pointers.
         */
        template <typename ComponentT>
        inline ComponentT *add(EntityId id);

        /**
         * Add a new temporary Component to given Entity.
         * If such Component already exists, it will be
         * overwritten with inplace constructed Component
         * with given constructor parameters.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs Constructor argument types.
         * @param id ID of the Entity.
         * @return Returns ptr to the new temporary Component,
         *   constructed with given constructor parameters.
         * @remarks Each call may invalidate pointers to
         *   previously returned Component pointers.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs);

        /**
         * Used for static dispatch of actions to
         * ComponentActionsSpec.
         * @tparam ComponentT Type of the Component.
         * @return Returns this pointer casted to
         *   requested specialization.
         */
        template <typename ComponentT>
        inline ComponentActionsSpec<ComponentT> *castToSpec();
    private:
    protected:
    }; // class ComponentActions

    /**
     * Class for storing Component actions, specialized
     * for one Component type.
     * @tparam ComponentT Type of the Component.
     */
    template <typename ComponentT>
    class ComponentActionsSpec : public ComponentActions
    {
    public:
        /// Cleanup.
        virtual ~ComponentActionsSpec();

        /**
         * Request removal of Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        inline void remove(EntityId id);

        /**
         * Get already added temporary Component.
         * @param id ID of the Entity.
         * @return Returns ptr to the Component data, or
         *   nullptr, if such Component does not exist.
         */
        inline ComponentT *get(EntityId id);

        /**
         * Add a temporary Component to given Entity.
         * If such Component already exists, return ptr
         * to it and leave it in current state.
         * @param id ID of the Entity.
         * @return Returns ptr to the new, or old, Component data.
         * @remarks Each call may invalidate pointers to
         *   previously returned Component pointers.
         */
        inline ComponentT *add(EntityId id);

        /**
         * Add a new temporary Component to given Entity.
         * If such Component already exists, it will be
         * overwritten with inplace constructed Component
         * with given constructor parameters.
         * @tparam CArgTs Constructor argument types.
         * @param id ID of the Entity.
         * @return Returns ptr to the new temporary Component,
         *   constructed with given constructor parameters.
         * @remarks Each call may invalidate pointers to
         *   previously returned Component pointers.
         */
        template <typename... CArgTs>
        inline ComponentT *add(EntityId id, CArgTs... cArgs);
    private:
        /**
         * List of Entities which will have their Component removed.
         */
        ent::SortedList<EntityId> mRemoved;

        /**
         * List of Entities which will either have a new Component
         * added, or the old one changed.
         */
        ent::SortedList<ComponentChange<ComponentT>,
            typename ComponentChange<ComponentT>::ComponentChangeCmp> mAdded;
    protected:
    }; // class ComponentActionsSpec

    /**
     * Class for storing actions which change Entity
     * metadata.
     */
    class MetadataActions
    {
    public:
        /**
         * Mark given Entity for activation.
         * @param id ID of the Entity.
         */
        inline void activate(EntityId id);

        /**
         * Mark given Entity for deactivation.
         * @param id ID of the Entity.
         */
        inline void deactivate(EntityId id);

        /**
         * Mark given Entity for deletion.
         * @param id ID of the Entity.
         */
        inline void destroy(EntityId id);
    private:
        /// List of Entities which should be activated.
        ent::SortedList<EntityId> mActivated;
        /// List of Entities which should be deactivated.
        ent::SortedList<EntityId> mDeactivated;
        /// List of Entities which should be destroyed.
        ent::SortedList<EntityId> mDestroyed;
    protected:
    }; // class MetadataActions

    /**
     * Container for the changes.
     */
    class ChangeSet
    {
    public:
        /// Clean up any used memory.
        inline ~ChangeSet();

        /**
         * Get already added temporary Component.
         * @tparam ComponentT Type of the Component.
         * @param compId ID of the Component.
         * @param id Entity ID.
         * @return Returns ptr to the Component data, or
         *   nullptr, if there is no such Component.
         * @remarks Does NOT invalidate previously returned pointers.
         */
        template <typename ComponentT>
        inline ComponentT *getComponent(u64 compId, EntityId id);

        /**
         * Add temporary Component for given Entity.
         * @tparam ComponentT Type of the Component.
         * @param compId ID of the Component.
         * @param id Entity ID.
         * @return Returns ptr to the temporary Component.
         * @remarks Calls invalidate previously returned pointers.
         */
        template <typename ComponentT>
        inline ComponentT *addComponent(u64 compId, EntityId id);

        /**
         * Add temporary Component for given Entity, and construct
         * it with given constructor parameters.
         * @tparam ComponentT Component type.
         * @tparam CArgTs Constructor argument types.
         * @param compId ID of the Component.
         * @param id Entity ID.
         * @param cArgs Constructor arguments.
         * @return Returns ptr to the temporary Component.
         * @remarks Calls invalidate previously returned pointers.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addComponent(u64 compId, EntityId id, CArgTs... cArgs);

        /**
         * Mark Component for removal.
         * @tparam ComponentT Type of the Component.
         * @param compId ID of the Component.
         * @param id Entity ID.
         */
        template <typename ComponentT>
        inline void removeComponent(u64 compId, EntityId id);

        /**
         * Mark Entity for activation.
         * @param id ID of the Entity.
         */
        inline void activateEntity(EntityId id);

        /**
         * Mark Entity for deactivation.
         * @param id ID of the Entity.
         */
        inline void deactivateEntity(EntityId id);

        /**
         * Mark Entity for removal.
         * @param id ID of the Entity.
         */
        inline void destroyEntity(EntityId id);
    private:
        /**
         * Get Component actions holder for hiven Component type.
         * @tparam ComponentT Type of the Component.
         * @param componentId Unique ID of the registered Component.
         * @return Returns reference to the action holder, which
         *   belongs to the Component type specified.
         */
        template <typename ComponentT>
        inline ComponentActionsSpec<ComponentT> &componentActions(u64 componentId);

        /**
         * List containing Component changes for each Component
         * type accessed by the thread.
         */
        ent::List<ComponentActions*> mComponentActions;
        /// Entity metadata changes.
        MetadataActions mMetadataActions;
    protected:
    }; // class ChangeSet

    /**
     * Used for aggregation of actions from parallel threads.
     */
    class ActionsContainer
    {
    public:
        /**
         * Prepare ChangeSet for use.
         */
        inline ActionsContainer();

        /**
         * Deletes ChangeSet.
         */
        inline ~ActionsContainer();

        /**
         * Release ownership of the current ChangeSet
         * and return a pointer to it.
         * Also initializes a new ChangeSet.
         * @return Returns ptr to the current ChangeSet.
         */
        inline ChangeSet *releaseChangeSet();

        /**
         * Get the ChangeSet currently in use.
         * @return Returns reference to the ChangeSet in use.
         */
        inline ChangeSet &currentChangeSet();
    private:
        /// ChangeSet currently in use.
        std::unique_ptr<ChangeSet> mCurrectChangeSet;
    protected:
    }; // class ActionsContainer
} // namespace ent

#include "ChangeSet.inl"

#endif //ECS_FIT_CHANGESET_H
