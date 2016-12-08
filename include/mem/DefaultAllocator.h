/**
 * @file mem/DefaultAllocator.h
 * @author Tomas Polasek
 * @brief Default allocator for memory structures.
 */

#ifndef MEM_DEFAULTALLOCATOR_H
#define MEM_DEFAULTALLOCATOR_H

#include "util/Types.h"

namespace mem
{
    template <typename T>
    class DefaultAllocator
    {
    public:
        using pointer               = T*;
        using const_pointer         = const T*;
        using reference             = T&;
        using const_reference       = const T&;
        using void_pointer          = void*;
        using const_void_pointer    = const void*;
        using value_type            = T;
        using size_type             = std::size_t;
        using difference_type       = std::ptrdiff_t;

        /// Rebind the allocator for type U.
        template <typename U>
        struct rebind
        {
            using other = DefaultAllocator<U>;
        };

        /// Get pointer to given variable
        pointer address(reference val) const
        {
            return &val;
        }

        /// Get pointer to given variable
        const_pointer address(const_reference val) const
        {
            return &val;
        }

        DefaultAllocator() noexcept {};
        DefaultAllocator(const DefaultAllocator<T> &other) noexcept {};
        DefaultAllocator(DefaultAllocator<T> &&other) noexcept {};
        ~DefaultAllocator() noexcept {};
        DefaultAllocator &operator=(const DefaultAllocator<T> &rhs) noexcept
        { return *this; };
        DefaultAllocator &operator=(DefaultAllocator<T> &&rhs) noexcept
        { return *this; };

        /**
         * Allocate space for n values. Does not initialize/construct.
         * @param n How many elements to allocate.
         * @param cvptr Locality helper - Unused.
         * @return Pointer to the allocated memory.
         */
        pointer allocate(size_type n, const_void_pointer cvptr = nullptr)
        {
            return static_cast<pointer>(::operator new(n * sizeof(value_type)));
        }

        /**
         * Deallocate space allocated by this allocator.
         * @param ptr Pointer to the memory.
         * @param n Number of elements allocated - Unused.
         */
        void deallocate(pointer ptr, size_type n = 0)
        {
            ::operator delete(static_cast<void_pointer>(ptr));
        }

        /// Get the maximum number of allocatable objects.
        size_type max_size() const noexcept
        {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }

        /**
         * Construct object of type value_type in previously
         * allocated space, pointed to by the xptr.
         * @param xpotr Pointer to previously allocated memory
         *   by this allocator
         * @param args Arguments passed to the constructor.
         */
        template <typename... Args>
        void construct(pointer xptr, Args... args)
        {
            ::new(static_cast<void*>(xptr)) value_type(std::forward<Args>(args)...);
        }

        /**
         * Destruct previously constructed object, allocated
         * by this allocator. Does not deallocate the memory.
         * @param xptr Pointer to the object.
         */
        void destroy(pointer xptr)
        {
            xptr->~value_type();
        }
    private:
    protected:
    };

    /// All DefaultAllocator instances are interchangable.
    template <typename T1,
              typename T2>
    bool operator==(const DefaultAllocator<T1>&,
                    const DefaultAllocator<T2>&)
    {
        return true;
    }

    /// All DefaultAllocator instances are interchangable.
    template <typename T1,
              typename T2>
    bool operator!=(const DefaultAllocator<T1>&,
                    const DefaultAllocator<T2>&)
    {
        return false;
    }
}

#endif //MEM_DEFAULTALLOCATOR_H
