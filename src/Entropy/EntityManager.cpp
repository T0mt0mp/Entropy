/**
 * @file Entropy/EntityManager.cpp
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#include "Entropy/EntityManager.h"

/// Main Entropy namespace
namespace ent
{
    EntityId EntityHolder::create()
    {
        EIdType index{0};
        EIdType gen{0};

        if (mFree.size() >= ENT_MIN_FREE)
        {
            index = mFree.front();
            ENT_ASSERT_SLOW(index < mRecords.size());
            mFree.pop_front();
            // Generation is incremented in Entity destroy.
            gen = mRecords[index].generation;
            mRecords[index].active = true;
        } else
        {
            u64 numEntities{mRecords.size()};
            ENT_ASSERT_SLOW(numEntities < EntityId::MAX_ENTITIES)
            index = static_cast<EIdType>(mRecords.size());
            gen = EntityId::START_GEN;
            mRecords.emplace_back(EntityRecord{true, 0, EntityId::START_GEN});
        }

        return EntityId(index, gen);
    }

    EntityId EntityHolder::create(EIdType id)
    {
        ENT_WARNING("Create with requested ID is not implemented yet!");
        return EntityId();
    }

    auto EntityHolder::createSequential(u64 size) -> SequenceRecord
    {
        ENT_WARNING("Create sequence is not implemented yet!");
        return {};
    }

    auto EntityHolder::createSequential(EIdType startId, u64 size) -> SequenceRecord
    {
        ENT_WARNING("Create sequence (id request) is not implemented yet!");
        return {};
    }
} // namespace ent
