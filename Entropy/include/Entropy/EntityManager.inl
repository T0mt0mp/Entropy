/**
 * @file Entropy/EntityManager.inl
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#include "EntityManager.h"

/// Main Entropy namespace
namespace ent
{
    // ActiveEntityIterator implementation.
    ActiveEntityIterator::ActiveEntityIterator(const EntityRecord *ptr,
                                               u64 size) :
        mIndex{0u}, mSize{size}, mPtr{ptr}
    { }

    void ActiveEntityIterator::increment()
    {
        while (isValid() && !mPtr->active())
        {
            mPtr++;
            mIndex++;
        }
    }
    // ActiveEntityIterator implementation end.

    // EntityHolder implementation.
    void EntityHolder::addComponent(EntityId id, u64 index)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].components.set(index); }

    void EntityHolder::removeComponent(EntityId id, u64 index)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].components.reset(index); }

    bool EntityHolder::hasComponent(EntityId id, u64 index) const
    { return valid(id) && mRecords[id.index()].components.test(index); }

    void EntityHolder::activate(EntityId id)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].groups.set(0u); }

    void EntityHolder::deactivate(EntityId id)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].groups.reset(0u); }

    bool EntityHolder::active(EntityId id) const
    { return valid(id) && activeImpl(id.index()); }

    bool EntityHolder::valid(EntityId id) const
    { return validImpl(id.index(), id.generation()); }

    const ComponentBitset &EntityHolder::components(EntityId id) const
    { ENT_ASSERT_SLOW(valid(id)); return mRecords[id.index()].components; }

    const GroupBitset &EntityHolder::groups(EntityId id)
    { ENT_ASSERT_SLOW(valid(id)); return mRecords[id.index()].groups; }

    void EntityHolder::setGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(valid(id) && groupId > 0u && groupId < MAX_GROUPS); mRecords[id.index()].groups.set(groupId); }

    void EntityHolder::resetGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(valid(id) && groupId > 0u && groupId < MAX_GROUPS); mRecords[id.index()].groups.reset(groupId); }

    ActiveEntityIterator EntityHolder::activeEntities() const
    { return ActiveEntityIterator(mRecords.begin(), mRecords.size()); }

    void EntityHolder::initEntity(EIdType index)
    {
        std::memset(&mRecords[index], 0u, sizeof(EntityRecord));

        /*
        EntityRecord &rec = mRecords[index];
        rec.generation = EntityId::START_GEN;

        if (sizeof(ComponentBitset) > sizeof(EIdType))
        {
            rec.components.reset();
        }
        else
        {
            rec.nextFree = 0u;
        }

        rec.groups.reset();
         */
    }

    void EntityHolder::pushFreeId(EIdType index)
    {
        EntityRecord &recNew = mRecords[index];
        EntityRecord &recOld = mRecords[mLastFree];

        if (mLastFree)
        { // If there is at least one element in the list, add the new one.
            recOld.nextFree = index;
        }
        else
        { // Else, we need to init the list.
            mFirstFree = index;
        }

        recNew.nextFree = 0;
        mLastFree = index;

        mNumFree++;
    }

    inline EIdType EntityHolder::popFreeId()
    {
        EIdType result{mFirstFree};

        mFirstFree = mRecords[result].nextFree;

        // If the result ID is 0, then there are no more IDs available.
        mNumFree = result ? mNumFree - 1 : 0;

        return result;
    }
    // EntityHolder implementation end.

    // EntityManager implementation.
    template <typename UT>
    EntityManager<UT>::EntityManager()
    { }

    template <typename UT>
    EntityManager<UT>::~EntityManager()
    { }

    template <typename UT>
    void EntityManager<UT>::refresh()
    {
        ENT_WARNING("EntityManager::refresh() is not finished yet!");
    }
    // EntityManager implementation end.
}
