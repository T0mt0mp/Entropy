/**
 * @file Entropy/ActionsCache.inl
 * @author Tomas Polasek
 * @brief Used for managing Actions in threaded enviroment.
 */

#include "ActionsCache.h"

/// Main Entropy namespace
namespace ent
{
    // ActionsCache implementation.
    template <typename UniverseT>
    thread_local ActionsContainer ActionsCache<UniverseT>::tActions;

    template <typename UniverseT>
    ChangeSet &ActionsCache<UniverseT>::changeSet()
    {
        return tActions.currentChangeSet();
    }

    template <typename UniverseT>
    void ActionsCache<UniverseT>::commitChangeSet()
    {
        std::lock_guard<std::mutex> lg(mCommitMutex);
        mCommittedChanges.emplace_back(tActions.releaseChangeSet());
    }

    template <typename UniverseT>
    void ActionsCache<UniverseT>::resetChangeSet()
    {
        // Take care of the returned pointer.
        delete tActions.releaseChangeSet();
    }

    template <typename UniverseT>
    void ActionsCache<UniverseT>::reset()
    {
        std::lock_guard<std::mutex> lg(mCommitMutex);
        mCommittedChanges.clear();
        mRegisteredExtractors.clear();
    }

    template <typename UniverseT>
    template <typename ComponentT>
    void ActionsCache<UniverseT>::registerComponent(u64 cId)
    {
        mRegisteredExtractors.push_back(&extractorGetter<ComponentT>());
    }

    template <typename UniverseT>
    void ActionsCache<UniverseT>::applyChangeSets(UniverseT *uni)
    {
        // TODO - optimize, parallelize.

        // Destroy Entities.
        for (std::unique_ptr<ChangeSet> &cs : mCommittedChanges)
        {
            for (EntityId id : cs->metadataChanges().destroyed())
            {
                uni->destroyEntity(id);
            }
        }

        // Create Entities.
        for (std::unique_ptr<ChangeSet> &cs : mCommittedChanges)
        {
            for (EntityId &id : cs->temporaryEntityMapper())
            {
                id = uni->createEntityId();
            }
        }

        // Remove / add Components.
        for (std::unique_ptr<ChangeSet> &cs : mCommittedChanges)
        {
            for (u64 index = 0;
                 index < cs->components().size() && index < mRegisteredExtractors.size();
                 ++index)
            {
                if (cs->components()[index])
                {
                    mRegisteredExtractors[index]->addRemoveComponents(cs->components()[index], uni);
                }
            }
        }

        // Change metadata.
    }
    template <typename UniverseT>
    template <typename ComponentT>
    void ActionsCache<UniverseT>::ComponentExtractorSpec<ComponentT>::
        addRemoveComponents(ComponentActions *ca, UniverseT *uni)
    {
        ComponentActionsSpec<ComponentT> *actions{
            ENT_CHOOSE_DEBUG(
                dynamic_cast<ComponentActionsSpec<ComponentT>*>(ca),
                static_cast<ComponentActionsSpec<ComponentT>*>(ca)
        )};

        for (EntityId id : actions->removed())
        {
            uni->template removeComponent<ComponentT>(id);
        }

        for (const ComponentChange<ComponentT> &cc : actions->added())
        {
            uni->template replaceComponent<ComponentT>(cc.id, cc.comp);
        }
    }
    // ActionsCache implementation end.
} // namespace ent
