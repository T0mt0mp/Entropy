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
     * Temporary Entity handle used for performing operations
     * on Entities which are not yet created.
     * @tparam UniverseT Type of the Universe.
     */
    class TemporaryEntity
    {
    public:
    private:
    protected:
    }; // class TemporaryEntity

    class UniverseActionsSender
    {
    public:

    private:
    protected:
    };

    template <typename UniverseT>
    class UniverseActionsSenderSpec : public UniverseActionsSender
    {
    public:
    private:
    protected:
    };

    /**
     * Holds information about newly crated Component.
     * @tparam ComponentT Type of the Component
     */
    template <typename ComponentT>
    struct ComponentChange
    {
        /// ID of the Entity, to which will own this Component.
        EntityId id;
        /// Component instance.
        ComponentT comp;
    }; // class ComponentChange

    template <typename UniverseT,
              typename ComponentT>
    class ComponentActionsSpec;

    /**
     * Abstract base class for storing Component actions.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ComponentActions
    {
    public:
        /**
         * Send any actions stored to given Universe instance.
         * @param uni Pointer to the Universe instance.
         */
        virtual void sendActions(UniverseT *uni) = 0;

        /**
         * Remove Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        template <typename ComponentT>
        inline void remove(EntityId id);

        /**
         * Add a new temporary Component to given Entity and
         * return a pointer to it.
         * Each new add invalidates previous pointers
         * returned by this method.
         * @tparam ComponentT
         * @param id
         * @return Returns ptr to the new temporary Component
         */
        template <typename ComponentT>
        inline ComponentT *add(EntityId id);

        /**
         * Used for static dispatch of actions to
         * ComponentActionsSpec.
         * @tparam ComponentT Type of the Component.
         * @return Returns this pointer casted to
         *   requested specialization.
         */
        template <typename ComponentT>
        inline ComponentActionsSpec<UniverseT, ComponentT> *castToSpec();
    private:
    protected:
    }; // class ComponentActions

    /**
     * Class for storing Component actions, specialized
     * for one Component type.
     * @tparam UniverseT Type of the Universe.
     * @tparam ComponentT Type of the Component.
     */
    template <typename UniverseT,
              typename ComponentT>
    class ComponentActionsSpec : public ComponentActions<UniverseT>
    {
    public:
        /**
         * Send any actions stored to given Universe instance.
         * @param uni Pointer to the Universe instance.
         */
        virtual inline void sendActions(UniverseT *uni) override;

        /**
         * Remove Component from given Entity.
         * @tparam ComponentT Component type.
         * @param id ID of the Entity.
         */
        inline void remove(EntityId id);

        /**
         * Add a new temporary Component to given Entity and
         * return a pointer to it.
         * Each new add invalidates previous pointers
         * returned by this method.
         * @tparam ComponentT
         * @param id
         * @return Returns ptr to the new temporary Component
         */
        inline ComponentT *add(EntityId id);
    private:
        /**
         * List of Entities which will have their Component removed.
         */
        ent::List<EntityId> mRemoved;

        /**
         * List of Entities which will either have a new Component
         * added, or the old one changed.
         */
        ent::List<ComponentChange<ComponentT>> mAdded;
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
         * Send any actions stored to given Universe instance.
         * @tparam UniverseT Type of the Universe.
         * @param uni Pointer to the Universe instance.
         */
        template <typename UniverseT>
        inline void sendActions(UniverseT *uni);

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
        ent::List<EntityId> mActivated;
        /// List of Entities which should be deactivated.
        ent::List<EntityId> mDeactivated;
        /// List of Entities which should be destroyed.
        ent::List<EntityId> mDestroyed;
    protected:
    }; // class MetadataActions

    /**
     * Container for the changes.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ActionsContainer
    {
    public:
        /**
         * Send all actions to given Universe.
         * @param uni Universe instance.
         */
        inline void sendActions(UniverseT *uni);

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
        inline ComponentActionsSpec<UniverseT, ComponentT> &componentActions(u64 componentId);

        /**
         * List containing Component changes for each Component
         * type accessed by the thread.
         */
        ent::List<ComponentActions<UniverseT>*> mComponentActions;
        /// Entity metadata changes.
        MetadataActions mMetadataActions;
    protected:
    }; // class ActionsContainer

    /**
     * Used for aggregation of actions from parallel threads.
     */
    class ChangeSet
    {
    public:
    private:
    protected:
    }; // class ChangeSet
}

#include "ChangeSet.inl"

#endif //ECS_FIT_CHANGESET_H
