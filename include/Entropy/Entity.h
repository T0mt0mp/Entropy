/**
 * @file Entropy/Entity.h
 * @author Tomas Polasek
 * @brief Entity is a object within the Entropy ECS Universe.
 */

#ifndef ECS_FIT_ENTITY_H
#define ECS_FIT_ENTITY_H

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
        /**
         * Default holder contructor
         */
        inline EntityHolder();

        /**
         * Create a new Entity and return its ID.
         * @return Returns ID of the new Entity.
         */
        inline EntityId create();

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
        /// Record about an Entity Pool.
        struct PoolRecord
        {
            /// Entity ID of the first Entity from this pool.
            EIdType poolStartId;
            /// Entity ID of the last (inclusive) Entity from this pool.
            EIdType poolEndId;
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
        /// TODO - Records of Entity pools.
        std::vector<PoolRecord> mPools;
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
    private:
    protected:
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

    EntityId EntityHolder::create()
    {
        EIdType index{0};
        EIdType gen{0};

        if (mFree.size() > ENT_MIN_FREE)
        {
            index = mFree.front();
            ENT_ASSERT_SLOW(index < mRecords.size());
            mFree.pop_front();
            // Generation is incremented in Entity destroy.
            gen = mRecords[index].generation;
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

        EntityRecord &rec{mRecords[id.index()]};
        rec.active = false;
        rec.components = 0;
        // Not actually an error...
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

#endif //ECS_FIT_ENTITY_H
