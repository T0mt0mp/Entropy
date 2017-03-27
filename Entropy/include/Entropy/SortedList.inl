/**
 * @file Entropy/SortedList.inl
 * @author Tomas Polasek
 * @brief Sorted List implemented using normal List.
 */

#include "SortedList.h"

/// Main Entropy namespace
namespace ent
{
    // SortedList implementation.
    template <typename T, typename C, typename A>
    SortedList<T, C, A>::SortedList(C cmp, A alloc) :
        mCmp{cmp}, mList(alloc)
    {

    }

    template <typename T, typename C, typename A>
    template <typename InputIt>
    SortedList<T, C, A>::SortedList(InputIt first, InputIt last,
                                    C cmp, A alloc) :
        mCmp{cmp}, mList(first, last, alloc)
    {
        sort();
    }

    template <typename T, typename C, typename A>
    SortedList<T, C, A>::SortedList(const SortedList &other)
    {
        copy(other);
    }

    template <typename T, typename C, typename A>
    SortedList<T, C, A>::SortedList(SortedList &&other)
    {
        swap(other);
    }

    template <typename T, typename C, typename A>
    SortedList<T, C, A>::SortedList(const ListT &other, C cmp) :
        mCmp{cmp}, mList(other)
    {
        sort();
    }

    template <typename T, typename C, typename A>
    SortedList<T, C, A>::SortedList(ListT &&other, C cmp) :
        mCmp{cmp}, mList(std::forward(other))
    {
        sort();
    }

    template <typename T, typename C, typename A>
    auto SortedList<T, C, A>::operator=(const SortedList &other) -> SortedList&
    {
        copy(other);
        return *this;
    }

    template <typename T, typename C, typename A>
    auto SortedList<T, C, A>::operator=(SortedList &&other) -> SortedList&
    {
        swap(other);
        return *this;
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::insert(const_reference val)
    {
        iterator findIt{std::lower_bound(begin(), end(), val, mCmp)};
        if (findIt == end() || mCmp(val, *findIt))
        { // Not found.
            mList.insert(findIt, val);
        }
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::insertUnique(const_reference val)
    {
        iterator findIt{std::lower_bound(begin(), end(), val, mCmp)};
        if (findIt == end() || mCmp(val, *findIt))
        { // Not found.
            mList.insert(findIt, val);
        }
    }

    template <typename T, typename C, typename A>
    auto SortedList<T, C, A>::find(const_reference val) const -> const_iterator
    {
        iterator findIt{std::lower_bound(begin(), end(), val, mCmp)};
        return (findIt == end() || mCmp(val, *findIt)) ? end() : findIt;
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::erase(const_reference val)
    {
        iterator findIt{std::lower_bound(begin(), end(), val, mCmp)};
        if (findIt != end() && !mCmp(val, *findIt))
        { // Found.
            mList.erase(findIt);
        }
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::swap(SortedList &&other)
    {
        std::swap(mCmp, other.mCmp);
        std::swap(mList, other.mList);
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::sort()
    {
        std::sort(begin(), end(), mCmp);
    }

    template <typename T, typename C, typename A>
    void SortedList<T, C, A>::copy(const SortedList &other)
    {
        mCmp = other.mCmp;
        mList = other.mList;
    }
    // SortedList implementation end.
} // namespace ent
