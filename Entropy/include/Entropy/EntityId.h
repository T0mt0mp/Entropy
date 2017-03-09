/**
 * @file Entropy/EntityId.h
 * @author Tomas Polasek
 * @brief Entity ID class.
 */

#ifndef ECS_FIT_ENTITY_ID_H
#define ECS_FIT_ENTITY_ID_H

#include <iostream>

#include "Types.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Wrapper around Entity ID.
     * Contains 2 parts :
     *  Entity index        (on the lower significance bits)
     *  Entity generation   (on the higher significance bits)
     */
    class EntityId final
    {
    public:
        /// Starting with generation number 0
        static ENT_CONSTEXPR EIdType START_GEN{0};

        /**
         * Create EntityID with given index and generation numbers.
         * @param index Index part of this EID.
         * @param generation Generation part of this EID.
         */
        EntityId(EIdType index = 0, EIdType generation = START_GEN);

        /**
         * Get the packed ID of this EID.
         * @return Packed ID of this EID.
         */
        ENT_CONSTEXPR EIdType id() const;

        /**
         * Get the index part of this EID
         * @return Index starting at the LSb.
         */
        ENT_CONSTEXPR EIdType index() const;

        /**
         * Get the generation part of this EID.
         * @return Generation starting at the LSb.
         */
        ENT_CONSTEXPR EIdType generation() const;

        /// Equal operator.
        ENT_CONSTEXPR bool operator==(const EntityId &rhs) const
        { return mId == rhs.mId; }

        /// Compare operator.
        ENT_CONSTEXPR bool operator<(const EntityId &rhs) const
        { return index() < rhs.index(); }

        /// Print operator.
        friend std::ostream &operator<<(std::ostream &out, const EntityId &id);
    private:
        /**
         * Take generation number aligned to the right (least significant bits) and
         * transform is to generation number aligned to the left (most significant bits).
         * @param rGen Generation aligned to the right.
         * @return Generation aligned to the left.
         */
        static ENT_CONSTEXPR EIdType rGenToLGen(EIdType rGen);

        /**
         * Take generation number aligned to the left (most significant bits) and
         * transform is to generation number aligned to the right (least significant bits).
         * @param lGen Generation aligned to the left.
         * @return Generation aligned to the right.
         */
        static ENT_CONSTEXPR EIdType lGenToRGen(EIdType lGen);

        /**
         * Take packed ID and extract the index part of it.
         * @param packedId Packed entity identifier containing generation and index.
         * @return Index part of the identifier aligned to the right.
         */
        static ENT_CONSTEXPR EIdType indexPart(EIdType packedId);

        /**
         * Take packed ID and extract the generation part of it
         * @param packedId Packed entity identifier containing generation and index.
         * @return Generation part of the identifier alighed to the right.
         */
        static ENT_CONSTEXPR EIdType genPart(EIdType packedId);

        /**
         * Take generation aligned to the left and index aligned to the right and
         * combine them into a packed entity identifier.
         * @param lGen Generation number aligned to the left.
         * @param index Index number aligned to the right
         * @return Packed identifier combining the 2 parts.
         */
        static ENT_CONSTEXPR EIdType combineGenIndex(EIdType lGen, EIdType index);

        /**
         * Packed Entity identifier:
         *  Index bits on the right.
         *  Generation bits on the left.
         */
        EIdType mId;

        /// Zero packed identifier.
        static ENT_CONSTEXPR EIdType ZERO{0};

        /// Mask for getting generation bits from the packed identifier.
        static ENT_CONSTEXPR EIdType GEN_MASK{(~ZERO) << EID_INDEX_BITS};
        /// Mask for getting index bits from the packed identifier.
        static ENT_CONSTEXPR EIdType INDEX_MASK{~GEN_MASK};
    protected:
    public:
        /// Maximum value for generation.
        static ENT_CONSTEXPR EIdType MAX_GEN{((~ZERO) >> EID_INDEX_BITS)};
        /// Maximum value for index.
        static ENT_CONSTEXPR EIdType MAX_INDEX{INDEX_MASK};
        /// Maximum number of generations per index.
        static ENT_CONSTEXPR EIdType MAX_GENS{MAX_GEN + 1};
        /// Maximum number of entity indexes.
        static ENT_CONSTEXPR EIdType MAX_ENTITIES{INDEX_MASK + 1};
    }; // EntityId

    // EntityId implementation.
    EntityId::EntityId(EIdType index, EIdType generation) :
        mId{combineGenIndex(rGenToLGen(generation), index)}
    { }

    ENT_CONSTEXPR EIdType EntityId::id() const
    { return mId; }

    ENT_CONSTEXPR EIdType EntityId::index() const
    { return indexPart(mId); }

    ENT_CONSTEXPR EIdType EntityId::generation() const
    { return genPart(mId); }

    ENT_CONSTEXPR EIdType EntityId::rGenToLGen(EIdType rGen)
    { return rGen << EID_INDEX_BITS; }

    ENT_CONSTEXPR EIdType EntityId::lGenToRGen(EIdType lGen)
    { return lGen >> EID_INDEX_BITS; }

    ENT_CONSTEXPR EIdType EntityId::indexPart(EIdType packedId)
    { return packedId & INDEX_MASK; }

    ENT_CONSTEXPR EIdType EntityId::genPart(EIdType packedId)
    { return lGenToRGen(packedId & GEN_MASK); }

    ENT_CONSTEXPR EIdType EntityId::combineGenIndex(EIdType lGen, EIdType index)
    { return lGen | index; }
    // EntityId implementation end.
} // namespace ent

#endif //ECS_FIT_ENTITY_ID_H
