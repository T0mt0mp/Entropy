/**
 * @file Entropy/GroupManager.inl
 * @author Tomas Polasek
 * @brief Manager class allowing the use of EntityGroups.
 */

#include "List.h"

/// Main Entropy namespace
namespace ent
{
    // List implementation.
    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(size_type count, const Allocator &alloc) :
        mAllocator{alloc}
    {
        initSize(count);
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(size_type count, const_reference value, const Allocator &alloc) :
        mAllocator{alloc}
    {
        initSize(count);
        setImpl(beginPtr(), useEndPtr(), value);
    }

    template <typename T,
        typename Allocator>
    template<typename InputIt,
        typename>
    List<T, Allocator>::List(InputIt first, InputIt last, const Allocator &alloc) :
        mAllocator{alloc}
    {
        rangeCopy(first, last, typename std::iterator_traits<InputIt>::iterator_category());
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(std::initializer_list<value_type> init, const Allocator &alloc) :
        List(init.begin(), init.end(), alloc)
    { }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(const List &other) :
        mAllocator{std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.getAllocator())}
    {
        copy(other);
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(const List &other, const Allocator &alloc) :
        mAllocator{alloc}
    {
        copy(other);
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(List &&other) :
        mAllocator{std::move(other.mAllocator)}
    {
        swap(other);
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::List(List &&other, const Allocator &alloc) :
        mAllocator{alloc}
    {
        swap(other);
    }

    template <typename T,
        typename Allocator>
    List<T, Allocator>::~List()
    {
        reset();
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::operator=(const List &rhs) -> List&
    {
        if (&rhs != this)
        {
            allocator_type newAllocator{mAllocator};

            // TODO - cleanup.
            if (capacity() < rhs.size())
            {
                pointer newMem{nullptr};
                if (typename std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
                {
                    newAllocator = rhs.getAllocator();
                    newMem = newAllocator.allocate(rhs.size());
                }
                else
                {
                    newMem = mAllocator.allocate(rhs.size());
                }

                deallocate(mData, mAllocated);
                mData = newMem;
                mAllocated = rhs.size();
                mInUse = 0;
            }

            copyData(rhs.mData, mData, rhs.size());

            mInUse = rhs.size();

            if (newAllocator != mAllocator)
            {
                mAllocator = newAllocator;
            }
        }

        return *this;
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::operator=(List &&rhs) -> List&
    {
        if (&rhs != this)
        {
            if (typename std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
            {
                mAllocator = rhs.getAllocator();
                swap(rhs);
            }
            else if (mAllocator == rhs.getAllocator())
            {
                swap(rhs);
            }
            else
            {
                copy(rhs);
            }
        }

        return *this;
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::at(size_type pos) -> reference
    {
        if (pos >= size())
        {
            throw std::out_of_range("Position is out of range of this List!");
        }

        return operator[](pos);
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::at(size_type pos) const -> const_reference
    {
        if (!(pos < size()))
        {
            throw std::out_of_range("Position is out of range of this List!");
        }

        return operator[](pos);
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::reserve(size_type newCapacity)
    {
        if (capacity() < newCapacity)
        {
            reserveImpl(pow2RoundUp(newCapacity));
        }
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::resize(size_type size)
    {
        reserve(size);
        mInUse = size;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::resize(size_type newSize, const_reference val)
    {
        reserve(newSize);

        if (newSize <= size())
        { // No new elements.
            mInUse = newSize;
            return;
        }

        size_type newElements{newSize - size()};
        setImpl(useEndPtr(), useEndPtr() + newElements, val);

        mInUse = newSize;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::shrinkToFit()
    {
        if (size() == 0)
        {
            reset();
        }
        else if (capacity() > size())
        {
            reserveImpl(size());
        }
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::reserveImpl(size_type newCapacity)
    {
        if (newCapacity >= size())
        {
            pointer newMemory{allocate(newCapacity)};
            if (mData)
            {
                try
                {
                    copyData(beginPtr(), newMemory, size());
                } catch (...)
                {
                    deallocate(newMemory, newCapacity);
                    throw;
                }

                deallocate(mData, capacity());
            }

            mData = newMemory;
            mAllocated = newCapacity;
        }
    }

    template <typename T,
              typename Allocator>
    void List<T, Allocator>::reserveOne()
    {
        if (size() == capacity())
        {
            reserveImpl(pow2RoundUp(size() + 1u));
        }
    }

    template <typename T,
              typename Allocator>
    void List<T, Allocator>::copy(const List &other)
    {
        reserveImpl(other.size());

        copyData(other.mData, mData, other.size());

        mInUse = other.size();
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::swap(List &other)
    {
        std::swap(mData, other.mData);
        std::swap(mAllocated, other.mAllocated);
        std::swap(mInUse, other.mInUse);
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::initSize(size_type size)
    {
        if (size)
        {
            mData = allocate(size);
            mAllocated = size;
            mInUse = size;
        }
    }

    template <typename T,
              typename Allocator>
    template <typename... CArgTs>
    void List<T, Allocator>::constructImpl(iterator it, CArgTs... cArgs)
    {
        new (it) T(std::forward<CArgTs>(cArgs)...);
    }

    template <typename T,
              typename Allocator>
    void List<T, Allocator>::setImpl(iterator it, const T &val)
    {
        std::memcpy(it, &val, sizeof(T));
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::setImpl(iterator it, iterator end, const T &val)
    {
        for (;it != end; ++it)
        {
            setImpl(it, val);
        }
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::insertImpl(size_type pos, const T &val) -> iterator
    {
        reserve(size() + 1);
        iterator it{begin() + pos};
        if (size())
        {
            rCopyData(it, it + 1, end() - it);
        }
        setImpl(it, val);
        mInUse++;

        return it;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::insertImpl(size_type beg, size_type num, const T &val)
    {
        reserve(size() + num);
        iterator bIt{begin() + beg};
        iterator eIt{begin() + num};
        if (size())
        {
            rCopyData(bIt, eIt, end() - bIt);
        }
        setImpl(bIt, eIt, val);
        mInUse += num;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::insertImpl(size_type pos, iterator beg, iterator end)
    {
        difference_type length{end - beg};
        ENT_ASSERT_SLOW(length > 0);
        reserve(size() + length);
        iterator it{begin() + pos};
        if (size())
        {
            rCopyData(it, it + length, List::end() - it);
        }
        copyData(beg, it, static_cast<size_type>(length));
        mInUse += length;
    }

    template <typename T,
              typename Allocator>
    template <typename... CArgTs>
    auto List<T, Allocator>::emplaceImpl(size_type pos, CArgTs... cArgs) -> iterator
    {
        reserve(size() + 1);
        iterator it{begin() + pos};
        if (size())
        {
            rCopyData(it, it + 1, end() - it);
        }
        constructImpl(it, std::forward<CArgTs>(cArgs)...);
        mInUse++;

        return it;
    }

    template <typename T,
        typename Allocator>
    template <typename InputIt>
    void List<T, Allocator>::rangeCopy(InputIt first, InputIt last, std::input_iterator_tag)
    {
        for (; first != last; ++first)
        {
            pushBack(*first);
        }
    }

    template <typename T,
        typename Allocator>
    template <typename InputIt>
    void List<T, Allocator>::rangeCopy(InputIt first, InputIt last, std::forward_iterator_tag)
    {
        size_type toCopy{static_cast<size_type>(std::distance(first, last))};

        reserve(size() + toCopy);

        for (;first != last; ++first)
        {
            pushBackImpl(*first);
        }
    }

    template <typename T,
        typename Allocator>
    auto List<T, Allocator>::allocate(size_type size) -> pointer
    {
        if (size)
        {
            return mAllocator.allocate(size);
        }

        return nullptr;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::deallocate(pointer arr, size_type size)
    {
        mAllocator.deallocate(arr, size);
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::copyData(pointer from, pointer to, size_type size) noexcept
    {
        std::memcpy(to, from, size * sizeof(T));
    }

    template <typename T,
              typename Allocator>
    void List<T, Allocator>::rCopyData(pointer from, pointer to, size_type size) noexcept
    {
        std::memmove(to, from, size * sizeof(T));
    }

    template <typename T,
              typename Allocator>
    void List<T, Allocator>::pushBack(const value_type &val)
    {
        reserveOne();
        pushBackImpl(val);
    }

    template <typename T,
              typename Allocator>
    template <typename... CArgTs>
    void List<T, Allocator>::emplaceBack(CArgTs... cArgs)
    {
        reserveOne();
        emplaceBackImpl(std::forward<CArgTs>(cArgs)...);
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::pushBackImpl(const value_type &val)
    {
        setImpl(useEndPtr(), val);
        ++mInUse;
    }

    template <typename T,
              typename Allocator>
    template <typename... CArgTs>
    void List<T, Allocator>::emplaceBackImpl(CArgTs... cArgs)
    {
        constructImpl(useEndPtr(), std::forward<CArgTs>(cArgs)...);
        ++mInUse;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::popBack()
    {
        --mInUse;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::reset()
    {
        if (mData)
        {
            deallocate(mData, mAllocated);
        }

        mData = nullptr;
        mAllocated = 0;
        mInUse = 0;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::eraseImpl(iterator pos)
    {
        copyData(pos + 1, pos, useEndPtr() - pos);
        mInUse--;
    }

    template <typename T,
        typename Allocator>
    void List<T, Allocator>::eraseImpl(iterator beg, iterator end)
    {
        copyData(end, beg, useEndPtr() - end);
        mInUse -= end - beg;
    }
    // List implementation end.
} // namespace ent

// TODO - remove?
namespace std
{
    template <typename LT, typename LA>
    decltype(ent::List<LT, LA>::iterator) begin(ent::List<LT, LA> &list)
    { return list.begin(); }
    template <typename LT, typename LA>
    decltype(ent::List<LT, LA>::const_iterator) cbegin(const ent::List<LT, LA> &list)
    { return list.cbegin(); }
    template <typename LT, typename LA>
    decltype(ent::List<LT, LA>::iterator) end(ent::List<LT, LA> &list)
    { return list.end(); }
    template <typename LT, typename LA>
    decltype(ent::List<LT, LA>::const_iterator) cend(const ent::List<LT, LA> &list)
    { return list.cend(); }
} // namespace std
