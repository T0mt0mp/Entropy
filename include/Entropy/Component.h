/**
 * @file Entropy/Component.h
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#ifndef ECS_FIT_COMPONENT_H
#define ECS_FIT_COMPONENT_H

#include "Types.h"

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
    class ComponentManager : NonCopyable
    {
    public:
    private:
    protected:
    }; // ComponentManager

    /**
     * Bitset, where each bit represents a single Component type.
     */
    class ComponentBitset
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
    class ComponentFilter
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
} // namespace ent

#endif //ECS_FIT_COMPONENT_H
