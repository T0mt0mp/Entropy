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
#include "EntityId.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Temporary Entity which will be returned by parallel createEntity.
     * @tparam UniverseT Type of the Universe.
     */
    class TemporaryEntity
    {
    public:
    private:
    protected:
    };

    struct EntityChange
    {
        EntityId id;
        ComponentBitset removedComponents;
    };

    /**
     * Holds information about newly crated Component.
     * @tparam ComponentT Type of the Component
     */
    template <typename ComponentT>
    struct ComponentChange
    {
        ComponentT comp;
        EntityId id;
    };

    /**
     * Container for the changes.
     * @tparam UniverseT Type of the Universe.
     */
    class ActionContainer
    {
    public:
        template <typename ComponentT>
        using ComponentSet = ent::List<ComponentChange<ComponentT>>;
        using EntitySet = ent::List<TemporaryEntity>;
    private:
        template <typename ComponentT>
        ComponentSet<ComponentT> &components(u64 componentId)
        {
            void *result{nullptr};

            if (mComponentSets.size() <= componentId)
            { // Resize the list of ComponentSets.
                mComponentSets.resize(componentId + 1u, nullptr);
            }

            result = mComponentSets[componentId];
            if (result == nullptr)
            { // ComponentSet has not been created yet.
                result = new ComponentSet<ComponentT>;
                mComponentSets[componentId] = result;
            }

            return *static_cast<ComponentSet<ComponentT>*>(result);
        }

        ent::List<void *> mComponentSets;
        EntitySet mEntities;
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
