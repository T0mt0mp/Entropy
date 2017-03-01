/**
 * @file Entropy/Group.cpp
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#include "Entropy/Group.h"

/// Main Entropy namespace
namespace ent
{
    // EntityGroup implementation.
    EntityGroup::EntityGroup(const ComponentFilter &filter, u64 groupId) :
        mFilter{filter},
        mId{groupId}
    {

    }
    // EntityGroup implementation end.
} // namespace ent
