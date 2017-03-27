/**
 * @file Entropy/EntityGroup.inl
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#include "EntityGroup.h"

/// Main Entropy namespace
namespace ent
{
    // EntityGroup implementation.
    EntityGroup::EntityGroup(const ComponentFilter &filter, u64 groupId) :
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
        // Sort the new list into the back buffer.
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

        u64 finalSize{static_cast<u64>(oit - entitiesBack()->begin())};
        entitiesBack()->resize(finalSize);

        swapEntityBuffers();
    }
    // EntityGroup implementation end.
} // namespace ent
