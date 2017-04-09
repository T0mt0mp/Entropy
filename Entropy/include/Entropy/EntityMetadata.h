/**
 * @file Entropy/EntityMetadata.h
 * @author Tomas Polasek
 * @brief Classes for handling of Entity metadata.
 */

#ifndef ECS_FIT_ENTITYMETADATA_H
#define ECS_FIT_ENTITYMETADATA_H

// For std::div.
#include <cstdlib>

#include "Util.h"
#include "Types.h"
#include "EntityId.h"
#include "List.h"
#include "SortedList.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * ComponentFilter is used for filtering Entities by their
     * present/missing Components and activity.
     */
    class EntityFilter final : NonCopyable
    {
    public:
        /// Bits used in other ways than the Component bits.
        static constexpr u64 USED_BITS{1u};
        /// Index of bit representing activity.
        static constexpr u64 ACTIVITY_BIT{ENT_GROUP_FILTER_BITS - 1u};
        /// Size of array containing mapping of Components.
        static constexpr u64 COMP_POS_SIZE{ENT_GROUP_FILTER_BITS - USED_BITS};

        /// Create empty filter.
        inline EntityFilter();

        /**
         * Set the activity value (active or inactive) for this
         * filter.
         * @param activity The activity value.
         */
        inline void setRequiredActivity(bool activity);

        /**
         * Add new required Component type.
         * @param cId ID of the Component.
         */
        inline void requireComponent(CIdType cId);

        /**
         * Add new rejected Component type.
         * @param cId ID of the Component.
         */
        inline void rejectComponent(CIdType cId);

        /**
         * Check if the given bitset passes this filter.
         * @param bitset Bitset to check.
         * @return Returns true, if the bitset passes through this filter.
         */
        inline bool match(const FilterBitset &bitset) const;

        /// Get array of Component positions.
        const CIdType *compPositions() const;

        /// Get number of used elements in the Component position array.
        const u64 compPositionsUsed() const;

        /// Print operator.
        friend inline std::ostream &operator<<(std::ostream &out, const EntityFilter &rhs);
    private:
        /// Required value in order to pass this filter.
        FilterBitset mValue;
        /// List of Component position within the filter.
        CIdType mCompPos[COMP_POS_SIZE];
        /// How many Component bits are in use.
        u64 mCompPosUsed;
    protected:
    }; // class EntityFilter

    /**
     * Iterator for read-only iteration over all valid
     * Entities and their metadata.
     */
    class ValidEntityIterator
    {
    public:
        /**
         * Create iterator for given range.
         * The range has to be over CREATED metadata
         * bitsets.
         * @param begin Starting iterator over
         *   CREATED metadata bitsets.
         * @param end End iterator over CREATED
         *   metadata bitsets.
         */
        inline ValidEntityIterator(const MetadataBitset *begin,
                                   const MetadataBitset *end);

        /**
         * Get the current valid Entity index.
         * @return Returns index for valid Entity.
         */
        inline EIdType index() const;

        /**
         * Is this iterator currently valid?
         * @return Returns true, if the iterator
         *   has a valid next index value.
         */
        inline bool valid() const;

        /**
         * Move to the next valid index.
         * @return Returns validity.
         */
        inline bool increment();
    private:
        /// Current state iterator.
        const MetadataBitset *mIt;
        /// End iterator.
        const MetadataBitset *mEnd;
        /// Current Entity index.
        EIdType mCurrentInd;
    protected:
    }; // class ActiveEntityIterator

    /**
     * Metadata group is a table with one or more
     * columns of metadata bitsets.
     * The number of columns can be changes on runtime.
     * Container allows uniform resizing of all
     * columns and work with rows (Entity metadata).
     * Uses an ent::List within.
     * Memory is always, unless specified otherwise, zero
     * initialized.
     */
    class MetadataGroup
    {
    public:
        /**
         * Construct the group with provided parameters.
         * All memory is zero initialized.
         * @param columns Number of columns in the table.
         * @param rows Number of rows (Entities) in the table.
         */
        inline MetadataGroup(u64 columns = 0u, u64 rows = 0u);

        /// Free all allocated memory.
        inline ~MetadataGroup();

        /**
         * Free all data and reset members.
         */
        inline void reset();

        /**
         * Clears all rows from the table.
         * Columns stay unchanged.
         */
        inline void clear();

        /**
         * Set the number of columns to requested value.
         * If the new number of columns is smaller than
         * the current number of columns, then only the
         * first number of columns is preserved.
         * If the new number of columns is larger than
         * the current number of columns, then all
         * of the data from the old columns is copied
         * and the new column is zero initialized.
         * @param columns The new number of columns.
         * @remarks If the requested number of columns
         *   is zero, all rows are deleted!
         */
        inline void setColumns(u64 columns);

        /**
         * Increase the number of rows to be at least
         * the requested capacity.
         * @param capacity The requested capacity.
         */
        inline void reserve(u64 capacity);

        /**
         * Resize the table to given dimensions.
         * @param columns The number of columns.
         * @param rows The number of rows.
         */
        inline void resize(u64 columns, u64 rows);

        /**
         * Push back row, resize if required.
         */
        inline void pushBackRow();

        /**
         * Are the 2 specified indices contained
         * within the same bitset?
         * @param first The first index.
         * @param second The second index.
         * @return Returns true, if they are in the
         *   same bitset.
         * @remarks Does not actualy check index validity!
         */
        inline bool inSameBitset(u64 first, u64 second) const;

        /**
         * Get bit located at given column and row.
         * @param column Column index.
         * @param row Row index.
         * @return Returns value on specified cell.
         */
        inline bool bit(u64 column, u64 row) const;

        /**
         * Set bit located at given column and row.
         * @param column Column index.
         * @param row Row index.
         */
        inline void setBit(u64 column, u64 row);

        /**
         * Set bit located at given column and row
         * to given value.
         * @param column Column index.
         * @param row Row index.
         * @param val Value to set.
         */
        inline void setBit(u64 column, u64 row, bool val);

        /**
         * Set bit located at given column and row
         * to given value.
         * @param column Column index.
         * @param row Row index.
         * @param val Value to set.
         * @return Returns the original value of the cell.
         */
        inline bool testSetBit(u64 column, u64 row, bool val);

        /**
         * Reset bit located at given column and row.
         * @param column Column index.
         * @param row Row index.
         */
        inline void resetBit(u64 column, u64 row);

        /**
         * Get bitset, specified by column, which
         * contains given row.
         * Also calculates position of the bit within
         * the returned bitset and returns it.
         * @param bitIndex Returns the bit index to
         *   this reference.
         * @param column Column index.
         * @param row Row index.
         * @return Returns reference to the requested
         *   bitset.
         */
        inline MetadataBitset &bitsetBit(u64 &bitIndex, u64 column, u64 row);

        /**
         * Get bitset, specified by column, which
         * contains given row.
         * Also calculates position of the bit within
         * the returned bitset and returns it.
         * @param bitIndex Returns the bit index to
         *   this reference.
         * @param column Column index.
         * @param row Row index.
         * @return Returns reference to the requested
         *   bitset.
         */
        inline const MetadataBitset &bitsetBit(u64 &bitIndex, u64 column, u64 row) const;

        /**
         * Get bitset, specified by column, which
         * contains given row.
         * @param column Column index.
         * @param row Row index.
         * @return Returns reference to the requested
         *   bitset.
         */
        inline MetadataBitset &bitset(u64 column, u64 row);

        /**
         * Get bitset, specified by column, which
         * contains given row.
         * @param column Column index.
         * @param row Row index.
         * @return Returns reference to the requested
         *   bitset.
         */
        inline const MetadataBitset &bitset(u64 column, u64 row) const;

        /**
         * Get the current number of columns.
         * @return The current number of columns.
         */
        inline u64 columns() const;

        /**
         * Get the current number of Entities
         * in the table.
         * @return The current number of Entities
         *   in the table.
         */
        inline u64 rows() const;

        /**
         * Get the current row capacity.
         * @return The current row capacity.
         */
        inline u64 capacity() const;

        /**
         * Get pointer to the first bitset in
         * requested column.
         * @param column Requested column.
         * @return Returns ptr to the first
         *   bitset in requested column.
         */
        inline MetadataBitset *begin(u64 column);

        /**
         * Get pointer one past the last bitset in
         * requested column.
         * @param column Requested column.
         * @return Returns ptr one past the last
         *   bitset in requested column.
         */
        inline MetadataBitset *end(u64 column);

        /**
         * Get pointer to the beginning of bitset data.
         * @return Returns ptr to the beginning of
         *   bitset data.
         */
        inline MetadataBitset *begin();

        /**
         * Get pointer to one past the last bitset.
         * @return Returns ptr to one past the
         *   last bitset.
         */
        inline MetadataBitset *end();

        /**
         * Set whole column to zero.
         * @param column ID of the column.
         */
        inline void setZero(u64 column);
    private:
        /// Number of Entities per bitset.
        static constexpr u64 ENT_PER_BITSET{MetadataBitset::size()};

        /**
         * First time initialization.
         * Used values of columns and rows
         * members.
         * @param columns Number of columns.
         * @param rows Number of rows.
         */
        inline void init(u64 columns, u64 rows);

        /**
         * Zero initialize memory between 2 pointers.
         * Includes beg and excludes end.
         * @param beg Begin iterator.
         * @param end End iterator, excluded from initialization.
         */
        inline void zeroInitialize(MetadataBitset *beg, MetadataBitset *end);

        /**
         * Copy data from source to destination.
         * @param dest Destination pointer.
         * @param src Source pointer.
         * @param numBitsets Number of bitsets to copy.
         */
        inline void copyData(MetadataBitset *dest, MetadataBitset *src, u64 numBitsets);

        /**
         * Get bitset index for given row.
         * @param row The row.
         * @return Returns index of bitset which
         *   containes given row.
         */
        inline u64 bitsetIndex(u64 row) const;

        /**
         * Get index of a bitset and a bit
         * within that bitset for given row.
         * @param row Desired row index.
         * @return Returned structure will be filled within
         *   the function. first = bitset index and
         *   second = bit index wihin the bitset.
         */
        inline std::pair<u64, u64> bitsetRowIndex(u64 row) const;

        /**
         * Get pointer to the first bitset in
         * requested column.
         * @param column Requested column.
         * @param columnSize Size of the column in bitsets.
         * @param data The data.
         * @return Returns ptr to the first
         *   bitset in requested column.
         */
        inline MetadataBitset *begin(u64 column, u64 columnSize, ent::List<MetadataBitset> &data);

        /**
         * Get pointer one past the last bitset in
         * requested column.
         * @param column Requested column.
         * @param columnSize Size of the column in bitsets.
         * @param data The data.
         * @return Returns ptr one past the last
         *   bitset in requested column.
         */
        inline MetadataBitset *end(u64 column, u64 columnSize, ent::List<MetadataBitset> &data);

        /**
         * Get pointer to the beginning of bitset data.
         * @param data The data.
         * @return Returns ptr to the beginning of
         *   bitset data.
         */
        inline MetadataBitset *begin(ent::List<MetadataBitset> &data);

        /**
         * Get pointer to one past the last bitset.
         * @param data The data.
         * @return Returns ptr to one past the
         *   last bitset.
         */
        inline MetadataBitset *end(ent::List<MetadataBitset> &data);

        /// How many columns does this table currently have?
        u64 mColumns;
        /// Number of rows in Entities.
        u64 mEntities;
        /// How many Entities can be created without resizing.
        u64 mEntityCapacity;
        /// Column size in a number of bitsets.
        u64 mColumnSize;
        /// The actual memory container.
        ent::List<MetadataBitset> mData;
    protected:
    }; // class MetadataGroup

    /**
     * Holder for Entity metadata.
     * Structure:
     *   Each Entity is represented by one row in the table.
     *   Each Entity has current value of its generation number.
     *   Each Entity has one bit representing its activity.
     *   Each Entity has a number of bits representing if it
     *     has a Component of specified type present.
     *   Each Entity has a number of bits representing its
     *     presence in EntityGroups.
     * Bits are stored in bitsets, where multiple Entities
     * share (specified in ENT_BITSET_GROUP_SIZE) a bitset
     * with multiple other Entities.
     * This means the bitsets span multiple Entities, representing
     * blocks of bits for multiple rows in the table.
     * This allows parallel access to different bitset groups
     * and bitset columns.
     */
    class EntityMetadata
    {
    public:
        /// Initialize the structure
        inline EntityMetadata();

        /// Reset the metadata to default state.
        inline void reset();

        /**
         * Initialize the internal structure for
         * given number of Component types.
         * @param numComponents Number of Component
         *   types.
         */
        inline void init(CIdType numComponents);

        /**
         * Refresh the metadata.
         */
        inline void refresh();

        /**
         * Create a new Entity and return its ID.
         * @return Returns ID of the new Entity.
         */
        inline EntityId create();

        /**
         * Mark component as present for given Entity.
         * @param id ID of the Entity.
         * @param compId Index of the Component.
         */
        inline void addComponent(EntityId id, CIdType compId);

        /**
         * Mark component as not present for given Entity.
         * @param id ID of the Entity.
         * @param compId Index of the Component.
         */
        inline void removeComponent(EntityId id, CIdType compId);

        /**
         * Does given Entity have the Component?
         * @param id ID of the Entity.
         * @param compId Index of the Component.
         * @return Returns true, if the Component is present.
         */
        inline bool hasComponent(EntityId id, CIdType compId) const;

        /**
         * Get the current generation for given
         * Entity index.
         * @param index Index of the Entity.
         * @return Returns the current generation.
         */
        inline EIdType currentGen(EIdType index) const;

        /**
         * Set activity of given Entity to
         * desired value.
         * @param id ID of the Entity.
         * @param activity Value of activity.
         * @return Returns true, if the activity
         *   has changed - has not stayed the same.
         */
        inline bool setActivity(EntityId id, bool activity);

        /**
         * Activate given Entity.
         * @param id ID of the Entity.
         */
        inline void activate(EntityId id);

        /**
         * Deactivate given Entity.
         * @param id ID of the Entity.
         */
        inline void deactivate(EntityId id);

        /**
         * Destroy given Entity.
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         */
        inline bool destroy(EntityId id);

        /**
         * Checks validity of given Entity.
         * Does not check for activity.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity exists.
         */
        inline bool valid(EntityId id) const;

        /**
         * Checks if the given Entity is active.
         * Also checks, if the Entity is valid (index + generation).
         * @param id ID of the Entity.
         * @return Returns true, if the Entity is active.
         */
        inline bool active(EntityId id) const;

        /**
         * Is given Entity in specified group?
         * @param id ID of the Entity.
         * @param groupId ID of the Entity Group.
         * @return Returns true, if the Entity is in
         *   specified Entity Group.
         */
        inline bool inGroup(EntityId id, u64 groupId) const;

        /**
         * Set Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the group. Starting at 0.
         */
        inline void setGroup(EntityId id, u64 groupId);

        /**
         * reset Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the group. Starting at 0.
         */
        inline void resetGroup(EntityId id, u64 groupId);

        /**
         * Add metadata for new EntityGroup.
         * @return Returns index of the metadata column.
         */
        inline u64 addGroup();

        /**
         * Remove EntityGroup metadata column.
         * @param groupId Index of the column.
         */
        inline void removeGroup(EIdType groupId);

        /**
         * Create a compressed filter value used for
         * filtering Entities.
         * @param filter Filter which will be used.
         * @param id Index of the Entity.
         * @return Returns compressed filter value, which
         *   can be used with provided filter.
         */
        inline FilterBitset compressInfo(
            const EntityFilter &filter, EIdType id) const;

        /**
         * Get iterator for all valid Entities.
         * @return Returns the iterator.
         */
        inline ValidEntityIterator validEntities() const;
    private:
        /**
         * Push new Entity to the metadata list.
         * All metadata attributes will be zero
         * initiated.
         * @return Returns ID of the new Entity.
         */
        inline EIdType pushEntity();

        /**
         * Push freed index to the list.
         * @param index The freed index.
         */
        inline void pushFreeIndex(EIdType index);

        /**
         * Pop freed index from the list.
         * @return Returns the free index.
         */
        inline EIdType popFreeIndex();

        /**
         * Get created flag of given Entity.
         * @param index Index of the Entity.
         */
        inline bool createdInd(EIdType index) const;

        /**
         * Set created flag to true for given
         * Entity.
         * @param index Index of the Entity.
         */
        inline void createInd(EIdType index);

        /**
         * Set created flag to false for given
         * Entity.
         * @param index Index of the Entity.
         */
        inline void destroyInd(EIdType index);

        /**
         * Set created flag to given value.
         * @param index Index of the Entity.
         * @param activity Activity value.
         */
        inline void setCreatedInd(EIdType index, bool activity);

        /**
         * Get activity flag of given Entity.
         * @param index Index of the Entity.
         */
        inline bool activityInd(EIdType index) const;

        /**
         * Set activity flag to true for given
         * Entity.
         * @param index Index of the Entity.
         */
        inline void activateInd(EIdType index);

        /**
         * Set activity flag to false for given
         * Entity.
         * @param index Index of the Entity.
         */
        inline void deactivateInd(EIdType index);

        /**
         * Set activity flag to given value.
         * @param index Index of the Entity.
         * @param activity Activity value.
         * @return Returns true, if the activity
         *   has changed.
         */
        inline bool setActivityInd(EIdType index, bool activity);

        /**
         * Get generation number of given Entity.
         * @param index Index of the Entity.
         * @return Returns reference to the
         *   generation.
         */
        inline EIdType &genInd(EIdType index);

        /**
         * Get generation number of given Entity.
         * @param index Index of the Entity.
         * @return Returns reference to the
         *   generation.
         */
        inline const EIdType &genInd(EIdType index) const;

        /**
         * Check all Group bits of given Entity, if
         * any of them are true, return false.
         * @return Returns true, if given Entity is
         *   not a member of any EntityGroup.
         * @remarks Slow operation!
         */
        inline bool groupNoneInd(EIdType index) const;

        /**
         * Set Group presence for given Entity to
         * provided value.
         * @param index Index of the Entity.
         * @param groupId Group ID.
         * @param value Presence information.
         */
        inline void setGroupInd(EIdType index, u64 groupId, bool value);

        /**
         * Get Group flag for given Entity.
         * @param index Index of the Entity.
         * @param groupId Group ID.
         * @return Returns the flag.
         */
        inline bool getGroupInd(EIdType index, u64 groupId) const;

        /**
         * Check all Component bits of given Entity, if
         * any of them are true, return false.
         * @return Returns true, if given Entity does
         *   not have any Components.
         * @remarks Slow operation!
         */
        inline bool compNoneInd(EIdType index) const;

        /**
         * Set Component presence for given Entity to
         * provided value.
         * @param index Index of the Entity.
         * @param compId Component ID.
         * @param value Presence information.
         */
        inline void setCompInd(EIdType index, CIdType compId, bool value);

        /**
         * Get Component flag for given Entity.
         * @param index Index of the Entity.
         * @param compId Component ID.
         * @return Returns the flag.
         */
        inline bool getCompInd(EIdType index, CIdType compId) const;

        /**
         * Check if given Entity ID is valid.
         * @param id ID of the Entity.
         * @return Returns true, if the ID is valid.
         */
        inline bool validImpl(EntityId id) const;

        /**
         * Check validity of given Entity index.
         * Entity needs to be created in order to be valid.
         * @param index Index of the Entity.
         * @return Returns true, if the index is valid.
         */
        inline bool validInd(EIdType index) const;

        /**
         * Check if given generation number
         * corresponds to the current generation
         * number of the Entity with given index.
         * @param index Index of the Entity.
         * @param gen Generation of the Entity.
         * @return Returns true, if the generation is valid.
         */
        inline bool validGen(EIdType index, EIdType gen) const;

        /**
         * Reset given Entity to default state.
         * @param index Index of the Entity.
         */
        inline void resetEntity(EIdType index);

        /// Container for all the different types of metadata.
        struct MetadataContainer
        {
            MetadataGroup components;
            MetadataGroup groups;
            MetadataGroup flags;
            ent::List<EIdType> generations;
        }; // struct MetadataContainer

        /// Misc flag types.
        enum Flags
        {
            ACTIVITY = 0u,
            CREATED = 1u,

            NUM_FLAGS
        }; // enum Flags

        /// Current Entity capacity.
        u64 mEntityCapacity;
        /// Last pushed Entity.
        EIdType mEntityLast;
        /// Contains the currently used metadata.
        MetadataContainer mMetadata;
        /// Indexes of free Entity IDs.
        ent::List<EIdType> mFreeIndexes;
        /// Free EntityGroup indexes.
        ent::SortedList<u64, std::greater<u64>> mFreeGroupIds;
        /// How many new groups will be created on refresh.
        u64 mNewGroupRequests;
    protected:
    }; // class EntityMetadata
} // namespace ent

#include "EntityMetadata.inl"

#endif //ECS_FIT_ENTITYMETADATA_H
