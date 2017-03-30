/**
 * @file Entropy/ChangeSet.h
 * @author Tomas Polasek
 * @brief Action cache used for aggregating actions from parallel threads.
 */

#include "ChangeSet.h"

/// Main Entropy namespace
namespace ent
{
    // ComponentActions implementation.
    template <typename UniverseT>
    template <typename ComponentT>
    void ComponentActions<UniverseT>::remove(EntityId id)
    { castToSpec<ComponentT>()->remove(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ComponentActions<UniverseT>::add(EntityId id)
    { return castToSpec<ComponentT>()->add(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentActionsSpec<UniverseT, ComponentT> *ComponentActions<UniverseT>::castToSpec()
    {
        return ENT_CHOOSE_DEBUG(
            (dynamic_cast<ComponentActionsSpec<UniverseT, ComponentT>*>(this)),
            (static_cast<ComponentActionsSpec<UniverseT, ComponentT>*>(this))
        );
    }
    // ComponentActions implementation end.

    // ComponentActionsSpec implementation.
    template <typename UniverseT,
              typename ComponentT>
    void ComponentActionsSpec<UniverseT, ComponentT>::sendActions(UniverseT *uni)
    {
        ENT_WARNING("ComponentActionsSpec<>::sendActions is not implemented yet!");
    }

    template <typename UniverseT,
              typename ComponentT>
    void ComponentActionsSpec<UniverseT, ComponentT>::remove(EntityId id)
    {
        mRemoved.pushBack(id);
    }

    template <typename UniverseT,
              typename ComponentT>
    ComponentT *ComponentActionsSpec<UniverseT, ComponentT>::add(EntityId id)
    {
        mAdded.pushBack();
        mAdded.back().id = id;
        return &(mAdded.back().comp);
    }
    // ComponentActionsSpec implementation end.

    // MetadataActions implementation.
    template <typename UniverseT>
    void MetadataActions::sendActions(UniverseT *uni)
    {
        ENT_WARNING("MetadataActions::sendActions is not implemented yet!");
    }

    void MetadataActions::activate(EntityId id)
    {
        mActivated.pushBack(id);
    }

    void MetadataActions::deactivate(EntityId id)
    {
        mDeactivated.pushBack(id);
    }

    void MetadataActions::destroy(EntityId id)
    {
        mDestroyed.pushBack(id);
    }
    // MetadataActions implementation end.

    // ActionsContainer implementation.
    template <typename UniverseT>
    void ActionsContainer<UniverseT>::sendActions(UniverseT *uni)
    {
        mMetadataActions.sendActions(uni);

        for (ComponentActions<UniverseT> *actions : mComponentActions)
        {
            actions->sendActions(uni);
        }
    }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ActionsContainer<UniverseT>::addComponent(u64 compId, EntityId id)
    { return componentActions<ComponentT>(compId).add(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    void ActionsContainer<UniverseT>::removeComponent(u64 compId, EntityId id)
    { componentActions<ComponentT>(compId).remove(id); }

    template <typename UniverseT>
    void ActionsContainer<UniverseT>::activateEntity(EntityId id)
    { mMetadataActions.activate(id); }

    template <typename UniverseT>
    void ActionsContainer<UniverseT>::deactivateEntity(EntityId id)
    { mMetadataActions.deactivate(id); }

    template <typename UniverseT>
    void ActionsContainer<UniverseT>::destroyEntity(EntityId id)
    { mMetadataActions.destroy(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentActionsSpec<UniverseT, ComponentT> &ActionsContainer<UniverseT>::componentActions(u64 componentId)
    {
        ComponentActions<UniverseT> *result{nullptr};

        if (mComponentActions.size() <= componentId)
        { // Resize the list of ComponentSets.
            mComponentActions.resize(componentId + 1u, nullptr);
        }

        result = mComponentActions[componentId];
        if (result == nullptr)
        { // ComponentSet has not been created yet.
            result = new ComponentActionsSpec<UniverseT, ComponentT>;
            mComponentActions[componentId] = result;
        }

        return *(result->template castToSpec<ComponentT>());
    }
    // ActionsContainer implementation end.
}
