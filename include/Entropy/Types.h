/**
 * @file Entropy/Types.h
 * @author Tomas Polasek
 * @brief Types used in Entropy ECS.
 */

#ifndef ECS_FIT_TYPES_H
#define ECS_FIT_TYPES_H

#include <type_traits>
#include <bitset>

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
    /**
     * Number of bits in EID used for entity index.
     * Default value : 24 => ~16M entities
     */
    static constexpr u64 EID_INDEX_BITS{24};
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

} // namespace ent

#endif //ECS_FIT_TYPES_H
