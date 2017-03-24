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
    struct EntityRecord
    {
        /// Is the Entity active?
        bool active() const
        { return groups.test(0u); }

        union {
            /// Present Components bitset.
            ComponentBitset components;
            /// Next ID in the chain of free IDs.
            EIdType nextFree;
        };

        /// Presence in EntityGroups, lowest significance bit represents information about activity of Entity.
        GroupBitset groups;

        /// Current generation number.
        EIdType generation;
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
        inline ActiveEntityIterator(const EntityRecord *ptr, u64 size);

        /**
         * Get ID for the current Entity.
         * @return ID of the current Entity.
         */
        EntityId id() const
        { return EntityId(mIndex, mPtr->generation); }

        /**
         * Get Groups bitset for the current Entity.
         * @return Groups bitset of the current Entity.
         */
        const GroupBitset &groups() const
        { return mPtr->groups; }

        /**
         * Get Components bitset for the current Entity.
         * @return Components bitset of the current Entity.
         */
        const ComponentBitset &components() const
        { return mPtr->components; }

        /**
         * Move the Iterator to the next valid Entity.
         * @return
         */
        void operator++()
        { increment(); }

        /**
         * Is this iterator on a valid EntityRecord?
         * @return Returns true, if this iterator is
         *   safe to use.
         */
        bool isValid() const
        { return mIndex < mSize; }
    private:
        /// Increment this iterator.
        inline void increment();

        /// Index counter.
        u64 mIndex;
        /// Size of the EntityRecord list.
        const u64 mSize;
        /// Internal pointer.
        const EntityRecord *mPtr;
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
        inline ActiveEntityIterator activeEntities() const;
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
        bool activeImpl(EIdType index) const
        { return mRecords[index].active(); }

        /**
         * Check for validity of given index and generation number.
         * @param index Index of the Entity.
         * @param gen Generation of the Entity.
         * @return Returns true, if the Entity is valid.
         */
        bool validImpl(EIdType index, EIdType gen) const
        { return indexValid(index) && genValid(index, gen); }

        /**
         * Check if given index is valid.
         * @param index Index of the Entity.
         * @return Returns true, if the index is valid.
         */
        bool indexValid(EIdType index) const
        { return index && index < mRecords.size(); }

        /**
         * Check if given generation corresponds to the index.
         * !! Does NOT check index bounds !!
         * @param index Index of the Entity.
         * @param gen Generation of the Entity.
         * @return Returns true, if the generation number is current.
         */
        bool genValid(EIdType index, EIdType gen) const
        { return mRecords[index].generation == gen; }

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
     * EntityManager base class containing code which does not need to be templated.
     */
    class EntityManagerBase : NonCopyable
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
    private:
    protected:
        /// Container for the Entities.
        EntityHolder mEntities;
    }; // EntityManagerBase

    /**
     * EntityManager is a part of Entropy ECS Universe.
     * Contains lists of Entities, their status and
     * currently used generation for given index.
     * Contains methods for adding/removing entities and
     * their refresh.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class EntityManager final : public EntityManagerBase
    {
    public:
        /**
         * Default EntityManger constructor.
         */
        EntityManager();

        /**
         * Destruct the Entity manager and the Entities.
         */
        ~EntityManager();

        /**
         * TODO - EntityManager refresh.
         */
        void refresh();
    private:
    protected:
    };// EntityManager
} // namespace ent

#include "EntityManager.inl"

#endif //ECS_FIT_ENTITYMANAGER_H
