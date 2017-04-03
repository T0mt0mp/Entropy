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
    }

    template <typename UniverseT>
    void ActionsCache<UniverseT>::applyChangeSets(UniverseT *uni)
    {
        // TODO - optimize, parallelize.

        // Destroy Entities.
        for (auto cs : mCommittedChanges)
        {
            for (EntityId id : cs->metadataChanges().destroyed())
            {
                uni->destroyEntity(id);
            }
        }

        // Create Entities.
        for (auto cs : mCommittedChanges)
        {
            for (EntityId &id : cs->temporaryEntityMapper())
            {
                id = uni->createEntityId();
            }
        }

        // Remove / add Components.
        for (auto cs : mCommittedChanges)
        {
            for (ComponentChange &cc : cs->)
        }

        // Change metadata.
    }
    // ActionsCache implementation end.
} // namespace ent
