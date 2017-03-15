/**
 * @file Entropy/Util.h
 * @author Tomas Polasek
 * @brief Utility functions/classes used in Entropy ECS.
 */

#ifndef ECS_FIT_UTIL_H
#define ECS_FIT_UTIL_H

#include <limits>
#include <utility>
#include <memory>
#include <string>
// CHAR_BIT
#include <climits>
// memcpy
#include <cstring>

#include "Types.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Used for making classes non copyable/movable, used through inheritance.
     * @code
     *  class MyClass : NonCopyable
     *  {
     *      ...
     *  }
     * @endcode
     */
    class NonCopyable
    {
    public:
        // Delete copy constructors.
        NonCopyable(const NonCopyable& rhs) = delete;
        NonCopyable(NonCopyable&& rhs) = delete;

        // Delete assignment copy operators.
        NonCopyable &operator=(const NonCopyable& rhs) = delete;
        NonCopyable &operator=(NonCopyable&& rhs) = delete;
    private:
    protected:
        // Only usable for inheritance.
        NonCopyable()
        { }
        virtual ~NonCopyable()
        { }
    }; // NonCopyable

    /**
     * Used for making classes instantiable only once + same behavior as NonCopyable.
     * @code
     *  class MyClass : OnceInstantiable<MyClass>
     *  {
     *      ...
     *  }
     * @endcode
     * @tparam T Curiously recurring template pattern.
     */
    template <typename T>
    class OnceInstantiable : NonCopyable
    {
    public:
        OnceInstantiable()
        {
            static bool instantiated{false};
            if (instantiated)
            {
                ENT_WARNING("Class instantiated multiple times, correct functionality is compromised!");
            }
            instantiated = true;
        }
    private:
    protected:
    }; // OnceInstantiable

    /**
     * Allows the allocation of memory for given type, without calling the constructor on it.
     * @tparam T Type which will be contained within.
     */
    template <typename T>
    class alignas(T) ConstructionHandler
    {
    public:
        /// Default constructor, does nothing.
        ConstructionHandler() {}

        /// Destructor, calls destructor on inner class iff it was constructed.
        ~ConstructionHandler()
        {
            if (constructed())
            {
                destruct();
            }
        }

        /**
         * Construct the inner object. If there is constructed object inside,
         * it will be destructed before constructing the new one.
         * @tparam CArgTs Constructor argument types.
         * @param args Constructor arguments
         */
        template <typename... CArgTs>
        void construct(CArgTs... args)
        {
            mHandler.reset(new (mObjectMem) T(std::forward<CArgTs>(args)...));
        }

        /**
         * Explicit destruct the inner object.
         */
        void destruct()
        { mHandler.reset(nullptr); }

        /**
         * Is the inner object constructed?
         * @return Returns true, if the inner object has been constructed.
         */
        bool constructed() const
        { return mHandler.get() != nullptr; }

        /**
         * Reference getter. The inner object should be
         * constructed first!
         * @return Returns reference to the inner object.
         */
        T &operator()()
        { return (*mHandler); }

        /**
         * Pointer getter, if the inner object is not
         * constructed, nullptr is returned.
         * @return Returns ptr to the inner object, or nullptr
         *   if the inner object is not constructed.
         */
        T *ptr()
        { return mHandler.get(); }
    private:
        /// Functor acting as a deletion function called by unique_ptr
        struct Destructor
        {
            void operator()(T *ptr) const
            { ptr->~T(); }
        };
        /// Memory for the inner object.
        alignas(T) u8 mObjectMem[sizeof(T)];
        /// Memory handler.
        std::unique_ptr<T, Destructor> mHandler{nullptr};
    protected:
    };

    /**
     * Used for creation of classes used for generation of unique IDs for types.
     * @code
     *  class MyGenerator : public StaticClassIdGenerator<MyGenerator>
     *  { }
     *  ...
     *  class FirstClass { }
     *  class SecondClass { }
     *  u64 id1 = MyGenerator::getId<FirstClass>() // == 0
     *  u64 id2 = MyGenerator::getId<SecondClass>() // == 1
     *  u64 id3 = MyGenerator::getId<int>() // == 2
     *  u64 id4 = MyGenerator::getId<FirstClass>() // == 0
     * @endcode
     * @tparam T Curiously recurring template pattern (CRTP).
     */
    template <typename T>
    class StaticClassIdGenerator
    {
    public:
        /**
         * Get unique id for given ClassT.
         * IDs begin at 0 and are incremented for each new type.
         * @tparam ClassT Type to generate id for.
         * @return Unique id for given type.
         */
        template <typename ClassT>
        static u64 getId()
        {
            static u64 id{mCounter++};
            return id;
        }
    private:
        /// Counter for unique IDs.
        static u64 mCounter;
    protected:
    };

    // Start the IDs at 0.
    template <typename T>
    u64 StaticClassIdGenerator<T>::mCounter = 0;

#ifdef NOT_USED
    /**
     * Used for creation of classes used for generation of unique IDs for types.
     * Values are generated at compile-time.
     * @code
     *  class MyGenerator : public ClassIdGenerator<MyGenerator>
     *  { }
     *  ...
     *  class FirstClass { }
     *  class SecondClass { }
     *  constexpr u64 id1 = MyGenerator::getId<FirstClass>() // == 0
     *  constexpr u64 id2 = MyGenerator::getId<SecondClass>() // == 1
     *  constexpr u64 id3 = MyGenerator::getId<int>() // == 2
     *  constexpr u64 id4 = MyGenerator::getId<FirstClass>() // == 0
     * @endcode
     * @tparam T Curiously recurring template pattern (CRTP).
     * @tparam S First ID generated.
     */
    template <typename T,
              u64 S = 0>
    class ClassIdGenerator
    {
    public:
        /// Starting ID.
        static constexpr u64 START{S};

        /**
         * Get unique id for given ClassT.
         * IDs begin at S and are incremented for each new type.
         * @tparam ClassT Type to generate id for.
         * @return Unique id for given type.
         */
        template <typename ClassT>
        static constexpr u64 getId()
        {
            return mId<ClassT>;
        }

        /**
         * Check, if an ID has been generated for given type.
         * !! Must be used as default template parameter value, or default function value !!
         * @tparam ClassT Type to check.
         * @param b Dummy parameter, do not pass.
         * @return Returns true, if ID has already been given.
         */
        template <typename ClassT>
        static constexpr bool generated(bool b = generatedImpl<ClassT>())
        {
            return b;
        }
    private:
        /// Wrapper around template generation hacks.
        struct ClassIdHolder
        {
            /**
             * Flag represents a template variable.
             * It can be UNSET, or SET.
             * @tparam ClassT For which type is this Flag used.
             */
            template <typename ClassT, u64 = 0>
            struct TFlag
            {
                template <u64 M>
                friend constexpr bool typeIded(TFlag<ClassT, M>);
            };

            /**
             * If this class has been generated, it means the corresponding
             * Flag is in SET state.
             * @tparam ClassT For which type is the Flag used.
             */
            template <typename ClassT>
            struct TSpecHolder
            {
                template <u64 M>
                friend constexpr bool typeIded(TFlag<ClassT, M>)
                { return true; }
                static constexpr bool value{true};
            };

            /**
             * Flag represents a template variable.
             * It can be UNSET, or SET.
             * @tparam N Which number does this Flag represent.
             */
            template <u64 N, u64 = N>
            struct Flag
            {
                template <u64 M>
                friend constexpr u64 cId(Flag<N, M>);
            };

            /**
             * If this class has been generated, it means the corresponding
             * Flag is in SET state.
             * @tparam N Which number does this Flag represent.
             */
            template <u64 N>
            struct SpecHolder
            {
                template <u64 M>
                friend constexpr u64 cId(Flag<N, M>)
                { return N; }
                static constexpr u64 value{N};
            };

            /**
             * Helper method for SETing Flag and TFlag.
             * @tparam N Which Flag should be SET.
             * @tparam ClassT Which TFlag should be SET.
             * @param val Used for inner template generation hack.
             * @param b Used for inner template generation hack.
             * @return Returns the value of FLAG which has been set.
             */
            template <u64 N,
                      typename ClassT>
            static constexpr u64 mark(u64 val = SpecHolder<N>::value,
                                      bool b = TSpecHolder<ClassT>::value)
            { ENT_UNUSED(b); return val; }

            /**
             * Reader for Flag template variable.
             * This is the method, which is called, when an UNSET Flag has been found.
             * @tparam N Which Flag are we checking.
             * @tparam Enable Used for inner template checking.
             * @return Returns which lowest UNSET Flag has been found.
             */
            template <u64 N,
                      typename Enable = typename std::enable_if<!noexcept(cId(Flag<N>{}))>::type>
            static constexpr u64 reader(int, Flag<N> = {})
            { static_assert(!noexcept(cId(Flag<N>{}))); return N; }

            /**
             * Reader for Flag template variable.
             * This is the searcher method, called, when Flag is SET.
             * @tparam N Which Flag are we checking.
             * @param val Used for searching of the lowest UNSET Flag.
             * @return Returns which lowest UNSET Flag has been found.
             */
            template <u64 N>
            static constexpr u64 reader(float, Flag<N> = {}, u64 val = reader<N + 1>(0))
            { return val; }

            /**
             * Interface method, returns next unused unique number and marks it as SET.
             * @tparam ClassT For which class are we generating ID.
             * @tparam V Finds the lowest UNSET Flag and SETs Flag and TFlag for the ClassT.
             * @return Next unique ID number.
             */
            template <typename ClassT,
                      u64 V = mark<reader<START>(0), ClassT>()>
            static constexpr u64 next()
            {
                return V;
            }

            /**
             * Method for checking if given type has been given an ID.
             * @tparam ClassT Which type should we check.
             * @tparam Enable Used for generating the right response method.
             * @return Returns true, if the type has been given an ID.
             */
            template <typename ClassT,
                      typename Enable = typename std::enable_if<!noexcept(typeIded(TFlag<ClassT>{}))>::type>
            static constexpr bool typeExists(int, TFlag<ClassT> = {})
            { return false; }

            /**
             * Method for checking if given type has been given an ID.
             * @tparam ClassT Which type should we check.
             * @return Returns true, if the type has been given an ID.
             */
            template <typename ClassT>
            static constexpr bool typeExists(float, TFlag<ClassT> = {})
            { return true; }

            /**
             * Interface method for checking if type has been given an ID.
             * @tparam ClassT Which type should we check.
             * @param val Used for getting the reponse value.
             * @return Returns true, if the type has been given an ID.
             */
            template <typename ClassT>
            static constexpr bool exists(bool val = typeExists<ClassT>(0))
            { return val; }
        };

        /**
         * Check, if an ID has been generated for given type.
         * @tparam ClassT Type to check.
         * @tparam B Dummy parameter, do not pass.
         * @return Returns true, if ID has already been given.
         */
        template <typename ClassT,
                  bool B = ClassIdHolder::template exists<ClassT>()>
        static constexpr bool generatedImpl()
        {
            return B;
        }

        /**
         * Contains unique ID for each type it is requested using getId<...>();
         * Automatically increments the counter.
         * @tparam ClassT Type for which the ID is generated.
         */
        template <typename ClassT>
        static constexpr u64 mId{ClassIdHolder::template next<ClassT>()};
    protected:
    }; // ClassIdGenerator
#endif

    /**
     * Bitset, where each bit represents information about presence of something.
     * The bitset is guaranteed to be contiguous and all memory used is inside the object.
     * Bitset is guaranteed to work correctly with copying/moving the bits of
     * the object, without using constructor.
     * @tparam N Number of bits in this bitset.
     */
    template <u64 N>
    class InfoBitset final
    {
    private:
        /// Number of bits requested.
        static ENT_CONSTEXPR_EXPR u64 NUM_BITS{N};
        static_assert(NUM_BITS != 0u, "Bitset must have at least 1 bit!");
        /// Base memory block type.
        using BMBType = u64;
        /// Number of chars (probably bytes) in one base memory block.
        static ENT_CONSTEXPR_EXPR u64 BLOCK_IN_CHARS{sizeof(BMBType) / sizeof(char)};
        static_assert((BLOCK_IN_CHARS * sizeof(char)) == sizeof(BMBType));
        /// Size of the base memory block type in bits.
        static ENT_CONSTEXPR_EXPR u64 BITS_IN_BLOCK{BLOCK_IN_CHARS * CHAR_BIT};
        static_assert(BITS_IN_BLOCK != 0u);
        /// Number of base memory blocks to get required number of bits.
        static ENT_CONSTEXPR_EXPR u64 NUM_BLOCKS{(N / BITS_IN_BLOCK) + ((N % BITS_IN_BLOCK) == 0u ? 0u : 1u)};
        /// Number of base memory blocks which are completely used by bitset bits.
        static ENT_CONSTEXPR_EXPR u64 NUM_WHOLE_BLOCKS{(N / BITS_IN_BLOCK)};
        /// Number of bytes used by bitset bits.
        static ENT_CONSTEXPR_EXPR u64 NUM_USED_BYTES{(N / (sizeof(char) * CHAR_BIT)) +
                                                     ((N % (sizeof(char) * CHAR_BIT)) == 0u ? 0u : 1u)};
        /// Byte value, where all bits are set to '0'.
        static ENT_CONSTEXPR_EXPR u8 BYTE_ZERO{0u};
        /// Byte value, where all bits are set to '1'.
        static ENT_CONSTEXPR_EXPR u8 BYTE_ONE{~BYTE_ZERO};
        /// Block value, where all bits are set to '0'.
        static ENT_CONSTEXPR_EXPR BMBType BLOCK_ZERO{0u};
        /// Block value, where all bits are set to '1'.
        static ENT_CONSTEXPR_EXPR BMBType BLOCK_ONE{~BLOCK_ZERO};
        /// Is there a partly-used memory block?
        static ENT_CONSTEXPR_EXPR bool PARTLY_USED{NUM_WHOLE_BLOCKS != NUM_BLOCKS};
        /// Number of bits used in the last memory block.
        static ENT_CONSTEXPR_EXPR u64 BITS_IN_LAST_BLOCK{NUM_BITS - (NUM_WHOLE_BLOCKS * BITS_IN_BLOCK)};

        /**
         * Get index of the block, where bit with given index is located.
         * @param pos Position of the bit.
         * @return Block, where the bit is located.
         */
        static ENT_CONSTEXPR_FUN u64 memBlock(u64 pos)
        { return pos / BITS_IN_BLOCK; }

        /**
         * Get index of the bit, where bit with given index is located.
         * @param pos Position of the bitset bit.
         * @return Position of the block bit.
         */
        static ENT_CONSTEXPR_FUN u64 memBit(u64 pos)
        { return pos % BITS_IN_BLOCK; }

        /**
         * Get mask for bit on given position.
         * This mask should be applied to the block, where the bit is located.
         * @param pos Position of the bit in bitset.
         * @return Returns AND mask.
         */
        static ENT_CONSTEXPR_FUN BMBType bitMask(u64 pos)
        { return static_cast<BMBType>(1u) << memBit(pos); }

        /**
         * Mask used for masking out unused part of last memory block.
         * @return Returns mask, used for 'and' masking.
         */
        static ENT_CONSTEXPR_FUN BMBType partlyUsedBlockMask()
        { return ~((~static_cast<BMBType>(0u)) << BITS_IN_LAST_BLOCK); }
    public:
        /**
         * Get number of bits.
         * @return The number of bits in this bitset.
         */
        static ENT_CONSTEXPR_FUN u64 size()
        { return NUM_BITS; }

        /**
         * Get number of unused bits.
         * @return The number of unused bits in the contiguous memory.
         */
        static ENT_CONSTEXPR_FUN u64 excess()
        { return (NUM_BLOCKS * BITS_IN_BLOCK) - size(); }

        /// Default constructor, initializes the value to 0.
        ENT_CONSTEXPR_FUN InfoBitset() :
            InfoBitset(0u)
        { }

        /// Construct from a number.
        ENT_CONSTEXPR_FUN InfoBitset(BMBType num) :
            mMemory{num}
        { }

        // Copy/move operators.
        InfoBitset(const InfoBitset &rhs)
        { copy(rhs); }
        InfoBitset(InfoBitset &&rhs)
        { move(rhs); }

        // Copy-assignment operators.
        InfoBitset &operator=(const InfoBitset &rhs)
        { copy(rhs); return *this; }
        InfoBitset &operator=(InfoBitset &&rhs)
        { move(rhs); return *this; }

        /**
         * Assign any value into this bitset, using memcpy.
         * Size of the value type must be <= to the number of bits.
         * @tparam T Type of the value.
         * @param val Value to copy.
         * @return Returns this.
         */
        template <typename T>
        InfoBitset &operator=(const T &val)
        { fromVal(val); return *this; }

        /// Set all bits to true.
        InfoBitset &set()
        { setImpl(); return *this; }

        /**
         * Set bit on given position to given value (true by default).
         * @param pos Position of the bit.
         * @param val Value to set.
         */
        InfoBitset &set(std::size_t pos)
        { setImpl(pos); return *this; }

        /// Set all bits to false.
        InfoBitset &reset()
        { resetImpl(); return *this; }

        /**
         * Reset bit on given position.
         * @param pos Position of the bit.
         */
        InfoBitset &reset(std::size_t pos)
        { resetImpl(pos); return *this; }

        /**
         * Count the number of bits set to true.
         * @return The number of bits set to true.
         */
        std::size_t count() const
        { return countImpl(); }

        /// Are all bits set to true?
        bool all() const;

        /// Are any bits set to true?
        bool any() const
        { return !noneImpl(); }

        /// Are none of the bits set to true?
        bool none() const
        { return noneImpl(); }

        /**
         * Get the value of the bit on given position.
         * @param pos Position of the bit.
         * @return Value of the bit.
         */
        bool test(std::size_t pos) const
        { return testImpl(pos); }

        /**
         * Copy bits from other bitset.
         * @param other The other bitset.
         */
        void copy(const InfoBitset &other);

        /**
         * Swap bits between 2 bitsets.
         * @param other The other bitset.
         */
        void swap(InfoBitset &other);

        /// Convert this bitset to string.
        std::string toString() const;

        /// Binary AND operator.
        InfoBitset operator&(const InfoBitset &rhs) const;

        /// Binary OR operator.
        InfoBitset operator|(const InfoBitset &rhs) const;

        /// Comparison operator.
        bool operator==(const InfoBitset &rhs) const;

        /// Comparison operator.
        bool operator!=(const InfoBitset &rhs) const
        { return !(*this == rhs);}

        /// Print operator.
        template <u64 M>
        friend std::ostream &operator<<(std::ostream &out, const InfoBitset<M> &rhs);
    private:
        /**
         * Move bits from other bitset.
         * @param other The other bitset.
         */
        void move(InfoBitset &other)
        { copy(other); }

        /**
         * Get block with given index.
         * @param block ID of the block.
         * @return Returns reference to the block.
         */
        ENT_CONSTEXPR_FUN BMBType &getBlock(u64 block)
        { return mMemory[block]; }
        ENT_CONSTEXPR_FUN const BMBType &getBlock(u64 block) const
        { return mMemory[block]; }

        /**
         * Perform memory copy of given value to the bit storage.
         * At most the number of bits copied will be the number of bits in this bitset.
         * @tparam T Type of the value.
         * @param val Value to copy.
         */
        template <typename T>
        void fromVal(const T &val);

        /**
         * Set all bits to true.
         */
        void setImpl();

        /**
         * Set bit on given position to true.
         * @param pos Position of the bit.
         */
        void setImpl(u64 pos);

        /**
         * Set all bits to false.
         */
        void resetImpl();

        /**
         * Set bit on given position to false.
         * @param pos Position of the bit.
         */
        void resetImpl(u64 pos);

        /**
         * Count the number of bits set to true.
         * @return The number of bits set to true.
         */
        u64 countImpl() const;

        /**
         * Test the bits, if none of the are set to true.
         * @return Returns true iff none of the bits are set to true.
         */
        bool noneImpl() const;

        /**
         * Test bit on given position and return its value.
         * @param pos Position to test.
         * @return Returns the value of given bit.
         */
        ENT_CONSTEXPR_FUN bool testImpl(u64 pos) const;
    protected:
        /// Memory blocks containing the bits.
        BMBType mMemory[NUM_BLOCKS];
    }; // InfoBitset

    /// Bitset, where each bit represents either a present, or absent Component.
    using ComponentBitset = InfoBitset<MAX_COMPONENTS>;

    /// Bitset, where each bit represents Entity presence in a group. +1 for the activity flag.
    using GroupBitset = InfoBitset<MAX_GROUPS + 1>;

    /**
     * Get the next higher or equal number, which is power of two.
     * @param value Value.
     * @return Returns a number >= value, which is power of two.
     */
    static ENT_CONSTEXPR_FUN u64 pow2RoundUp(u64 value)
    {
        --value;
        value |= value >> 0b1;
        value |= value >> 0b10;
        value |= value >> 0b100;
        value |= value >> 0b1000;
        value |= value >> 0b10000;
        value |= value >> 0b100000;

        return value + 1;
    }

    // InfoBitset implementation.
    template <u64 N>
    bool InfoBitset<N>::all() const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != BLOCK_ONE)
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the last block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) !=
                (BLOCK_ONE & partlyUsedBlockMask()))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 N>
    void InfoBitset<N>::copy(const InfoBitset &other)
    {
        std::memcpy(mMemory, other.mMemory, sizeof(mMemory));
    }

    template <u64 N>
    void InfoBitset<N>::swap(InfoBitset &other)
    {
        for (u64 block = 0; block < NUM_BLOCKS; ++block)
        {
            std::swap(mMemory[block], other.mMemory[block]);
        }
    }

    template <u64 N>
    std::string InfoBitset<N>::toString() const
    {
        std::string out;
        out.resize(NUM_BITS);

        for (u64 bit = 0u; bit < NUM_BITS; ++bit)
        {
            out[bit] = testImpl(bit) ? '1' : '0';
        }

        return out;
    }

    template <u64 N>
    auto InfoBitset<N>::operator&(const InfoBitset &rhs) const -> InfoBitset
    {
        InfoBitset result;

        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            result.getBlock(block) = getBlock(block) & rhs.getBlock(block);
        }

        return result;
    }

    template <u64 N>
    auto InfoBitset<N>::operator|(const InfoBitset &rhs) const -> InfoBitset
    {
        InfoBitset result;

        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            result.getBlock(block) = getBlock(block) | rhs.getBlock(block);
        }

        return result;
    }

    template <u64 N>
    bool InfoBitset<N>::operator==(const InfoBitset &rhs) const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != rhs.getBlock(block))
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) !=
                (rhs.getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 M>
    std::ostream &operator<<(std::ostream &out, const InfoBitset<M> &rhs)
    {
        out << rhs.toString();
        return out;
    }

    template <u64 N>
    template <typename T>
    void InfoBitset<N>::fromVal(const T &val)
    {
        std::memcpy(mMemory, &val, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::setImpl()
    {
        std::memset(mMemory, BYTE_ONE, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::setImpl(u64 pos)
    {
        getBlock(memBlock(pos)) |= bitMask(pos);
    }

    template <u64 N>
    void InfoBitset<N>::resetImpl()
    {
        std::memset(mMemory, BYTE_ZERO, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::resetImpl(u64 pos)
    {
        getBlock(memBlock(pos)) &= ~bitMask(pos);
    }

    template <u64 N>
    u64 InfoBitset<N>::countImpl() const
    {
        u64 result{0u};

        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            result += popcount64(getBlock(block));
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            result += popcount64(getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask());
        }

        return result;
    }

    template <u64 N>
    bool InfoBitset<N>::noneImpl() const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != static_cast<BMBType>(0u))
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) != static_cast<BMBType>(0u))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 N>
    ENT_CONSTEXPR_FUN bool InfoBitset<N>::testImpl(u64 pos) const
    {
        return getBlock(memBlock(pos)) & bitMask(pos);
    }
    // InfoBitset implementation end.
} // namespace ent

#endif //ECS_FIT_UTIL_H
