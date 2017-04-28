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
#include "ComponentManager.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Holds information about newly crated Component.
     * Alternatively can hold Component removal action.
     * @tparam ComponentT Type of the Component
     */
    template <typename ComponentT>
    struct ComponentChange
    {
        /**
         * Used as compare functor.
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
        ComponentChange(EntityId id, bool remove, CArgTs... cArgs);

        /// ID of the owner.
        EntityId id;
        /// Type of action.
        bool remove;
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
        /// Holder type for contained Component
        using HolderT = typename HolderExtractor<ComponentT>::type;
        // TODO - Use Holder for temporary Component storage?

        using AddedListT = ent::SortedList<ComponentChange<ComponentT>,
            typename ComponentChange<ComponentT>::ComponentChangeCmp>;

        /// Cleanup.
        virtual ~ComponentActionsSpec();

        /**
         * Request removal of Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        inline void remove(EntityId id);

        /**
         * Remove temporary Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        inline void removeTemp(EntityId id);
        inline void removeTempT(EntityId id);

        /**
         * Get already added temporary Component.
         * @param id ID of the Entity.
         * @return Returns ptr to the Component data, or
         *   nullptr, if such Component does not exist.
         */
        inline ComponentT *get(EntityId id);
        inline ComponentT *getT(EntityId id);

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
        inline ComponentT *addT(EntityId id);

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
        template <typename... CArgTs>
        inline ComponentT *addT(EntityId id, CArgTs... cArgs);

        /// Get List of added Components.
        const AddedListT &added() const
        { return mAdded; };

        /// Get List of added Components for temporary Entities.
        const AddedListT &tempAdded() const
        { return mTempAdded; };
    private:
        /// Get pointer from iterator to given list.
        inline ComponentT *ptrFromIt(typename AddedListT::iterator it, const AddedListT &list);

        /**
         * List of Entities which will either have a new Component
         * added, or the old one changed.
         */
        AddedListT mAdded;

        /// List of Components for temporary Entities.
        AddedListT mTempAdded;
    protected:
    }; // class ComponentActionsSpec

    /// Represents activation/deactivation action.
    struct ActivityChange
    {
        struct ActivityChangeCmp
        {
            /// Comparison operator.
            inline bool operator()(const EntityId &id, const ActivityChange &ac);
            inline bool operator()(const ActivityChange &ac, const EntityId &id);
            inline bool operator()(const ActivityChange &ac1, const ActivityChange &ac2);
        };

        /// ID of the Entity.
        EntityId id;
        /// New value of the activity flag.
        bool activity;
    }; // struct ActivityChange.

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

        /**
         * Set activity of given temporary Entity
         * to active.
         * @param id ID of the temporary Entity.
         */
        inline void activateT(EntityId id);

        /**
         * Set activity of given temporary Entity
         * to inactive.
         * @param id ID of the temporary Entity.
         */
        inline void deactivateT(EntityId id);

        /// Get list of requested Entity activity changes.
        const auto &changes() const
        { return mChanges; }

        /// Get list of requested temporary Entity activity changes.
        const auto &tempChanges() const
        { return mTempChanges; }

        /// Get list of Entities scheduled for removal.
        const auto &destroyed() const
        { return mDestroyed; }
    private:
        /// List of requested Entity activity changes.
        ent::SortedList<ActivityChange, ActivityChange::ActivityChangeCmp> mChanges;
        /// List of requested temporary Entity activity changes.
        ent::SortedList<ActivityChange, ActivityChange::ActivityChangeCmp> mTempChanges;
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
         * Does given Entity have a temporary Component of
         * specified type?
         * @param compId ID of the Component.
         * @param id Entity ID.
         * @return Returns true, if there is such temporary
         *   Component.
         */
        template <typename ComponentT>
        inline bool hasComponent(u64 compId, EntityId id);
        template <typename ComponentT>
        inline bool hasComponentT(u64 compId, EntityId id);

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
        template <typename ComponentT>
        inline ComponentT *getComponentT(u64 compId, EntityId id);

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
        template <typename ComponentT>
        inline ComponentT *addComponentT(u64 compId, EntityId id);

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
        template <typename ComponentT,
                  typename... CArgTs>
        inline ComponentT *addComponentT(u64 compId, EntityId id, CArgTs... cArgs);

        /**
         * Mark Component for removal.
         * @tparam ComponentT Type of the Component.
         * @param compId ID of the Component.
         * @param id Entity ID.
         */
        template <typename ComponentT>
        inline void removeComponent(u64 compId, EntityId id);

        /**
         * Remove temporary Component.
         * @tparam ComponentT Type of the Component.
         * @param compId ID of the Component.
         * @param id Entity ID.
         */
        template <typename ComponentT>
        inline void removeTempComponent(u64 compId, EntityId id);
        template <typename ComponentT>
        inline void removeTempComponentT(u64 compId, EntityId id);

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

        /**
         * Set activity of given temporary Entity
         * to active.
         * @param id ID of the temporary Entity.
         */
        inline void activateTempEntity(EntityId id);

        /**
         * Set activity of given temporary Entity
         * to inactive.
         * @param id ID of the temporary Entity.
         */
        inline void deactivateTempEntity(EntityId id);

        /**
         * Destroy given temporary Entity.
         * @param id ID of the temporary Entity.
         */
        inline void destroyTempEntity(EntityId id);

        /**
         * Create new temporary Entity.
         * @return Returns ID of the temporary Entity. Generation
         *   is always EntityId::TEMP_ENTITY_GEN.
         */
        inline EntityId createEntity();

        /// Metadata changes getter.
        inline const MetadataActions &metadataChanges() const;

        /// Temporary Entity mapping list getter.
        inline ent::List<EntityId> &temporaryEntityMapper();

        /// ComponentsActions list getter.
        inline ent::List<ComponentActions*> &components();
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

        /**
         * List mapping temporary EntityId (index) to
         * EntityId of the created Entity.
         */
        ent::List<EntityId> mTempEntities;
    protected:
    }; // class ChangeSet

#ifdef ENT_UNUSED
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
#endif
} // namespace ent

#include "ChangeSet.inl"

#endif //ECS_FIT_CHANGESET_H
