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
        std::lock_guard<std::mutex> lg(mCommitMutex);

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
                if (!id.id())
                { // Only create new Entity if the ID is unset.
                    id = uni->createEntityId();
                }
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
                    mRegisteredExtractors[index]->addRemoveComponents(cs->components()[index],
                                                                      cs->temporaryEntityMapper(), uni);
                }
            }
        }

        // Change metadata.
        for (std::unique_ptr<ChangeSet> &cs : mCommittedChanges)
        {
            for (const ActivityChange &ac : cs->metadataChanges().changes())
            {
                uni->setActivityEntity(ac.id, ac.activity);
            }

            for (const ActivityChange &ac : cs->metadataChanges().tempChanges())
            {
                EntityId realId{cs->temporaryEntityMapper()[ac.id.index()]};
                if (!realId.isTemp())
                {
                    uni->setActivityEntity(realId, ac.activity);
                }
            }
        }

        mCommittedChanges.clear();
    }

    template <typename UniverseT>
    template <typename ComponentT>
    void ActionsCache<UniverseT>::ComponentExtractorSpec<ComponentT>::
        addRemoveComponents(ComponentActions *ca, const ent::List<EntityId> &tempMapping, UniverseT *uni)
    {
        ComponentActionsSpec<ComponentT> *actions{
            ENT_CHOOSE_DEBUG(
                dynamic_cast<ComponentActionsSpec<ComponentT>*>(ca),
                static_cast<ComponentActionsSpec<ComponentT>*>(ca)
        )};

        for (const ComponentChange<ComponentT> &cc : actions->added())
        {
            ENT_ASSERT_SLOW(!cc.id.isTemp());
            // TODO - Find a way to assure that only valid Entities get here.
            if (uni->entityValid(cc.id))
            { // If the Entity still exists.
                if (cc.remove)
                {
                    uni->template removeComponent<ComponentT>(cc.id);
                }
                else
                {
                    uni->template replaceComponent<ComponentT>(cc.id, cc.comp);
                }
            }
        }

        for (const ComponentChange<ComponentT> &cc : actions->tempAdded())
        {
            ENT_ASSERT_SLOW(cc.id.isTemp());
            EntityId realId{tempMapping[cc.id.index()]};
            if (!realId.isTemp())
            {
                if (cc.remove)
                {
                    uni->template removeComponent<ComponentT>(realId);
                }
                else
                {
                    uni->template replaceComponent<ComponentT>(realId, cc.comp);
                }
            }
        }
    }
    // ActionsCache implementation end.

    // ChangedEntitiesHolder implementation.
    template <typename UT>
    std::mutex ChangedEntitiesHolder<UT>::sChangesMutex;
    template <typename UT>
    std::vector<std::pair<std::unique_ptr<SortedList<EntityId>>, bool>>
        ChangedEntitiesHolder<UT>::sChanges;
    template <typename UT>
    SortedList<EntityId> ChangedEntitiesHolder<UT>::sResultList;

    template <typename UT>
    ChangedEntitiesHolder<UT>::ChangedEntitiesHolder()
    {
        mChangedEntities = new SortedList<EntityId>;
        registerList(mChangedEntities);
    }

    template <typename UT>
    ChangedEntitiesHolder<UT>::~ChangedEntitiesHolder()
    {
        if (mChangedEntities)
        {
            unregisterList(mChangedEntities);
        }
    }

    template <typename UT>
    void ChangedEntitiesHolder<UT>::refresh()
    {
        std::lock_guard<std::mutex> g(sChangesMutex);

        /*
         * Delete the lists marked for removal, but
         * call delete on them first.
         */
        sChanges.erase(std::remove_if(sChanges.begin(),
                                      sChanges.end(),
            [] (std::pair<std::unique_ptr<SortedList<EntityId>>, bool> &p)
                                      {
                                          /*
                                          if (p.second)
                                          {
                                              delete p.first;
                                          }
                                           */
                                          return p.second;
                                      }
        ), sChanges.end());
    }

    template <typename UT>
    void ChangedEntitiesHolder<UT>::reset()
    {
        std::lock_guard<std::mutex> g(sChangesMutex);

        for (auto &p : sChanges)
        { // Clear the individual lists.
            p.first->reclaim();
        }
        // TODO - What about the registered lists?

        sResultList.reclaim();
    }

    template <typename UT>
    void ChangedEntitiesHolder<UT>::entityChanged(EntityId id)
    { mChangedEntities->insertUnique(id); }

    template <typename UT>
    SortedList<EntityId> &ChangedEntitiesHolder<UT>::createResultList()
    {
        std::lock_guard<std::mutex> g(sChangesMutex);

        sResultList.clear();

        if (sChanges.size())
        {
            // Get the content of the first list.
            sResultList = (*(sChanges[0].first));

            // TODO - optimized implementation.
            for (u64 iii = 1; iii < sChanges.size(); ++iii)
            { // Merge the rest into it.
                for (EntityId id : (*sChanges[iii].first))
                {
                    sResultList.insertUnique(id);
                }
            }

            // Clear the individual lists.
            for (auto &p : sChanges)
            {
                p.first->clear();
            }
        }

        return sResultList;
    }

    template <typename UT>
    void ChangedEntitiesHolder<UT>::registerList(SortedList<EntityId> *list)
    {
        std::lock_guard<std::mutex> g(sChangesMutex);

        sChanges.push_back(std::make_pair(
            std::unique_ptr<SortedList<EntityId>>(list), false));
    }

    template <typename UT>
    void ChangedEntitiesHolder<UT>::unregisterList(SortedList<EntityId> *list)
    {
        std::lock_guard<std::mutex> g(sChangesMutex);

        for (auto &p : sChanges)
        {
            if (p.first.get() == list)
            {
                p.second = true;
            }
        }
    }
    // ChangedEntitiesHolder implementation end.
} // namespace ent
