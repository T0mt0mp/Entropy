/**
 * @file mem/List.h
 * @author Tomas Polasek
 * @brief List holds data in contiguous memory.
 */

#ifndef MEM_LIST_H
#define MEM_LIST_H

#include <iterator>
#include <cstring>
#include <iostream>

#include "util/Types.h"
#include "util/Meta.h"
#include "math/Math.h"
#include "mem/DefaultAllocator.h"

namespace mem
{
    // TODO - implement memory manager and start using it.
    /**
     * List implementation.
     * List holds data in contiguous memory.
     */
    template <typename ElementT,
              typename AllocatorT = mem::DefaultAllocator<ElementT>>
    class List
    {
    public:
        /**
         * Iterator for traversing the list.
         */
        template <bool isConst = false>
        class ListIterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = mp_choose_t<isConst,
                                const ElementT,
                                ElementT>;
            using ref_type = mp_choose_t<isConst,
                                const ElementT&,
                                ElementT&>;
            using ptr_type = mp_choose_t<isConst,
                                const ElementT*,
                                ElementT*>;
            using ThisType = ListIterator<isConst>;

            ListIterator() = default;
            ListIterator(const ThisType &other) = default;
            ListIterator(ThisType &&other) = default;
            ThisType &operator=(const ThisType &rhs) = default;
            ThisType &operator=(ThisType &&rhs) = default;

            ListIterator(ptr_type ptr) :
                mPtr{ptr} {};

            bool operator==(const ThisType &rhs) const
            {
                return mPtr == rhs.mPtr;
            }

            bool operator!=(const ThisType &rhs) const
            {
                return mPtr != rhs.mPtr;
            }

            ref_type operator*() const
            {
                return *mPtr;
            }

            ptr_type operator->() const
            {
                return mPtr;
            }

            ThisType &operator++()
            {
                mPtr++;
                return *this;
            }

            ThisType operator++(int)
            {
                ThisType temp(mPtr);

                ++(*this);

                return temp;
            }

            ThisType &operator--()
            {
                mPtr--;
                return *this;
            }

            ThisType operator--(int)
            {
                ThisType temp(mPtr);

                --(*this);

                return temp;
            }

            ThisType operator+(u64 val) const
            {
                return ThisType(mPtr + val);
            }

            ThisType &operator+=(u64 val)
            {
                mPtr += val;
                return *this;
            }

            friend ThisType operator+(u64 val, const ThisType &rhs)
            {
                return ThisType(val + rhs.mPtr);
            }

            ThisType operator-(u64 val) const
            {
                return ThisType(mPtr - val);
            }

            ThisType &operator-=(u64 val)
            {
                mPtr -= val;
                return *this;
            }

            friend ThisType operator-(u64 val, const ThisType &rhs)
            {
                return ThisType(val - rhs.mPtr);
            }

            bool operator<(const ThisType &rhs) const
            {
                return mPtr < rhs.mPtr;
            }

            bool operator>(const ThisType &rhs) const
            {
                return mPtr > rhs.mPtr;
            }

            bool operator<=(const ThisType &rhs) const
            {
                return mPtr <= rhs.mPtr;
            }

            bool operator>=(const ThisType &rhs) const
            {
                return mPtr >= rhs.mPtr;
            }

            ref_type operator[](u64 index) const
            {
                return mPtr[index];
            }
        private:
            ptr_type mPtr{nullptr};
        protected:
        };

        using ListType = List<ElementT, AllocatorT>;
        using Iterator = ListIterator<false>;
        using ConstIterator = ListIterator<true>;

        /// Copy constructor.
        List(const ListType &other);
        /// Move constructor.
        List(ListType &&other);

        /**
         * Destructor calls destructor of each
         * element in reverse order (back -> front)
         * and frees the memory.
         */
        ~List();

        /**
         * Create a list with given size.
         * @param startSize Starting size of the list.
         */
        List(u64 startSize = 0);

        /**
         * Create a list with given size.
         * @param startSize Starting size of the list.
         * @param val All elements will be set to this value.
         */
        List(u64 startSize, const ElementT &val);

        /// Copy-assignment operator
        ListType &operator=(const ListType &rhs);
        /// Move-assignment operator.
        ListType &operator=(ListType &&rhs);

        /**
         * @return The iterator at the beginning of this list.
         */
        Iterator begin();

        /**
         * @return The iterator at the beginning of this list.
         */
        ConstIterator begin() const;

        /**
         * @return The const iterator at the beginning of this list.
         */
        ConstIterator cbegin() const;

        /**
         * @return The iterator one after the end of the list.
         */
        Iterator end();

        /**
         * @return The iterator one after the end of the list.
         */
        ConstIterator end() const;

        /**
         * @return The const iterator one after the end of the list.
         */
        ConstIterator cend() const;

        /**
         * Swap the internal structures of 2 lists.
         * @param other The other list.
         */
        void swap(ListType &other);

        /**
         * Appends a copy of given element to the list.
         * @param element The element to push.
         */
        void pushBack(const ElementT &element);

        /**
         * Uses move assignment instead of copy assignment.
         * @param element The element to push.
         */
        void pushBack(ElementT &&element);

        /**
         * Uses emplacement new to construct a new element
         * with given parameters.
         * @param args Arguments of the constructor.
         */
        template <typename... Args>
        void emplaceBack(Args... args);

        /**
         * Remove element on given index.
         * @param index Index of the element.
         * @return Returns true, if the removing was successful.
         */
        bool remove(u64 index);

        /**
         * Access element on given index.
         * @param index The index.
         * @return Reference to the element.
         */
        ElementT &operator[](u64 index);

        /**
         * Access element on given index.
         * @param index The index.
         * @return Reference to the element.
         */
        const ElementT &operator[](u64 index) const;

        /**
         * Access the last element in this list.
         * @return Reference to the last element.
         */
        ElementT &back();

        /**
         * Access the last element in this list.
         * @return Reference to the last element.
         */
        const ElementT &back() const;

        /**
         * Delete the last element.
         */
        void popBack();

        /**
         * Delete all of the elements.
         */
        void clear();

        /**
         * Checks if there is enough space to push at least
         * given number of elements. If there is not enough
         * space, then the memory is resized.
         * @param amount The number of elements.
         */
        void makeSpace(u64 amount = 1);

        /**
         * Is this list empty?
         * @return Returns true, if the list is empty.
         */
        bool empty() const;

        /**
         * Get the number of elements in this list.
         * @return The number of elements in this list.
         */
        u64 size() const;

        /**
         * Copy contents of other list.
         * Uses copy constructor for each element.
         * @param other The other list.
         */
        void copy(const List &other);

        /**
         * Uses memcpy to copy the elements.
         * Constructors are not called!!
         * @param other The other list.
         */
        void memoryCopy(const List &other);

        /**
         * Reallocate a new space just big enough to hold
         * the current number of elements and move the
         * data over to it.
         */
        void shrinkToFit();

        /**
         * Reallocate a new space big enough to hold
         * the current number of elements and move the
         * data over to it.
         * Shrinks to the smallest power of 2 number >=
         * the number of elements.
         */
        void shrink();
    private:
        /// Contains information about allocated memory.
        struct MemoryInfo
        {
            /// Current maximal capacity of the data array.
            u64 capacity{0};
            /**
             * How many positions are currently used.
             * Since the data is stored contiguously, this
             * number is also the index to the next free
             * position.
             */
            u64 used{0};
            /// Memory containing the data.
            ElementT *data{nullptr};
        };

        /**
         * Reallocate the memory structure to given size.
         * Uses free[] on the data.
         * @param mem The memory structure to reallocate.
         * @param size The requested size.
         */
        void realloc(MemoryInfo &mem, u64 size);

        /**
         * Free the used memory and reset the internal
         * memory structure.
         * @param mem The memory structure to free.
         */
        void freeMem(MemoryInfo &mem);

        /**
         * Resize the memory structure to given size and
         * copy over the existing data.
         * If the size is smaller than the one
         * occupied, nothing happens.
         * @param mem The memory structure to resize.
         * @param size The requested size.
         */
        void resize(MemoryInfo &mem, u64 size);

        /**
         * Checks if there is enough space to push at least
         * given number of elements. If there is not enough
         * space, then the memory is resized.
         * @param mem The memory structure to check.
         * @param amount The number of elements.
         */
        void makeSpaceInt(MemoryInfo &mem, u64 amount);

        /**
         * Delete all elements of given memory structure.
         * Calls destructors.
         * @param mem The memory structure to work on.
         */
        void clearInt(MemoryInfo &mem);

        /**
         * Push element into given memory structure.
         * Uses copy constructor.
         * @param mem The memory structure.
         * @param element The element to push.
         */
        void pushBackInt(MemoryInfo &mem, const ElementT &element);

        /**
         * Push element into given memory structure.
         * Uses move constructor.
         * @param mem The memory structure.
         * @param element The element to push.
         */
        void pushBackInt(MemoryInfo &mem, ElementT &&element);

        /**
         * Constructs an element with given arguments in place.
         * @param mem The memory structure.
         * @param args Arguments to pass to the constructor.
         */
        template <typename... Args>
        void emplaceBackInt(MemoryInfo &mem, Args... args);

        /**
         * Remove element on given index.
         * @param mem The memory structure.
         * @param index Index of the element to remove.
         * @return Returns true, if the removal was successful.
         */
        bool removeInt(MemoryInfo &mem, u64 index);

        /**
         * Get the pointer to the raw memory.
         * @return Pointer to the raw memory.
         */
        ElementT *getMemoryPtr(MemoryInfo &mem);

        /**
         * Allocate raw memory for given number of elements.
         * @param amount Size in elements.
         * @return Pointer to the memory.
         */
        ElementT *allocRawMemory(u64 amount);

        /// Currently used memory.
        MemoryInfo mMem;

        /// Allocator for allocating element array.
        AllocatorT mAllocator;

        /// Minimal starting size of the list.
        static constexpr u64 MIN_SIZE{8};
        /// How many more elements to expect, when pushBack is called.
        static constexpr u64 PUSH_EXPECT{4};
    protected:
    };

    template <typename ET,
              typename AT>
    inline
    List<ET, AT>::List(const List<ET, AT> &other) :
        mMem{0, 0, nullptr}
    {
        copy(other);
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT>::List(List<ET, AT> &&other) :
        mMem{0, 0, nullptr}
    {
        swap(other);
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT>::~List()
    {
        clear();

        freeMem(mMem);
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT>::List(u64 startSize) :
        mMem{0, 0, nullptr}
    {
        if (startSize)
        {
            realloc(mMem, startSize > MIN_SIZE ? startSize : MIN_SIZE);
        }
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT>::List(u64 startSize, const ET &val) :
        mMem{0, 0, nullptr}
    {
        realloc(mMem, startSize > MIN_SIZE ? startSize : MIN_SIZE);

        for (u64 iii = 0; iii < mMem.capacity; ++iii)
        {
            mMem.data[iii] = val;
        }

        mMem.used = mMem.capacity;
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT> &List<ET, AT>::operator=(const List<ET, AT> &rhs)
    {
        copy(rhs);
        return *this;
    }

    template <typename ET,
              typename AT>
    inline
    List<ET, AT> &List<ET, AT>::operator=(List<ET, AT> &&rhs)
    {
        swap(rhs);
        return *this;
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::Iterator List<ET, AT>::begin()
    {
        return List<ET, AT>::Iterator(mMem.data);
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator List<ET, AT>::cbegin() const
    {
        return List<ET, AT>::ConstIterator(mMem.data);
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator List<ET, AT>::begin() const
    {
        return cbegin();
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::Iterator List<ET, AT>::end()
    {
        return List<ET, AT>::Iterator(mMem.data + mMem.used);
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator List<ET, AT>::cend() const
    {
        return List<ET, AT>::ConstIterator(mMem.data + mMem.used);
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator List<ET, AT>::end() const
    {
        return cend();
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::swap(List<ET, AT> &other)
    {
        if (this == (&other))
        {
            return;
        }

        std::swap(mAllocator, other.mAllocator);
        std::swap(mMem, other.mMem);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::pushBack(const ET &element)
    {
        pushBackInt(mMem, element);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::pushBack(ET &&element)
    {
        pushBackInt(mMem, std::forward<ET>(element));
    }

    template <typename ET,
              typename AT>
    template <typename... Args>
    inline
    void List<ET, AT>::emplaceBack(Args... args)
    {
        emplaceBackInt(mMem, args...);
    }

    template <typename ET,
              typename AT>
    bool List<ET, AT>::remove(u64 index)
    {
        return removeInt(mMem, index);
    }

    template <typename ET,
              typename AT>
    inline
    ET &List<ET, AT>::operator[](u64 index)
    {
        /// Debug check if the index is out of bounds.
        ASSERT_SLOW(index < mMem.capacity);

        return mMem.data[index];
    }

    template <typename ET,
              typename AT>
    inline
    const ET &List<ET, AT>::operator[](u64 index) const
    {
        /// Debug check if the index is out of bounds.
        ASSERT_SLOW(index < mMem.capacity);

        return mMem.data[index];
    }

    template <typename ET,
              typename AT>
    inline
    ET &List<ET, AT>::back()
    {
        return mMem.data[mMem.used - 1];
    }

    template <typename ET,
              typename AT>
    inline
    const ET &List<ET, AT>::back() const
    {
        return mMem.data[mMem.used - 1];
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::popBack()
    {
        if (empty())
        {
            return;
        }

        mAllocator.destroy(mAllocator.address(back()));

        mMem.used--;

        u64 filled{(100ull * mMem.used) / mMem.capacity};

        if (filled < 50)
        {
            shrink();
        }
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::clear()
    {
        clearInt(mMem);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::makeSpace(u64 amount)
    {
        makeSpaceInt(mMem, amount);
    }

    template <typename ET,
              typename AT>
    inline
    bool List<ET, AT>::empty() const
    {
        return mMem.used == 0;
    }

    template <typename ET,
              typename AT>
    inline
    u64 List<ET, AT>::size() const
    {
        return mMem.used;
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::copy(const List<ET, AT> &other)
    {
        if (this == &other)
        {
            return;
        }

        clear();

        mAllocator = other.mAllocator;

        realloc(mMem, other.mMem.capacity);

        for (const ET &element : other)
        {
            pushBack(element);
        }
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::memoryCopy(const List<ET, AT> &other)
    {
        if (this == &other)
        {
            return;
        }

        clear();

        realloc(mMem, other.mMem.capacity);
        mMem.used = other.mMem.used;

        std::memcpy(mMem.data, other.mMem.data, sizeof(ET) * mMem.used);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::shrinkToFit()
    {
        resize(mMem, mMem.used);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::shrink()
    {
        // Adding one in case of pop, push repeating.
        u64 newSize = math::pow2RoundUp(mMem.used + 1);
        resize(mMem, newSize);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::realloc(MemoryInfo &mem, u64 size)
    {
        //delete[] getMemoryPtr(mem);
        mAllocator.deallocate(getMemoryPtr(mem));

        //mem.data = reinterpret_cast<ET*>(new u8[sizeof(ET) * size]);
        mem.data = allocRawMemory(size);
        mem.used = 0;
        mem.capacity = size;
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::freeMem(MemoryInfo &mem)
    {
        mAllocator.deallocate(getMemoryPtr(mem));
        mem.capacity = 0;
        mem.used = 0;
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::resize(MemoryInfo &mem, u64 size)
    {
        if (mem.used <= size)
        {
            MemoryInfo newMem{0, 0, nullptr};
            realloc(newMem, size);

            try
            {
                // Move construction
                ET *begin = mem.data;
                ET *end = mem.data + mem.used;
                ET *dest = newMem.data;
                for (ET *it = begin; it != end; ++it, ++dest)
                {
                    new (dest) ET(std::move(*it));
                }
            } catch (...)
            {
                freeMem(newMem);
                throw;
            }

            newMem.used = mem.used;
            newMem.capacity = size;

            mAllocator.deallocate(getMemoryPtr(mem));
            mem = newMem;
        }
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::makeSpaceInt(MemoryInfo &mem, u64 amount)
    {
        if (mem.capacity - mem.used > amount)
        {
            return;
        }

        u64 newSize = math::pow2RoundUp(mem.used + amount);
        resize(mem, newSize);
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::clearInt(MemoryInfo &mem)
    {
        auto begin = mem.data;
        auto end = mem.data + mem.used;

        for (auto it = begin; it != end; ++it)
        {
            //it->~ET();
            mAllocator.destroy(it);
        }

        mem.used = 0;
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::pushBackInt(MemoryInfo &mem, const ET &element)
    {
        // Expect more pushBacks
        makeSpaceInt(mem, PUSH_EXPECT);

        //new (&(mem.data[mem.used])) ET(element);
        mAllocator.construct(&(mem.data[mem.used]), element);

        mem.used++;
    }

    template <typename ET,
              typename AT>
    inline
    void List<ET, AT>::pushBackInt(MemoryInfo &mem, ET &&element)
    {
        // Expect more pushBacks
        makeSpaceInt(mem, PUSH_EXPECT);

        //new (&(mem.data[mem.used])) ET(std::forward<ET>(element));
        mAllocator.construct(&(mem.data[mem.used]), std::forward<ET>(element));

        mem.used++;
    }

    template <typename ET,
              typename AT>
    template <typename... Args>
    inline
    void List<ET, AT>::emplaceBackInt(MemoryInfo &mem, Args... args)
    {
        // Expect more pushBacks
        makeSpaceInt(mem, PUSH_EXPECT);

        //new (&(mem.data[mem.used])) ET(args...);
        mAllocator.construct(&(mem.data[mem.used]), std::forward<Args>(args)...);

        mem.used++;
    }

    template <typename ET,
              typename AT>
    inline
    bool List<ET, AT>::removeInt(MemoryInfo &mem, u64 index)
    {
        if (index >= mem.used)
        { // Index is out of bounds.
            return false;
        }

        // Move the elements
        std::move(mem.data + index + 1,
                  mem.data + mem.used,
                  mem.data + index);

        // Destruct the last element.
        mAllocator.destroy(mem.data + mem.used - 1);

        mem.used--;

        return true;
    }

    template <typename ET,
              typename AT>
    inline
    ET *List<ET, AT>::getMemoryPtr(MemoryInfo &mem)
    {
        return mem.data;
    }

    template <typename ET,
              typename AT>
    inline
    ET *List<ET, AT>::allocRawMemory(u64 amount)
    {
        //return reinterpret_cast<ET*>(new u8[sizeof(ET) * amount]);
        //return static_cast<ET*>(operator new[](sizeof(ET) * amount));
        return mAllocator.allocate(amount);
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::Iterator begin(List<ET, AT> &list)
    {
        return list.begin();
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator begin(const List<ET, AT> &list)
    {
        return list.cbegin();
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::Iterator end(List<ET, AT> &list)
    {
        return list.end();
    }

    template <typename ET,
              typename AT>
    inline
    typename List<ET, AT>::ConstIterator end(const List<ET, AT> &list)
    {
        return list.cend();
    }
}

#endif //MEM_LIST_H
