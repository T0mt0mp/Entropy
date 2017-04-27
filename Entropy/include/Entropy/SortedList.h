/**
 * @file Entropy/SortedList.h
 * @author Tomas Polasek
 * @brief Sorted List implemented using normal List.
 */

#ifndef ECS_FIT_SORTEDLIST_H
#define ECS_FIT_SORTEDLIST_H

#include "Types.h"
#include "Util.h"
#include "List.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * List, which is always in a sorted state.
     * @tparam T Type of the inner element.
     * @tparam Allocator Type of the allocator.
     */
    template <typename T,
              typename Compare = std::less<T>,
              typename Allocator = std::allocator<T>>
    class SortedList
    {
    public:
        using ListT = List<T, Allocator>;

        using value_type = typename ListT::value_type;
        using const_value_type = typename ListT::const_value_type;
        using allocator_type = typename ListT::allocator_type ;
        using size_type = typename ListT::size_type;
        using difference_type = typename ListT::difference_type ;
        using reference = typename ListT::reference ;
        using const_reference = typename ListT::const_reference ;
        using pointer = typename ListT::pointer;
        using const_pointer = typename ListT::const_pointer ;
        using iterator = typename ListT::iterator;
        using const_iterator = typename ListT::const_iterator ;
        using reverse_iterator = typename ListT::reverse_iterator ;
        using const_reverse_iterator = typename ListT::const_reverse_iterator ;

        iterator begin()
        { return mList.begin(); }
        iterator end()
        { return mList.end(); }
        const_iterator begin() const
        { return mList.begin(); }
        const_iterator cbegin() const
        { return mList.cbegin(); }
        const_iterator end() const
        { return mList.end(); }
        const_iterator cend() const
        { return mList.cend(); }

        SortedList(Compare cmp = {}, Allocator alloc = {});
        template <typename InputIt>
        SortedList(InputIt first, InputIt last,
                   Compare cmp = {}, Allocator alloc = {});

        SortedList(const SortedList &other);
        SortedList(SortedList &&other);
        SortedList(const ListT &other, Compare = {});
        SortedList(ListT &&other, Compare = {});

        SortedList &operator=(const SortedList &other);
        SortedList &operator=(SortedList &&other);

        /// Insert element into the sorted list.
        inline void insert(const_reference val);
        /// Insert element into the sorted list, if the element is already present, nothing happens.
        inline iterator insertUnique(const_reference val);
        /**
         * Insert element into the sorted list, if the element is already present, nothing happens.
         * New element is constructed only if there are no matching.
         * @tparam SearchT Type of the search value.
         * @tparam CArgTs Type of the constructor arguments.
         * @param search Searched key.
         * @param cArgs Constructor arguments.
         * @return Returns iterator to the element.
         */
        template <typename SearchT,
                  typename... CArgTs>
        inline iterator insertUnique(const SearchT &search, CArgTs... cArgs);
        /**
         * Insert element into the sorted list, if the element is already
         * present, it will be overwritten.
         * !Does NOT destruct the object first, if it is being overwritten!
         * @tparam SearchT Type of the search value.
         * @tparam CArgTs Type of the constructor arguments.
         * @param search Searched key.
         * @param cArgs Constructor arguments.
         * @return Returns iterator to the element.
         */
        template <typename SearchT,
                  typename... CArgTs>
        inline iterator replaceUnique(const SearchT &search, CArgTs... cArgs);

        /**
         * Find element in the List.
         * @tparam SearchT Type of the search key.
         * @param val Value of the search key.
         * @return Returns iterator to the element, or
         *   end(), if the element has not been found.
         */
        template <typename SearchT>
        inline iterator find(const SearchT &val);

        /**
         * Find element in the List.
         * @tparam SearchT Type of the search key.
         * @param val Value of the search key.
         * @return Returns iterator to the element, or
         *   end(), if the element has not been found.
         */
        template <typename SearchT>
        inline const_iterator find(const SearchT &val) const;

        /**
         * Erase element from the List.
         * If the element is not found, nothing happens.
         * @tparam SearchT Type of the search key.
         * @param val Value of the search key.
         */
        template <typename SearchT>
        inline void erase(const SearchT &val);

        inline void swap(SortedList &other);

        /// Explicitly sort the list.
        inline void sort();

        auto back()
        { return mList.back(); }

        void popBack()
        { mList.popBack(); }

        void clear()
        { return mList.clear(); }

        void reclaim()
        { return mList.reclaim(); }

        size_type size() const
        { return mList.size(); }

        size_type capacity() const
        { return mList.capacity(); }

        reference at(size_type pos)
        { return mList.at(pos); }
        const_reference at(size_type pos) const
        { return mList.at(pos); }

        reference operator[](size_type pos)
        { return mList[pos]; }
        const_reference operator[](size_type pos) const
        { return mList[pos]; }

        void resize(u64 size)
        { mList.resize(size); }

        /// Create a new list, with the same data.
        ListT toList() const
        { return ListT(mList); }
        /// Move inner list to a new list, destroys the SortedList contents.
        ListT toListDestructive()
        { return ListT(std::move(mList)); }

        /// Copy data from another List and sort it.
        void fromList(const ListT &list)
        { mList = list; sort(); }
        /// Move data from another List and sort it.
        void fromList(ListT &&list)
        { mList = std::move(list); sort(); }

        /// Copy data from another List, WITHOUT sorting it!
        void fromSortedList(const ListT &list)
        { mList = list; }
        /// Move data from another List, WITHOUT sorting it!
        void fromSortedList(ListT &&list)
        { mList = std::move(list); }
    private:
        /// Copy data from another SortedList.
        inline void copy(const SortedList &other);

        /// Comparator object.
        Compare mCmp;
        /// Raw List.
        List<T, Allocator> mList;
    protected:
    }; // class SortedList
} // namespace ent

#include "SortedList.inl"

#endif //ECS_FIT_SORTEDLIST_H
