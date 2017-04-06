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
#include <set>

#include "Assert.h"

#if defined(__GNUC__) || defined(__GNUG__)
#   define ENT_GCC
#   define popcount64(var) __builtin_popcountll(var)
#elif defined(__clang__)
#   define ENT_CLANG
#   define popcount64(var) __builtin_popcountll(var)
#elif defined(_MSC_VER)
#   define ENT_MSC
#   include<intrin.h>
#   define popcount64(var) __popcnt64(var)
#endif

#ifdef ENT_MSC
#	define ENT_CONSTEXPR_EXPR const
#	define ENT_CONSTEXPR_FUN
#else
#	define ENT_CONSTEXPR_EXPR constexpr
#	define ENT_CONSTEXPR_FUN constexpr
#endif

#if !defined(NDEBUG) && !defined(ENT_NO_STATS) && !defined(ENT_STATS_ENABLED)
#   define ENT_STATS_ENABLED
#endif

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
    static constexpr u64 EID_INDEX_BITS{24u};

    /// Minimal number of free Entity indices, before new are created.
    static constexpr u64 ENT_MIN_FREE{8u};

    /// Limit for haw many elements will get printed in printStats.
    static constexpr u64 ENT_PRINT_LIMIT{30u};

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
    static constexpr std::size_t ENT_MAX_COMPONENTS{64};
    /// Maximum number of EntityGroups per Universe. +1 will be added for Entity flags.
    static constexpr std::size_t ENT_MAX_GROUPS{63};
    /// How many Entities share a single bitset, changes granularity of inner parallelism.
    static constexpr std::size_t ENT_BITSET_GROUP_SIZE{64u};
} // namespace ent

#endif //ECS_FIT_TYPES_H
