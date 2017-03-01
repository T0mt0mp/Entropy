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
        ~ConstructionHandler() {}

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
        { ENT_ASSERT_FAST(constructed()); return (*mHandler); }

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
    u64 StaticClassIdGenerator<T>::mCounter{0};

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

    /**
     * Bitset, where each bit represents information about presence of something.
     * @tparam N Number of bits in this bitset.
     */
    template <u64 N>
    class InfoBitset final
    {
    public:
        // TODO - rewrite bitset, trivial constructor.
        /// Type of the inner bitset.
        using CBitset = std::bitset<N>;

        /// Default constructor.
        constexpr InfoBitset() :
            InfoBitset(0u)
        { }

        /// Construct from a number.
        constexpr InfoBitset(u64 num) :
            mBitset(num)
        { }

        // Copy/move operators.
        constexpr InfoBitset(const InfoBitset &rhs) :
            mBitset(rhs.mBitset) { }
        constexpr InfoBitset(InfoBitset &&rhs) :
            mBitset(std::move(rhs.mBitset)) { }
        constexpr InfoBitset(const CBitset &rhs) :
            mBitset(rhs) { }
        constexpr InfoBitset(CBitset &&rhs) :
            mBitset(std::move(rhs)) { }

        // Copy-assignment operators.
        constexpr InfoBitset &operator=(const InfoBitset &rhs)
        { mBitset = rhs.mBitset; return *this; }
        constexpr InfoBitset &operator=(InfoBitset &&rhs)
        { mBitset = std::move(rhs.mBitset); return *this; }
        constexpr InfoBitset &operator=(const CBitset &rhs)
        { mBitset = rhs; return *this; }
        constexpr InfoBitset &operator=(CBitset &&rhs)
        { mBitset = std::move(rhs); return *this; }

        InfoBitset &operator=(u64 val)
        { mBitset = val; return *this; }

        /// Set all bits to true.
        InfoBitset &set()
        { mBitset.set(); return *this; }

        /**
         * Set bit on given position to given value (true by default).
         * @param pos Position of the bit.
         * @param val Value to set.
         */
        InfoBitset &set(std::size_t pos, bool val = true)
        { mBitset.set(pos, val); return *this; }

        /// Set all bits to false.
        InfoBitset &reset()
        { mBitset.reset(); return *this; }

        /**
         * Reset bit on given position.
         * @param pos Position of the bit.
         */
        InfoBitset &reset(std::size_t pos)
        { mBitset.reset(pos); return *this; }

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

        /// Binary AND operator.
        InfoBitset operator&(const InfoBitset &rhs) const
        { return InfoBitset(mBitset & rhs.mBitset); }

        /// Binary OR operator.
        InfoBitset operator|(const InfoBitset &rhs) const
        { return InfoBitset(mBitset | rhs.mBitset); }

        /// Comparison operator.
        bool operator==(const InfoBitset &rhs) const
        { return mBitset == rhs.mBitset; }

        /// Print operator.
        template <u64 M>
        friend std::ostream &operator<<(std::ostream &out, const InfoBitset<M> &rhs)
        {
            out << rhs.mBitset;
            return out;
        }
    private:
        /// Inner bitset.
        CBitset mBitset;
    protected:
    }; // InfoBitset

    /// Bitset, where each bit represents either a present, or absent Component.
    using ComponentBitset = InfoBitset<MAX_COMPONENTS>;

    /// Bitset, where each bit represents Entity presence in a group.
    using GroupBitset = InfoBitset<MAX_GROUPS>;
} // namespace ent

#endif //ECS_FIT_UTIL_H