/**
 * @file Entropy/Util.h
 * @author Tomas Polasek
 * @brief Utility functions/classes used in Entropy ECS.
 */

#ifndef ECS_FIT_UTIL_H
#define ECS_FIT_UTIL_H

#include <limits>

#include "Types.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Used for making classes non copyable, used through inheritance.
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
        ~NonCopyable()
        { }
    }; // NonCopyable


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
        // TODO - Comments!
        struct ClassIdHolder
        {
            template <typename ClassT, u64 = 0>
            struct TFlag
            {
                template <u64 M>
                friend constexpr bool typeIded(TFlag<ClassT, M>);
            };

            template <typename ClassT>
            struct TSpecHolder
            {
                template <u64 M>
                friend constexpr bool typeIded(TFlag<ClassT, M>)
                { return true; }
                static constexpr bool value{true};
            };

            template <u64 N, u64 = N>
            struct Flag
            {
                template <u64 M>
                friend constexpr u64 cId(Flag<N, M>);
            };

            template <u64 N>
            struct SpecHolder
            {
                template <u64 M>
                friend constexpr u64 cId(Flag<N, M>)
                { return N; }
                static constexpr u64 value{N};
            };

            template <u64 N,
                      typename ClassT>
            static constexpr u64 mark(u64 val = SpecHolder<N>::value,
                                      bool b = TSpecHolder<ClassT>::value)
            { ENT_UNUSED(b); return val; }

            template <u64 N,
                      typename Enable = typename std::enable_if<!noexcept(cId(Flag<N>{}))>::type>
            static constexpr u64 reader(int, Flag<N> = {})
            { static_assert(!noexcept(cId(Flag<N>{}))); return N; }

            template <u64 N>
            static constexpr u64 reader(float, Flag<N> = {}, u64 val = reader<N + 1>(0))
            { return val; }

            template <typename ClassT,
                      u64 V = mark<reader<START>(0), ClassT>()>
            static constexpr u64 next()
            {
                return V;
            }

            template <typename ClassT,
                      typename Enable = typename std::enable_if<!noexcept(typeIded(TFlag<ClassT>{}))>::type>
            static constexpr bool typeExists(int, TFlag<ClassT> = {})
            { return false; }

            template <typename ClassT>
            static constexpr bool typeExists(float, TFlag<ClassT> = {})
            { return true; }

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
    };
} // namespace ent

#endif //ECS_FIT_UTIL_H
