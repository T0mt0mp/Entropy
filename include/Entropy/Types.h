/**
 * @file Entropy/Types.h
 * @author Tomas Polasek
 * @brief Types used in Entropy ECS.
 */

#ifndef ECS_FIT_TYPES_H
#define ECS_FIT_TYPES_H

#include <type_traits>
#include <bitset>
#include <deque>
#include <map>
#include <vector>

#include "Assert.h"
#include "Util.h"

/// Main Entropy namespace
namespace ent
{
    // Specification of basic types.
    using u8 = unsigned char;
    using i8 = signed char;
    using u16 = unsigned short;
    using i16 = signed short;
    using u32 = unsigned int;
    using i32 = signed int;
    using u64 = unsigned long long int;
    using i64 = signed long long int;

    /// Entity ID type
    using EIdType = u32;
    class EntityId;

    /**
     * Number of bits in EID used for entity index.
     * Default value : 24 => ~16M entities
     */
    static constexpr u64 EID_INDEX_BITS{24};

    /// Minimal number of free Entity indices, before new are created.
    static constexpr u64 ENT_MIN_FREE{8};

    /**
     * Number of bits in EID used for entity generation.
     * Default value : 8 => 256 generations
     */
    static constexpr u64 EID_GEN_BITS{sizeof(EIdType) * 8 - EID_INDEX_BITS};

    static_assert((EID_INDEX_BITS + EID_GEN_BITS) == sizeof(EIdType) * 8,
                  "The sum of INDEX and GENERATION bits has to equal the number of bits in EIDType!");
    static_assert(EID_INDEX_BITS != 0 && EID_GEN_BITS != 0,
                  "Number of INDEX and GENERATION bits cannot be 0!");

    /// Maximum number of component types per Universe.
    static constexpr std::size_t MAX_COMPONENTS{64};

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

        /// Equal operator.
        constexpr bool operator==(const EntityId &rhs) const
        { return mId == rhs.mId; }

        /// Compare operator.
        constexpr bool operator<(const EntityId &rhs) const
        { return index() < rhs.index(); }
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
        ENT_ASSERT_SLOW(rGen <= MAX_GEN);
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
        ENT_ASSERT_SLOW(index <= MAX_INDEX);
        return lGen | index;
    }
    // EntityId implementation end.
} // namespace ent

#endif //ECS_FIT_TYPES_H
