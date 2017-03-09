/**
 * @file Entropy/List.h
 * @author Tomas Polasek
 * @brief Simple List without element construction.
 */

#ifndef ECS_FIT_LIST_H
#define ECS_FIT_LIST_H

#include <memory>
#include <iterator>
#include <cstring>
#include <algorithm>

#include "Types.h"
#include "Util.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Simple List implementation :
     *  Constructors are NOT called automatically on elements.
     *  Destructors are NOT called automatically on elements.
     * @tparam T Type of the element.
     * @tparam Allocator Type of the allocator used.
     */
    template <typename T,
        typename Allocator = std::allocator<T>>
    class List final
    {
    private:
    public:
        /*
        static_assert(std::is_pod<T>::value,
                      "List is only safe for POD, since data is moved using std::memcpy! "
                      "(If you are sure, define UNSAFE_LIST macro)");
        static_assert(std::is_trivially_destructible<T>::value,
                      "Destructors are NOT called, when using List! "
                      "(If you are sure, define UNSAFE_LIST macro)");
        static_assert(std::is_trivially_constructible<T>::value,
                      "Constructors are NOT called, when using List! "
                      "(If you are sure, define UNSAFE_LIST macro)");
                      */

        using value_type = T;
        using const_value_type = const T;
        using allocator_type = Allocator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = pointer;
        using const_reverse_iterator = const_pointer;
        /*
        using iterator = std::iterator<std::random_access_iterator_tag, value_type>;
        using const_iterator = std::iterator<std::random_access_iterator_tag, const_value_type>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
         */

        /// Default constructor, creates empty List.
        explicit List() : List(Allocator{}) {};

        /// Default constructor, creates empty List.
        explicit List(const Allocator &alloc) : mAllocator{alloc} {};

        /**
         * Create uninitialized list with given number of elements.
         * @param count Number of elements.
         * @param alloc Allocator instance.
         */
        inline explicit List(size_type count,
                             const Allocator &alloc = {});

        /**
         * Create list with given number of elements, copied
         * from given value.
         * @param count Number of elements.
         * @param value Copied value.
         * @param alloc Allocator instance.
         */
        inline explicit List(size_type count,
                             const_reference value,
                             const Allocator &alloc = {});

        /**
         * Initialize the List by copying values from
         * iterated sequence.
         * @tparam InputIt Iterator type.
         * @param first Starting iterator.
         * @param last End iterator.
         * @param alloc Allocator instance.
         */
        template <typename InputIt,
            typename = typename std::enable_if<
                std::is_convertible<
                    typename std::iterator_traits<InputIt>::iterator_category,
                    std::input_iterator_tag>
                ::value>
            ::type>
        inline List(InputIt first, InputIt last,
                    const Allocator &alloc = {});

        /**
         * Specify elements using initializer list.
         * @param init List of elements.
         * @param alloc Allocator instance.
         */
        inline List(std::initializer_list<value_type> init,
                    const Allocator &alloc = {});

        /// Copy constructor.
        inline List(const List &other);
        /// Copy constructor.
        inline List(const List &other, const Allocator &alloc);

        /// Move constructor.
        inline List(List &&other);
        /// Move constructor.
        inline List(List &&other, const Allocator &alloc);

        /// Destructor clears memory.
        inline ~List();

        /// Copy-assign operator.
        List &operator=(const List &rhs);

        /// Copy-move operator.
        List &operator=(List &&rhs);

        /**
         * Get pointer to the beginning of the data.
         * @return Pointer to the beginning of the data.
         */
        pointer data()
        { return beginPtr(); }
        const_pointer data() const
        { return beginPtr(); }

        /**
         * Get the first element in the List.
         * Undefined behaviour on empty List.
         * @return Reference to the first element.
         */
        reference front()
        { return *beginPtr(); }
        const_reference front() const
        { return *beginPtr(); }

        /**
         * Get the begin iterator.
         * @return Returns the begin iterator.
         */
        iterator begin()
        { return beginPtr(); }
        const_iterator begin() const
        { return beginPtr(); }
        const_iterator cbegin() const
        { return beginPtr(); }

        /**
         * Get the end iterator.
         * @return Returns the end iterator.
         */
        iterator end()
        { return useEndPtr(); }
        const_iterator end() const
        { return useEndPtr(); }
        const_iterator cend() const
        { return useEndPtr(); }

        /**
         * Get the last element in the List.
         * Undefined behaviour on empty List.
         * @return Reference to the last element.
         */
        reference back()
        { return *(useEndPtr() - 1); }
        const_reference back() const
        { return *(useEndPtr() - 1); }

        /**
         * Get the element on given position with
         * bounds checking.
         * @param pos Position of the element.
         * @return Returns reference to the element.
         */
        inline reference at(size_type pos);
        inline const_reference at(size_type pos) const;

        /**
         * Get element on given position without bounds checking.
         * @param pos Position of the element.
         * @return Returns reference to the element.
         */
        reference operator[](size_type pos)
        { return beginPtr()[pos]; }
        const_reference operator[](size_type pos) const
        { return beginPtr()[pos]; }

        /**
         * Get size of the List.
         * @return Number of elements in use.
         */
        size_type size() const
        { return mInUse; }

        /**
         * Get size of the allocated array.
         * @return Size of the allocated array.
         */
        size_type capacity() const
        { return mAllocated; }

        /// Get the allocator in use by this List.
        Allocator getAllocator() const
        { return mAllocator; }

        /**
         * Reserve space for given number of elements in this List.
         * If the newCapacity is lower than current size, this
         * method does nothing.
         * @param newCapacity Requested new capacity.
         */
        inline void reserve(size_type newCapacity);

        /**
         * Resize the List to given size.
         * @param size New size of the List.
         */
        inline void resize(size_type size);

        /**
         * Shrink this List to find the number of used elements.
         */
        inline void shrinkToFit();

        /**
         * Push given element to the back of the List.
         * Resizing may occur.
         * Element will NOT be copy or move
         * constructed, std::memcpy will be used!
         * @param val Value to push back.
         */
        inline void pushBack(const value_type &val = {});
        inline void pushBack(value_type &&val)
        { pushBack(val); }

        /**
         * Pop the element from the back of the List.
         * Destructor will NOT be called!
         * Behavior is undefined, if the List is empty.
         */
        inline void popBack();

        /**
         * Set element on given position to value.
         * Memory is copied using std::memcpy.
         * @param pos Position of the element.
         * @param val Value to set the element to.
         */
        inline void set(size_type pos, const value_type &val = {})
        { setImpl(beginPtr() + pos, val); }

        /**
         * Set element on given position to value.
         * Memory is copied using std::memcpy.
         * @param pos Position of the element.
         * @param val Value to set the element to.
         */
        inline void set(iterator pos, const value_type &val = {})
        { setImpl(pos, val); }

        /**
         * Insert the value between begin iterator to end iterator.
         * @param beg Begin iterator.
         * @param num Number of elements to insert.
         * @param val Value to insert
         */
        void insert(iterator beg, size_type num, const T &val)
        { insertImpl(beg - begin(), num, val); }

        /**
         * Insert values from other List.
         * @param pos Where to insert.
         * @param beg Input iterator.
         * @param end Input end iterator.
         */
        void insert(iterator pos, iterator beg, iterator end)
        { insertImpl(pos - begin(), beg, end); }

        /**
         * Insert element at given position.
         * @param pos Position.
         * @param val Element value.
         */
        inline void insert(iterator pos, const value_type &val = {})
        { insertImpl(pos - begin(), val); }

        /**
         * Erase the element on given position.
         * Invalidates any iterators at, or after this point.
         * @param pos Position to erase.
         */
        inline void erase(size_type pos)
        { eraseImpl(beginPtr() + pos); }

        /**
         * Erase the element on given position.
         * Invalidates any iterators at, or after this point.
         * @param pos Position to erase.
         */
        inline void erase(iterator pos)
        { eraseImpl(pos); }

        /**
         * Erase the elements in given sequence.
         * @param beg Beginning iterator.
         * @param end End iterator.
         */
        inline void erase(iterator beg, iterator end)
        { eraseImpl(beg, end); }

        /**
         * Set the number of elements in use to 0.
         * Does NOT deallocate.
         */
        inline void clear() noexcept
        { mInUse = 0; }

        /**
         * Reset the vector to default state and deallocate.
         */
        inline void reclaim()
        { reset(); }

        /**
         * Swap values with the other List.
         * @param other The swapped List.
         */
        inline void swap(List &other);

        template <typename LT, typename AT>
        friend iterator begin(List<LT, AT> &list);
        template <typename LT, typename AT>
        friend const_iterator cbegin(List<LT, AT> &list);
        template <typename LT, typename AT>
        friend iterator end(List<LT, AT> &list);
        template <typename LT, typename AT>
        friend const_iterator cend(List<LT, AT> &list);
    private:
        /**
         * Reserve space for given number of elements in this List.
         * If the newCapacity is lower than current size, this
         * method does nothing.
         * @param newCapacity Requested new capacity.
         */
        void reserveImpl(size_type newCapacity);

        /**
         * Copy values from given List to this one.
         * @param other The copied List.
         */
        inline void copy(const List &other);

        /**
         * Initialize the array to given size.
         * @param size Requested size of the array.
         */
        inline void initSize(size_type size);

        /**
         * Set element on given position to the value.
         * @param it Position, where to set.
         * @param val Value to set.
         */
        inline void setImpl(iterator it, const T &val);

        /**
         * Set all elements in this range to given value.
         * @param beg Where to begin.
         * @param end One behind the end.
         * @param val Value to set.
         */
        inline void setImpl(iterator beg, iterator end, const T &val);

        /**
         * Insert element at given position.
         * @param pos Position, where to insert the element.
         * @param val Value of the element.
         */
        inline void insertImpl(size_type pos, const T &val);

        /**
         * Insert elements to the List.
         * @param beg Beginning position.
         * @param num Number of elements.
         * @param val Value to insert.
         */
        inline void insertImpl(size_type beg, size_type num, const T &val);

        /**
         * Insert values from other List.
         * @param pos Position, where to insert.
         * @param beg Beginning iterator.
         * @param end End iterator.
         */
        inline void insertImpl(size_type pos, iterator beg, iterator end);

        /**
         * Erase element on given position and move the List
         * into contiguous array.
         * @param pos Position to erase.
         */
        inline void eraseImpl(iterator pos);

        /**
         * Erase elements in given range and move the List
         * into contiguous array.
         * @param beg Beginning iterator.
         * @param end End iterator.
         */
        inline void eraseImpl(iterator beg, iterator end);

        /**
         * Push given element to the back of the List.
         * Does NOT check for enough space!
         * constructed, std::memcpy will be used!
         * @param val Value to push back.
         */
        inline void pushBackImpl(const value_type &val);

        /**
         * Copy range between iterators into this List.
         * Version for iterators, which can be iterated over only once.
         * @tparam InputIt Type of the interator.
         * @param first Starting iterator.
         * @param last End iterator.
         */
        template <typename InputIt>
        inline void rangeCopy(InputIt first, InputIt last, std::input_iterator_tag);

        /**
         * Copy range between iterators into this List.
         * Version for iterators, which can be iterated over multiple times.
         * @tparam InputIt Type of the interator.
         * @param first Starting iterator.
         * @param last End iterator.
         */
        template <typename InputIt>
        inline void rangeCopy(InputIt first, InputIt last, std::forward_iterator_tag);

        /**
         * Get pointer to the beginning of allocated data.
         * @return Pointer to the start of allocated data.
         */
        inline pointer beginPtr()
        { return mData; }
        inline const_pointer beginPtr() const
        { return mData; }

        /**
         * Get pointer to one behind the end of the data in use.
         * @return Pointer to one behind the end of the data in use.
         */
        inline pointer useEndPtr()
        { return beginPtr() + size(); }
        inline const_pointer useEndPtr() const
        { return beginPtr() + size(); }

        /**
         * Get pointer to one behind the end of the allocated data.
         * @return Pointer to one behind the end of the allocated data.
         */
        inline pointer endPtr()
        { return beginPtr() + capacity(); }
        inline const_pointer endPtr() const
        { return beginPtr() + capacity(); }

        /**
         * Allocate array of given size, using the active allocator.
         * @param size Size of the allocated array.
         * @return Returns pointer to the allocated array.
         */
        inline pointer allocate(size_type size);

        /**
         * Deallocate given array, using the active allocator.
         * @param arr Array to deallocate.
         * @param size Size of the array.
         */
        inline void deallocate(pointer arr, size_type size);

        /**
         * Copy elements from the 'from' array to the 'to' array.
         * 'size' elements will be copied.
         * Both array MUST be at least 'size' large.
         * @param from Data will be copied from this array.
         * @param to Data will be copied to this array.
         * @param size Number of elements copied.
         */
        inline void copyData(pointer from, pointer to, size_type size) noexcept;

        /**
         * Copy elements from the 'from' array to the 'to' array.
         * 'size' elements will be copied.
         * Both array MUST be at least 'size' large.
         * Copies data from end to start.
         * @param from Data will be copied from this array.
         * @param to Data will be copied to this array.
         * @param size Number of elements copied.
         */
        inline void rCopyData(pointer from, pointer to, size_type size) noexcept;

        /**
         * Reset the List to default state.
         */
        inline void reset();

        /// Allocated data array.
        pointer mData{nullptr};
        /// Size of the allocated data array.
        size_type mAllocated{0};
        /// Number of elements in use.
        size_type mInUse{0};
        /// Allocator used by this List.
        Allocator mAllocator;
    protected:
    };

    // List implementation.
    template <typename T, 
		typename Allocator>
    List<T, Allocator>::template List(size_type count, const Allocator &alloc) :
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
        if (mInUse < size)
        {
            reserve(size);
            mInUse = size;
        }
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
    void List<T, Allocator>::insertImpl(size_type pos, const T &val)
    {
        reserve(size() + 1);
        iterator it{begin() + pos};
        if (size())
        {
            rCopyData(it, it + 1, end() - it);
        }
        setImpl(it, val);
        mInUse++;
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
        if (size() == capacity())
        {
            reserveImpl(pow2RoundUp(size() + 1));
        }

        pushBackImpl(val);
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

#endif //ECS_FIT_LIST_H
