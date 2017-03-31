/**
 * @file Entropy/ChangeSet.h
 * @author Tomas Polasek
 * @brief Action cache used for aggregating actions from parallel threads.
 */

#include "ChangeSet.h"

/// Main Entropy namespace
namespace ent
{
    // ComponentChange implementation.
    template <typename ComponentT>
    template <typename... CArgTs>
    ComponentChange<ComponentT>::ComponentChange(EntityId id, CArgTs... cArgs) :
        id{id}, comp(std::forward<CArgTs>(cArgs)...)
    { }
    // ComponentChange implementation end.

    // ComponentChangeCmp implementation.
    template <typename ComponentT>
    bool ComponentChange<ComponentT>::ComponentChangeCmp::operator()(
        const EntityId &id,
        const ComponentChange<ComponentT> &cc)
    { return id < cc.id; }

    template <typename ComponentT>
    bool ComponentChange<ComponentT>::ComponentChangeCmp::operator()(
        const ComponentChange<ComponentT> &cc,
        const EntityId &id)
    { return cc.id < id; }

    template <typename ComponentT>
    bool ComponentChange<ComponentT>::ComponentChangeCmp::operator()(
        const ComponentChange<ComponentT> &cc1,
        const ComponentChange<ComponentT> &cc2)
    { return cc1.id < cc2.id; }
    // ComponentChangeCmp implementation end.

    // ComponentActions implementation.
    template <typename UniverseT>
    ComponentActions<UniverseT>::~ComponentActions()
    { }

    template <typename UniverseT>
    template <typename ComponentT>
    void ComponentActions<UniverseT>::remove(EntityId id)
    { castToSpec<ComponentT>()->remove(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ComponentActions<UniverseT>::get(EntityId id)
    { return castToSpec<ComponentT>()->get(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ComponentActions<UniverseT>::add(EntityId id)
    { return castToSpec<ComponentT>()->add(id); }

    template <typename UniverseT>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ComponentActions<UniverseT>::add(EntityId id, CArgTs... cArgs)
    { return castToSpec<ComponentT>()->add(id, std::forward<CArgTs>(cArgs)...); }

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
    ComponentActionsSpec<UniverseT, ComponentT>::~ComponentActionsSpec()
    { }

    template <typename UniverseT,
              typename ComponentT>
    void ComponentActionsSpec<UniverseT, ComponentT>::remove(EntityId id)
    {
        mRemoved.insertUnique(id);
    }

    template <typename UniverseT,
              typename ComponentT>
    ComponentT *ComponentActionsSpec<UniverseT, ComponentT>::get(EntityId id)
    {
        auto it = mAdded.find(id);
        return it ? &(it->comp) : nullptr;
    }

    template <typename UniverseT,
              typename ComponentT>
    ComponentT *ComponentActionsSpec<UniverseT, ComponentT>::add(EntityId id)
    {
        auto it =  mAdded.insertUnique(id, id);
        return it ? &(it->comp) : nullptr;
    }

    template <typename UniverseT,
              typename ComponentT>
    template <typename... CArgTs>
    ComponentT *ComponentActionsSpec<UniverseT, ComponentT>::add(EntityId id,
                                                                 CArgTs... cArgs)
    {
        auto it = mAdded.replaceUnique(id, id, std::forward<CArgTs>(cArgs)...);
        return it ? &(it->comp) : nullptr;
    }
    // ComponentActionsSpec implementation end.

    // MetadataActions implementation.
    void MetadataActions::activate(EntityId id)
    {
        mActivated.insertUnique(id);
    }

    void MetadataActions::deactivate(EntityId id)
    {
        mDeactivated.insertUnique(id);
    }

    void MetadataActions::destroy(EntityId id)
    {
        mDestroyed.insertUnique(id);
    }
    // MetadataActions implementation end.

    // ActionsContainer implementation.
    template <typename UniverseT>
    void ActionsContainer<UniverseT>::sendActions(UniverseT *uni)
    {
        ENT_WARNING("ActionContainer<>::sendActions is not implemented yet!");

        /*
        mMetadataActions.sendActions(uni);

        for (ComponentActions<UniverseT> *actions : mComponentActions)
        {
            actions->sendActions(uni);
        }
         */
    }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ActionsContainer<UniverseT>::getComponent(u64 compId, EntityId id)
    { return componentActions<ComponentT>(compId).get(id); }

    template <typename UniverseT>
    template <typename ComponentT>
    ComponentT *ActionsContainer<UniverseT>::addComponent(u64 compId, EntityId id)
    { return componentActions<ComponentT>(compId).add(id); }

    template <typename UniverseT>
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ActionsContainer<UniverseT>::addComponent(u64 compId, EntityId id, CArgTs... cArgs)
    { return componentActions<ComponentT>(compId).add(id, std::forward<CArgTs>(cArgs)...); }

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
