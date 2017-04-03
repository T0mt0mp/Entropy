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
    // ActionsCache implementation end.
} // namespace ent
