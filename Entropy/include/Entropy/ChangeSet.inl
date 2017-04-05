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
    ComponentActions::~ComponentActions()
    { }

    template <typename ComponentT>
    void ComponentActions::remove(EntityId id)
    { castToSpec<ComponentT>()->remove(id); }

    template <typename ComponentT>
    ComponentT *ComponentActions::get(EntityId id)
    { return castToSpec<ComponentT>()->get(id); }

    template <typename ComponentT>
    ComponentT *ComponentActions::add(EntityId id)
    { return castToSpec<ComponentT>()->add(id); }

    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ComponentActions::add(EntityId id, CArgTs... cArgs)
    { return castToSpec<ComponentT>()->add(id, std::forward<CArgTs>(cArgs)...); }

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
        mRemoved.insertUnique(id);
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::get(EntityId id)
    {
        auto it = mAdded.find(id);
        return it ? &(it->comp) : nullptr;
    }

    template <typename ComponentT>
    ComponentT *ComponentActionsSpec<ComponentT>::add(EntityId id)
    {
        auto it =  mAdded.insertUnique(id, id);
        return it ? &(it->comp) : nullptr;
    }

    template <typename ComponentT>
    template <typename... CArgTs>
    ComponentT *ComponentActionsSpec<ComponentT>::add(EntityId id,
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

    const ent::SortedList<EntityId> &MetadataActions::activated() const
    { return mActivated; }

    const ent::SortedList<EntityId> &MetadataActions::deactivated() const
    { return mDeactivated; }

    const ent::SortedList<EntityId> &MetadataActions::destroyed() const
    { return mDestroyed; }
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
    { return componentActions<ComponentT>(compId).get(id) != nullptr; }

    template <typename ComponentT>
    ComponentT *ChangeSet::getComponent(u64 compId, EntityId id)
    { return componentActions<ComponentT>(compId).get(id); }

    template <typename ComponentT>
    ComponentT *ChangeSet::addComponent(u64 compId, EntityId id)
    { return componentActions<ComponentT>(compId).add(id); }

    template <typename ComponentT,
              typename... CArgTs>
    ComponentT *ChangeSet::addComponent(u64 compId, EntityId id, CArgTs... cArgs)
    { return componentActions<ComponentT>(compId).add(id, std::forward<CArgTs>(cArgs)...); }

    template <typename ComponentT>
    void ChangeSet::removeComponent(u64 compId, EntityId id)
    { componentActions<ComponentT>(compId).remove(id); }

    void ChangeSet::activateEntity(EntityId id)
    { mMetadataActions.activate(id); }

    void ChangeSet::deactivateEntity(EntityId id)
    { mMetadataActions.deactivate(id); }

    void ChangeSet::destroyEntity(EntityId id)
    { mMetadataActions.destroy(id); }

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
