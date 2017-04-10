/**
 * @file Entropy/EntityGroup.h
 * @author Tomas Polasek
 * @brief Group represents a group of Entities which pass the same ComponentFilter.
 */

#ifndef ECS_FIT_ENTITYGROUP_H
#define ECS_FIT_ENTITYGROUP_H

#include "Types.h"
#include "Memory.h"
#include "Util.h"
#include "ComponentManager.h"
#include "Entity.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Helper structure for specifying required Component types
     * for a System.
     * @tparam RequireComponentTs List of required Component types.
     */
    template<typename... RequireComponentTs>
    struct Require {
    };

    /**
     * Helper structure for specifying rejected Component types
     * for a System.
     * @tparam RejectComponentTs List of rejected Component types.
     */
    template<typename... RejectComponentTs>
    struct Reject {
    };

    /**
     * Iterator used for iterating over Entities within Entity group.
     * @tparam UniverseT Type of the Universe.
     * @tparam IterT Type of the Iterator.
     */
    template <typename UniverseT,
              typename IterT>
    class EntityGroupIterator final
    {
    public:
        using ValueT = typename UniverseT::EntityT;
        using RefT = ValueT&;
        using PtrT = ValueT*;

        /**
         * Construct Entity iterator.
         * @param uni Universe, where the Entities exist.
         * @param it Iterator.
         */
        EntityGroupIterator(UniverseT *uni, IterT it) :
            mIterator{it}, mEntity{uni}, mUniverse{uni} { }

        /// Prefix increment.
        EntityGroupIterator &operator++()
        {
            ++mIterator;
            return *this;
        }

        /// Postfix increment.
        EntityGroupIterator operator++(int)
        {
            EntityGroupIterator tmp{*this};
            ++(*this);
            return tmp;
        }

        /// Equality comparison operator.
        bool operator==(const EntityGroupIterator &rhs) const
        { return mIterator == rhs.mIterator; }

        /// Inequality comparison operator.
        bool operator!=(const EntityGroupIterator &rhs) const
        { return !(*this == rhs); }

        /// Access operator.
        RefT operator*()
        { updateEntity(); return mEntity; }

        /// Access operator.
        PtrT operator->()
        { updateEntity(); return &mEntity; }

        EntityGroupIterator operator+(u64 val)
        { return EntityGroupIterator(mUniverse, mIterator + val); }

        /**
         * Subscript operator proxy.
         * decltype(std::declval<IterT>()[std::declval<IndexT>()])
         */
        template <typename IndexT>
        RefT operator[](IndexT idx)
        { updateEntity(mIterator[idx]); return mEntity; }
    private:
        /**
         * Update the value of the innter Entity.
         */
        void updateEntity()
        { mEntity.setId(*mIterator); }

        /**
         * Update Entity to given ID.
         * @param id ID of the Entity.
         */
        void updateEntity(EntityId id)
        { mEntity.setId(id); }

        /// Inner iterator.
        IterT mIterator;
        /// Entity object, which will be returned.
        ValueT mEntity;
        /// Universe pointer;
        UniverseT *mUniverse;
    protected:
    }; // class EntityGroupIterator

    /**
     * Helper object, used in foreach loops.
     * @tparam UniverseT Type of the Universe.
     * @tparam IteratedT Over which type does it iterate?
     */
    template <typename UniverseT,
              typename IteratedT>
    class EntityList
    {
    public:
        using IteratorT = EntityGroupIterator<UniverseT, typename IteratedT::iterator>;
        using ConstIteratorT = EntityGroupIterator<UniverseT, typename IteratedT::const_iterator>;

        /**
         * Create iterable object, iterating over given object.
         * @param it Iterate over this object.
         */
        EntityList(UniverseT *uni, IteratedT &it) :
            mUniverse{uni}, mIt{it}
        { }

        typename std::enable_if<true, decltype(std::declval<IteratedT>().size())>::type
        size() const
        { return mIt.size(); }

        /// Get begin iterator.
        IteratorT begin()
        { return IteratorT(mUniverse, mIt.begin()); }
        /// Get begin iterator.
        ConstIteratorT begin() const
        { return IteratorT(mUniverse, mIt.begin()); }

        /// Get end iterator.
        IteratorT end()
        { return IteratorT(mUniverse, mIt.end()); }
        /// Get end iterator.
        ConstIteratorT end() const
        { return IteratorT(mUniverse, mIt.end()); }

        template <typename UT, typename IT>
        friend IteratorT begin(EntityList<UT, IT> &entityList);
        template <typename UT, typename IT>
        friend IteratorT end(EntityList<UT, IT> &entityList);
    private:
        /// Universe object.
        UniverseT *mUniverse;
        /// Object to iterate over.
        IteratedT &mIt;
    protected:
    };

    /**
     * EntityGroup class contains a list of Entities, which pass
     * a group specific filter. They are used for caching the Entities for
     * use in Systems.
     */
    class EntityGroup final : NonCopyable
    {
    public:
        template <typename UT>
        friend class GroupManager;
        using EntityListT = SortedList<EntityId>;
        using AddedListT = List<EntityId>;
        using RemovedListT = List<EntityId>;

        /**
         * Construct a EntityGroup, specifying which
         * Entities belong into it, using the filter.
         * @param filter Specifies which Entities belong
         *   into this Group.
         * @param groupId ID of the group, starting at 0.
         */
        inline EntityGroup(const EntityFilter &filter, u64 groupId);

        /// Filter getter.
        const EntityFilter &filter() const
        { return mFilter; }

        /// Group ID getter.
        u64 id() const
        { return mId; }

        /// Return the usage counter.
        u64 usage() const
        { return mUsageCounter; }

        /// Is this Group in use?
        bool inUse() const
        { return usage() != 0u; }

        /**
         * Decrease the usage counter of this Group.
         * If it reached zero, it will be removed on
         * the next refresh.
         * Should not be called on Groups, which are
         * being used inside Systems, unless the Group
         * has been acquired using addGetGroup method
         * from Universe.
         * @return Returns current value of the usage
         *   counter after decrement.
         */
        u64 abandon()
        { return decUsage(); }

        /**
         * Get foreach iterator object, iterating over Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, EntityListT> foreach(UT *uni)
        { return EntityList<UT, EntityListT>(uni, *entitiesFront()); }

        /**
         * Get foreach iterator object, iterating over added Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, AddedListT> foreachAdded(UT *uni)
        { return EntityList<UT, AddedListT>(uni, mAdded); }

        /**
         * Get foreach iterator object, iterating over removed Entities.
         * @tparam UT Universe type.
         * @param uni Universe ptr.
         * @return Returns object, which can be used in foreach loop.
         */
        template <typename UT>
        EntityList<UT, RemovedListT> foreachRemoved(UT *uni)
        { return EntityList<UT, RemovedListT>(uni, mRemoved); }
    private:
        /**
         * Increment the usage counter.
         * @return Returns usage counter after incremenatation.
         */
        inline u64 incUsage();

        /**
         * Decrement the usage counter. Will not go
         * below 0.
         * @return Returns usage counter after decrementation.
         */
        inline u64 decUsage();

        /**
         * Reset this group to default state.
         */
        inline void reset();

        /**
         * Add given Entity ID to this group.
         * @param id ID of the Entity.
         */
        inline void add(EntityId id);

        /**
         * Remove given Entity ID from this group.
         * @param id ID of the Entity.
         */
        inline void remove(EntityId id);

        /**
         * Refresh this Group - clear added/removed lists.
         */
        inline void refresh();

        /**
         * Finalize adding and removing of Entities.
         */
        inline void finalize();

        /// Get the front Entity buffer.
        EntityListT *entitiesFront()
        { return mEntities; }
        /// Get the back Entity buffer.
        EntityListT *entitiesBack()
        { return mEntitiesBack; }
        /// Swap the Entity buffers = front <-> back.
        void swapEntityBuffers()
        { std::swap(mEntities, mEntitiesBack); }

        /// Filter specifying which Entities belong into this group.
        EntityFilter mFilter;
        /// ID of this Group.
        u64 mId;
        /// List of Entities, corresponding with the filter.
        EntityListT *mEntities;
        /// List of Entities, corresponding with the filter.
        EntityListT *mEntitiesBack;
        /// Front and back buffers.
        EntityListT mEntityBuffers[2];
        /// List of Entities, which were added since the last refresh.
        AddedListT mAdded;
        /// List of Entities, which were removed since the last refresh.
        RemovedListT mRemoved;
        /// "Reference" counter of how many objects are using this Group.
        u64 mUsageCounter;
    protected:
    }; // EntityGroup

    /**
     * Extract Require and Reject type lists from given System type.
     * Default value (if either list is not present) is empty type list.
     * @tparam SystemT Type of the System.
     */
    template<typename SystemT>
    struct RequireRejectExtractor {
        struct RequireGetter {
            template<typename ST>
            static typename ST::Require getRequire(void *unused);

            template<typename ST>
            static Require<> getRequire(...);

            using RequireT = decltype(getRequire<SystemT>(nullptr));
        };

        struct RejectGetter {
            template<typename ST>
            static typename ST::Reject getReject(void *unused);

            template<typename ST>
            static Reject<> getReject(...);

            using RejectT = decltype(getReject<SystemT>(nullptr));
        };

        using RequireT = typename RequireGetter::RequireT;
        using RejectT = typename RejectGetter::RejectT;
    };
} // namespace ent

#include "EntityGroup.inl"

#endif //ECS_FIT_GROUP_H
