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
     * Wrapper around Entity ID.
     * Contains 2 parts :
     *  Entity index        (on the lower significance bits)
     *  Entity generation   (on the higher significance bits)
     */
    class EntityId
    {
    public:
        /// Starting with generation number 0
        static constexpr EIdType START_GEN{0};

        /**
         * Create EntityID with given index and generation numbers.
         * @param index Index part of this EID.
         * @param generation Generation part of this EID.
         */
        constexpr EntityId(EIdType index = 0, EIdType generation = START_GEN);

        /**
         * Get the packed ID of this EID.
         * @return Packed ID of this EID.
         */
        constexpr EIdType id() const;

        /**
         * Get the index part of this EID
         * @return Index starting at the LSb.
         */
        constexpr EIdType index() const;

        /**
         * Get the generation part of this EID.
         * @return Generation starting at the LSb.
         */
        constexpr EIdType generation() const;

        /// Equal operator
        constexpr bool operator==(const EntityId &rhs) const
        { return mId == rhs.mId; }
    private:
        /**
         * Take generation number aligned to the right (least significant bits) and
         * transform is to generation number aligned to the left (most significant bits).
         * @param rGen Generation aligned to the right.
         * @return Generation aligned to the left.
         */
        static constexpr EIdType rGenToLGen(EIdType rGen);

        /**
         * Take generation number aligned to the left (most significant bits) and
         * transform is to generation number aligned to the right (least significant bits).
         * @param lGen Generation aligned to the left.
         * @return Generation aligned to the right.
         */
        static constexpr EIdType lGenToRGen(EIdType lGen);

        /**
         * Take packed ID and extract the index part of it.
         * @param packedId Packed entity identifier containing generation and index.
         * @return Index part of the identifier aligned to the right.
         */
        static constexpr EIdType indexPart(EIdType packedId);

        /**
         * Take packed ID and extract the generation part of it
         * @param packedId Packed entity identifier containing generation and index.
         * @return Generation part of the identifier alighed to the right.
         */
        static constexpr EIdType genPart(EIdType packedId);

        /**
         * Take generation aligned to the left and index aligned to the right and
         * combine them into a packed entity identifier.
         * @param lGen Generation number aligned to the left.
         * @param index Index number aligned to the right
         * @return Packed identifier combining the 2 parts.
         */
        static constexpr EIdType combineGenIndex(EIdType lGen, EIdType index);

        /**
         * Packed Entity identifier:
         *  Index bits on the right.
         *  Generation bits on the left.
         */
        EIdType mId;

        /// Zero packed identifier.
        static constexpr EIdType ZERO{0};

        /// Mask for getting generation bits from the packed identifier.
        static constexpr EIdType GEN_MASK{(~ZERO) << EID_INDEX_BITS};
        /// Mask for getting index bits from the packed identifier.
        static constexpr EIdType INDEX_MASK{~GEN_MASK};
    protected:
    public:
        /// Maximum value for generation.
        static constexpr EIdType MAX_GEN{((~ZERO) >> EID_INDEX_BITS)};
        /// Maximum value for index.
        static constexpr EIdType MAX_INDEX{INDEX_MASK};
        /// Maximum number of generations per index.
        static constexpr EIdType MAX_GENS{MAX_GEN + 1};
        /// Maximum number of entity indexes.
        static constexpr EIdType MAX_ENTITIES{INDEX_MASK + 1};
    }; // EntityId

    /**
     * Wrapper around Entity management implementation.
     */
    class EntityHolder : NonCopyable
    {
    public:
        /**
         * Default holder contructor
         */
        EntityHolder();

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
                active{false}, components{0}, generation{0}
            { }
            EntityRecord(bool a, ComponentBitset b, EIdType g) :
                active{a}, components{b}, generation{g}
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
        vector<EntityRecord> mRecords;
        /// Records of Entity pools.
        vector<PoolRecord> mPools;
        /// Freed Entity indices.
        deque<EIdType> mFree;
    protected:
    };

    /**
     * EntityManager is a part of Entropy ECS Universe.
     * Contains lists of Entities, their status and
     * currently used generation for given index.
     * Contains methods for adding/removing entities and
     * their refresh.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class EntityManager : NonCopyable
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

    // EntityId implementation.
    constexpr EntityId::EntityId(EIdType index, EIdType generation) :
        mId{combineGenIndex(rGenToLGen(generation), index)}
    { }

    constexpr EIdType EntityId::id() const
    { return mId; }

    constexpr EIdType EntityId::index() const
    { return indexPart(mId); }

    constexpr EIdType EntityId::generation() const
    { return genPart(mId); }

    constexpr EIdType EntityId::rGenToLGen(EIdType rGen)
    {
        // Check for generation overflow.
        ASSERT_SLOW(rGen <= MAX_GEN);
        return rGen << EID_INDEX_BITS;
    }

    constexpr EIdType EntityId::lGenToRGen(EIdType lGen)
    { return lGen >> EID_INDEX_BITS; }

    constexpr EIdType EntityId::indexPart(EIdType packedId)
    { return packedId & INDEX_MASK; }

    constexpr EIdType EntityId::genPart(EIdType packedId)
    { return lGenToRGen(packedId & GEN_MASK); }

    constexpr EIdType EntityId::combineGenIndex(EIdType lGen, EIdType index)
    {
        // Check for index overflow.
        ASSERT_SLOW(index <= MAX_INDEX);
        return lGen | index;
    }
    // EntityId implementation end.
} // namespace ent

#endif //ECS_FIT_ENTITY_H
