/**
 * @file Entropy/Group.h
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#ifndef ECS_FIT_GROUP_H
#define ECS_FIT_GROUP_H

#include "Types.h"
#include "Util.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * GroupManager base class containing code which does not need to be templated.
     */
    class GroupManagerBase : NonCopyable
    {
    public:
    private:
    protected:
    }; // GroupManagerBase

    /**
     * GroupManager is a part of Entropy ECS Universe.
     * GroupManager keeps a catalogue of EntityGroups for
     * Systems to use.
     * It has method which take care about creating/removing
     * EntityGroups, and their refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class GroupManager final : public GroupManagerBase
    {
    public:
    private:
    protected:
    }; // GroupManager
} // namespace ent

#endif //ECS_FIT_GROUP_H
