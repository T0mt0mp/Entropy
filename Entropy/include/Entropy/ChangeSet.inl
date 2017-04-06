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
    ComponentChange<ComponentT>::ComponentChange(EntityId id, bool removeAct, CArgTs... cArgs) :
        id{id}, remove{removeAct}, comp(std::forward<CArgTs>(cArgs)...)
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
    ComponentActions::~ComponentActions()
    { }

    template <typename ComponentT>
    ComponentActionsSpec<ComponentT> *ComponentActions::castToSpec()
    {
        return ENT_CHOOSE_DEBUG(
            (dynamic_cast<ComponentActionsSpec<ComponentT>*>(this)),
            (static_cast<ComponentActionsSpec<ComponentT>*>(this))
        );
    }
    // ComponentActions implementation end.

    // ComponentActionsSpec implementation.
    template <typename ComponentT>
    ComponentActionsSpec<ComponentT>::~ComponentActionsSpec()
    { }

    template <typename ComponentT>
    void ComponentActionsSpec<ComponentT>::remove(EntityId id)
    {
        mAdded.replaceUnique(id, id, true);
    }

    template <typename ComponentT>
    void ComponentActionsSpec<ComponentT>::removeTemp(EntityId id)
    {
        mAdded.erase(id);
    }

    template <typename ComponentT>
    void ComponentActionsSpec<ComponentT>::removeTempT(EntityId id)
    {
        mTempAdded.erase(id);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::get(EntityId id)
    {
        auto it = mAdded.find(id);
        return ptrFromIt(it, mAdded);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::getT(EntityId id)
    {
        auto it = mTempAdded.find(id);
        return ptrFromIt(it, mTempAdded);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::add(EntityId id)
    {
        auto it =  mAdded.replaceUnique(id, id, false);
        return ptrFromIt(it, mAdded);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::addT(EntityId id)
    {
        auto it =  mTempAdded.replaceUnique(id, id, false);
        return ptrFromIt(it, mTempAdded);
    }

    template <typename ComponentT>
    template <typename... CArgTs>
    ComponentT *ComponentActionsSpec<ComponentT>::add(EntityId id,
                                                      CArgTs... cArgs)
    {
        auto it = mAdded.replaceUnique(id, id, false, std::forward<CArgTs>(cArgs)...);
        return ptrFromIt(it, mAdded);
    }

    template <typename ComponentT>
    template <typename... CArgTs>
    ComponentT *ComponentActionsSpec<ComponentT>::addT(EntityId id,
                                                      CArgTs... cArgs)
    {
        auto it = mTempAdded.replaceUnique(id, id, false, std::forward<CArgTs>(cArgs)...);
        return ptrFromIt(it, mTempAdded);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::ptrFromIt(typename AddedListT::iterator it, const AddedListT &list)
    { return it != list.end() ? &(it->comp) : nullptr; }
    // ComponentActionsSpec implementation end.

    // ActivityChangeCmp implementation.
    bool ActivityChange::ActivityChangeCmp::operator()(const EntityId &id, const ActivityChange &ac)
    { return id < ac.id; }
    bool ActivityChange::ActivityChangeCmp::operator()(const ActivityChange &ac, const EntityId &id)
    { return ac.id < id; }
    bool ActivityChange::ActivityChangeCmp::operator()(const ActivityChange &ac1, const ActivityChange &ac2)
    { return ac1.id < ac2.id; }
    // ActivityChangeCmp implementation end.

    // MetadataActions implementation.
    void MetadataActions::activate(EntityId id)
    {
        mChanges.replaceUnique(id, ActivityChange{id, true});
    }

    void MetadataActions::deactivate(EntityId id)
    {
        mChanges.replaceUnique(id, ActivityChange{id, false});
    }

    void MetadataActions::destroy(EntityId id)
    {
        mDestroyed.insertUnique(id);
    }

    void MetadataActions::activateT(EntityId id)
    {
        mTempChanges.replaceUnique(id, ActivityChange{id, true});
    }

    void MetadataActions::deactivateT(EntityId id)
    {
        mTempChanges.replaceUnique(id, ActivityChange{id, false});
    }
    // MetadataActions implementation end.

    // ChangeSet implementation.
    ChangeSet::~ChangeSet()
    {
        for (ComponentActions *cc : mComponentActions)
        {
            delete cc;
        }
    }

    template <typename ComponentT>
    bool ChangeSet::hasComponent(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); return componentActions<ComponentT>(compId).get(id) != nullptr; }
    template <typename ComponentT>
    bool ChangeSet::hasComponentT(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); return componentActions<ComponentT>(compId).getT(id) != nullptr; }

    template <typename ComponentT>
    ComponentT *ChangeSet::getComponent(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); return componentActions<ComponentT>(compId).get(id); }
    template <typename ComponentT>
    ComponentT *ChangeSet::getComponentT(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); return componentActions<ComponentT>(compId).getT(id); }

    template <typename ComponentT>
    ComponentT *ChangeSet::addComponent(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); return componentActions<ComponentT>(compId).add(id); }
    template <typename ComponentT>
    ComponentT *ChangeSet::addComponentT(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); return componentActions<ComponentT>(compId).addT(id); }

    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ChangeSet::addComponent(u64 compId, EntityId id, CArgTs... cArgs)
    { ENT_ASSERT_SLOW(!id.isTemp()); return componentActions<ComponentT>(compId).add(id, std::forward<CArgTs>(cArgs)...); }
    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ChangeSet::addComponentT(u64 compId, EntityId id, CArgTs... cArgs)
    { ENT_ASSERT_SLOW(id.isTemp()); return componentActions<ComponentT>(compId).addT(id, std::forward<CArgTs>(cArgs)...); }

    template <typename ComponentT>
    void ChangeSet::removeComponent(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); componentActions<ComponentT>(compId).remove(id); }

    template <typename ComponentT>
    void ChangeSet::removeTempComponent(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); componentActions<ComponentT>(compId).removeTemp(id); }
    template <typename ComponentT>
    void ChangeSet::removeTempComponentT(u64 compId, EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); componentActions<ComponentT>(compId).removeTempT(id); }

    void ChangeSet::activateEntity(EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); mMetadataActions.activate(id); }

    void ChangeSet::deactivateEntity(EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); mMetadataActions.deactivate(id); }

    void ChangeSet::destroyEntity(EntityId id)
    { ENT_ASSERT_SLOW(!id.isTemp()); mMetadataActions.destroy(id); }

    void ChangeSet::activateTempEntity(EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); mMetadataActions.activateT(id); }

    void ChangeSet::deactivateTempEntity(EntityId id)
    { ENT_ASSERT_SLOW(id.isTemp()); mMetadataActions.deactivateT(id); }

    void ChangeSet::destroyTempEntity(EntityId id)
    {
        ENT_ASSERT_SLOW(id.isTemp());
        if (id.index() < mTempEntities.size())
        { // If the temporary Entity actually exists.
            // Mark the temporary Entity as not in use.
            mTempEntities[id.index()] = EntityId(0u, EntityId::TEMP_ENTITY_GEN);
        }
    }

    EntityId ChangeSet::createEntity()
    {
        mTempEntities.pushBack(EntityId(0u, 0u));
        return EntityId(static_cast<EIdType>(mTempEntities.size()) - 1u, EntityId::TEMP_ENTITY_GEN);
    }

    const MetadataActions &ChangeSet::metadataChanges() const
    { return mMetadataActions; }

    ent::List<EntityId> &ChangeSet::temporaryEntityMapper()
    { return mTempEntities; }

    ent::List<ComponentActions*> &ChangeSet::components()
    { return mComponentActions; }

    template <typename ComponentT>
    ComponentActionsSpec<ComponentT> &ChangeSet::componentActions(u64 componentId)
    {
        ComponentActions *result{nullptr};

        if (mComponentActions.size() <= componentId)
        { // Resize the list of ComponentSets.
            mComponentActions.resize(componentId + 1u, nullptr);
        }

        result = mComponentActions[componentId];
        if (result == nullptr)
        { // ComponentSet has not been created yet.
            result = new ComponentActionsSpec< ComponentT>;
            mComponentActions[componentId] = result;
        }

        return *(result->template castToSpec<ComponentT>());
    }
    // ChangeSet implementation end.

    // ActionsContainer implementation.
    ActionsContainer::ActionsContainer() :
        mCurrectChangeSet{new ChangeSet}
    { }

    ActionsContainer::~ActionsContainer()
    { }

    ChangeSet &ActionsContainer::currentChangeSet()
    { return *mCurrectChangeSet; }

    ChangeSet *ActionsContainer::releaseChangeSet()
    {
        ChangeSet *result{mCurrectChangeSet.release()};
        mCurrectChangeSet.reset(new ChangeSet);
        return result;
    }
    // ActionsContainer implementation.
} // namespace ent
