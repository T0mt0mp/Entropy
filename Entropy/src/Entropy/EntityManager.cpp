/**
 * @file Entropy/EntityManager.cpp
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#include "Entropy/EntityManager.h"

/// Main Entropy namespace
namespace ent
{
    // EntityHolder implementation.
    EntityHolder::EntityHolder()
    {
        reset();
    }

    void EntityHolder::reset()
    {
        mNumFree = 0;
        mFirstFree = 0;
        mLastFree = 0;
        mRecords.reclaim();

        mRecords.reserve(ENT_MIN_FREE);
        // Create the 0th record, valid Entity indexes start at 1!.
        mRecords.resize(1u);
        initEntity(0u);
    }

    EntityId EntityHolder::create()
    {
        EIdType index{0};
        EIdType gen{0};

        if (mNumFree >= ENT_MIN_FREE)
        {
            index = popFreeId();
            ENT_ASSERT_SLOW(index);

            mRecords[index].groups.set(0u);
            mRecords[index].components.reset();

            // Generation is incremented in Entity destroy.
            gen = mRecords[index].generation;
        }
        else
        {
            u64 numEntities{mRecords.size()};
            ENT_ASSERT_SLOW(numEntities < EntityId::MAX_ENTITIES);

            index = static_cast<EIdType>(mRecords.size());

            /// Create new EntityRecord without initializing it.
            mRecords.resize(index + 1u);
            initEntity(index);

            mRecords[index].groups.set(0u);
            mRecords[index].generation = EntityId::START_GEN;
            gen = EntityId::START_GEN;
        }

        return EntityId(index, gen);
    }

    bool EntityHolder::destroy(EntityId id)
    {
        if (!valid(id))
        { // Entity does not exist!
            return false;
        }

        EntityRecord &rec(mRecords[id.index()]);
        rec.groups.reset();
        rec.generation = rec.generation == EntityId::MAX_GEN ? 0u : rec.generation + 1;
        pushFreeId(id.index());

        return true;
    }

    EntityId EntityHolder::create(EIdType id)
    {
        ENT_WARNING("Create with requested ID is not implemented yet!");
        ENT_UNUSED(id);
        return EntityId();
    }

    auto EntityHolder::createSequential(u64 size) -> SequenceRecord
    {
        ENT_WARNING("Create sequence is not implemented yet!");
        ENT_UNUSED(size);
        return {};
    }

    auto EntityHolder::createSequential(EIdType startId, u64 size) -> SequenceRecord
    {
        ENT_WARNING("Create sequence (id request) is not implemented yet!");
        ENT_UNUSED(startId);
        ENT_UNUSED(size);
        return {};
    }
    // EntityHolder implementation end.
} // namespace ent
