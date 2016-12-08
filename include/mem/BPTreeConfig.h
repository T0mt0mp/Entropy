/**
 * @file mem/BPTreeConfig.h
 * @author Tomas Polasek
 * @brief Configuration for the BPTree.
 */

#ifndef MEM_BPTREECONFIG_H
#define MEM_BPTREECONFIG_H

#include "util/Types.h"
#include "math/Math.h"
#include "util/Meta.h"
#include "mem/DefaultAllocator.h"
#include "BPTree.h"

namespace mem
{
    template <typename Config>
    class BPTree;

#ifndef NDEBUG
#   define BPTreeDebug
#endif

#ifdef BPTreeDebug
#define BPTreeAssert(COND_EXP) ASSERT_SLOW(COND_EXP)
#else
#define BPTreeAssert(COND_EXP)
#endif

    /**
     * Template configurator (traits) for the B+ tree.
     * @param DataKeyT Type of the key.
     * @param ElementT Type of the stored data.
     * @param AllocatorT Type of the allocator.
     * @param CACHE_LINE_SIZE Size of a single cache line in bytes.
     */
    template <
        typename DataKeyT,
        typename ElementT,
        typename CompareT = std::less<DataKeyT>,
        typename AllocatorT = mem::DefaultAllocator<ElementT>,
        u16 CACHE_LINE_SIZE = 64,
        u16 MANUAL_NODE_BRANCHING_FACTOR = 0,
        u16 MANUAL_LEAF_BRANCHING_FACTOR = 0>
    class BPTreeConfig
    {
    public:
        using ET = ElementT;
        using KT = DataKeyT;
        using AT = AllocatorT;

        using SlotsUsedT = u16;
        using OffsetT = u16;
        using LeafDataT = ET;

        /**
         * Compare given keys.
         * @param first The first key.
         * @param second The second key.
         * @return Value returned by the comparator
         */
        static bool CmpKeys(const KT &first, const KT &second)
        {
            return CompareT()(first, second);
        }

        static constexpr u16 MAX_NODE_SIZE{2 * CACHE_LINE_SIZE};
        /**
         * Size of the node should be <= MAX_NODE_SIZE :
         *
         * sizeof(childGroup) + sizeof(slotsUsed) +
         * N * sizeof(key) =
         * MAX_NODE_SIZE
         *
         *      MAX_NODE_SIZE - sizeof(childGroup) -  sizeof(slotsUsed)
         * N =  -------------------------------------------------------
         *                                sizeof(key)
         */
        static constexpr u16 NODE_MIN_BRANCHING_FACTOR{8};
        static constexpr u16 NODE_BRANCHING_FACTOR_CALC{
            (MAX_NODE_SIZE - sizeof(void *) - sizeof(SlotsUsedT)) /
            (sizeof(KT))
        };
        static constexpr u16 NODE_BRANCHING_FACTOR{
            MANUAL_NODE_BRANCHING_FACTOR ? MANUAL_NODE_BRANCHING_FACTOR :
            math::max<u16>(NODE_MIN_BRANCHING_FACTOR, NODE_BRANCHING_FACTOR_CALC)
        };

        static constexpr u16 MAX_LEAF_SIZE{2 * CACHE_LINE_SIZE};
        /**
         * Size of the leaf should be <= MAX_LEAF_SIZE :
         *
         * sizeof(slotsUsed) + 2 * sizeof(void*) +
         * M * sizeof(key) + M * sizeof(leafData) =
         * MAX_LEAF_SIZE
         *      MAX_LEAF_SIZE - sizeof(slotsUsed) - 2 * sizeof(void*)
         * M =  -----------------------------------------------------
         *        sizeof(key) + sizeof(leafData)
         */
        static constexpr u16 LEAF_MIN_BRANCHING_FACTOR{8};
        static constexpr u16 LEAF_BRANCHING_FACTOR_CALC{
            (MAX_LEAF_SIZE - sizeof(SlotsUsedT) - 2 * sizeof(void*)) /
            (sizeof(KT) + sizeof(LeafDataT))
        };
        static constexpr u16 LEAF_BRANCHING_FACTOR{
            MANUAL_LEAF_BRANCHING_FACTOR ? MANUAL_LEAF_BRANCHING_FACTOR :
            math::max<u16>(LEAF_MIN_BRANCHING_FACTOR, LEAF_BRANCHING_FACTOR_CALC)
        };

        /**
         * BPTree inner node.
         * Keys are always sorted.
         * ChildGroup contains nodes/leafs
         * in the order of the keys.
         */
        class Node;

        /**
         * BPTree group of inner nodes.
         * Nodes are always ordered.
         */
        class NodeGroup;

        /// Used for Node alignment calculation.
        struct _DummyNode
        {
            SlotsUsedT _su;
            KT _k;
            NodeGroup *_cgptr;
        };

        /// Used for NodeGroup alignment calculation.
        struct _DummyNodeGroup
        {
            SlotsUsedT _su;
            _DummyNode _dn;
        };

        /**
         * BPTree leaf node.
         * Keys are always ordered.
         * storage and a ptr to a data block.
         */
        class Leaf;

        /// Used for Leaf alignment calculation.
        struct _DummyLeaf
        {
            SlotsUsedT _su;
            KT _k;
            LeafDataT _d;
        };

        /**
         * BPTree leaf node.
         * Leafs are always ordered.
         */
        class LeafGroup;

        /// Used for LeafGroup alignment calculation.
        struct _DummyLeafGroup
        {
            LeafGroup *_prev;
            LeafGroup *_next;
            SlotsUsedT _su;
            _DummyLeaf _dl;
        };

        using NodeT = Node;
        using NodePtr = NodeT*;

        using LeafT = Leaf;
        using LeafPtr = LeafT*;

        using NodeGroupT = NodeGroup;

        using LeafGroupT = LeafGroup;

        union ChildGroupPtr
        {
            NodeGroupT *nodeGroup;
            LeafGroupT *leafGroup;
            void* group;

            explicit ChildGroupPtr(void *ptr) :
                group{ptr}
            { }
        };

        static_assert(sizeof(ChildGroupPtr) == sizeof(void*));

        union ConstChildGroupPtr
        {
            const NodeGroupT *nodeGroup;
            const LeafGroupT *leafGroup;
            const void* group;

            explicit ConstChildGroupPtr(ChildGroupPtr other) :
                group{other.group}
            { }

            explicit ConstChildGroupPtr(const void *ptr) :
                group{ptr}
            { }
        };

        class alignas(_DummyNode) Node
        {
        public:
            template <typename CFG>
            friend class BPTree;

            using KeyT = KT;

            /// Number of keys.
            static constexpr u16 N{NODE_BRANCHING_FACTOR};
            static constexpr u16 MAX_KEYS{N};
            static constexpr u16 MIN_KEYS{MAX_KEYS / 2u};

            Node()
            {
                slotsUsed() = 0;
                childGroup().nodeGroup = nullptr;
            }
            Node(const Node &other)
            {
                slotsUsed() = 0;
                childGroup().nodeGroup = nullptr;
                copy(other);
            }
            Node(Node &&other)
            {
                slotsUsed() = 0;
                childGroup().nodeGroup = nullptr;
                move(std::move(other));
            }
            Node &operator=(const Node &rhs)
            { copy(rhs); return *this; }
            Node &operator=(Node &&rhs)
            { move(std::move(rhs)); return *this; }

            /// Get pointer to the group of children.
            ConstChildGroupPtr childGroup() const
            { return ConstChildGroupPtr{mem().childGroup.nodeGroup}; }
            /// Get how many slots(keys) are used.
            SlotsUsedT slotsUsed() const
            { return mem().slotsUsed; }
            /// Get key on given index = <0; MAX_KEYS).
            const KeyT &key(OffsetT index) const
            { BPTreeAssert(index < MAX_KEYS); return mem().keys[index]; }
            /// Is this node under minimal capacity?
            bool underMinimalCapacity() const
            { return slotsUsed() < MIN_KEYS; }
            /// Is this group at minimal capacity?
            bool atMinimalCapacity() const
            { return slotsUsed() == MIN_KEYS; }
            /// Does this node have empty slots?
            bool freeSlot() const
            { return MAX_KEYS > slotsUsed(); }
            /// How many free slots are there?
            SlotsUsedT numFreeSlots() const
            { return MAX_KEYS - slotsUsed(); }
            /// Is this node empty?
            bool empty() const
            { return slotsUsed() == 0; }
            /// Is this node full?
            bool full() const
            { return slotsUsed() == MAX_KEYS; }
            /// Get the index of the middle.
            OffsetT middleOffset() const
            { return static_cast<OffsetT>((slotsUsed() + 1) >> 1); }
            const KeyT *keyBegin() const
            { return mem().keys; }
            const KeyT *keyEnd() const
            { return mem().keys + slotsUsed(); }

            /**
             * Get the defining key for this node.
             * @return The key, on empty node, the return
             *   value is undefined.
             */
            const KeyT &defKey() const
            {
                BPTreeAssert(slotsUsed() != 0);
                return key(static_cast<OffsetT>(slotsUsed() - 1));
            }

            /**
             * Search this node for the key and return
             * index in the child group.
             * @param k Searched key.
             * @return Returns index of the slot.
             */
            OffsetT find(const KeyT& k) const
            {
                register OffsetT currSlot{0};
                register SlotsUsedT used{slotsUsed()};

                while(currSlot < used && !CmpKeys(k, key(currSlot)))
                {
                    ++currSlot;
                }

                return currSlot;
            }

            /**
             * Search this node for the key and
             * return index in the key array.
             * @param k The searched key.
             * @return Returns index of the key, or
             * index, where the key should have been.
             */
            OffsetT findK(const KeyT& k) const
            {
                u16 currSlot{0};

                for (currSlot = 0;
                     currSlot < slotsUsed() &&
                     CmpKeys(key(currSlot), k);
                    //k > key(currSlot);
                     ++currSlot)
                {}

                return currSlot;
            }
        public: // TODO - change back to private
            /**
             * Move element on given index by given offset.
             * @param index Index of the existing element.
             * @param offset How many times should the
             *   element be swapped with its neighbor.
             */
            void moveElement(OffsetT index, i32 offset)
            {
                if (offset == 0)
                {
                    return;
                }

                BPTreeAssert(index < slotsUsed());

                if (offset > 0)
                { // Move to the right.
                    BPTreeAssert(index + offset < slotsUsed());

                    OffsetT restPos{static_cast<OffsetT>(index + offset)};
                    for (OffsetT idx = index; idx != restPos; ++idx)
                    {
                        std::swap(key(idx), key(static_cast<OffsetT>(idx + 1)));
                    }
                }
                else
                { // Move to the left.
                    BPTreeAssert(static_cast<i32>(index) + offset >= 0);

                    OffsetT restPos{static_cast<OffsetT>(index + offset)};
                    for (OffsetT idx = index; idx != restPos; --idx)
                    {
                        std::swap(key(idx), key(static_cast<OffsetT>(idx - 1)));
                    }
                }
            }

            /**
             * Copy keys from other node.
             * If the offset is out of bounds, no
             * elements are copied.
             * _reset is called on this node inside.
             * @param other The other node.
             * @param offset Starting offset.
             */
            void copy(const NodeT &other, OffsetT offset = 0)
            {
                if (this == &other) {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed()) {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii) {
                        // Copy key.
                        _constructKey(iii - offset, other.key(iii));
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                } else {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }

                childGroup() = other.mem().childGroup;
            }

            /**
             * Move keys from other node.
             * If the offset is out of bounds, no
             * elements are moved.
             * _reset is called on this node inside.
             * @param other The other node.
             * @param offset Starting offset in the source.
             */
            void move(NodeT &&other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Move key.
                        _constructKey(iii - offset, std::move(other.key(iii)));
                        other._destructKey(iii);
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                    other.slotsUsed() = offset;
                }
                else
                {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }

                childGroup() = other.childGroup();
            }

            /**
             * Split this node.
             * M = SlotsUsed.
             * keys = <0, M)
             * middle = (M + 1) / 2
             * <0, middle) stays in this node.
             * (middle, M) is move to the rightHalf.
             * Only friends should be able to use this...
             * @param rightHalf Space for the right half.
             * @param middleInsert If set to true, then
             *   <middle, M) is moved to the rightHalf.
             * @return Returns the index, where the split occured
             */
            SlotsUsedT split(NodeT &rightHalf, bool middleInsert,
                             OffsetT insertIndex)
            {
                BPTreeAssert(this != &rightHalf);

                /**
                 * Calculate the middle point,
                 * The larger part should stay in this leaf.
                 */
                //SlotsUsedT middle{static_cast<SlotsUsedT>((slotsUsed() + 1) / 2)};
                SlotsUsedT middle{middleOffset()};

                if (middle != 0)
                { // There are slots to be moved...
                    if (middleInsert)
                    {
                        rightHalf.move(std::move(*this), middle);
                    }
                    else
                    {
                        if (insertIndex < middle)
                        {
                            middle--;
                        }

                        rightHalf.move(std::move(*this), middle + 1);
                        // Delete the middle key.
                        _destructKey(middle);

                        slotsUsed()--;
                    }
                }
                else
                {
                    rightHalf._reset();

                    //rightHalf.slotsUsed() = 0;
                    // slotsUsed unchanged
                }

                return middle;
            }

            /**
             * Combine this node with the other half.
             * slotsUsed() + rightHalf.slotsUsed() <= MAX_KEYS.
             * Content of this node will be on the left.
             * The rightHalf will be left empty.
             * @param rightHalf The other half.
             */
            void combine(NodeT &rightHalf)
            {
                BPTreeAssert(this != &rightHalf);
                BPTreeAssert(slotsUsed() + rightHalf.slotsUsed() <= MAX_KEYS);

                OffsetT rhIt{0};
                OffsetT lhIt{slotsUsed()};
                OffsetT rhEnd{rightHalf.slotsUsed()};
                for (; rhIt < rhEnd; ++rhIt, ++lhIt)
                {
                    // Move the key.
                    _constructKey(lhIt, std::move(rightHalf.key(rhIt)));
                }

                // Set the correct number of used slots.
                slotsUsed() = lhIt;

                // Empty out the rightHalf.
                rightHalf._reset();
            }

            /**
             * Get pointer to the group of children.
             * Only friends should be able to change this...
             */
            ChildGroupPtr &childGroup()
            { return mem().childGroup; }

            /**
             * Get how many slots are used(keys).
             * Only friends should be able to change usage...
             */
            SlotsUsedT &slotsUsed()
            { return mem().slotsUsed; }

            /**
             * Add given key to this node.
             * Position will be returned.
             * If the key already exists, its
             * position is returned instead.
             * If there is not enough space,
             * MAX_KEYS is returned instead.
             * @param k Key to add.
             * @param index Where to put the key.
             * @return Index of the new key.
             */
            OffsetT addKeyNoFind(const KeyT &k, OffsetT index)
            {
                if (full() || index >= MAX_KEYS)
                {
                    return MAX_KEYS;
                }

                if (index >= slotsUsed() || key(index) != k)
                { // The key does not exist.

                    // Call constructor on the last key.
                    _constructKey(slotsUsed(), k);

                    if (index < slotsUsed())
                    { // Move the other keys by one.
                        std::move_backward(keyBegin() + index,
                                           keyEnd(),
                                           keyEnd() + 1);
                    }

                    slotsUsed()++;
                }

                return index;
            }

            /**
             * Remove the given key from
             * this Node.
             * If the key does not exist,
             * this method has no effect.
             * @param index Index of the key.
             */
            void removeKeyNoFind(OffsetT index)
            {
                BPTreeAssert(index < slotsUsed());

                std::move(keyBegin() + index + 1, keyEnd(), keyBegin() + index);

                slotsUsed()--;

                _destructKey(slotsUsed());
            }

            /**
             * Add given key to this node.
             * Position will be returned.
             * If the key already exists, its
             * position is returned instead.
             * If there is not enough space,
             * MAX_KEYS is returned instead.
             * @param k Key to add.
             * @return Index of the new key.
             */
            OffsetT addKey(const KeyT &k)
            {
                if (full())
                {
                    return MAX_KEYS;
                }

                OffsetT index{findK(k)};

                if (key(index) != k)
                { // The key does not exist.

                    // Call constructor on the last key.
                    _constructKey(slotsUsed(), k);

                    if (index < slotsUsed())
                    { // Move the other keys by one.
                        std::move_backward(keyBegin() + index,
                                           keyEnd(),
                                           keyEnd() + 1);
                    }

                    slotsUsed()++;
                }

                return index;
            }

            /**
             * Remove the given key from this
             * node.
             * If the key does not exist,
             * this method has no effect.
             * @param k Key to remove.
             */
            void removeKey(const KeyT &k)
            {
                OffsetT index{findK(k)};

                if (index < slotsUsed() && key(index) == k)
                { // The key has been found.
                    // Make key array contiguous.
                    std::move(keyBegin() + index + 1,
                              keyEnd(),
                              keyBegin() + index);

                    slotsUsed()--;

                    // Call destructor on the last key.
                    _destructKey(slotsUsed());
                }
                // Else nothing happens...
            }

            /**
             * Make space for elements in the beginning of
             * the Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space is required.
             */
            void makeSpaceFront(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < MAX_KEYS);

                if (slotsUsed())
                { // Move the existing elements.
                    OffsetT dest{slotsUsed() + space - 1};
                    OffsetT src{slotsUsed() - 1};

                    // Move construct the elements.
                    while (src > 0)
                    {
                        _constructKey(dest, std::move(key(src)));

                        --src;
                        --dest;
                    }
                    // Move the last element.
                    _constructKey(dest, std::move(key(src)));
                }

                if (space > slotsUsed())
                { // We need to default construct the remaining slots.
                    OffsetT beg{slotsUsed()};
                    OffsetT end{space};

                    for (OffsetT iii = beg; iii != end; ++iii)
                    {
                        _constructKey(iii);
                    }
                }

                slotsUsed() += space;
            }

            /**
             * Remove elements from the front of this Leaf.
             * Correctly sets slotsUsed.
             * @param elements How many elements should be
             *   removed.
             */
            void removeFont(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{elements};
                OffsetT end{slotsUsed()};

                // Move the elements to the front.
                for (OffsetT dest = 0; it < end; ++it, ++dest)
                {
                    key(dest) = std::move(key(it));
                }

                it = slotsUsed() - elements;

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructKey(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Make space for elements in the back of the
             * Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space.
             */
            void makeSpaceBack(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < MAX_KEYS);

                OffsetT endIt{slotsUsed() + space};

                // Default construct the elements.
                for (OffsetT it = slotsUsed(); it < endIt; ++it)
                {
                    _constructKey(it);
                }

                slotsUsed() += space;
            }

            /**
             * Remove number of elements from the
             * back of this Leaf.
             * Correctly sets the slotsUsed.
             * @param elements How many elements to remove.
             */
            void removeBack(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{slotsUsed() - elements};
                OffsetT end{slotsUsed()};

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructKey(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Balance this Leaf with elements from
             * another leaf.
             * @param from Source Leaf.
             * @param fromOnRight Is the source Leaf
             *   on the right of this Leaf?
             */
            void balanceFrom(NodeT &from, bool fromOnRight)
            {
                BPTreeAssert(this != &from);

                // How many elements above the minimum are there?
                OffsetT aboveMin{from.slotsUsed() - MIN_KEYS};
                // How many elements we will move to the this Leaf?
                OffsetT numMove{(aboveMin + 1) >> 1};

                BPTreeAssert(slotsUsed() + numMove < MAX_KEYS);

                OffsetT fromIt{0};
                OffsetT fromEnd{0};
                OffsetT destIt{0};

                if (fromOnRight)
                {
                    makeSpaceBack(numMove);

                    // fromIt = 0;
                    fromEnd = numMove;
                    destIt = slotsUsed() - numMove;
                }
                else
                {
                    makeSpaceFront(numMove);

                    fromIt = from.slotsUsed() - numMove;
                    fromEnd = from.slotsUsed();
                    // destIt = 0;
                }

                // Move the elements.
                while (fromIt < fromEnd)
                {
                    key(destIt) = std::move(from.key(fromIt));

                    ++fromIt;
                    ++destIt;
                }

                if (fromOnRight)
                {
                    from.removeFont(numMove);
                }
                else
                {
                    from.removeBack(numMove);
                }
            }

            /**
             * Construct the key from the given key using
             * a placement new.
             * Friends should not use this...
             * @param index Index in the key array.
             * @param args Arguments passed to the constructor.
             */
            template <typename... Args>
            void _constructKey(OffsetT index, Args&&... args)
            {
                BPTreeAssert(index < MAX_KEYS);
                new (&mem().keys[index]) KeyT(std::forward<Args...>(args)...);
            }

            /**
             * Construct the key from the given key using
             * a placement new.
             * Friends should not use this...
             * @param it Iterator - position of the key.
             */
            void _constructKey(OffsetT index, KeyT *it)
            {
                BPTreeAssert(index < MAX_KEYS);
                new (&mem().keys[index]) KeyT(*it);
            }

            /**
             * Destruct the key.
             * Friends should not use this...
             * @param index Index in the key array.
             */
            void _destructKey(OffsetT index)
            {
                key(index).~KeyT();
            }
            /**
             * Destruct the key.
             * Friends should not use this...
             * @param it Iterator - position of the key.
             */
            void _destructKey(KeyT* it)
            {
                it->~KeyT();
            }

            /**
             * Reset this node structure.
             */
            void _reset()
            {
                // Destruct all keys.
                for (auto it = keyBegin(); it != keyEnd(); ++it)
                {
                    _destructKey(it);
                }

                // Reset the slot counter.
                slotsUsed() = 0;
            }

            /**
             * Get key on given index = <0; MAX_KEYS).
             * Only friends should be able to change keys...
             */
            KeyT &key(u16 index)
            { BPTreeAssert(index < MAX_KEYS); return mem().keys[index]; }
            KeyT *keyBegin()
            { return mem().keys; }
            KeyT *keyEnd()
            { return mem().keys + slotsUsed(); }

            // Memory layouts
            struct LayoutA
            {
                KeyT keys[MAX_KEYS];
                ChildGroupPtr childGroup;
                SlotsUsedT slotsUsed;
            };

            struct LayoutB
            {
                ChildGroupPtr childGroup;
                KeyT keys[MAX_KEYS];
                SlotsUsedT slotsUsed;
            };

            struct LayoutC
            {
                ChildGroupPtr childGroup;
                SlotsUsedT slotsUsed;
                KeyT keys[MAX_KEYS];
            };

            struct LayoutD
            {
                KeyT keys[MAX_KEYS];
                SlotsUsedT slotsUsed;
                ChildGroupPtr childGroup;
            };

            struct LayoutE
            {
                SlotsUsedT slotsUsed;
                KeyT keys[MAX_KEYS];
                ChildGroupPtr childGroup;
            };

            struct LayoutF
            {
                SlotsUsedT slotsUsed;
                ChildGroupPtr childGroup;
                KeyT keys[MAX_KEYS];
            };

            /// Select the memory layout with the smallest size.
            using MemLayout = mp_choose_best_t<
                mp_list<LayoutA, LayoutB, LayoutC, LayoutD, LayoutE, LayoutF>,
                std::less<std::size_t>,
                sizeof(LayoutA),
                sizeof(LayoutB),
                sizeof(LayoutC),
                sizeof(LayoutD),
                sizeof(LayoutE),
                sizeof(LayoutF)
            >;
            // Raw memory, without construction/destruction of inner Keys.
            union RawMem
            {
                u8 raw[sizeof(MemLayout)];
                MemLayout mem;

                RawMem() : raw{} { };
                ~RawMem() { };
            } _rawMem;

            MemLayout &mem()
            { return _rawMem.mem; }
            const MemLayout &mem() const
            { return _rawMem.mem; }
        protected:
        };

        class alignas(_DummyNodeGroup) NodeGroup
        {
        public:
            template <typename CFG>
            friend class BPTree;

            using KeyT = typename NodeT::KeyT;

            /// Number of Nodes.
            static constexpr u16 N{NODE_BRANCHING_FACTOR + 1};
            static constexpr u16 MIN_NODES{(N + 1) / 2};
            static_assert(Node::MIN_KEYS + 1 == MIN_NODES);

            NodeGroup()
            {
                mem().slotsUsed = 0;
            }
            NodeGroup(const NodeGroup &other)
            { mem().slotsUsed = 0; copy(other); }
            NodeGroup(NodeGroup &&other)
            { mem().slotsUsed = 0; move(std::move(other)); }
            NodeGroup &operator=(const NodeGroup &other)
            { copy(other); return *this; }
            NodeGroup &operator=(NodeGroup &&other)
            { move(std::move(other)); return *this; }
            ~NodeGroup()
            { _reset(); }

            /// Get how many slots are used.
            SlotsUsedT slotsUsed() const
            { return mem().slotsUsed; }
            /// Get the node on given index = <0, N).
            const NodeT &node(OffsetT index) const
            { BPTreeAssert(index < N); return mem().nodes[index]; }
            /// Does this node have empty slots?
            bool freeSlot() const
            { return N > slotsUsed(); }
            /// How many free slots are there?
            SlotsUsedT numFreeSlots() const
            { return N - slotsUsed(); }
            /// Is this NodeGroup empty?
            bool empty() const
            { return slotsUsed() == 0; }
            /// Is this NodeGroup full?
            bool full() const
            { return slotsUsed() == N; }
            /// Is this group under minimal capacity?
            bool underMinimalCapacity() const
            { return slotsUsed() < MIN_NODES; }
            /// Is this group at minimal capacity?
            bool atMinimalCapacity() const
            { return slotsUsed() == MIN_NODES; }
            /// Does Node on given index have left neighbour?
            bool hasLNeighbour(OffsetT index)
            { BPTreeAssert(index < slotsUsed()); return index > 0; }
            /// Does Node on given index have right neighbour?
            bool hasRNeighbour(OffsetT index)
            { BPTreeAssert(index < slotsUsed()); return index < slotsUsed() - 1; }
            /// Get the middle offset.
            OffsetT middleOffset() const
            { return static_cast<OffsetT>((slotsUsed() >> 1) + 1); }
            const NodeT *nodeBegin() const
            { return mem().nodes; }
            const NodeT *nodeEnd() const
            { return mem().nodes + slotsUsed(); }

            /**
             * Search this NodeGroup for a Node,
             * which contains keys <= as the given
             * key.
             * If such Node does not exist in
             * this NodeGroup, the returned
             * value is equal to slotsUsed().
             * @param k Searched key.
             * @return Returns index of the node.
             */
            OffsetT find(const KeyT &k) const
            {
                OffsetT currNode{0};
                SlotsUsedT used{slotsUsed()};

                for (currNode = 0;
                     currNode < used &&
                     CmpKeys(node(currNode).defKey(), k);
                     ++currNode)
                {}

                return currNode;
            }
        public: // TODO - change back to private
            /**
             * Merge 2 of the Nodes in this
             * NodeGroup.
             * The 2 Nodes have to be neighbours.
             * The Node on the right is always
             * merged into the Node on the left.
             * @param first Index of the first Leaf.
             * @param second Index of the second Leaf;
             */
            void merge(OffsetT first, OffsetT second)
            {
                OffsetT leftIndex{0};

                if (first < second)
                {
                    leftIndex = first;
                    BPTreeAssert(second - first == 1);
                    BPTreeAssert(second < slotsUsed());
                }
                else
                {
                    leftIndex = second;
                    BPTreeAssert(first - second == 1);
                    BPTreeAssert(first < slotsUsed());
                }

                NodeT &leftNode{node(leftIndex)};
                NodeT &rightNode{node(leftIndex + 1)};
                leftNode.combine(rightNode);

                if (leftIndex + 2 < slotsUsed())
                { // Make Leafs contiguous.
                    std::move(nodeBegin() + leftIndex + 2,
                              nodeEnd(),
                              nodeBegin() + leftIndex + 1);
                }

                _destructNode(slotsUsed() - 1);
                slotsUsed()--;
            }

            /**
             * Balance 2 of the Nodes in this
             * NodeGroup.
             * The 2 Nodes have to be neighbours.
             * @param toBalance Index of the Node, with
             *   less than minimal number of keys.
             * @param balanced Index of the chosen neighbour.
             */
            void balance(OffsetT toBalance, OffsetT balanced)
            {
                BPTreeAssert(node(balanced).slotsUsed() > Leaf::MIN_KEYS);
                BPTreeAssert(toBalance < slotsUsed());
                BPTreeAssert(balanced < slotsUsed());
                BPTreeAssert(toBalance != balanced);

                NodeT &nToBalance{node(toBalance)};
                NodeT &nBalanced{node(balanced)};

                nToBalance.balanceFrom(nBalanced, balanced > toBalance);
            }
            /**
             * Copy Nodes from other NodeGroup.
             * If the offset is out of bounds,
             * no elements are copied.
             * _reset is called on this NodeGroup
             * inside.
             * @param other The other NodeGroup.
             * @param offset Starting offset.
             */
            void copy(const NodeGroup &other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Copy Node.
                        _constructNode(iii - offset, other.node(iii));
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                }
                else
                {
                    // Nothing happens.
                }
            }

            /**
             * Move Nodes from other NodeGroup.
             * If the offset is out of bounds,
             * no elements are moved.
             * _reset is called on this NodeGroup
             * inside.
             * @param other The other NodeGroup.
             * @param offset Starting offset.
             */
            void move(NodeGroup &&other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Move Node.
                        _constructNode(iii - offset, std::move(other.node(iii)));
                        other._destructNode(iii);
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                    other.slotsUsed() = offset;
                }
                else
                {
                    // Nothing happens.
                }
            }

            /**
             * Split this NodeGroup.
             * M = SlotsUsed.
             * keys = <0, M)
             * middle = (M + 1) / 2
             * <0, middle) stays in this NodeGroup.
             * <middle, M) is move to the rightHalf.
             * Only friends should be able to use this...
             * @param rightHalf Space for the right half.
             * @return Returns the index, where the split occured
             */
            SlotsUsedT split(NodeGroup &rightHalf, OffsetT insertIndex)
            {
                BPTreeAssert(this != &rightHalf);

                /**
                 * Calculate the middle point,
                 * The larger part should stay in this NodeGroup.
                 */
                SlotsUsedT middle{middleOffset()};

                if (middle != 0)
                { // There are nodes to be moved...
                    if (insertIndex < middle)
                    {
                        middle--;
                    }

                    rightHalf.move(std::move(*this), middle);

                }
                else
                {
                    rightHalf._reset();
                }

                return middle;
            }

            /**
             * Combine this NodeGroup with the other half.
             * slotsUsed() + rightHalf.slotsUsed() <= N.
             * Content of this NodeGroup will be on the left.
             * The rightHalf will be left empty.
             * @param rightHalf The other half.
             */
            void combine(NodeGroupT &rightHalf)
            {
                BPTreeAssert(this != &rightHalf);
                BPTreeAssert(slotsUsed() + rightHalf.slotsUsed() <= N);

                OffsetT rhIt{0};
                OffsetT lhIt{slotsUsed()};
                OffsetT rhEnd{rightHalf.slotsUsed()};
                for (; rhIt < rhEnd; ++rhIt, ++lhIt)
                {
                    // Move the Node.
                    _constructNode(lhIt, std::move(rightHalf.node(rhIt)));
                }

                // Set the correct number of used slots.
                slotsUsed() = lhIt;

                // Empty out the rightHalf.
                rightHalf._reset();
            }

            /**
             * Access slotsUsed variable
             * Only friends should be able to use this...
             */
            SlotsUsedT &slotsUsed()
            { return mem().slotsUsed; }

            /**
             * Creates the first Node.
             * If the Node already exists, nothing
             * happens.
             * @return Returns index of the Node.
             */
            OffsetT createFirst()
            {
                if (empty())
                {
                    _constructNode(0);
                    slotsUsed() = 1;
                }

                return 0;
            }

            /**
             * Remove Node on given index.
             * If the index is valid ,the Node will
             * be removed.
             * @param index Index of the Node.
             */
            void removeNode(OffsetT index)
            {
                if (index < slotsUsed())
                {
                    return;
                }

                _destructNode(index);

                if (index != slotsUsed() - 1)
                { // If the Node is not last already.
                    // Make the Nodes contiguous.
                    std::move(nodeBegin() + index + 1,
                              nodeEnd(),
                              nodeBegin() + index);
                }

                slotsUsed()--;
            }

            /**
             * Make space next to Node on given index.
             * If the tree is full, or the index is not
             * valid, the index is returned instead.
             * The new Node is default constructed.
             * @param index Index of the Node.
             * @return Returns the index of the new Node.
             */
            OffsetT makeSpace(OffsetT index)
            {
                if (index >= slotsUsed() || full())
                {
                    return index;
                }

                OffsetT newIndex{static_cast<OffsetT>(index + 1)};

                // Create the new Node
                _constructNode(slotsUsed());

                if (newIndex != slotsUsed())
                {
                    // Move the Nodes next to the index by one.
                    std::move_backward(nodeBegin() + index + 1,
                                       nodeEnd(),
                                       nodeEnd() + 1);
                    // Reset the existing Node.
                    node(newIndex)._reset();
                }

                // We added a new Node.
                slotsUsed()++;

                return newIndex;
            }

            /**
             * Make space for elements in the beginning of
             * the Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space is required.
             */
            void makeSpaceFront(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < N);

                if (slotsUsed())
                { // Move the existing elements.
                    OffsetT dest{slotsUsed() + space - 1};
                    OffsetT src{slotsUsed() - 1};

                    // Move construct the elements.
                    while (src > 0)
                    {
                        _constructNode(dest, std::move(node(src)));

                        --src;
                        --dest;
                    }
                    // Move the last element.
                    _constructNode(dest, std::move(node(src)));
                }

                if (space > slotsUsed())
                { // We need to default construct the remaining slots.
                    OffsetT beg{slotsUsed()};
                    OffsetT end{space};

                    for (OffsetT iii = beg; iii != end; ++iii)
                    {
                        _constructNode(iii);
                    }
                }

                slotsUsed() += space;
            }

            /**
             * Remove elements from the front of this Leaf.
             * Correctly sets slotsUsed.
             * @param elements How many elements should be
             *   removed.
             */
            void removeFont(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{elements};
                OffsetT end{slotsUsed()};

                // Move the elements to the front.
                for (OffsetT dest = 0; it < end; ++it, ++dest)
                {
                    node(dest) = std::move(node(it));
                }

                it = slotsUsed() - elements;

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructNode(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Make space for elements in the back of the
             * Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space.
             */
            void makeSpaceBack(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < N);

                OffsetT endIt{slotsUsed() + space};

                // Default construct the elements.
                for (OffsetT it = slotsUsed(); it < endIt; ++it)
                {
                    _constructNode(it);
                }

                slotsUsed() += space;
            }

            /**
             * Remove number of elements from the
             * back of this Leaf.
             * Correctly sets the slotsUsed.
             * @param elements How many elements to remove.
             */
            void removeBack(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{slotsUsed() - elements};
                OffsetT end{slotsUsed()};

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructNode(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Balance this Leaf with elements from
             * another leaf.
             * @param from Source Leaf.
             * @param fromOnRight Is the source Leaf
             *   on the right of this Leaf?
             */
            void balanceFrom(NodeGroup &from, bool fromOnRight)
            {
                BPTreeAssert(this != &from);

                // How many elements above the minimum are there?
                OffsetT aboveMin{from.slotsUsed() - MIN_NODES};
                // How many elements we will move to the this Leaf?
                OffsetT numMove{(aboveMin + 1) >> 1};

                BPTreeAssert(slotsUsed() + numMove < N);

                OffsetT fromIt{0};
                OffsetT fromEnd{0};
                OffsetT destIt{0};

                if (fromOnRight)
                {
                    makeSpaceBack(numMove);

                    // fromIt = 0;
                    fromEnd = numMove;
                    destIt = slotsUsed() - numMove;
                }
                else
                {
                    makeSpaceFront(numMove);

                    fromIt = from.slotsUsed() - numMove;
                    fromEnd = from.slotsUsed();
                    // destIt = 0;
                }

                // Move the elements.
                while (fromIt < fromEnd)
                {
                    node(destIt) = std::move(from.node(fromIt));

                    ++fromIt;
                    ++destIt;
                }

                if (fromOnRight)
                {
                    from.removeFont(numMove);
                }
                else
                {
                    from.removeBack(numMove);
                }
            }

            /**
             * Construct the Node from the given arguments using
             * a placement new.
             * Friends should not use this...
             * @param index Index in the Node array.
             * @param args Arguments passed to the constructor.
             */
            template <typename... Args>
            void _constructNode(OffsetT index, Args&&... args)
            {
                BPTreeAssert(index < N);
                new (&mem().nodes[index]) NodeT(std::forward<Args...>(args)...);
            }

            /**
             * Construct the data from the given data using
             * a placement new.
             * Friends should not use this...
             * @param index Where to construct the Node.
             * @param it Iterator - position of the Node.
             */
            void _constructNode(OffsetT index, LeafT *it)
            {
                BPTreeAssert(index < N);
                new (&mem().nodes[index]) NodeT(*it);
            }

            /**
             * Destruct the Node.
             * Friends should not use this...
             * @param index Index in the Node array.
             */
            void _destructNode(OffsetT index)
            {
                node(index).~NodeT();
            }

            /**
             * Destruct the Node.
             * Friends should not use this...
             * @param it Iterator - position of the Node.
             */
            void _destructNode(NodeT *it)
            {
                it->~NodeT();
            }

            /**
             * Reset this NodeGroup structure.
             */
            void _reset()
            {
                for (auto it = nodeBegin(); it != nodeEnd(); ++it)
                {
                    _destructNode(it);
                }

                mem().slotsUsed = 0;
            }

            /**
             * Get node on given index = <0, N).
             * Only friends should be able to use this...
             */
            NodeT &node(u16 index)
            { BPTreeAssert(index < N); return mem().nodes[index]; }
            NodeT *nodeBegin()
            { return mem().nodes; }
            NodeT *nodeEnd()
            { return mem().nodes + slotsUsed(); }

            struct LayoutA
            {
                SlotsUsedT slotsUsed;
                NodeT nodes[N];
            };

            struct LayoutB
            {
                NodeT nodes[N];
                SlotsUsedT slotsUsed;
            };

            // Select the memory layout with the smallest size.
            using MemLayout = mp_choose_best_t<
                mp_list<LayoutA, LayoutB>,
                std::less<std::size_t>,
                sizeof(LayoutA),
                sizeof(LayoutB)
            >;
            // Raw memory, without construction/destruction of inner Nodes.
            union RawMem
            {
                u8 raw[sizeof(MemLayout)];
                MemLayout mem;

                RawMem() : raw{} { };
                ~RawMem() { };
            } _rawMem;

            MemLayout &mem()
            { return _rawMem.mem; }
            const MemLayout &mem() const
            { return _rawMem.mem; }
        protected:
        };

        class alignas(_DummyLeaf) Leaf
        {
        public:
            template <typename CFG>
            friend class BPTree;

            using KeyT = KT;
            using DataT = LeafDataT;

            static constexpr OffsetT M{LEAF_BRANCHING_FACTOR};
            static constexpr OffsetT MAX_CHILDREN{M};
            static constexpr OffsetT MAX_KEYS{M};
            static constexpr OffsetT MIN_KEYS{MAX_KEYS / 2u};

            Leaf()
            {
                slotsUsed() = 0;
                // The rest of raw memory is uninitialized.
            }
            Leaf(const Leaf &other)
            { slotsUsed() = 0; copy(other); }
            Leaf(Leaf &&other)
            { slotsUsed() = 0; move(std::move(other)); }
            Leaf &operator=(const Leaf &rhs)
            { copy(rhs); return *this; }
            Leaf &operator=(Leaf &&rhs)
            { move(std::move(rhs)); return *this; }
            ~Leaf()
            { _reset(); }

            /// Get how many slots(keys) are used.
            SlotsUsedT slotsUsed() const
            { return mem().slotsUsed; }
            /// Get key on given index = <0; MAX_KEYS).
            const KeyT &key(OffsetT index) const
            { return mem().keys[index]; }
            /// Get data on given index = <0; MAX_CHILDREN).
            const DataT &data(OffsetT index) const
            { return mem().data[index]; }
            /// Is this leaf under minimal capacity?
            bool underMinimalCapacity() const
            { return slotsUsed() < MIN_KEYS; }
            /// Is this leaf at minimal capacity?
            bool atMinimalCapacity() const
            { return slotsUsed() == MIN_KEYS; }
            /// Does this leaf have empty slots?
            bool freeSlot() const
            { return MAX_KEYS > slotsUsed(); }
            /// How many free slots are there?
            SlotsUsedT numFreeSlots() const
            { return MAX_KEYS - slotsUsed(); }
            /// Is this leaf empty?
            bool empty() const
            { return slotsUsed() == 0; }
            /// Is this leaf full?
            bool full() const
            { return slotsUsed() == MAX_KEYS; }
            /// Get the middle offset.
            OffsetT middleOffset() const
            { return static_cast<OffsetT>((slotsUsed() + 1) >> 1); }
            const KeyT *keyBegin() const
            { return mem().keys; }
            const KeyT *keyEnd() const
            { return mem().keys + slotsUsed(); }
            const DataT *dataBegin() const
            { return mem().data; }
            const DataT *dataEnd() const
            { return mem().data + slotsUsed(); }

            /**
             * Get the defining key for this leaf.
             * @return The key, on empty leaf, the return
             *   value is undefined.
             */
            const KeyT &defKey() const
            {
                BPTreeAssert(slotsUsed() != 0);
                return key(static_cast<OffsetT>(slotsUsed() - 1));
            }

            /**
             * Search this leaf for the key and return
             * index in the data array.
             * If the key is not found, the position,
             * where the key should have been is
             * returned instead.
             * @param k Searched key.
             * @return Returns index of the slot.
             */
            OffsetT find(const KeyT &k) const
            {
                register OffsetT currSlot{0};
                register SlotsUsedT used{slotsUsed()};

                /*
                for (currSlot = 0;
                     currSlot < used &&
                     CmpKeys(key(currSlot), k);
                     ++currSlot)
                {}
                 */

                while (currSlot < used && CmpKeys(key(currSlot), k))
                {
                    ++currSlot;
                }

                return currSlot;
            }
        public: // TODO - change back to private after testing.

            /**
             * Copy key/data pairs from other leaf.
             * If the offset is out of bounds, no
             * elements are copied. _reset is called
             * on this leaf inside.
             * @param other The other leaf.
             * @param offset Starting offset.
             */
            void copy(const LeafT &other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Copy key.
                        _constructKey(iii - offset, other.key(iii));
                        // Copy data.
                        _constructData(iii - offset, other.data(iii));
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                }
                else
                {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }
            }

            /**
             * Move key/data pairs from other leaf.
             * If the offset is out of bounds, no
             * elements are moved. _reset is called
             * on this leaf inside.
             * @param other The other leaf.
             * @param offset Starting offset.
             */
            void move(LeafT &&other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Move key.
                        _constructKey(iii - offset, std::move(other.key(iii)));
                        other._destructKey(iii);
                        // Move data.
                        _constructData(iii - offset, std::move(other.data(iii)));
                        other._destructData(iii);
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                    other.slotsUsed() = offset;
                }
                else
                {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }
            }

            /**
             * Split this leaf.
             * M = SlotsUsed.
             * keys = <0, M)
             * middle = (M + 1) / 2
             * <0, middle) stays in this leaf.
             * <middle, M) is move to the rightHalf.
             * Only friends should be able to use this...
             * @param rightHalf Space for the right half.
             * @return Returns the index, where the split occured
             */
            SlotsUsedT split(LeafT &rightHalf, OffsetT insertIndex)
            {
                BPTreeAssert(this != &rightHalf);

                /**
                 * Calculate the middle point,
                 * The larger part should stay in this leaf.
                 */
                SlotsUsedT middle{middleOffset()};

                if (middle != 0)
                { // There are slots to be moved...
                    if (insertIndex < middle)
                    {
                        middle--;
                    }

                    rightHalf.move(std::move(*this), middle);
                }
                else
                {
                    rightHalf._reset();

                    //rightHalf.slotsUsed() = 0;
                    // slotsUsed unchanged
                }

                return middle;
            }

            /**
             * Combine this Leaf with the other half.
             * slotsUsed() + rightHalf.slotsUsed() <= MAX_KEYS.
             * Content of this Leaf will be on the left.
             * The rightHalf will be left empty.
             * @param rightHalf The other half.
             */
            void combine(LeafT &rightHalf)
            {
                BPTreeAssert(this != &rightHalf);
                BPTreeAssert(slotsUsed() + rightHalf.slotsUsed() <= MAX_KEYS);

                OffsetT rhIt{0};
                OffsetT lhIt{slotsUsed()};
                OffsetT rhEnd{rightHalf.slotsUsed()};
                for (; rhIt < rhEnd; ++rhIt, ++lhIt)
                {
                    // Move the Key.
                    _constructKey(lhIt, std::move(rightHalf.key(rhIt)));
                    // Move the Data.
                    _constructData(lhIt, std::move(rightHalf.data(rhIt)));
                }

                // Set the correct number of used slots.
                slotsUsed() = lhIt;

                // Empty out the rightHalf.
                rightHalf._reset();
            }

            /**
             * Get how many slots are used(keys).
             * Only friends should be able to change usage...
             */
            SlotsUsedT &slotsUsed()
            { return mem().slotsUsed; }

            /**
             * Add given key to this leaf.
             * The key should not already exist in this leaf.
             * Returns nullptr on error.
             * Only friends should be able to use this...
             * @param k Key to insert.
             * @param index Index where the key should have been.
             * @return Reference to the data.
             */
            DataT *addKeyNoFind(const KeyT &k, OffsetT index)
            {
                DataT *resPtr{nullptr};

                if (index < MAX_KEYS && slotsUsed() < MAX_KEYS)
                { // There is still enough place for a key.

                    if (index < slotsUsed())
                    { // Move the other keys by one.
                        // Call constructor on the last key.
                        _constructKey(slotsUsed());
                        _constructData(slotsUsed());

                        std::move_backward(keyBegin() + index,
                                           keyEnd(),
                                           keyEnd() + 1);
                        std::move_backward(dataBegin() + index,
                                           dataEnd(),
                                           dataEnd() + 1);

                        key(index) = k;
                        data(index).~DataT();
                    }
                    else
                    {
                        _constructKey(slotsUsed(), k);
                    }

                    resPtr = mem().data + index;

                    slotsUsed()++;
                }
                else
                { // Error, not enough space...
                }

                return resPtr;
            }

            /**
             * Get ptr to data for given key.
             * If the key does not exist, it is created.
             * If there is not enough space to create the
             * key, nulltr is returned.
             * If creted was set to true, the caller must
             * construct the object on the other side of
             * returned ptr!
             * @param k The key.
             * @param created This variable is set to true,
             *   if the place for the data was newly created.
             * @return Ptr to the data.
             */
            DataT *getCreateData(const KeyT &k, bool &created)
            {
                OffsetT index{find(k)};

                DataT *retPtr{nullptr};

                if (index < slotsUsed() && key(index) == k)
                { // This key already exists in the tree.
                    created = false;
                    retPtr = dataBegin() + index;
                }
                else
                { // We got the position, where the key should have been.
                    created = true;
                    retPtr = addKeyNoFind(k, index);
                }

                return retPtr;
            }

            /**
             * Remove the given key from
             * this leaf.
             * If the key does not exist,
             * this method has no effect.
             * @param index Index of the key.
             */
            void removeKeyNoFind(OffsetT index)
            {
                BPTreeAssert(index < slotsUsed());

                // Make both arrays contiguous again.
                std::move(keyBegin() + index + 1, keyEnd(), keyBegin() + index);
                std::move(dataBegin() + index + 1, dataEnd(), dataBegin() + index);

                slotsUsed()--;

                // Call destructors on the last elements.
                _destructKey(slotsUsed());
                _destructData(slotsUsed());
            }

            /**
             * Make space for elements in the beginning of
             * the Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space is required.
             */
            void makeSpaceFront(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < MAX_KEYS);

                if (slotsUsed())
                { // Move the existing elements.
                    OffsetT dest{slotsUsed() + space - 1};
                    OffsetT src{slotsUsed() - 1};

                    // Move construct the elements.
                    while (src > 0)
                    {
                        _constructKey(dest, std::move(key(src)));
                        _constructData(dest, std::move(data(src)));

                        --src;
                        --dest;
                    }
                    // Move the last element.
                    _constructKey(dest, std::move(key(src)));
                    _constructData(dest, std::move(data(src)));
                }

                if (space > slotsUsed())
                { // We need to default construct the remaining slots.
                    OffsetT beg{slotsUsed()};
                    OffsetT end{space};

                    for (OffsetT iii = beg; iii != end; ++iii)
                    {
                        _constructKey(iii);
                        _constructData(iii);
                    }
                }

                slotsUsed() += space;
            }

            /**
             * Remove elements from the front of this Leaf.
             * Correctly sets slotsUsed.
             * @param elements How many elements should be
             *   removed.
             */
            void removeFont(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{elements};
                OffsetT end{slotsUsed()};

                // Move the elements to the front.
                for (OffsetT dest = 0; it < end; ++it, ++dest)
                {
                    key(dest) = std::move(key(it));
                    data(dest) = std::move(data(it));
                }

                it = slotsUsed() - elements;

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructKey(it);
                    _destructData(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Make space for elements in the back of the
             * Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space.
             */
            void makeSpaceBack(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < MAX_KEYS);

                OffsetT endIt{slotsUsed() + space};

                // Default construct the elements.
                for (OffsetT it = slotsUsed(); it < endIt; ++it)
                {
                    _constructKey(it);
                    _constructData(it);
                }

                slotsUsed() += space;
            }

            /**
             * Remove number of elements from the
             * back of this Leaf.
             * Correctly sets the slotsUsed.
             * @param elements How many elements to remove.
             */
            void removeBack(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{slotsUsed() - elements};
                OffsetT end{slotsUsed()};

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructKey(it);
                    _destructData(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Balance this Leaf with elements from
             * another leaf.
             * @param from Source Leaf.
             * @param fromOnRight Is the source Leaf
             *   on the right of this Leaf?
             */
            void balanceFrom(LeafT &from, bool fromOnRight)
            {
                BPTreeAssert(this != &from);

                // How many elements above the minimum are there?
                OffsetT aboveMin{from.slotsUsed() - MIN_KEYS};
                // How many elements we will move to the this Leaf?
                OffsetT numMove{(aboveMin + 1) >> 1};

                BPTreeAssert(slotsUsed() + numMove < MAX_KEYS);

                OffsetT fromIt{0};
                OffsetT fromEnd{0};
                OffsetT destIt{0};

                if (fromOnRight)
                {
                    makeSpaceBack(numMove);

                    // fromIt = 0;
                    fromEnd = numMove;
                    destIt = slotsUsed() - numMove;
                }
                else
                {
                    makeSpaceFront(numMove);

                    fromIt = from.slotsUsed() - numMove;
                    fromEnd = from.slotsUsed();
                    // destIt = 0;
                }

                // Move the elements.
                while (fromIt < fromEnd)
                {
                    key(destIt) = std::move(from.key(fromIt));
                    data(destIt) = std::move(from.data(fromIt));

                    ++fromIt;
                    ++destIt;
                }

                if (fromOnRight)
                {
                    from.removeFont(numMove);
                }
                else
                {
                    from.removeBack(numMove);
                }
            }

            /**
             * Construct the data from the given arguments using
             * a placement new.
             * Friends should not use this...
             * @param index Index in the data array.
             * @param args Arguments passed to the constructor.
             */
            template <typename... Args>
            void _constructData(OffsetT index, Args&&... args)
            {
                BPTreeAssert(index < MAX_CHILDREN);
                new (&mem().data[index]) DataT(std::forward<Args...>(args)...);
            }

            /**
             * Construct the data from the given data using
             * a placement new.
             * Friends should not use this...
             * @param index Where to construct the data.
             * @param it Iterator - position of the data.
             */
            void _constructData(OffsetT index, DataT *it)
            {
                BPTreeAssert(index < MAX_KEYS);
                new (&mem().data[index]) DataT(*it);
            }

            /**
             * Destruct the data.
             * Friends should not use this...
             * @param index Index in the data array.
             */
            void _destructData(OffsetT index)
            {
                data(index).~DataT();
            }

            /**
             * Destruct the data.
             * Friends should not use this...
             * @param it Iterator - position of the data.
             */
            void _destructData(DataT *it)
            {
                it->~DataT();
            }

            /**
             * Construct the key from the given key using
             * a placement new.
             * Friends should not use this...
             * @param index Index in the key array.
             * @param args Arguments passed to the constructor.
             */
            template <typename... Args>
            void _constructKey(OffsetT index, Args&&... args)
            {
                BPTreeAssert(index < MAX_KEYS);
                new (&mem().keys[index]) KeyT(std::forward<Args...>(args)...);
            }

            /**
             * Construct the key from the given key using
             * a placement new.
             * Friends should not use this...
             * @param it Iterator - position of the key.
             */
            void _constructKey(OffsetT index, KeyT *it)
            {
                BPTreeAssert(index < MAX_KEYS);
                new (&mem().keys[index]) KeyT(*it);
            }

            /**
             * Destruct the key.
             * Friends should not use this...
             * @param index Index in the key array.
             */
            void _destructKey(OffsetT index)
            {
                key(index).~KeyT();
            }
            /**
             * Destruct the key.
             * Friends should not use this...
             * @param it Iterator - position of the key.
             */
            void _destructKey(KeyT* it)
            {
                it->~KeyT();
            }

            /**
             * Reset this leaf structure.
             * Friends should not use this...
             */
            void _reset()
            {
                // Destruct all keys.
                for (auto it = keyBegin(); it != keyEnd(); ++it)
                {
                    _destructKey(it);
                }

                // Destruct all data.
                for (auto it = dataBegin(); it != dataEnd(); ++it)
                {
                    _destructData(it);
                }

                // Reset slot counter.
                slotsUsed() = 0;
            }

            /**
             * Get key on given index = <0; MAX_KEYS).
             * Only friends should be able to change keys...
             */
            KeyT &key(OffsetT index)
            { BPTreeAssert(index < MAX_KEYS); return mem().keys[index]; }
            /// Safe iteration begin.
            KeyT *keyBegin()
            { return mem().keys; }
            /// Safe iteration end.
            KeyT *keyEnd()
            { return mem().keys + slotsUsed(); }

            /**
             * Get data on given index = <0; MAX_CHILDREN).
             * Only friends should be able to change offsets...
             */
            DataT &data(OffsetT index)
            { BPTreeAssert(index < MAX_CHILDREN); return mem().data[index]; }
            /// Safe iteration begin.
            DataT *dataBegin()
            { return mem().data; }
            /// Safe iteration end.
            DataT *dataEnd()
            { return mem().data + slotsUsed(); }

            struct LayoutA
            {
                SlotsUsedT slotsUsed;
                KeyT keys[MAX_KEYS];
                DataT data[MAX_CHILDREN];
            };

            struct LayoutB
            {
                KeyT keys[MAX_KEYS];
                SlotsUsedT slotsUsed;
                DataT data[MAX_CHILDREN];
            };

            struct LayoutC
            {
                KeyT keys[MAX_KEYS];
                DataT data[MAX_CHILDREN];
                SlotsUsedT slotsUsed;
            };

            struct LayoutD
            {
                DataT data[MAX_CHILDREN];
                SlotsUsedT slotsUsed;
                KeyT keys[MAX_KEYS];
            };

            struct LayoutE
            {
                SlotsUsedT slotsUsed;
                DataT data[MAX_CHILDREN];
                KeyT keys[MAX_KEYS];
            };

            struct LayoutF
            {
                DataT data[MAX_CHILDREN];
                KeyT keys[MAX_KEYS];
                SlotsUsedT slotsUsed;
            };

            /// Select the memory layout with the smallest size.
            using MemLayout = mp_choose_best_t<
                mp_list<LayoutA, LayoutB, LayoutC, LayoutD, LayoutE, LayoutF>,
                std::less<std::size_t>,
                sizeof(LayoutA),
                sizeof(LayoutB),
                sizeof(LayoutC),
                sizeof(LayoutD),
                sizeof(LayoutE),
                sizeof(LayoutF)
            >;
            // Raw memory, without construction/destruction of inner Keys/Data.
            union RawMem
            {
                u8 raw[sizeof(MemLayout)];
                MemLayout mem;

                RawMem() : raw{} { };
                ~RawMem() { };
            } _rawMem;

            MemLayout &mem()
            { return _rawMem.mem; }
            const MemLayout &mem() const
            { return _rawMem.mem; }
        protected:
        };

        class alignas(_DummyLeafGroup) LeafGroup
        {
        public:
            template <typename CFG>
            friend class BPTree;

            using KeyT = typename LeafT::KeyT;

            static constexpr u16 N{NODE_BRANCHING_FACTOR + 1};
            static constexpr u16 MIN_LEAFS{(N + 1) / 2};
            static_assert(Node::MIN_KEYS + 1 == MIN_LEAFS);

            LeafGroup()
            {
                mem().slotsUsed = 0;
                mem().nextLeafGroup = nullptr;
                mem().prevLeafGroup = nullptr;
            }
            LeafGroup(const LeafGroup &other)
            {
                mem().slotsUsed = 0;
                mem().nextLeafGroup = nullptr;
                mem().prevLeafGroup = nullptr;

                copy(other);
            }
            LeafGroup(LeafGroup &&other)
            {
                mem().slotsUsed = 0;
                mem().nextLeafGroup = nullptr;
                mem().prevLeafGroup = nullptr;

                move(std::move(other));
            }
            LeafGroup &operator=(const LeafGroup &rhs)
            { copy(rhs); return *this; }
            LeafGroup &operator=(LeafGroup &&other)
            { move(other); return *this; }
            ~LeafGroup()
            { _reset(); }

            /// Get how many slots are used.
            SlotsUsedT slotsUsed() const
            { return mem().slotsUsed; }
            /// Get the leaf on given index = <0, N).
            const LeafT &node(OffsetT index) const
            { BPTreeAssert(index < slotsUsed()); return mem().leafs[index]; }
            /// Does this leaf have empty slots?
            bool freeSlot() const
            { return N > slotsUsed(); }
            /// How many free slots are there?
            SlotsUsedT numFreeSlots() const
            { return N - slotsUsed(); }
            /// Is this LeafGroup empty?
            bool empty() const
            { return slotsUsed() == 0; }
            /// Is this LeafGroup full?
            bool full() const
            { return slotsUsed() == N; }
            /// Is this group under minimal capacity?
            bool underMinimalCapacity() const
            { return slotsUsed() < MIN_LEAFS; }
            /// Is this group at minimal capacity?
            bool atMinimalCapacity() const
            { return slotsUsed() == MIN_LEAFS; }
            /// Does Leaf on given index have left neighbour?
            bool hasLNeighbour(OffsetT index)
            { BPTreeAssert(index < slotsUsed()); return index > 0; }
            /// Does Leaf on given index have right neighbour?
            bool hasRNeighbour(OffsetT index)
            { BPTreeAssert(index < slotsUsed()); return index < slotsUsed() - 1; }
            /// Get a pointer to the previous leaf group.
            const LeafGroup *prev() const
            { return mem().prevLeafGroup; }
            /// Get a pointer to the next leaf group.
            const LeafGroup *next() const
            { return mem().nextLeafGroup; }
            /// Get the middle offset.
            OffsetT middleOffset() const
            { return static_cast<OffsetT>((slotsUsed() >> 1) + 1); }
            const LeafT *nodeBegin() const
            { return mem().leafs; }
            const LeafT *nodeEnd() const
            { return mem().leafs + slotsUsed(); }

            /**
             * Search this LeafGroup for a Leaf,
             * which contains keys <= as the given
             * key.
             * If such Leaf does not exist in
             * this LeafGroup, the returned
             * value is equal to slotsUsed().
             * @param k Searched key.
             * @return Returns index of the leaf.
             */
            OffsetT find(const KeyT &k) const
            {
                OffsetT currLeaf{0};
                SlotsUsedT used{slotsUsed()};

                for (currLeaf = 0;
                     currLeaf < used &&
                     CmpKeys(node(currLeaf).defKey(), k);
                     ++currLeaf)
                {}

                return currLeaf;
            }
        public: // TODO - change back to private.
            /**
             * Merge 2 of the Leafs in this
             * LeafGroup.
             * The 2 Leafs have to be neighbours.
             * The Leaf on the right is always
             * merged into the Leaf on the left.
             * @param first Index of the first Leaf.
             * @param second Index of the second Leaf;
             */
            void merge(OffsetT first, OffsetT second)
            {
                OffsetT leftIndex{0};

                if (first < second)
                {
                    leftIndex = first;
                    BPTreeAssert(second - first == 1);
                    BPTreeAssert(second < slotsUsed());
                }
                else
                {
                    leftIndex = second;
                    BPTreeAssert(first - second == 1);
                    BPTreeAssert(first < slotsUsed());
                }

                LeafT &leftLeaf{node(leftIndex)};
                LeafT &rightLeaf{node(leftIndex + 1)};
                leftLeaf.combine(rightLeaf);

                if (leftIndex + 2 < slotsUsed())
                { // Make Leafs contiguous.
                    std::move(leafBegin() + leftIndex + 2,
                              leafEnd(),
                              leafBegin() + leftIndex + 1);
                }

                _destructLeaf(slotsUsed() - 1);
                slotsUsed()--;
            }

            /**
             * Balance 2 of the Leafs in this
             * LeafGroup.
             * The 2 Leafs have to be neighbours.
             * @param toBalance Index of the Node, with
             *   less than minimal number of keys.
             * @param balanced Index of the chosen neighbour.
             */
            void balance(OffsetT toBalance, OffsetT balanced)
            {
                BPTreeAssert(node(balanced).slotsUsed() > Leaf::MIN_KEYS);
                BPTreeAssert(toBalance < slotsUsed());
                BPTreeAssert(balanced < slotsUsed());
                BPTreeAssert(toBalance != balanced);

                LeafT &lToBalance{node(toBalance)};
                LeafT &lBalanced{node(balanced)};

                lToBalance.balanceFrom(lBalanced, balanced > toBalance);
            }

            /**
             * Copy Leafs from other LeafGroup.
             * If the offset is out of bounds, no
             * elements are copied. _reset is called
             * on this LeafGroup inside.
             * @param other The other LeafGroup.
             * @param offset Starting offset.
             */
            void copy(const LeafGroupT &other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Copy leaf.
                        _constructLeaf(iii - offset, other.node(iii));
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                }
                else
                {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }

                mem().nextLeafGroup = other.mem().nextLeafGroup;
                mem().prevLeafGroup = other.mem().prevLeafGroup;
            }

            /**
             * Move leafs from other LeafGroup.
             * If the offset is out of bounds, no
             * elements are moved. _reset is called
             * on this LeafGroup inside.
             * @param other The other leaf.
             * @param offset Starting offset.
             */
            void move(LeafGroup &&other, OffsetT offset = 0)
            {
                if (this == &other)
                {
                    return;
                }

                _reset();

                if (offset < other.slotsUsed())
                {
                    for (OffsetT iii = offset; iii < other.slotsUsed(); ++iii)
                    {
                        // Move leaf.
                        _constructLeaf(iii - offset, std::move(other.node(iii)));
                        other._destructLeaf(iii);
                    }

                    slotsUsed() = other.slotsUsed() - offset;
                    other.slotsUsed() = offset;
                }
                else
                {
                    // Nothing happens.
                    // slotsUsed are already set correctly.
                }

                mem().nextLeafGroup =  other.mem().nextLeafGroup;
                mem().prevLeafGroup =  other.mem().prevLeafGroup;
            }

            /**
             * Split this LeafGroup.
             * M = SlotsUsed.
             * keys = <0, M)
             * middle = (M + 1) / 2
             * <0, middle) stays in this LeafGroup.
             * <middle, M) is move to the rightHalf.
             * Only friends should be able to use this...
             * @param rightHalf Space for the right half.
             * @return Returns the index, where the split occured
             */
            SlotsUsedT split(LeafGroup &rightHalf, OffsetT insertIndex)
            {
                BPTreeAssert(this != &rightHalf);

                /**
                 * Calculate the middle point,
                 * The larger part should stay in this LeafGroup.
                 */
                SlotsUsedT middle{middleOffset()};

                if (middle != 0)
                { // There are leafs to be moved...
                    if (insertIndex < middle)
                    {
                        middle--;
                    }
                    else
                    rightHalf.move(std::move(*this), middle);
                }
                else
                {
                    rightHalf._reset();
                }

                return middle;
            }

            /**
             * Combine this LeafGroup with the other half.
             * slotsUsed() + rightHalf.slotsUsed() <= N.
             * Content of this LeafGroup will be on the left.
             * The rightHalf will be left empty.
             * @param rightHalf The other half.
             */
            void combine(LeafGroup &rightHalf)
            {
                BPTreeAssert(this != &rightHalf);
                BPTreeAssert(slotsUsed() + rightHalf.slotsUsed() <= N);

                OffsetT rhIt{0};
                OffsetT lhIt{slotsUsed()};
                OffsetT rhEnd{rightHalf.slotsUsed()};
                for (; rhIt < rhEnd; ++rhIt, ++lhIt)
                {
                    // Move the Leaf.
                    _constructLeaf(lhIt, std::move(rightHalf.node(rhIt)));
                }

                // Set the correct number of used slots.
                slotsUsed() = lhIt;

                // Empty out the rightHalf.
                rightHalf._reset();
            }

            /**
             * Access to leafsUsed variable.
             * only friends should be able to use this...
             */
            SlotsUsedT &slotsUsed()
            { return mem().slotsUsed; }

            /**
             * Creates the first leaf.
             * If the first leaf already exists, nothing
             * happens.
             * @return Returns index of the leaf.
             */
            OffsetT createFirst()
            {
                if (empty())
                {
                    _constructLeaf(0);
                    slotsUsed() = 1;
                }

                return 0;
            }

            /**
             * Remove leaf on given index.
             * If the index is valid ,the leaf will
             * be removed.
             * @param index Index of the leaf.
             */
            void removeLeaf(OffsetT index)
            {
                if (index < slotsUsed())
                {
                    return;
                }

                _destructLeaf(index);

                if (index != slotsUsed() - 1)
                { // If the leaf is not last already.
                    // Make the leafs contiguous.
                    std::move(leafBegin() + index + 1,
                              leafEnd(),
                              leafBegin() + index);
                }

                slotsUsed()--;
            }

            /**
             * Make space next to leaf on given index.
             * If the tree is full, or the index is not
             * valid, the index is returned instead.
             * The new leaf is default constructed.
             * @param index Index of the leaf.
             * @return Returns the index of the new leaf.
             */
            OffsetT makeSpace(OffsetT index)
            {
                if (index >= slotsUsed() || full())
                {
                    return index;
                }

                OffsetT newIndex{static_cast<OffsetT>(index + 1)};

                // Create the new leaf
                _constructLeaf(slotsUsed());

                if (newIndex != slotsUsed())
                {
                    // Move the leafs next to the index by one.
                    std::move_backward(leafBegin() + index + 1,
                                       leafEnd(),
                                       leafEnd() + 1);

                    // Reset the existing leaf.
                    node(newIndex)._reset();
                }

                // We added a new leaf.
                slotsUsed()++;

                return newIndex;
            }

            /**
             * Make space for elements in the beginning of
             * the Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space is required.
             */
            void makeSpaceFront(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < N);

                if (slotsUsed())
                { // Move the existing elements.
                    OffsetT dest{slotsUsed() + space - 1};
                    OffsetT src{slotsUsed() - 1};

                    // Move construct the elements.
                    while (src > 0)
                    {
                        _constructLeaf(dest, std::move(node(src)));

                        --src;
                        --dest;
                    }
                    // Move the last element.
                    _constructLeaf(dest, std::move(node(src)));
                }

                if (space > slotsUsed())
                { // We need to default construct the remaining slots.
                    OffsetT beg{slotsUsed()};
                    OffsetT end{space};

                    for (OffsetT iii = beg; iii != end; ++iii)
                    {
                        _constructLeaf(iii);
                    }
                }

                slotsUsed() += space;
            }

            /**
             * Remove elements from the front of this Leaf.
             * Correctly sets slotsUsed.
             * @param elements How many elements should be
             *   removed.
             */
            void removeFont(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{elements};
                OffsetT end{slotsUsed()};

                // Move the elements to the front.
                for (OffsetT dest = 0; it < end; ++it, ++dest)
                {
                    node(dest) = std::move(node(it));
                }

                it = slotsUsed() - elements;

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructLeaf(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Make space for elements in the back of the
             * Leaf.
             * Correctly sets slotsUsed;
             * @param space How much space.
             */
            void makeSpaceBack(OffsetT space)
            {
                if (space == 0)
                {
                    return;
                }

                BPTreeAssert(slotsUsed() + space < N);

                OffsetT endIt{slotsUsed() + space};

                // Default construct the elements.
                for (OffsetT it = slotsUsed(); it < endIt; ++it)
                {
                    _constructLeaf(it);
                }

                slotsUsed() += space;
            }

            /**
             * Remove number of elements from the
             * back of this Leaf.
             * Correctly sets the slotsUsed.
             * @param elements How many elements to remove.
             */
            void removeBack(OffsetT elements)
            {
                if (elements == 0)
                {
                    return;
                }

                BPTreeAssert(elements <= slotsUsed());

                OffsetT it{slotsUsed() - elements};
                OffsetT end{slotsUsed()};

                // Destruct the elements at the end.
                while (it < end)
                {
                    _destructLeaf(it);

                    ++it;
                }

                slotsUsed() -= elements;
            }

            /**
             * Balance this Leaf with elements from
             * another leaf.
             * @param from Source Leaf.
             * @param fromOnRight Is the source Leaf
             *   on the right of this Leaf?
             */
            void balanceFrom(LeafGroup &from, bool fromOnRight)
            {
                BPTreeAssert(this != &from);

                // How many elements above the minimum are there?
                OffsetT aboveMin{from.slotsUsed() - MIN_LEAFS};
                // How many elements we will move to the this Leaf?
                OffsetT numMove{(aboveMin + 1) >> 1};

                BPTreeAssert(slotsUsed() + numMove < N);

                OffsetT fromIt{0};
                OffsetT fromEnd{0};
                OffsetT destIt{0};

                if (fromOnRight)
                {
                    makeSpaceBack(numMove);

                    // fromIt = 0;
                    fromEnd = numMove;
                    destIt = slotsUsed() - numMove;
                }
                else
                {
                    makeSpaceFront(numMove);

                    fromIt = from.slotsUsed() - numMove;
                    fromEnd = from.slotsUsed();
                    // destIt = 0;
                }

                // Move the elements.
                while (fromIt < fromEnd)
                {
                    node(destIt) = std::move(from.node(fromIt));

                    ++fromIt;
                    ++destIt;
                }

                if (fromOnRight)
                {
                    from.removeFont(numMove);
                }
                else
                {
                    from.removeBack(numMove);
                }
            }

            /**
             * Construct the leaf from the given arguments using
             * a placement new.
             * Friends should not use this...
             * @param index Index in the leaf array.
             * @param args Arguments passed to the constructor.
             */
            template <typename... Args>
            void _constructLeaf(OffsetT index, Args&&... args)
            {
                BPTreeAssert(index < N);
                new (&mem().leafs[index]) LeafT(std::forward<Args...>(args)...);
            }

            /**
             * Construct the data from the given data using
             * a placement new.
             * Friends should not use this...
             * @param index Where to construct the leaf.
             * @param it Iterator - position of the leaf.
             */
            void _constructLeaf(OffsetT index, LeafT *it)
            {
                BPTreeAssert(index < N);
                new (&mem().leafs[index]) LeafT(*it);
            }

            /**
             * Destruct the leaf.
             * Friends should not use this...
             * @param index Index in the leaf array.
             */
            void _destructLeaf(OffsetT index)
            {
                node(index).~LeafT();
            }

            /**
             * Destruct the leaf.
             * Friends should not use this...
             * @param it Iterator - position of the leaf.
             */
            void _destructLeaf(LeafT *it)
            {
                it->~LeafT();
            }

            /**
             * Get leaf on given index = <0, N).
             * Only friends should be able to use this...
             */
            LeafT &node(OffsetT index)
            { BPTreeAssert(index < slotsUsed()); return mem().leafs[index]; }
            LeafT *leafBegin()
            { return mem().leafs; }
            LeafT *leafEnd()
            { return mem().leafs + slotsUsed(); }

            /**
             * Get a ptr to the previous leaf group.
             * Only friends should be able to use this...
             */
            LeafGroup *&prev()
            { return mem().prevLeafGroup; }

            /**
             * Get a ptr to the next leaf group.
             * Only friends should be able to use this...
             */
            LeafGroup *&next()
            { return mem().nextLeafGroup; }

            /**
             * Reset this LeafGroup to default state.
             * This should not be used by friends...
             */
            void _reset()
            {
                for (auto it = leafBegin(); it != leafEnd(); ++it)
                {
                    _destructLeaf(it);
                }

                //mem().prevLeafGroup = nullptr;
                //mem().nextLeafGroup = nullptr;
                mem().slotsUsed = 0;
            }

            struct LayoutA
            {
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
                SlotsUsedT slotsUsed;
                LeafT leafs[N];
            };

            struct LayoutB
            {
                SlotsUsedT slotsUsed;
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
                LeafT leafs[N];
            };

            struct LayoutC
            {
                SlotsUsedT slotsUsed;
                LeafT leafs[N];
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
            };

            struct LayoutD
            {
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
                LeafT leafs[N];
                SlotsUsedT slotsUsed;
            };

            struct LayoutE
            {
                LeafT leafs[N];
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
                SlotsUsedT slotsUsed;
            };

            struct LayoutF
            {
                LeafT leafs[N];
                SlotsUsedT slotsUsed;
                LeafGroup *prevLeafGroup;
                LeafGroup *nextLeafGroup;
            };

            // Select the memory layout with the smallest size.
            using MemLayout = mp_choose_best_t<
                mp_list<LayoutA, LayoutB, LayoutC, LayoutD, LayoutE, LayoutF>,
                std::less<std::size_t>,
                sizeof(LayoutA),
                sizeof(LayoutB),
                sizeof(LayoutC),
                sizeof(LayoutD),
                sizeof(LayoutE),
                sizeof(LayoutF)
            >;
            // Raw memory, without construction/destruction of inner Leafs.
            union RawMem
            {
                u8 raw[sizeof(MemLayout)];
                MemLayout mem;

                RawMem() : raw{} { };
                ~RawMem() { };
            } _rawMem;

            MemLayout &mem()
            { return _rawMem.mem; }
            const MemLayout &mem() const
            { return _rawMem.mem; }
        protected:
        };

        static_assert(sizeof(NodeT) <= MAX_NODE_SIZE ||
                      MANUAL_NODE_BRANCHING_FACTOR ||
                      NODE_BRANCHING_FACTOR == NODE_MIN_BRANCHING_FACTOR);
        static_assert(sizeof(LeafT) <= MAX_LEAF_SIZE ||
                      MANUAL_LEAF_BRANCHING_FACTOR ||
                      LEAF_BRANCHING_FACTOR == LEAF_MIN_BRANCHING_FACTOR);
    private:
    protected:
    };

#undef BPTreeDebug
#undef BPTreeAssert
}

#endif //MEM_BPTREECONFIG_H
