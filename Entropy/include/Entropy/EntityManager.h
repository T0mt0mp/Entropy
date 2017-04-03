/**
 * @file Entropy/EntityManager.h
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#ifndef ECS_FIT_ENTITYMANAGER_H
#define ECS_FIT_ENTITYMANAGER_H

#include "Util.h"
#include "Types.h"
#include "EntityId.h"
#include "List.h"

/// Main Entropy namespace
namespace ent
{
    /// Record about state of a single Entity
    class EntityRecord
    {
    public:
        /// Reset all the metadata to 0.
        inline void reset();

        /// Is the Entity active?
        inline bool active() const;
        /// Activate this Entity.
        inline void activate();
        /// Deactivate this Entity.
        inline void deactivate();

        /// Set metadata for given Component.
        inline void setComp(u64 index);
        /// Reset metadata for given Component.
        inline void resetComp(u64 index);
        /// Test for given Component.
        inline bool testComp(u64 index) const;
        /// Components getter.
        inline const ComponentBitset &comp() const;
        inline ComponentBitset &comp();

        /// Set metadata for given Group.
        inline void setGrp(u64 index);
        /// Reset metadata for given Group.
        inline void resetGrp(u64 index);
        /// Test for given Group.
        inline bool testGrp(u64 index) const;
        /// Group getter.
        inline const GroupBitset &grp() const;
        inline GroupBitset &grp();

        /// Generation getter.
        inline EIdType &gen();
        inline const EIdType &gen() const;

        /// Next free getter.
        inline EIdType &next();
        inline const EIdType &next() const;
    private:
        union {
            /// Present Components bitset.
            ComponentBitset components;
            /// Next ID in the chain of free IDs.
            EIdType nextFree;
        };

        /**
         * Presence in EntityGroups.
         * The most significant bit is '1' if the Entity is active, else it is '0'.
         */
        GroupBitset groups;

        /// Current generation number.
        EIdType generation;
    protected:
    };

    /**
     * Iterator for iterating over all active Entities
     * within EntityHolder.
     */
    class ActiveEntityIterator
    {
    public:
        /**
         * Create iterator with given pointer.
         * @param ptr Pointer to the EntityRecord list.
         */
        inline ActiveEntityIterator(EntityRecord *ptr, EIdType size);

        /**
         * Get ID for the current Entity.
         * @return ID of the current Entity.
         */
        inline EntityId id() const;

        /// Get the current record.
        inline EntityRecord &record();
        inline const EntityRecord &record() const;

        /**
         * Move the Iterator to the next valid Entity.
         * @return
         */
        inline void operator++();

        /**
         * Is this iterator on a valid EntityRecord?
         * @return Returns true, if this iterator is
         *   safe to use.
         */
        inline bool valid() const;

        /**
         * Is the current Entity record acive?
         * @return Returns true, if the current Entity is active.
         */
        inline bool active() const;
    private:
        /// Increment this iterator.
        inline void increment();

        /// Move to the next Entity record.
        inline void moveNext();

        /// Index counter.
        EIdType mIndex;
        /// Size of the EntityRecord list.
        const EIdType mSize;
        /// Internal pointer.
        EntityRecord *mPtr;
    protected:
    };

    /**
     * Wrapper around Entity management implementation.
     */
    class EntityHolder final : NonCopyable
    {
    public:
        /// Information about Entity sequence.
        struct SequenceRecord
        {
            // TODO - change into coherent object, something like Entity class.
            /// First Entity ID in the sequence.
            EntityId firstEntity;
            /// Last Entity ID in the sequence.
            EntityId lastEntity;
            /// Number of Entities in the sequence.
            u64 size;
        };

        /**
         * Default holder constructor.
         */
        EntityHolder();

        /**
         * Reset the Entities.
         */
        void reset();

        /**
         * Create a new Entity and return its ID.
         * @return Returns ID of the new Entity.
         */
        EntityId create();

        /**
         * TODO - implement.
         * Try to create Entity with given ID (without generation), if
         * the ID is free, the Entity will be created and the same Entity ID
         * will be returned. If the requested ID is invalid or unavailable, invalid
         * Entity ID will be returned instead.
         * @param id Requested ID.
         * @return Returns Entity with the same ID, or if the operation fails, returns
         *   invalid ID instead.
         */
        EntityId create(EIdType id);

        /**
         * TODO - implement.
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * If the operation fails the returned sequence record will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        SequenceRecord createSequential(u64 size);

        /**
         * TODO - implement.
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * Attempts to create sequence starting with startId, if that fails, returned sequence record
         * will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        SequenceRecord createSequential(EIdType startId, u64 size);

        /**
         * Mark component as present for given Entity.
         * @param id ID of the Entity.
         * @param index Index of the Component.
         */
        inline void addComponent(EntityId id, u64 index);

        /**
         * Mark component as not present for given Entity.
         * @param id ID of the Entity.
         * @param index Index of the Component.
         */
        inline void removeComponent(EntityId id, u64 index);

        /**
         * Does given Entity have the Component?
         * @param id ID of the Entity.
         * @param index Index of the Component.
         * @return Returns true, if the Component is present.
         */
        inline bool hasComponent(EntityId id, u64 index) const;

        /**
         * Activate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        inline void activate(EntityId id);

        /**
         * Deactivate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        inline void deactivate(EntityId id);

        /**
         * Destroy given Entity.
         * TODO - Should we actually check validity of Entity?
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         */
        bool destroy(EntityId id);

        /**
         * Checks validity of given Entity.
         * Does not check for activity, or if the Entity
         * is created.
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
         * Get Component bitset for given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         * @return Returns reference to the bitset.
         */
        inline const ComponentBitset &components(EntityId id) const;

        /**
         * Get Group bitset for given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         * @return Returns reference to the bitset.
         */
        inline const GroupBitset &groups(EntityId id);

        /**
         * Set Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the group. Starting at 0 - up to MAX_GROUPS.
         */
        inline void setGroup(EntityId id, u64 groupId);

        /**
         * reset Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the group. Starting at 0 - up to MAX_GROUPS.
         */
        inline void resetGroup(EntityId id, u64 groupId);

        /**
         * Get iterator for all active Entities.
         * @return Iterator for all active Entities.
         */
        inline ActiveEntityIterator activeEntities();

        /**
         * Apply andMask to the Group metadata.
         * @param andMask AND mask applied to the Group metadata.
         */
        inline void resetGroups(const GroupBitset &andMask);
    private:
        /**
         * Initialize Entity on given index to 0 values.
         * @param index Index of the Entity.
         */
        inline void initEntity(EIdType index);

        /**
         * Check if Entity on given index is active.
         * Does NOT check index bounds by itself.
         * @param index
         */
        inline bool activeImpl(EIdType index) const;

        /**
         * Check for validity of given index and generation number.
         * @param index Index of the Entity.
         * @param gen Generation of the Entity.
         * @return Returns true, if the Entity is valid.
         */
        inline bool validImpl(EIdType index, EIdType gen) const;

        /**
         * Check if given index is valid.
         * @param index Index of the Entity.
         * @return Returns true, if the index is valid.
         */
        inline bool indexValid(EIdType index) const;

        /**
         * Check if given generation corresponds to the index.
         * !! Does NOT check index bounds !!
         * @param index Index of the Entity.
         * @param gen Generation of the Entity.
         * @return Returns true, if the generation number is current.
         */
        inline bool genValid(EIdType index, EIdType gen) const;

        /**
         * Free Entity on given ID, no checks are performed!
         * @param index Index of the Entity.
         */
        inline void pushFreeId(EIdType index);

        /**
         * Pop ID from the free list. No checks are performed!
         * @return Returns free Entity ID.
         */
        inline EIdType popFreeId();

        /// Records of Entities.
        List<EntityRecord> mRecords;
        /// Start of the chain of free Entity IDs.
        EIdType mFirstFree;
        /// Last free Entity ID.
        EIdType mLastFree;
        /// Length of the free chain.
        u64 mNumFree;
    protected:
    };

    /**
     * EntityManager is a part of Entropy ECS Universe.
     * Contains lists of Entities, their status and
     * currently used generation for given index.
     * Contains methods for adding/removing entities and
     * their refresh.
     */
    class EntityManager final : NonCopyable
    {
    public:
        /**
         * Create a new Entity and return its ID.
         * @return Returns ID of the new Entity.
         */
        EntityId create()
        { return mEntities.create(); }

        /**
         * Reset the Entities.
         */
        void reset()
        { mEntities.reset(); }

        /**
         * TODO - implement.
         * Try to create Entity with given ID (without generation), if
         * the ID is free, the Entity will be created and the same Entity ID
         * will be returned. If the requested ID is invalid or unavailable, invalid
         * Entity ID will be returned instead.
         * @param id Requested ID.
         * @return Returns Entity with the same ID, or if the operation fails, returns
         *   invalid ID instead.
         */
        EntityId create(EIdType id)
        { return mEntities.create(id); }

        /**
         * TODO - implement.
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * If the operation fails the returned sequence record will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        EntityHolder::SequenceRecord createSequential(u64 size)
        { return mEntities.createSequential(size); }

        /**
         * TODO - implement.
         * Create Entity sequence of given size. Sequence is always a contiguous block of Entities.
         * Minimal sequence size is 1.
         * Attempts to create sequence starting with startId, if that fails, returned sequence record
         * will contain size equal to 0.
         * @param size Number of Entities in the sequence.
         * @return Information about created sequence.
         */
        EntityHolder::SequenceRecord createSequential(EIdType startId, u64 size)
        { return mEntities.createSequential(startId, size); }

        /**
         * Mark component as present for given Entity.
         * @param id ID of the Entity.
         * @param index Index of the Component.
         */
        void addComponent(EntityId id, u64 index)
        { mEntities.addComponent(id, index); }

        /**
         * Mark component as not present for given Entity.
         * @param id ID of the Entity.
         * @param index Index of the Component.
         */
        void removeComponent(EntityId id, u64 index)
        { mEntities.removeComponent(id, index); }

        /**
         * Does given Entity have the Component?
         * @param id ID of the Entity.
         * @param index Index of the Component.
         * @return Returns true, if the Component is present.
         */
        bool hasComponent(EntityId id, u64 index) const
        { return mEntities.hasComponent(id, index); }

        /**
         * Activate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        void activate(EntityId id)
        { mEntities.activate(id); }

        /**
         * Deactivate given Entity.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         */
        void deactivate(EntityId id)
        { mEntities.deactivate(id); }

        /**
         * Destroy given Entity.
         * TODO - Should we actually check validity of Entity?
         * @param id ID of the Entity.
         * @return Returns false, if the Entity does not exist.
         */
        bool destroy(EntityId id)
        { return mEntities.destroy(id); }

        /**
         * Checks validity of given Entity.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity exists.
         */
        bool valid(EntityId id) const
        { return mEntities.valid(id); }

        /**
         * Checks if the given Entity is active.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         * @return Returns true, if the Entity is active.
         */
        bool active(EntityId id) const
        { return mEntities.active(id); }

        /**
         * Get Component bitset for given Entity.
         * @param id ID of the Entity.
         * @return Returns reference to the bitset.
         */
        const ComponentBitset &components(EntityId id) const
        { return mEntities.components(id); }

        /**
         * Get Groups bitset for given Entity.
         * @param id ID of the Entity.
         * @return Returns reference to the bitset.
         */
        const GroupBitset &groups(EntityId id)
        { return mEntities.groups(id); }

        /**
         * Set Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the Group - starts at 1, up to MAX_GROUPS.
         */
        void setGroup(EntityId id, u64 groupId)
        { mEntities.setGroup(id, groupId); }

        /**
         * Reset Entity group flag.
         * @param id ID of the Entity.
         * @param groupId ID of the Group - starts at 1, up to MAX_GROUPS.
         */
        void resetGroup(EntityId id, u64 groupId)
        { mEntities.resetGroup(id, groupId); }

        /**
         * Get iterator for all active Entities.
         * @return Iterator for all active Entities.
         */
        ActiveEntityIterator activeEntities()
        { return mEntities.activeEntities(); }

        /**
         * Apply andMask to the Group metadata.
         * @param andMask AND mask applied to the Group metadata.
         */
        void resetGroups(const GroupBitset &andMask)
        { mEntities.resetGroups(andMask); }
    private:
    protected:
        /// Container for the Entities.
        EntityHolder mEntities;
    };// EntityManager
} // namespace ent

#include "EntityManager.inl"

#endif //ECS_FIT_ENTITYMANAGER_H
