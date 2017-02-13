/**
 * @file Entropy/EntityManager.h
 * @author Tomas Polasek
 * @brief Entity management system.
 */

#ifndef ECS_FIT_ENTITYMANAGER_H
#define ECS_FIT_ENTITYMANAGER_H

#include "Types.h"
#include "Component.h"

/// Main Entropy namespace
namespace ent
{
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
        inline EntityHolder();

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
        inline bool destroy(EntityId id);

        /**
         * Checks validity of given Entity.
         * @param id ID of the Entity.
         * @return Returns true, if the Entity exists.
         */
        inline bool valid(EntityId id) const;

        /**
         * Checks if the given Entity is active.
         * !! Does NOT check index bounds !!
         * @param id ID of the Entity.
         * @return Returns true, if the Entity is active.
         */
        inline bool active(EntityId id) const;
    private:
        /// Record about state of a single Entity
        struct EntityRecord
        {
            EntityRecord() :
                components{0}, generation{0}, active{false}
            { }
            EntityRecord(bool a, ComponentBitset b, EIdType g) :
                components{b}, generation{g}, active{a}
            { }
            /// Present Components bitset.
            ComponentBitset components;
            /// Current generation number.
            EIdType generation;
            /// Is the Entity active?
            bool active;
        };

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

        /// Records of Entities.
        std::vector<EntityRecord> mRecords;
        /// Freed Entity indices.
        std::deque<EIdType> mFree;
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
    private:
    protected:
    };// EntityManager

    // EntityHolder implementation.
    EntityHolder::EntityHolder()
    {
        // Create the 0th record, valid Entity indexes start at 1!.
        mRecords.emplace_back();
    }

    void EntityHolder::activate(EntityId id)
    {
        mRecords[id.index()].active = true;
    }

    void EntityHolder::deactivate(EntityId id)
    {
        mRecords[id.index()].active = false;
    }

    bool EntityHolder::destroy(EntityId id)
    {
        if (!valid(id))
        { // Entity does not exist!
            return false;
        }

        EntityRecord &rec(mRecords[id.index()]);
        rec.active = false;
        rec.components = 0;
        // TODO - Not actually an error?
        ENT_ASSERT_SLOW(rec.generation < EntityId::MAX_GEN);
        rec.generation++;

        mFree.push_back(id.index());

        return true;
    }

    bool EntityHolder::valid(EntityId id) const
    { return validImpl(id.index(), id.generation()); }

    bool EntityHolder::active(EntityId id) const
    { return mRecords[id.index()].active; }
    // EntityHolder implementation end.

    // EntityManager implementation.
    template <typename U>
    EntityManager<U>::EntityManager()
    {

    }
    // EntityManager implementation end.
} // namespace ent

#endif //ECS_FIT_ENTITYMANAGER_H
