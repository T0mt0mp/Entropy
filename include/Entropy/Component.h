/**
 * @file Entropy/Component.h
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#ifndef ECS_FIT_COMPONENT_H
#define ECS_FIT_COMPONENT_H

#include "Types.h"
#include "Util.h"
#include "EntityId.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * ComponentManager is a part of Entropy ECS Universe.
     * Used for handling list of ComponentHolders.
     * Contains methods for adding new ComponentHolders and
     * their refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class ComponentManager final : NonCopyable
    {
    public:
        /**
         * Default constructor.
         */
        ComponentManager();

        /// Destructor.
        ~ComponentManager();

        /**
         * Register given Component with its ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam HolderT Type of the Holder.
         * @tparam CArgTs Constructor argument types.
         * @param args Constructor arguments passed to the Holder constructor.
         */
        template <typename ComponentT,
                  typename HolderT,
                  typename... CArgTs>
        inline void registerComponent(CArgTs... args);
    private:
        /**
         * Holder instance.
         * @tparam HolderT Type of the Holder.
         */
        template <typename HolderT>
        static HolderT* mHolder{nullptr};


    protected:
    }; // ComponentManager

    /**
     * Base Component holder.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class BaseComponentHolder : NonCopyable
    {
    public:
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual CompPtr add(EntityId id) noexcept = 0;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual CompPtr get(EntityId id) noexcept = 0;

        /**
         * Does given Entity have an Component associated with it?
         * @param id Id of the Entity.
         * @return Returns true, if there IS an Component associated.
         */
        virtual bool has(EntityId id) const noexcept = 0;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual void remove(EntityId id) noexcept = 0;
    private:
    protected:
    };

    /**
     * Default ComponentHolder with all required functionality.
     * Basic implementation.
     * @tparam ComponentT Type of the Component contained within.
     */
    template <typename ComponentT>
    class ComponentHolder final : public BaseComponentHolder<ComponentT>
    {
    public:
        using CompT = ComponentT;
        using CompRef = CompT&;
        using CompPtr = CompT*;

        /**
         * Default constructor.
         */
        ComponentHolder();

        /// Destructor
        ~ComponentHolder();

        /**
         * Add Component for given EntityId, if the Component
         * already exists, nothing happens.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component.
         */
        virtual inline CompPtr add(EntityId id) noexcept;

        /**
         * Get Component belonging to given EntityId.
         * @param id Id of the Entity.
         * @return Returns pointer to the Component, or nullptr, if it does not exist.
         */
        virtual inline CompPtr get(EntityId id) noexcept;

        /**
         * Does given Entity have an Component associated with it?
         * @param id Id of the Entity.
         * @return Returns true, if there IS an Component associated.
         */
        virtual inline bool has(EntityId id) const noexcept;

        /**
         * Remove Component for given Entity. If the Entity does not have
         * Component associated with it, nothing happens.
         * @param id Id of the Entity.
         */
        virtual inline void remove(EntityId id) noexcept;
    private:
        /// Mapping from EntityId to Component.
        std::map<EntityId, CompT> mMap;
    protected:
    };

    /**
     * Bitset, where each bit represents a single Component type.
     */
    class ComponentBitset final
    {
    public:
        /// Type of the inner bitset.
        using CBitset = std::bitset<MAX_COMPONENTS>;

        /// Default constructor.
        constexpr ComponentBitset() = default;

        /// Construct from a number.
        constexpr ComponentBitset(u64 num) :
            mBitset(num)
        { }

        // Copy/move operators.
        constexpr ComponentBitset(const ComponentBitset &rhs) :
            mBitset(rhs.mBitset) { }
        constexpr ComponentBitset(ComponentBitset &&rhs) :
            mBitset(std::move(rhs.mBitset)) { }
        constexpr ComponentBitset(const CBitset &rhs) :
            mBitset(rhs) { }
        constexpr ComponentBitset(CBitset &&rhs) :
            mBitset(std::move(rhs)) { }

        // Copy-assignment operators.
        constexpr ComponentBitset &operator=(const ComponentBitset &rhs)
        { mBitset = rhs.mBitset; return *this; }
        constexpr ComponentBitset &operator=(ComponentBitset &&rhs)
        { mBitset = std::move(rhs.mBitset); return *this; }
        constexpr ComponentBitset &operator=(const CBitset &rhs)
        { mBitset = rhs; return *this; }
        constexpr ComponentBitset &operator=(CBitset &&rhs)
        { mBitset = std::move(rhs); return *this; }

        ComponentBitset &operator=(u64 val)
        { mBitset = val; return *this; }

        /// Set all bits to true.
        void set()
        { mBitset.set(); }

        /**
         * Set bit on given position to given value (true by default).
         * @param pos Position of the bit.
         * @param val Value to set.
         */
        void set(std::size_t pos, bool val = true)
        { mBitset.set(pos, val); }

        /// Set all bits to false.
        void reset()
        { mBitset.reset(); }

        /**
         * Reset bit on given position.
         * @param pos Position of the bit.
         */
        void reset(std::size_t pos)
        { mBitset.reset(pos); }

        /**
         * Get the max number of component types.
         * @return The max number of component types.
         */
        std::size_t size() const
        { return mBitset.size(); }

        /**
         * Count the number of bits set to true.
         * @return The number of bits set to true.
         */
        std::size_t count() const
        { return mBitset.count(); }

        /// Are any bits set to true?
        bool any() const
        { return mBitset.any(); }

        /// Are none of the bits set to true?
        bool none() const
        { return mBitset.none(); }

        /// Are all of the bits set to true?
        bool all() const
        { return mBitset.all(); }

        /**
         * Get the value of the bit on given position.
         * @param pos Position of the bit.
         * @return Value of the bit.
         */
        bool test(std::size_t pos) const
        { return mBitset.test(pos); }

        ComponentBitset operator&(const ComponentBitset &rhs) const
        { return ComponentBitset(mBitset & rhs.mBitset); }

        ComponentBitset operator|(const ComponentBitset &rhs) const
        { return ComponentBitset(mBitset | rhs.mBitset); }

        bool operator==(const ComponentBitset &rhs) const
        { return mBitset == rhs.mBitset; }
    private:
        /// Inner bitset.
        CBitset mBitset;
    protected:
    };

    /**
     * ComponentFilter is used for filtering Entities by their present/missing Components.
     */
    class ComponentFilter final
    {
    public:
        /**
         * Create a filter from require and reject masks.
         * @param require Which bits need to be true.
         * @param reject Which bits must not be true, unless they are in the require mask.
         */
        ComponentFilter(const ComponentBitset &require,
                        const ComponentBitset &reject) :
            mRequire(require), mMask(require | reject) { }

        /**
         * Create a filter from require mask.
         * @param require Which bits need to be true.
         */
        ComponentFilter(const ComponentBitset &require) :
            mRequire(require), mMask(require) { }

        /**
         * Create a filter from require and reject masks.
         * @param require Which bits need to be true.
         * @param reject Which bits must not be true, unless they are in the require mask.
         */
        ComponentFilter(ComponentBitset&& require,
                        ComponentBitset&& reject) :
            mRequire(require), mMask(require | reject) { }

        /**
         * Check if the given bitset passes this filter.
         * @param bitset Bitset to check.
         * @return Returns true, if the bitset passes through this filter.
         */
        bool match(const ComponentBitset &bitset) const
        { return (bitset & mMask) == mRequire; }
    private:
        /// Bitset signifying which Component types need to be present.
        ComponentBitset mRequire;
        /// Bitset mask representing which bits are of interest.
        ComponentBitset mMask;
    protected:
    };

    // ComponentManager implementation.
    template <typename UniverseT>
    ComponentManager<UniverseT>::ComponentManager()
    { }

    template <typename UniverseT>
    ComponentManager<UniverseT>::~ComponentManager()
    { }
    // ComponentManager implementation end.

    // ComponentHolder implementation.
    template <typename ComponentT>
    ComponentT* ComponentHolder<ComponentT>::add(EntityId id) noexcept
    {
        return get(id);
    }

    template <typename ComponentT>
    ComponentT* ComponentHolder<ComponentT>::get(EntityId id) noexcept
    {
        ComponentT* result{nullptr};
        try {
            result = &mMap[id];
        } catch(...) {
            ENT_WARNING("Get has thrown an exception!");
        }
        return result;
    }

    template <typename ComponentT>
    bool ComponentHolder<ComponentT>::has(EntityId id) const noexcept
    {
        return (mMap.find(id) != mMap.end());
    }

    template <typename ComponentT>
    void ComponentHolder<ComponentT>::remove(EntityId id) noexcept
    {
        try {
            mMap.erase(id);
        } catch(...) {
            ENT_WARNING("Remove has thrown an exception!");
        }
    }
    // ComponentHolder implementation end.
} // namespace ent

#endif //ECS_FIT_COMPONENT_H
