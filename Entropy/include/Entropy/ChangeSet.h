/**
 * @file Entropy/ChangeSet.h
 * @author Tomas Polasek
 * @brief Action cache used for aggregating actions from parallel threads.
 */

#ifndef ECS_FIT_CHANGESET_H
#define ECS_FIT_CHANGESET_H

#include "Types.h"
#include "Util.h"
#include "SortedList.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Temporary Entity which will be returned by parallel createEntity.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class TemporaryEntity
    {
    public:
    private:
    protected:
    };

    /**
     * Container for the changes.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ActionContainer
    {
    public:
        template <typename ComponentT>
        using ComponentSet = ent::List<ComponentT>;

        using EntitySet = ent::List<TemporaryEntity>;
    private:
        template <typename ComponentT>
        ComponentSet<ComponentT> &components()
        {
            static thread_local ComponentSet<ComponentT> compSet;
            return compSet;
        }
    protected:
    }; // class ActionContainer

    /**
     * Used for aggregation of actions from parallel threads.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class ChangeSet
    {
    public:
    private:
    protected:
    }; // class ChangeSet
}

#endif //ECS_FIT_CHANGESET_H
