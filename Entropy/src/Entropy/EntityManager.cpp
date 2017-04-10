/**
 * @file Entropy/EntityManager.cpp
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#include "Entropy/EntityManager.h"

/// Main Entropy namespace
namespace ent
{
#ifdef ENT_NOT_USED
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

    void EntityHolder::refresh()
    {

    }

    EntityId EntityHolder::create()
    {
        EIdType index{0};
        EIdType gen{EntityId::START_GEN};

        if (mNumFree >= ENT_MIN_FREE)
        {
            index = popFreeId();
            ENT_ASSERT_SLOW(index);

            ENT_ASSERT_SLOW(!mRecords[index].active());
            ENT_ASSERT_SLOW(mRecords[index].grp().none());

            mRecords[index].activate();
            mRecords[index].comp().reset();

            // Generation is incremented in Entity destroy.
            gen = mRecords[index].gen();
        }
        else
        {
            index = static_cast<EIdType>(mRecords.size());

            if (index >= EntityId::MAX_ENTITIES)
            {
                ENT_WARNING("Unable to create more Entities, increase EntityId size!");
                return EntityId(0u, 0u);
            }

            /// Create new EntityRecord without initializing it.
            mRecords.resize(index + 1u);
            initEntity(index);

            ENT_ASSERT_SLOW(!mRecords[index].active());
            ENT_ASSERT_SLOW(mRecords[index].grp().none());

            mRecords[index].activate();
            mRecords[index].gen() = EntityId::START_GEN;
            //gen = EntityId::START_GEN;
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
        // Entity Groups still need to know, if the Entity is within.
        rec.deactivate();
        rec.gen() = (rec.gen() + 1u == EntityId::MAX_GEN) ? 0u : rec.gen() + 1u;
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
#endif
} // namespace ent
