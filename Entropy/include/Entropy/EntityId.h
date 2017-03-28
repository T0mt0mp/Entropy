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
        static ENT_CONSTEXPR_EXPR EIdType START_GEN{0};

        /**
         * Create EntityID with given index and generation numbers.
         * @param index Index part of this EID.
         * @param generation Generation part of this EID.
         */
        inline ENT_CONSTEXPR_FUN EntityId(EIdType index = 0, EIdType generation = START_GEN);

        /**
         * Get the packed ID of this EID.
         * @return Packed ID of this EID.
         */
        inline ENT_CONSTEXPR_FUN EIdType id() const;

        /**
         * Get the index part of this EID
         * @return Index starting at the LSb.
         */
        inline ENT_CONSTEXPR_FUN EIdType index() const;

        /**
         * Get the generation part of this EID.
         * @return Generation starting at the LSb.
         */
        inline ENT_CONSTEXPR_FUN EIdType generation() const;

        /// Equal operator.
        ENT_CONSTEXPR_FUN bool operator==(const EntityId &rhs) const
        { return index() == rhs.index(); }

        /// Compare operator.
        ENT_CONSTEXPR_FUN bool operator<(const EntityId &rhs) const
        { return index() < rhs.index(); }
        ENT_CONSTEXPR_FUN bool operator<=(const EntityId &rhs) const
        { return index() <= rhs.index(); }
        ENT_CONSTEXPR_FUN bool operator>(const EntityId &rhs) const
        { return index() > rhs.index(); }
        ENT_CONSTEXPR_FUN bool operator>=(const EntityId &rhs) const
        { return index() >= rhs.index(); }

        /// Print operator.
        friend std::ostream &operator<<(std::ostream &out, const EntityId &id);
    private:
        /**
         * Take generation number aligned to the right (least significant bits) and
         * transform is to generation number aligned to the left (most significant bits).
         * @param rGen Generation aligned to the right.
         * @return Generation aligned to the left.
         */
        inline static ENT_CONSTEXPR_FUN EIdType rGenToLGen(EIdType rGen);

        /**
         * Take generation number aligned to the left (most significant bits) and
         * transform is to generation number aligned to the right (least significant bits).
         * @param lGen Generation aligned to the left.
         * @return Generation aligned to the right.
         */
        inline static ENT_CONSTEXPR_FUN EIdType lGenToRGen(EIdType lGen);

        /**
         * Take packed ID and extract the index part of it.
         * @param packedId Packed entity identifier containing generation and index.
         * @return Index part of the identifier aligned to the right.
         */
        inline static ENT_CONSTEXPR_FUN EIdType indexPart(EIdType packedId);

        /**
         * Take packed ID and extract the generation part of it
         * @param packedId Packed entity identifier containing generation and index.
         * @return Generation part of the identifier alighed to the right.
         */
        inline static ENT_CONSTEXPR_FUN EIdType genPart(EIdType packedId);

        /**
         * Take generation aligned to the left and index aligned to the right and
         * combine them into a packed entity identifier.
         * @param lGen Generation number aligned to the left.
         * @param index Index number aligned to the right
         * @return Packed identifier combining the 2 parts.
         */
        inline static ENT_CONSTEXPR_FUN EIdType combineGenIndex(EIdType lGen, EIdType index);

        /**
         * Packed Entity identifier:
         *  Index bits on the right.
         *  Generation bits on the left.
         */
        EIdType mId;

        /// Zero packed identifier.
        static ENT_CONSTEXPR_EXPR EIdType ZERO{0};

        /// Mask for getting generation bits from the packed identifier.
        static ENT_CONSTEXPR_EXPR EIdType GEN_MASK{(~ZERO) << EID_INDEX_BITS};
        /// Mask for getting index bits from the packed identifier.
        static ENT_CONSTEXPR_EXPR EIdType INDEX_MASK{~GEN_MASK};
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
} // namespace ent

#include "EntityId.inl"

#endif //ECS_FIT_ENTITY_ID_H
