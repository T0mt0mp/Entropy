/**
 * @file Entropy/EntityMetadata.inl
 * @author Tomas Polasek
 * @brief Classes for handling of Entity metadata.
 */

#include "EntityMetadata.h"

/// Main Entropy namespace
namespace ent
{
    // ValidEntityIterator implementation.
    ValidEntityIterator::ValidEntityIterator(const MetadataBitset *begin,
                                             const MetadataBitset *end,
                                             EIdType last) :
        mIt{begin}, mEnd{end}, mCurrentInd{0u}, mEndInd{last}
    { increment(); }

    EIdType ValidEntityIterator::index() const
    { return mCurrentInd; }

    bool ValidEntityIterator::valid() const
    { return mCurrentInd != mEndInd && mIt != mEnd; }

    bool ValidEntityIterator::increment()
    {
        // TODO - refactor, rethink.

        if (!valid())
        { // Invalid stays invalid.
            return false;
        }

        // Move onto the next index.
        mCurrentInd++;

        if (mCurrentInd % MetadataBitset::size() == 0u)
        { // If we got into the next block.
            // Move onto the next block.
            ++mIt;

            while (mIt != mEnd && !mIt->any())
            { // Find the next block with valid Entities.
                ++mIt;
                mCurrentInd += MetadataBitset::size();
            }

            if (!valid())
            {
                return false;
            }
        }

        return true;
    }

    // ValidEntityIterator implementation end.

    // EntityFilter implementation.
    EntityFilter::EntityFilter() :
        mValue{0u}, mCompPos{0u},
        mCompPosUsed{0u}
    { }

    void EntityFilter::setRequiredActivity(bool activity)
    {
        mValue.set(ACTIVITY_BIT, activity);
    }

    void EntityFilter::requireComponent(CIdType cId)
    {
        ENT_ASSERT_FAST(mCompPosUsed < ENT_GROUP_FILTER_BITS);
        mValue.set(mCompPosUsed);
        mCompPos[mCompPosUsed++] = cId;
    }

    void EntityFilter::rejectComponent(CIdType cId)
    {
        ENT_ASSERT_FAST(mCompPosUsed < ENT_GROUP_FILTER_BITS);
        // Zero initialized - should not be required.
        //mValue.reset(mCompPosUsed);
        mCompPos[mCompPosUsed++] = cId;
    }

    void EntityFilter::addComponent(CIdType cId, bool required)
    {
        ENT_ASSERT_FAST(mCompPosUsed < ENT_GROUP_FILTER_BITS);
        mValue.set(mCompPosUsed, required);
        mCompPos[mCompPosUsed++] = cId;
    }

    bool EntityFilter::match(const FilterBitset &bitset) const
    {
        return mValue == bitset;
    }

    const CIdType *EntityFilter::compPositions() const
    { return mCompPos; }

    const u64 EntityFilter::compPositionsUsed() const
    { return mCompPosUsed; }

    bool EntityFilter::operator==(const EntityFilter &rhs) const
    {
        return mCompPosUsed == rhs.mCompPosUsed &&
               compPosEqual(rhs.mCompPos) &&
               mValue == rhs.mValue;
    }

    bool EntityFilter::compPosEqual(const CIdType *rhsCompPos) const
    {
        for (u64 index = 0; index < mCompPosUsed; ++index)
        {
            if (mCompPos[index] != rhsCompPos[index])
            {
                return false;
            }
        }
        return true;
    }

    std::ostream &operator<<(std::ostream &out, const EntityFilter &rhs)
    {
        out << "val: " << rhs.mValue;
        return out;
    }
    // EntityFilter implementation end.

    // MetadataGroup implementation.
    MetadataGroup::MetadataGroup(u64 columns, u64 rows) :
        mColumns{0u}, mEntities{0u}, mEntityCapacity{0u},
        mColumnSize{0u}
    {
        init(columns, rows);
    }

    MetadataGroup::~MetadataGroup()
    { }

    void MetadataGroup::setColumns(u64 columns)
    {
        resize(columns, mEntities);
    }

    void MetadataGroup::reserve(u64 capacity)
    {
        if (capacity > mEntityCapacity)
        {
            resize(mColumns, capacity);
        }
    }

    void MetadataGroup::resize(u64 columns, u64 rows)
    {
        if (columns == mColumns && mEntityCapacity >= rows)
        { // Nothing to be done.
            return;
        }
        else if (rows == 0u || columns == 0u)
        { // No data allocation needed.
            mData.reclaim();
            mColumns = columns;
            mEntities = mEntities < rows ? mEntities : rows;
            mEntityCapacity = rows;
            mColumnSize = 0u;
            return;
        }

        // Allocate new memory.
        // Real number of bitsets after allocation.
        u64 realSize{pow2RoundUp((bitsetIndex(rows - 1u) + 1u) * columns)};
        // Real number of bitsets per columns after allocation.
        u64 realColumnSize{realSize / columns};
        // Real number or rows after allocation.
        u64 realRows{realColumnSize * ENT_PER_BITSET};
        ENT_ASSERT_SLOW(rows <= realRows);

        decltype(mData) newData;
        newData.resize(realSize);
        ENT_ASSERT_FAST(newData.capacity() == realSize);

        u64 rowsToCopy{mEntities <= rows ? mEntities : rows};
        u64 bitsetsToCopy{rowsToCopy != 0u ? bitsetIndex(rowsToCopy - 1u) + 1u : 0u};

        u64 colsToCopy{mColumns <= columns ? mColumns : columns};

        u64 col{0u};

        // Copy the old data.
        for (; col < colsToCopy; ++col)
        {
            MetadataBitset *beg{begin(col, realColumnSize, newData)};
            // Copy the data.
            copyData(beg, begin(col), bitsetsToCopy);
            // Zero initialize the rest.
            zeroInitialize(beg + bitsetsToCopy, end(col, realColumnSize, newData));
        }
        // Completely zero initialize the rest of the new columns.
        if (col < columns)
        {
            zeroInitialize(begin(col, realColumnSize, newData), begin(columns, realColumnSize, newData));
        }

        // Swap new and old.
        mData.swap(newData);

        mColumns = columns;
        // The number of Entities stays the same.
        mEntityCapacity = realRows;
        mColumnSize = realColumnSize;
    }

    void MetadataGroup::pushBackRow()
    {
        reserve(mEntities + 1u);
        mEntities++;
    }

    bool MetadataGroup::inSameBitset(u64 first, u64 second) const
    {
        return bitsetIndex(first) == bitsetIndex(second);
    }

    bool MetadataGroup::bit(u64 column, u64 row) const
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        return mData[index].test(r.second);
    }

    void MetadataGroup::setBit(u64 column, u64 row)
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        mData[index].set(r.second);
    }

    void MetadataGroup::setBit(u64 column, u64 row, bool val)
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        mData[index].set(r.second, val);
    }

    bool MetadataGroup::testSetBit(u64 column, u64 row, bool val)
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        return mData[index].testAndSet(r.second, val);
    }

    void MetadataGroup::resetBit(u64 column, u64 row)
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        mData[index].reset(r.second);
    }

    MetadataBitset &MetadataGroup::bitsetBit(u64 &bitIndex, u64 column, u64 row)
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(row < mEntities && index < mData.size());
        bitIndex = r.second;
        return mData[index];
    }

    const MetadataBitset &MetadataGroup::bitsetBit(u64 &bitIndex, u64 column, u64 row) const
    {
        std::pair<u64, u64> r{bitsetRowIndex(row)};
        u64 index{mColumnSize * column + r.first};
        ENT_ASSERT_SLOW(index < mData.size());
        bitIndex = r.second;
        return mData[index];
    }

    MetadataBitset &MetadataGroup::bitset(u64 column, u64 row)
    {
        u64 index{mColumnSize * column + bitsetIndex(row)};
        ENT_ASSERT_SLOW(index < mData.size());
        return mData[index];
    }

    const MetadataBitset &MetadataGroup::bitset(u64 column, u64 row) const
    {
        u64 index{mColumnSize * column + bitsetIndex(row)};
        ENT_ASSERT_SLOW(index < mData.size());
        return mData[index];
    }

    u64 MetadataGroup::columns() const
    { return mColumns; }

    u64 MetadataGroup::rows() const
    { return mEntities; }

    u64 MetadataGroup::capacity() const
    { return mEntityCapacity; }

    void MetadataGroup::setZero(u64 column)
    { zeroInitialize(begin(column), end(column)); }

    void MetadataGroup::init(u64 columns, u64 rows)
    {
        resize(columns, rows);
        mEntities = rows;
    }

    void MetadataGroup::reset()
    {
        mColumns = 0u;
        mEntities = 0u;
        mEntityCapacity = 0u;
        mColumnSize = 0u;
        mData.reclaim();
    }

    void MetadataGroup::clear()
    {
        // mColumns stay the same.
        mEntities = 0u;
        mEntityCapacity = 0u;
        mColumnSize = 0u;
        mData.reclaim();
    }

    void MetadataGroup::zeroInitialize(MetadataBitset *beg, MetadataBitset *end)
    {
        std::memset(beg, 0, (end - beg) * sizeof(MetadataBitset));
    }

    void MetadataGroup::copyData(MetadataBitset *dest, MetadataBitset *src, u64 numBitsets)
    {
        std::memcpy(dest, src, numBitsets * sizeof(MetadataBitset));
    }

    u64 MetadataGroup::bitsetIndex(u64 row) const
    { return row / ENT_PER_BITSET; }

    std::pair<u64, u64> MetadataGroup::bitsetRowIndex(u64 row) const
    {
        std::lldiv_t res = std::lldiv(row, ENT_PER_BITSET);
        return {res.quot, res.rem};
    }

    MetadataBitset *MetadataGroup::begin(u64 column)
    { ENT_ASSERT_SLOW(column < mColumns); return begin(column, mColumnSize, mData); }

    MetadataBitset *MetadataGroup::begin(u64 column) const
    { return const_cast<MetadataGroup*>(this)->begin(column); }

    MetadataBitset *MetadataGroup::begin(u64 column, u64 columnSize, ent::List<MetadataBitset> &data)
    { return begin(data) + column * columnSize; }

    MetadataBitset *MetadataGroup::end(u64 column)
    { ENT_ASSERT_SLOW(column < mColumns); return end(column, mColumnSize, mData); }

    MetadataBitset *MetadataGroup::end(u64 column) const
    { return const_cast<MetadataGroup*>(this)->end(column); }

    MetadataBitset *MetadataGroup::end(u64 column, u64 columnSize, ent::List<MetadataBitset> &data)
    { return begin(data) + (column + 1u) * columnSize; }

    MetadataBitset *MetadataGroup::begin()
    { return begin(mData); }

    MetadataBitset *MetadataGroup::begin(ent::List<MetadataBitset> &data)
    { return data.begin(); }

    MetadataBitset *MetadataGroup::end()
    { return end(mData); }

    MetadataBitset *MetadataGroup::end(ent::List<MetadataBitset> &data)
    { return data.end(); }
    // MetadataGroup implementation end.

    // EntityMetadata implementation.
    EntityMetadata::EntityMetadata() :
        mEntityCapacity{0u}, mEntityLast{0u}, mNewGroupRequests{0u}
    {
    }

    void EntityMetadata::reset()
    {
        mEntityCapacity = 0u;
        mEntityLast = 0u;

        mMetadata.components.reset();
        mMetadata.groups.reset();
        mMetadata.flags.reset();
        mMetadata.generations.reclaim();

        mFreeIndexes.clear();
        mFreeGroupIds.reclaim();
        mNewGroupRequests = 0u;
    }

    void EntityMetadata::init(CIdType numComponents)
    {
        mMetadata.components.setColumns(numComponents);
        mMetadata.flags.setColumns(Flags::NUM_FLAGS);
        // Create the first Entity, valid Entities start at 1.
        pushEntity();
        ENT_ASSERT_FAST(mEntityLast == 1u);
    }

    void EntityMetadata::refresh()
    {
        // Find how many of the groups we can delete.
        u64 groupsToRemove{0u};
        u64 expectedId{mMetadata.groups.columns()};
        for (u64 id : mFreeGroupIds)
        {
            if (id != expectedId)
            {
                break;
            }
            else
            {
                groupsToRemove++;
            }
        }

        u64 origGroups{mMetadata.groups.columns()};
        u64 finalGroups{origGroups + mNewGroupRequests - groupsToRemove};

        if (finalGroups != origGroups)
        {
            mMetadata.groups.setColumns(finalGroups);
        }

        mNewGroupRequests = 0u;
    }

    EntityId EntityMetadata::create()
    {
        // If initialized, value should not be zero.
        ENT_ASSERT_SLOW(mEntityLast);

        EIdType index{0u};
        EIdType gen{EntityId::START_GEN};

        if (mFreeIndexes.size() >= ENT_MIN_FREE)
        {
            index = popFreeIndex();
            ENT_ASSERT_SLOW(index);
            ENT_ASSERT_SLOW(!createdInd(index));
            ENT_ASSERT_SLOW(!activityInd(index));
            ENT_ASSERT_SLOW(groupNoneInd(index));
            ENT_ASSERT_SLOW(compNoneInd(index));

            createInd(index);
            activateInd(index);

            // Generation is incremented on Entity destroy.
            gen = genInd(index);
        }
        else
        {
            index = pushEntity();
            ENT_ASSERT_SLOW(index);

            createInd(index);
            activateInd(index);

            gen = EntityId::START_GEN;
            ENT_ASSERT_SLOW(gen == genInd(index));
        }

        return EntityId(index, gen);
    }

    void EntityMetadata::addComponent(EntityId id, CIdType compId)
    { ENT_ASSERT_SLOW(validImpl(id)); setCompInd(id.index(), compId, true); }

    void EntityMetadata::removeComponent(EntityId id, CIdType compId)
    { ENT_ASSERT_SLOW(validInd(id.index())); setCompInd(id.index(), compId, false); }

    bool EntityMetadata::hasComponent(EntityId id, CIdType compId) const
    { ENT_ASSERT_SLOW(validInd(id.index())); return getCompInd(id.index(), compId); }

    EIdType EntityMetadata::currentGen(EIdType index) const
    { ENT_ASSERT_SLOW(validInd(index)); return genInd(index); }

    bool EntityMetadata::setActivity(EntityId id, bool activity)
    { ENT_ASSERT_SLOW(validImpl(id)); return setActivityInd(id.index(), activity); }

    void EntityMetadata::activate(EntityId id)
    { ENT_ASSERT_SLOW(validImpl(id)); activateInd(id.index()); }

    void EntityMetadata::deactivate(EntityId id)
    { ENT_ASSERT_SLOW(validImpl(id)); deactivateInd(id.index()); }

    bool EntityMetadata::destroy(EntityId id)
    {
        if (!valid(id))
        { // Entity does not exist!
            return false;
        }

        EIdType index{id.index()};
        EIdType &gen(genInd(index));
        gen = (gen + 1u == EntityId::MAX_GEN) ? 0u : gen + 1u;
        resetEntity(index);
        pushFreeIndex(index);

        return true;
    }

    bool EntityMetadata::valid(EntityId id) const
    { return validImpl(id); }

    bool EntityMetadata::active(EntityId id) const
    { return valid(id) && activityInd(id.index()); }

    bool EntityMetadata::inGroup(EntityId id, u64 groupId) const
    { ENT_ASSERT_SLOW(validInd(id.index())); return getGroupInd(id.index(), groupId); }

    void EntityMetadata::setGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(validInd(id.index())); setGroupInd(id.index(), groupId, true); }

    void EntityMetadata::resetGroup(EntityId id, u64 groupId)
    { ENT_ASSERT_SLOW(validInd(id.index())); setGroupInd(id.index(), groupId, false); }

    u64 EntityMetadata::addGroup()
    {
        u64 result{0u};

        if (mFreeGroupIds.size())
        { // We can use on which already exists.
            result = mFreeGroupIds.back();
            mFreeGroupIds.popBack();
        }
        else
        { // We will need to create a new one.
            result = mMetadata.groups.columns() + mNewGroupRequests;
            mNewGroupRequests++;
        }

        return result;
    }

    void EntityMetadata::removeGroup(u64 groupId)
    {
        mMetadata.groups.setZero(groupId);
        mFreeGroupIds.insertUnique(groupId);
    }

    FilterBitset EntityMetadata::compressInfo(
        const EntityFilter &filter, EIdType index) const
    {
        FilterBitset result{0u};

        const CIdType *comps{filter.compPositions()};
        u64 compSize{filter.compPositionsUsed()};

        for (u64 iii = 0; iii < compSize; ++iii)
        {
            result.set(iii, getCompInd(index, comps[iii]));
        }

        result.set(EntityFilter::ACTIVITY_BIT, activityInd(index));

        return result;
    }

    ValidEntityIterator EntityMetadata::validEntities() const
    {
        return ValidEntityIterator(
            mMetadata.flags.begin(Flags::CREATED),
            mMetadata.flags.end(Flags::CREATED),
            mEntityLast
        );
    }

    EIdType EntityMetadata::pushEntity()
    {
        if (mEntityLast < mEntityCapacity)
        { // No need to reallocate.
        }
        else
        { // Need to allocate more space.
            ENT_ASSERT_SLOW(mEntityCapacity == mMetadata.components.rows());
            ENT_ASSERT_SLOW(mEntityCapacity == mMetadata.groups.rows());
            ENT_ASSERT_SLOW(mEntityCapacity == mMetadata.flags.rows());
            ENT_ASSERT_SLOW(mEntityCapacity == mMetadata.generations.size());

            mMetadata.components.reserve(mEntityCapacity + ENT_PUSH_NUM);
            mMetadata.groups.reserve(mEntityCapacity + ENT_PUSH_NUM);
            mMetadata.flags.reserve(mEntityCapacity + ENT_PUSH_NUM);
            mMetadata.generations.resize(mEntityCapacity + ENT_PUSH_NUM, 0u);
            mEntityCapacity += ENT_PUSH_NUM;
        }

        mMetadata.groups.pushBackRow();
        mMetadata.components.pushBackRow();
        mMetadata.flags.pushBackRow();

        ENT_ASSERT_SLOW(mEntityLast < EntityId::MAX_INDEX);
        return mEntityLast++;
    }

    void EntityMetadata::pushFreeIndex(EIdType index)
    { mFreeIndexes.push_back(index); }

    EIdType EntityMetadata::popFreeIndex()
    {
        ENT_ASSERT_SLOW(mFreeIndexes.size());

        EIdType result{mFreeIndexes.front()};
        mFreeIndexes.pop_front();

        return result;
    }

    bool EntityMetadata::createdInd(EIdType index) const
    { return mMetadata.flags.bit(Flags::CREATED, index); }

    void EntityMetadata::createInd(EIdType index)
    { setCreatedInd(index, true); }

    void EntityMetadata::destroyInd(EIdType index)
    { setCreatedInd(index, false); }

    void EntityMetadata::setCreatedInd(EIdType index, bool activity)
    { mMetadata.flags.setBit(Flags::CREATED, index, activity); }

    bool EntityMetadata::activityInd(EIdType index) const
    { return mMetadata.flags.bit(Flags::ACTIVITY, index); }

    void EntityMetadata::activateInd(EIdType index)
    { setActivityInd(index, true); }

    void EntityMetadata::deactivateInd(EIdType index)
    { setActivityInd(index, false); }

    bool EntityMetadata::setActivityInd(EIdType index, bool activity)
    { return mMetadata.flags.testSetBit(Flags::ACTIVITY, index, activity); }

    EIdType &EntityMetadata::genInd(EIdType index)
    { return mMetadata.generations[index]; }

    const EIdType &EntityMetadata::genInd(EIdType index) const
    { return mMetadata.generations[index]; }

    bool EntityMetadata::groupNoneInd(EIdType index) const
    {
        u64 numGroups{mMetadata.groups.columns()};
        for (u64 iii = 0; iii < numGroups; ++iii)
        {
            if (getGroupInd(index, iii))
            {
                return false;
            }
        }
        return true;
    }

    void EntityMetadata::setGroupInd(EIdType index, u64 groupId, bool value)
    { mMetadata.groups.setBit(groupId, index, value); }

    bool EntityMetadata::getGroupInd(EIdType index, u64 groupId) const
    { return mMetadata.groups.bit(groupId, index); }

    bool EntityMetadata::compNoneInd(EIdType index) const
    {
        u64 numGroups{mMetadata.components.columns()};
        for (u64 iii = 0; iii < numGroups; ++iii)
        {
            if (getCompInd(index, iii))
            {
                return false;
            }
        }
        return true;
    }

    void EntityMetadata::setCompInd(EIdType index, CIdType compId, bool value)
    { mMetadata.components.setBit(compId, index, value); }

    bool EntityMetadata::getCompInd(EIdType index, CIdType compId) const
    { return mMetadata.components.bit(compId, index); }

    bool EntityMetadata::validImpl(EntityId id) const
    { return validInd(id.index()) && createdInd(id.index()) && validGen(id.index(), id.generation()); }

    bool EntityMetadata::validInd(EIdType index) const
    { return index < mEntityLast; }

    bool EntityMetadata::validGen(EIdType index, EIdType gen) const
    { return genInd(index) == gen; }

    void EntityMetadata::resetEntity(EIdType index)
    {
        deactivateInd(index);
        destroyInd(index);
    }

    // EntityMetadata implementation end.
} // namespace ent

