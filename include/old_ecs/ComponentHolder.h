/**
 * @file ecs/ComponentHolder.h
 * @author Tomas Polasek
 * @brief Holder for component data.
 */

#ifndef ECS_COMPONENTHOLDER_H
#define ECS_COMPONENTHOLDER_H

// TODO - Implement memory management
#include <vector>
#include <unordered_map>

#include "ecs/Types.h"

namespace ecs
{
    /**
     * Holder for the component data.
     * Component data is held in a contiguous memory block (array).
     * Components are sorted in order of entity ID (most of the time).
     * Index 0 is unused and invalid (like a nullptr).
     */
    template <typename CompT>
    class ComponentHolder
    {
    private:
        template <typename Config>
        friend class EntityManager;
        friend class ComponentIterator;
    public:
        /**
         * Used for iterating through all components of this
         * type for a single entity.
         */
        class EntityComponentIterator
        {
        public:
        private:
        protected:
        };

        /**
         * Used for iterating through all components of this
         * type.
         */
        class ComponentIterator
        {
        public:
        private:
        protected:
        };

        ComponentHolder();

        /**
         * Get component that belongs to given entity.
         * @param id Identifier of the entity.
         * @return Returns iterator for accessing all of the components.
         */
        inline ComponentIterator &get(ecs::IdType id);

        /**
         * Get component that belongs to given entity.
         * @param id Identifier of the entity.
         * @return Returns iterator for accessing all of the components.
         */
        inline ConstComponentIterator &get(ecs::IdType id) const;

        /**
         * Check if given entity has a component in this holder.
         * @param id Identifier of the entity.
         * @return Returns true if the entity has at least one component.
         */
        inline bool hasComponent(ecs::IdType id) const;

        /**
         * Add component for given entity.
         * @param id Identifier of the entity.
         * @return Returns reference to the component.
         */
        inline CompT &add(ecs::IdType id);

        /**
         * Remove components from given component iterator.
         * Beginning iterator will be on the next (non-deleted) component,
         * if the method returns true, else the iterator is invalid.
         * @param beg Beginning iterator.
         * @param num Number of components to delete, maximum deleted is
         *  the number of existing components.
         * @return Returns true, if there are more components.
         */
        inline bool remove(ComponentIterator &beg, u64 num = 1);

    private:
        /**
         * Refresh the memory structure of this holder.
         * Completes the removing/adding of components.
         * Possible actions :
         *  Slight reordering,
         *  Full sort,
         *  Full sort into a bigger buffer.
         * @param forceFull Forces the full sorting.
         */
        inline void refresh(bool forceFull = false);

        /**
         * Memory structure for holding required data.
         * Also contains information required for mapping
         * of Entity IDs to components and components to
         * Entity IDs.
         */
        struct ComponentMemory
        {
            // TODO - Create my own map implementation.
            using MapType = std::unordered_map<ecs::IdType, u64>;

            /// Current size in components.
            u64 size{0};
            /// Index of the next free position, on the end.
            u64 nextFree{0};
            /// Number of components added/moved since the last sort.
            u64 outOfPlace{0};

            /**
             * Holds the component data.
             * Always contiguous, up to the first empty (nextFree).
             */
            CompT *data{nullptr};
            /// Holds the information about the next component of the entity.
            u64 *next{nullptr};
            /// Holds the information about which entity owns a given component.
            ecs::IdType *owner{nullptr};

            /// Translation from entity ID to index.
            MapType indexTable;
        };

        /**
         * Structure containg data about newly added/removed components.
         * This structure is used to keep the information, until a refresh is
         * called and all of the operations are executed.
         */
        struct ComponentOperations
        {
            /// Number of components "added" since the last refresh.
            u64 added{0};
            /// Data for the added components.
            CompT *addedData{nullptr};
            /// Entity IDs for added components.
            ecs::IdType *owner{nullptr};

            /// Number of components "removed" since the last refresh.
            u64 removed{0};
            /// List of indexes of the removed components.
            u64 *removedIndex{nullptr};
        };

        /// Starting size of the internal structures.
        static constexpr u64 SIZE_START{8};
        /// How many times is the new size larger than the previous one.
        static constexpr u64 SIZE_MULTIPLIER{2};
    protected:
    };

    template <typename CompT>
    ComponentHolder<CompT>::ComponentHolder()
    {

    }
}

#endif //ECS_COMPONENTHOLDER_H
