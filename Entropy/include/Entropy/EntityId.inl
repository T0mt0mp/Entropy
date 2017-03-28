/**
 * @file Entropy/EntityId.inl
 * @author Tomas Polasek
 * @brief Entity ID class.
 */

#include "EntityId.h"

/// Main Entropy namespace
namespace ent
{
    // EntityId implementation.
    ENT_CONSTEXPR_FUN EntityId::EntityId(EIdType index, EIdType generation) :
    //mId{combineGenIndex(rGenToLGen(generation), index)}
        mId{generation << EID_INDEX_BITS | index}
    { }

    ENT_CONSTEXPR_FUN EIdType EntityId::id() const
    { return mId; }

    ENT_CONSTEXPR_FUN EIdType EntityId::index() const
    //{ return indexPart(mId); }
    { return mId & INDEX_MASK; }

    ENT_CONSTEXPR_FUN EIdType EntityId::generation() const
    //{ return genPart(mId); }
    { return (mId & GEN_MASK) >> EID_INDEX_BITS; }

    ENT_CONSTEXPR_FUN EIdType EntityId::rGenToLGen(EIdType rGen)
    { return rGen << EID_INDEX_BITS; }

    ENT_CONSTEXPR_FUN EIdType EntityId::lGenToRGen(EIdType lGen)
    { return lGen >> EID_INDEX_BITS; }

    ENT_CONSTEXPR_FUN EIdType EntityId::indexPart(EIdType packedId)
    { return packedId & INDEX_MASK; }

    ENT_CONSTEXPR_FUN EIdType EntityId::genPart(EIdType packedId)
    { return lGenToRGen(packedId & GEN_MASK); }

    ENT_CONSTEXPR_FUN EIdType EntityId::combineGenIndex(EIdType lGen, EIdType index)
    { return lGen | index; }
    // EntityId implementation end.
} // namespace ent
