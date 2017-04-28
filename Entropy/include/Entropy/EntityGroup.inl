/**
 * @file Entropy/EntityGroup.inl
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#include "EntityGroup.h"

/// Main Entropy namespace
namespace ent
{
    // EntityListParallel implementation.
    template <typename UniverseT,
              typename IteratedT,
              bool IsConst>
    typename EntityListParallel<UniverseT, IteratedT, IsConst>::IterationHelper
        EntityListParallel<UniverseT, IteratedT, IsConst>::sEmptyHelper{nullptr, nullptr};

    template <typename UniverseT,
              typename IteratedT,
              bool IsConst>
    EntityListParallel<UniverseT, IteratedT, IsConst>::
        EntityListParallel(UniverseT *uni, IteratedT &it, u64 numThreads) :
        mUniverse{uni}
    {
        u64 size{it.size()};
        u64 sizePerThread{size / numThreads};
        // TODO - choose proper iterator, when IsConst == true.
        auto iter{it.begin()};
        auto end{it.end()};

        for (u64 threadId = 0; threadId < numThreads; ++threadId)
        {
            auto start{iter};
            iter += sizePerThread;
            if (iter > end)
            {
                iter = end;
            }

            auto last{iter};
            while (iter != end && MetadataGroup::inSameBitset(last->index(), iter->index()))
            {
                iter++;
            }

            mHelpers.emplace_back(start, iter);
        }
    }

    template <typename UniverseT,
              typename IteratedT,
              bool IsConst>
    auto EntityListParallel<UniverseT, IteratedT, IsConst>::forThread(u64 threadId) ->
        EntityList<UniverseT, IterationHelper>
    {
        if (threadId < mHelpers.size())
        {
            return EntityList<UniverseT, IterationHelper>(mUniverse, mHelpers[threadId]);
        }
        else
        {
            return EntityList<UniverseT, IterationHelper>(mUniverse, sEmptyHelper);
        }
    }
    // EntityListParallel implementation end.

    // EntityGroup implementation.
    EntityGroup::EntityGroup(const EntityFilter &filter, u64 groupId) :
        mFilter{filter},
        mId{groupId},
        mUsageCounter{0u}
    {
        mEntities = &mEntityBuffers[0];
        mEntitiesBack = &mEntityBuffers[1];
    }

    u64 EntityGroup::incUsage()
    {
        return ++mUsageCounter;
    }

    u64 EntityGroup::decUsage()
    {
        if (mUsageCounter)
        {
            mUsageCounter--;
        }

        return mUsageCounter;
    }

    void EntityGroup::reset()
    {
        mEntityBuffers[0].reclaim();
        mEntityBuffers[1].reclaim();
        mAdded.reclaim();
        mRemoved.reclaim();
    }

    void EntityGroup::add(EntityId id)
    {
        //mEntities.insertUnique(id);
        mAdded.pushBack(id);
    }

    void EntityGroup::remove(EntityId id)
    {
        //mEntities.erase(id);
        mRemoved.pushBack(id);
    }

    void EntityGroup::refresh()
    {
        mAdded.clear();
        mAdded.shrinkToFit();
        mRemoved.clear();
        mAdded.shrinkToFit();
    }

    void EntityGroup::finalize()
    {
        if (mAdded.size() == 0u && mRemoved.size() == 0u)
        { // Nothing needs to be done in this case.
            return;
        }

        std::sort(mAdded.begin(), mAdded.end());
        std::sort(mRemoved.begin(), mRemoved.end());

        // Assure the size for the back buffer.
        entitiesBack()->resize(entitiesFront()->size() + mAdded.size());

        auto ait = mAdded.cbegin();
        auto aeit = mAdded.cend();
        auto rit = mRemoved.cbegin();
        auto reit = mRemoved.cend();
        auto fit = entitiesFront()->cbegin();
        auto feit = entitiesFront()->cend();

        auto oit = entitiesBack()->begin();
        auto oeit = entitiesBack()->end();

        for (;
            oit != oeit && (ait != aeit || fit != feit);
            ++oit)
        {
            /*
             * Merge-sort mAdded and entities front buffer, while removing Entities
             * from the mRemoved list.
             * All 3 lists are sorted and contain only unique elements.
             * mAdded and mRemoved do not contain same elements.
             * mAdded and entities front buffer do not contain same elements.
             * Each element in mRemoved MUST be in entities front buffer.
             */

            // mRemoved should be used up at most at the same time as the entities front buffer.
            while (rit != reit && (*rit == *fit))
            { // Eat the elements which should be removed.
                ++rit;
                ++fit;
            }

            if (ait == aeit && fit == feit)
            { // All lists are used up.
                break;
            }

            // *ait != *fit is always true.
            *oit = (fit == feit) || ((ait != aeit) && (*ait < *fit)) ? *(ait++) : *(fit++);
        }

        // All inputs should be fully used.
        ENT_ASSERT_SLOW(ait == aeit);
        ENT_ASSERT_SLOW(rit == reit);
        ENT_ASSERT_SLOW(fit == feit);

        // Sort the new list into the back buffer.
        u64 finalSize{static_cast<u64>(oit - entitiesBack()->begin())};
        entitiesBack()->resize(finalSize);

        swapEntityBuffers();
    }
    // EntityGroup implementation end.
} // namespace ent
