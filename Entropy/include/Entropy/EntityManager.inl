/**
 * @file Entropy/EntityManager.inl
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#include "EntityManager.h"

/// Main Entropy namespace
namespace ent
{
#ifdef ENT_NOT_USED
    // EntityRecord implementation.
    void EntityRecord::reset()
    { std::memset(this, 0u, sizeof(EntityRecord)); }

    bool EntityRecord::active() const
    { return groups.test(ENT_ACTIVITY_BIT); }
    void EntityRecord::activate()
    { groups.set(ENT_ACTIVITY_BIT); }
    void EntityRecord::deactivate()
    { groups.reset(ENT_ACTIVITY_BIT); }
    bool EntityRecord::setActivity(bool activity)
    {
        bool result{groups.testAndSet(ENT_ACTIVITY_BIT, activity)};
        return result != activity;
    }

    void EntityRecord::setComp(u64 index)
    { components.set(index); }
    void EntityRecord::resetComp(u64 index)
    { components.reset(index); }
    bool EntityRecord::testComp(u64 index) const
    { return components.test(index); }
    const ComponentBitset &EntityRecord::comp() const
    { return components; }
    ComponentBitset &EntityRecord::comp()
    { return components; }

    void EntityRecord::setGrp(u64 index)
    { groups.set(index); }
    void EntityRecord::resetGrp(u64 index)
    { groups.reset(index); }
    bool EntityRecord::testGrp(u64 index) const
    { return groups.test(index); }
    const GroupBitset &EntityRecord::grp() const
    { return groups; }
    GroupBitset &EntityRecord::grp()
    { return groups; }

    EIdType &EntityRecord::gen()
    { return generation; }
    const EIdType &EntityRecord::gen() const
    { return generation; }

    EIdType &EntityRecord::next()
    { return nextFree; }
    const EIdType &EntityRecord::next() const
    { return nextFree; }
    // EntityRecord implementation end.

    // ActiveEntityIterator implementation.
    ActiveEntityIterator::ActiveEntityIterator(EntityRecord *ptr,
                                               EIdType size) :
        mIndex{0u}, mSize{size}, mPtr{ptr}
    {
        if (!mPtr->active())
        { // We started on non-active Entity record.
            increment();
        }
    }

    EntityId ActiveEntityIterator::id() const
    { return EntityId(mIndex, mPtr->gen()); }

    EntityRecord &ActiveEntityIterator::record()
    { return *mPtr; }
    const EntityRecord &ActiveEntityIterator::record() const
    { return *mPtr; }

    void ActiveEntityIterator::operator++()
    { increment(); }

    bool ActiveEntityIterator::valid() const
    { return mIndex < mSize; }

    bool ActiveEntityIterator::active() const
    { return valid() && mPtr->active(); }

    void ActiveEntityIterator::increment()
    {
        if (valid())
        {
            do {
                moveNext();
            } while (valid() && !mPtr->active());
        }
    }

    void ActiveEntityIterator::moveNext()
    {
        mPtr++;
        mIndex++;
    }
    // ActiveEntityIterator implementation end.

    // EntityHolder implementation.
    void EntityHolder::addComponent(EntityId id, u64 index)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].setComp(index); }

    void EntityHolder::removeComponent(EntityId id, u64 index)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].resetComp(index); }

    bool EntityHolder::hasComponent(EntityId id, u64 index) const
    { return valid(id) && mRecords[id.index()].testComp(index); }

    bool EntityHolder::setActivity(EntityId id, bool activity)
    {
        ENT_ASSERT_SLOW(valid(id));
        return mRecords[id.index()].setActivity(activity);
    }

    void EntityHolder::activate(EntityId id)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].activate(); }

    void EntityHolder::deactivate(EntityId id)
    { ENT_ASSERT_SLOW(valid(id)); mRecords[id.index()].deactivate(); }

    bool EntityHolder::active(EntityId id) const
    { return valid(id) && activeImpl(id.index()); }

    bool EntityHolder::valid(EntityId id) const
    { return validImpl(id.index(), id.generation()); }

    const ComponentBitset &EntityHolder::components(EntityId id) const
    { ENT_ASSERT_SLOW(indexValid(id.index())); return mRecords[id.index()].comp(); }

    const GroupBitset &EntityHolder::groups(EntityId id)
    { ENT_ASSERT_SLOW(indexValid(id.index())); return mRecords[id.index()].grp(); }

    void EntityHolder::setGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(valid(id) && groupId < ENT_MAX_GROUPS); mRecords[id.index()].setGrp(groupId); }

    void EntityHolder::resetGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(indexValid(id.index()) && groupId < ENT_MAX_GROUPS); mRecords[id.index()].resetGrp(groupId); }

    ActiveEntityIterator EntityHolder::activeEntities()
    { return ActiveEntityIterator(mRecords.begin(), mRecords.size()); }

    void EntityHolder::resetGroups(const GroupBitset &andMask)
    {
        if (andMask.all())
        { // No work to be done.
            return;
        }

        for (EntityRecord &rec : mRecords)
        {
            rec.grp() &= andMask;
        }
    }

    void EntityHolder::initEntity(EIdType index)
    { mRecords[index].reset(); }

    bool EntityHolder::activeImpl(EIdType index) const
    { return mRecords[index].active(); }

    bool EntityHolder::validImpl(EIdType index, EIdType gen) const
    { return indexValid(index) && genValid(index, gen); }

    bool EntityHolder::indexValid(EIdType index) const
    { return index && index < mRecords.size(); }

    bool EntityHolder::genValid(EIdType index, EIdType gen) const
    { return mRecords[index].gen() == gen; }

    void EntityHolder::pushFreeId(EIdType index)
    {
        EntityRecord &recNew = mRecords[index];
        EntityRecord &recOld = mRecords[mLastFree];

        if (mLastFree)
        { // If there is at least one element in the list, add the new one.
            recOld.next() = index;
        }
        else
        { // Else, we need to init the list.
            mFirstFree = index;
        }

        recNew.next() = 0;
        mLastFree = index;

        mNumFree++;
    }

    EIdType EntityHolder::popFreeId()
    {
        EIdType result{mFirstFree};

        mFirstFree = mRecords[result].next();

        // If the result ID is 0, then there are no more IDs available.
        mNumFree = result ? mNumFree - 1 : 0;

        return result;
    }

    u64 EntityHolder::addGroup()
    {
        u64 result;

        if (mFreeGroupIds.size())
        {
            result = mFreeGroupIds.back();
            mFreeGroupIds.popBack();
        }
        else
        {
            result = mLastGroupId++;
        }

        return result;
    }

    void EntityHolder::removeGroup(u64 groupId)
    {
        mFreeGroupIds.insertUnique(groupId);
    }
    // EntityHolder implementation end.
#endif
}
