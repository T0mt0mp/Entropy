/**
 * @file mem/BPTree.h
 * @author Tomas Polasek
 * @brief B+ tree implementation.
 */

#ifndef MEM_BPTREE_H
#define MEM_BPTREE_H

#include "mem/BPTreeConfig.h"
#include "mem/List.h"

namespace mem
{
#ifndef NDEBUG
#   define BPTreeDebug
#endif

#ifdef BPTreeDebug
#define BPTreeAssert(COND_EXP) ASSERT_SLOW(COND_EXP)
#else
#define BPTreeAssert(COND_EXP)
#endif

    /**
     * B+ tree class.
     * Pointers to data is stored only in leaf nodes.
     * Data is stored in "pages" of static size (e.g. 8192 elements).
     * Support for bulk operations - insert, delete, or both.
     */
    template <typename Config>
    class BPTree
    {
    public:
        using KeyT = typename Config::KT;
        using EleT = typename Config::ET;
        using AllocT = typename Config::AT;
        using OffsetT = typename Config::OffsetT;

        using NodeT = typename Config::NodeT;
        using NodeGroupT = typename Config::NodeGroupT;

        using LeafT = typename Config::LeafT;
        using LeafGroupT = typename Config::LeafGroupT;

        using ChildGroupPtr = typename Config::ChildGroupPtr;
        using ConstChildGroupPtr = typename Config::ConstChildGroupPtr;

        /// Used for passing a node, including its NodeGroup.
        using NodeGroupIndexPair = std::pair<NodeGroupT*, OffsetT>;
        using ConstNodeGroupIndexPair = std::pair<const NodeGroupT*, OffsetT>;
        /// Used for passing a leaf, including its LeafGroup.
        using LeafGroupIndexPair = std::pair<LeafGroupT*, OffsetT>;
        using ConstLeafGroupIndexPair = std::pair<const LeafGroupT*, OffsetT>;

        using NodeGroupAllocT = typename AllocT::
            template rebind<NodeGroupT>::other;
        using LeafGroupAllocT = typename AllocT::
            template rebind<LeafGroupT>::other;

        /// Tree statistics.
        struct StatsT
        {
            u64 numNodeGroups{0};
            u64 numNodes{0};
            u64 numNodesFilledKeys{0};

            u64 numLeafGroups{0};
            u64 numLeafs{0};
            u64 numLeafsFilledKeys{0};

            /// Reset the stats.
            void reset()
            {
                numNodeGroups = 0;
                numNodes = 0;
                numNodesFilledKeys = 0;

                numLeafGroups = 0;
                numLeafs = 0;
                numLeafsFilledKeys = 0;
            }

            /// Are all values set to 0?
            bool empty() const
            {
                return !numNodeGroups && !numNodes && !numNodesFilledKeys &&
                       !numLeafGroups && !numLeafs && !numLeafsFilledKeys;
            }
        };

        /**
         * Data iterator for the B+ tree.
         * It is "random access" iterator, but
         * only withing a single leaf!
         */
        template <bool isConst = false>
        class BPTreeIterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = mp_choose_t<isConst,
                std::pair<const KeyT, const EleT&>,
                std::pair<KeyT, EleT>>;
            using ref_type = mp_choose_t<isConst,
                std::pair<const KeyT&, const EleT&>,
                std::pair<const KeyT&, EleT&>>;
            using ptr_type = mp_choose_t<isConst,
                const EleT*,
                EleT*>;
            using ThisType = BPTreeIterator<isConst>;

            using ItLeafGroupPtr = mp_choose_t<isConst,
                const LeafGroupT*,
                LeafGroupT*>;

            using ItLeafGroupIndexPair = mp_choose_t<isConst,
                ConstLeafGroupIndexPair,
                LeafGroupIndexPair>;

            BPTreeIterator(const ThisType &other) = default;
            BPTreeIterator(ThisType &&other) = default;
            ThisType &operator=(const ThisType &rhs) = default;
            ThisType &operator=(ThisType &&rhs) = default;

            /**
             * Create an iterator on given position.
             * If lg == nullptr, then the iterator is
             * automatically invalid.
             * @param lg LeafGroup pointer.
             * @param go Offset within the LeafGroup.
             * @param lo Offset within the Leaf.
             */
            BPTreeIterator(ItLeafGroupPtr lg = nullptr,
                           OffsetT go = 0,
                           OffsetT lo = 0) :
                mValid{lg != nullptr},
                mCLG{lg},
                mGO{go},
                mLO{lo}
            { }

            /**
             * Create an iterator on given position.
             * If lgi.first == nullptr, then the iterator
             * if automatically invalid.
             * @param lgi LeafGroup + offset within.
             * @param lo Offset within the Leaf.
             */
            BPTreeIterator(ItLeafGroupIndexPair lgi,
                           OffsetT lo = 0) :
                mValid{lgi.first != nullptr},
                mCLG{lgi.first},
                mGO{lgi.second},
                mLO{lo}
            { }

            void swap(ThisType &other)
            {
                std::swap(mValid, other.mValid);
                std::swap(mCLG, other.mCLG);
                std::swap(mGO, other.mGO);
                std::swap(mLO, other.mLO);
            }

            bool operator==(const ThisType &rhs) const
            {
                return (valid() && rhs.valid() &&
                       mCLG == rhs.mCLG &&
                       mGO == rhs.mGO &&
                       mLO == rhs.mLO) ||
                       (!valid() && !rhs.valid());
            }
            bool operator!=(const ThisType &rhs) const
            { return !(*this == rhs); }

            ref_type operator*() const
            {
                BPTreeAssert(mCLG != nullptr);
                auto &leaf{mCLG->node(mGO)};
                return ref_type(
                    leaf.key(mLO),
                    leaf.data(mLO)
                );
            }
            ptr_type operator->() const
            {
                auto &leaf{mCLG->leaf(mGO)};
                return &leaf.data(mLO);
            }

            ThisType &operator++()
            {
                auto &leaf{mCLG->node(mGO)};
                OffsetT maxLeafOffset{leaf.slotsUsed()};

                if (mLO < maxLeafOffset - 1)
                { // There are more keys in current leaf.
                    mLO++;
                }
                else
                { // We need to move to the next leaf.
                    OffsetT maxGroupOffset{static_cast<OffsetT>(mCLG->slotsUsed() - 1)};

                    if (mGO < maxGroupOffset)
                    { // There are more leafs in current LeafGroup.
                        mGO++;
                    }
                    else
                    { // We need to move to the next LeafGroup.

                        mCLG = mCLG->next();

                        if (mCLG == nullptr)
                        {
                            mValid = false;
                        }

                        // Go to the first leaf in the group.
                        mGO = 0;
                    }

                    // Go to the first key in the leaf.
                    mLO = 0;
                }

                return *this;
            }
            ThisType operator++(int)
            {
                ThisType temp(*this);

                ++(*this);

                return temp;
            }

            ThisType &operator--()
            {
                if (mLO > 0)
                { // There are more keys in current leaf.
                    mLO--;
                }
                else
                { // We need to move to the previous leaf.

                    if (mGO > 0)
                    { // There are more leafs in current LeafGroup.
                        mGO--;

                        auto &leaf{mCLG->leaf(mGO)};
                        // Go to the last key in the leaf.
                        mLO = leaf.slotsUsed() - 1;
                    }
                    else
                    { // We need to move to the previous LeafGroup.

                        mCLG = mCLG->prev();

                        if (mCLG == nullptr)
                        {
                            mValid = false;
                        }
                        else
                        {
                            // Go to the last leaf in the group.
                            mGO = mCLG->leafsUsed() - 1;

                            auto &leaf{mCLG->leaf(mGO)};
                            // Go to the last key in the leaf.
                            mLO = leaf.slotsUsed() - 1;
                        }
                    }
                }

                return *this;
            }
            ThisType operator--(int)
            {
                ThisType temp(*this);

                --(*this);

                return temp;
            }

            /**
             * Move by val slots, in the current leaf.
             * If the val is out of valid range, it is
             * instead set to the last valid slot.
             * @param val How many slots to move.
             * @return The new Iterator.
             */
            ThisType operator+(OffsetT val) const
            {
                OffsetT clampedVal{val};
                auto &leaf{mCLG->leaf(mGO)};
                OffsetT maxVal{leaf.slotsUsed()};

                if (clampedVal + mLO >= maxVal)
                {
                    clampedVal = maxVal - 1;
                }

                return ThisType(mCLG, mGO, clampedVal);
            }
            /**
             * Move by val slots, in the current leaf.
             * If the val is out of valid range, it is
             * instead set to the last valid slot.
             * @param val How many slots to move.
             * @return The changed Iterator.
             */
            ThisType &operator+=(OffsetT val)
            {
                auto &leaf{mCLG->leaf(mGO)};
                OffsetT maxVal{leaf.slotsUsed()};

                if (val + mLO >= maxVal)
                {
                    mLO = maxVal - 1;
                }
                else
                {
                    mLO = val;
                }

                return *this;
            }
            friend ThisType operator+(OffsetT val, const ThisType &rhs)
            {
                return rhs + val;
            }

            /**
             * Move by -val slots, in the current leaf.
             * If the val is out of valid range, it is
             * instead set to the first valid slot.
             * @param val How many slots to move.
             * @return The new Iterator.
             */
            ThisType operator-(OffsetT val) const
            {
                OffsetT newLO{0};

                if (static_cast<i32>(mLO) - val > 0)
                {
                    newLO = mLO - val;
                }

                return ThisType(mCLG, mGO, newLO);
            }
            /**
             * Move by -val slots, in the current leaf.
             * If the val is out of valid range, it is
             * instead set to the first valid slot.
             * @param val How many slots to move.
             * @return The changed Iterator.
             */
            ThisType &operator-=(u64 val)
            {
                if (static_cast<i32>(mLO) - val > 0)
                {
                    mLO = mLO - val;
                }
                else
                {
                    mLO = 0;
                }

                return *this;
            }
            friend ThisType operator-(u64 val, const ThisType &rhs)
            {
                return rhs - val;
            }

            /// Works only for iterators within the same LeafGroup.
            bool operator<(const ThisType &rhs) const
            {
                BPTreeAssert(mCLG == rhs.mCLG);
                return mGO < rhs.mGO ||
                    (mGO == rhs.mGO && mLO < rhs.mLO);
            }

            /// Works only for iterators within the same LeafGroup.
            bool operator>(const ThisType &rhs) const
            {
                BPTreeAssert(mCLG == rhs.mCLG);
                return mGO > rhs.mGO ||
                       (mGO == rhs.mGO && mLO > rhs.mLO);
            }

            /// Works only for iterators within the same LeafGroup.
            bool operator<=(const ThisType &rhs) const
            {
                BPTreeAssert(mCLG == rhs.mCLG);
                return mGO < rhs.mGO ||
                       (mGO == rhs.mGO && mLO <= rhs.mLO);
            }

            /// Works only for iterators within the same LeafGroup.
            bool operator>=(const ThisType &rhs) const
            {
                BPTreeAssert(mCLG == rhs.mCLG);
                return mGO > rhs.mGO ||
                       (mGO == rhs.mGO && mLO >= rhs.mLO);
            }

            /// Works only within a single leaf.
            ref_type operator[](OffsetT index) const
            {
                BPTreeAssert(index < LeafT::M && mCLG != nullptr);
                auto &leaf{mCLG->leaf(mGO)};
                return ref_type(
                    leaf.key(mLO + index),
                    leaf.data(mLO + index)
                );
            }

            /**
             * Is this iterator valid?
             * Only checks for a valid LeafGroup,
             * or can be set manually.
             * @return Returns true, if this iterator is valid.
             */
            bool valid() const
            { return mValid; }
        private:
            /**
             * To iterate on the leaf level, the
             * iterator needs following information :
             *   1) Ptr to the current LeafGroup.
             *   2) Current offset in the group.
             *   3) Current offset in the leaf.
             * On top of that, the iterator needs
             * information about when to jump to
             * the next LeafGroup / Leaf.
             */

            /// Is this iterator valid?
            bool            mValid{false};
            /// Current LeafGroup ptr.
            ItLeafGroupPtr  mCLG{nullptr};
            /// Current offset withing the LeafGroup.
            OffsetT         mGO{0};
            /// Current offset within the Leaf;
            OffsetT         mLO{0};
        protected:
        };
    public:
        using Iterator = BPTreeIterator<false>;
        using ConstIterator = BPTreeIterator<true>;

        /// Default constructor.
        BPTree();
        /// Copy constructor.
        BPTree(const BPTree &other);
        /// Move constructor.
        BPTree(BPTree &&other);

        /// Destructor frees whole tree.
        ~BPTree();

        /// Copy assignment operator.
        BPTree &operator=(const BPTree &rhs);
        /// Move assignment operator.
        BPTree &operator=(BPTree &&rhs);

        /**
         * Swap the internal structures of two B+ trees.
         * @param other Who to swap with.
         */
        void swap(BPTree &other);

        /**
         * Is this tree empty?
         * @return Returns true, if the tree has no
         *   elements inside it.
         */
        bool empty() const
        { return mRoot.group == nullptr; }

        /**
         * Search for given key.
         * @param key Key to search for.
         * @return If the key was found, returns an iterator
         *   pointing to the first occurrence of the key, else
         *   returns invalid iterator.
         */
        Iterator search(const KeyT &key)
        { return findInt(key); }

        /**
         * Search for given key.
         * @param key Key to search for.
         * @return If the key was found, returns an iterator
         *   pointing to the first occurrence of the key, else
         *   returns invalid iterator.
         */
        ConstIterator search(const KeyT &key) const;

        /**
         * Add a new element to given key.
         * If the element already exists, it will be destructed.
         * The element is constructed with given constructor
         * parameters.
         * @param key Key of the element.
         * @param args Arguments for the constructor.
         * @return Returns a reference to the newly constructed
         *   element.
         */
        template <typename... Args>
        Iterator insert(const KeyT &key, Args... args);

        /**
         * Search for element with given key.
         * If the element doesn't exists, it will be constructed.
         * The element is constructed with given constructor
         * parameters.
         * @param key Key of the element.
         * @param args Arguments for the constructor.
         * @return Returns a reference to the newly constructed
         *   element.
         */
        template <typename... Args>
        Iterator searchOrInsert(const KeyT &key, Args... args);

        /**
         * Replaces element pointed to by the iterator.
         * The element is constructed with given constructor
         * parameters.
         * @param element Iterator pointing to the element to replace.
         * @param args Arguments for the constructor.
         * @return Returns a reference to the newly constructed
         *   element.
         */
        template <typename... Args>
        EleT &replace(Iterator element, Args... args);

        /**
         * Remove the first element under given key.
         * @param key The key of the element.
         */
        bool remove(const KeyT &key);

        /**
         * Get statistics about this tree.
         * @return Statistics structure.
         */
        const StatsT &stats() const
        { return mStatistics; }

#ifndef NDEBUG
        /**
         * Print content of this tree,
         * used for debugging.
         * @param out Output stream.
         */
        void debugPrint(std::ostream &out) const;

        /**
         * Print stats about this tree.
         * @param out Output stream.
         */
        void debugPrintStats(std::ostream &out) const;

        /**
         * Verify attributes of this tree.
         * Works correctly only when the debugging is enabled.
         */
        void verify() const;
#endif

        /**
         * Get iterator to the first element.
         * @return Iterator to the first element.
         */
        Iterator begin();

        /**
         * Get the end iterator for the begin iterator.
         * @return End iterator.
         */
        Iterator end();

        /**
         * Get the iterator for the last element.
         * @return Iterator to the last element.
         */
        Iterator rBegin();

        /**
         * Get the end iterator for the rBegin iterator.
         * @return End iterator for the rBegin.
         */
        Iterator rEnd();
    private:
#ifndef NDEBUG
        void debugPrintNodeGroup(std::ostream &out, const NodeGroupT* ng, bool last) const;
        void debugPrintNode(std::ostream &out, const NodeT &n, bool last) const;
        void debugPrintLeafGroup(std::ostream &out, const LeafGroupT* lg) const;
        void debugPrintLeaf(std::ostream &out, const LeafT &l) const;
        void debugPrintPath(std::ostream &out, const void *first, OffsetT firstSlot,
                            const void *second, OffsetT secondSlot = 0) const;
#endif
        // Helper functions for accessing data inside pairs.

        /// Get the node.
        static NodeT &getNode(NodeGroupIndexPair pair)
        { return pair.first->node(pair.second); }
        /// Get the node.
        static const NodeT &getNode(ConstNodeGroupIndexPair pair)
        { return pair.first->node(pair.second); }

        /// Get the leaf.
        static LeafT &getLeaf(LeafGroupIndexPair pair)
        { return pair.first->node(pair.second); }
        /// Get the leaf.
        static const LeafT &getLeaf(ConstLeafGroupIndexPair pair)
        { return pair.first->node(pair.second); }

        /**
         * Post-order tree traversal.
         * @param root Root of the traversed tree.
         * @param height Height of the tree.
         * @param fun Functor crawler.
         */
        template <typename RootT,
                  typename FunctorT>
        void travPostOrder(RootT root,
                           u64 height,
                           FunctorT &fun) const;

        /**
         * Reset this structure to default state.
         */
        void reset();

        /**
         * Check, if there is already a first LeafGroup
         * created.
         * If not, create one.
         */
        void checkCreateFirst();

        /**
         * Create a root NodeGroup with with a single Node inside.
         * @return The new root NodeGroup.
         */
        NodeGroupT *createRootNode();

        /**
         * Allocate a new NodeGroup using the
         * allocator.
         * @param locality Locality information
         *   passed to the allocator.
         * @return Returns ptr returned from
         *   the allocator.
         */
        NodeGroupT *newNodeGroup(const NodeGroupT *locality = nullptr);

        /**
         * Deallocate given NodeGroup ptr.
         * @param ptr Pointer to memory allocated
         *   by NodeGroup allocator.
         */
        void delNodeGroup(NodeGroupT *ptr);


        /**
         * Allocate a new LeafGroup using the
         * allocator.
         * @param locality Locality information
         *   passed to the allocator.
         * @return Returns ptr returned from
         *   the allocator.
         */
        LeafGroupT *newLeafGroup(const LeafGroupT *locality = nullptr);

        /**
         * Deallocate given LeafGroup ptr.
         * @param ptr Pointer to memory allocated
         *   by LeafGroup allocator.
         */
        void delLeafGroup(LeafGroupT *ptr);

        friend struct GroupDeleter;
        /// Helper class for deleting Groups using tree traversal.
        struct GroupDeleter
        {
            GroupDeleter(BPTree<Config> *tree) :
            mTree(tree)
                { }
            void operator()(ChildGroupPtr grp, u64 height)
            {
                if (height != mTree->mHeight)
                {
                    mTree->delNodeGroup(grp.nodeGroup);
                }
                else
                {
                    mTree->delLeafGroup(grp.leafGroup);
                }
            }
            BPTree<Config> *mTree{nullptr};
        };

        /**
         * Free all of the Leafs and Nodes of given
         * tree.
         * @param root Root of the tree.
         * @param height Height of the given tree.
         */
        void freeTree(ChildGroupPtr root, u64 height);

        /**
         * Find the first data element for
         * given key.
         * @param key The searched key.
         * @return Iterator to the first found element.
         *   If no elements were found, the iterator is invalid.
         */
        Iterator findInt(const KeyT &key);

        /**
         * Find the leaf that should contain the given key.
         * @param key Searched key.
         * @return Returns leaf which should contain the key.
         */
        LeafGroupIndexPair findLeaf(const KeyT &key);
        ConstLeafGroupIndexPair findLeaf(const KeyT &key) const;

        /**
         * Find element inside a leaf.
         * @param key Key of the searched element.
         * @param leaf The searched leaf.
         * @return Returns iterator, if the element was not found, the
         *   returned iterator is invalid.
         */
        Iterator findElement(const KeyT &key,
                             LeafGroupIndexPair leaf);
        ConstIterator findElement(const KeyT &key,
                                  ConstLeafGroupIndexPair leaf) const;


        /**
         * Find first element for given key, or create
         * it, if it doesn't exist yet.
         * @param key Key of the element.
         * @param created This variable is set to true,
         *   if the element was created and the Data memory
         *   needs to be constructed. Else it is set to false.
         * @return Returns iterator to the element.
         */
        Iterator findCreateElement(const KeyT &key,
                                   bool &created);
        /// Record for the taken path.
        struct NodeRecord
        {
            // Pointer to the NodeGroup.
            NodeGroupT *first;
            // Offset within the NodeGroup = Node.
            OffsetT second;
            // Offset within the Node = Key
            OffsetT third;

            NodeRecord() { };
        };

        /**
         * Find first element for given key, and remove
         * it. If it doesn't exist, nothing happens.
         * @param key Key of the element.
         * @return Returns true, if an element has been
         *   removed.
         */
        bool findRemoveElement(const KeyT &key);
        /// Operation types, used when removing element.
        enum class RemOp
        {
            None,
            Balance,
            Merge,
        };
        /**
         * Helper method for choosing the best neighbour
         * for merge/balance operations.
         * @param group Group, to search for neighbour in.
         * @param slot Slot of the Node/Leaf.
         * @param nIndex Will be set to the index of the neighbour.
         * @return Returns the index for the Node/Leaf,
         *   which is more to the left.
         */
        template <typename GroupT>
        OffsetT remChooseNeighbour(GroupT *group, OffsetT slot, OffsetT &nIndex);

        /**
         * Helpre method for fixing *Groups under minimal
         * capacity.
         * rec will be unchanged.
         * @param group Group to fix.
         * @param rec Record about for the Node above.
         * @param grouOp Will be set to the operation performed.
         * @return Returns the index of the chosen neighbour.
         */
        template <typename GroupT>
        OffsetT remGroupMinCap(GroupT *group, NodeRecord &rec, RemOp &groupOp);

        /**
         * Find the right-most LeafGroup for given Node.
         * @param node Node to search through.
         * @param nodeHeight Height of the given Node.
         * @param maxHeight Total height of the tree.
         * @return Returns ptr to the right-most LeafGroup.
         */
        LeafGroupT *remFindRightMostLeafGroup(NodeT &node, u64 nodeHeight, u64 maxHeight);

        /**
         * Find the left-most LeafGroup for given Node.
         * @param node Node to search through.
         * @param nodeHeight Height of the given Node.
         * @param maxHeight Total height of the tree.
         * @return Returns ptr to the right-most LeafGroup.
         */
        LeafGroupT *remFindLeftMostLeafGroup(NodeT &node, u64 nodeHeight, u64 maxHeight);

        /**
         * Current height of the tree.
         * If the height is 0, then the root is a leaf.
         */
        u64 mHeight{0};

        /**
         * Root of the tree.
         * If the height == 0, then it points
         * to a LeafGroup with 1 leaf.
         * Else it points to a NodeGroup, where
         * the first node is the root node
         * of this tree.
         */
        ChildGroupPtr mRoot{nullptr};

        /**
         * Start of the LeafGroup linked list.
         * If the height == 0, then this
         * list has length 1.
         */
        LeafGroupT *mFirstLeafGroup{nullptr};

        /**
         * End of the LeafGroup linked list.
         * If the height == 0, then this
         * list has length 1.
         */
        LeafGroupT *mLastLeafGroup{nullptr};

        /// Statistics about this tree.
        StatsT mStatistics;

        /// Allocator used for allocating LeafGroups.
        LeafGroupAllocT mLeafGroupAllocator;

        /// Allocator used for allocating NodeGroups.
        NodeGroupAllocT mNodeGroupAllocator;
    protected:
    };

#ifndef NDEBUG

    template <typename CFG>
    void BPTree<CFG>::debugPrintStats(std::ostream &out) const
    {
        out << "Node branching factor : " << NodeT::N << std::endl;
        out << "Node branching factor calc : " << CFG::NODE_BRANCHING_FACTOR_CALC << std::endl;
        out << "Node : " << sizeof(NodeT) << std::endl;
        out << "Leaf branching factor : " << LeafT::M << std::endl;
        out << "Leaf branching factor calc : " << CFG::LEAF_BRANCHING_FACTOR_CALC << std::endl;
        out << "Leaf : " << sizeof(LeafT) << std::endl;
        out << "NodeGroup : " << sizeof(NodeGroupT) << std::endl;
        out << "LeafGroup : " << sizeof(LeafGroupT) << std::endl;

        out << "Tree statistics : " << std::endl;
        out << "Height : " << mHeight << std::endl;
        out << "NodeGroups : " << stats().numNodeGroups << std::endl;
        out << "Nodes : " << stats().numNodes << std::endl;
        out << "FilledNodeKeys : " << stats().numNodesFilledKeys << std::endl;
        out << "LeafGroups : " << stats().numLeafGroups << std::endl;
        out << "Leafs : " << stats().numLeafs << std::endl;
        out << "FilledLeafKeys : " << stats().numLeafsFilledKeys << std::endl;
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrint(std::ostream &out) const
    {
        out <<
                  "digraph g { \n"
                  "graph [ \n"
                  "rankdir = LR \n"
                  //"overlap=prism, overlap_scaling=0.01, ratio=0.7 \n"
                  "];\n"
                  "node [ \n"
                  "fontsize = \"16\" \n"
                  "shape = \"ellipse\" \n"
                  "]; \n"
                  "edge [ \n"
                  "]; \n";

        if (mRoot.nodeGroup == nullptr)
        {
            // Do nothing...
        }
        else if (mHeight == 0)
        { // There is only a single LeafGroup.
            debugPrintLeafGroup(out, mRoot.leafGroup);
        }
        else
        {
            mem::List<NodeGroupIndexPair> path(mHeight);
            ChildGroupPtr child{mRoot};
            OffsetT offset{0};
            u64 currHeight{0};

            path[0].first = child.nodeGroup;
            path[0].second = offset;

            while (true)
            {
                if (currHeight < mHeight - 1)
                { // We are on a node level.
                    child.nodeGroup = path[currHeight].first;
                    offset = path[currHeight].second;

                    if (offset < child.nodeGroup->slotsUsed())
                    { // More children available.
                        // Move to the next child.
                        path[currHeight + 1].first =
                            child.nodeGroup->node(offset).childGroup().nodeGroup;
                        path[currHeight + 1].second = 0;

                        path[currHeight].second++;
                        currHeight++;
                    }
                    else
                    { // No more children.
                        debugPrintNodeGroup(out, child.nodeGroup, false);

                        /*
                        for (OffsetT iii = 0; iii < child.nodeGroup->slotsUsed(); ++iii)
                        {
                            LeafGroupT *ptr{child.nodeGroup->node(iii).childGroup().leafGroup};

                            debugPrintPath(out, child.nodeGroup, iii, ptr);
                        }
                         */

                        if (currHeight > 0)
                        {
                            currHeight--;
                        }
                        else
                        { // We are finished.
                            break;
                        }
                    }
                }
                else
                { // We are one level above leaf level.
                    child.nodeGroup = path[currHeight].first;
                    offset = path[currHeight].second;

                    debugPrintNodeGroup(out, child.nodeGroup, true);

                    for (OffsetT iii = 0; iii < child.nodeGroup->slotsUsed(); ++iii)
                    {
                        LeafGroupT *ptr{child.nodeGroup->node(iii).childGroup().leafGroup};
                        std::cout << static_cast<void*>(ptr) << std::endl;

                        debugPrintLeafGroup(out, ptr);
                        //debugPrintPath(out, child.nodeGroup, iii, ptr);
                    }

                    if (currHeight > 0)
                    {
                        currHeight--;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        out << "}" << std::endl;
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrintNodeGroup(std::ostream &out, const NodeGroupT* ng, bool last) const
    {
        //out << "\"" << static_cast<const void*>(ng) << "\" [\n label = \"";
        out << "subgraph \"ng" << static_cast<const void*>(ng) << "\" {\n";

        for (OffsetT iii = 0; iii < ng->slotsUsed(); ++iii)
        {
            //out << "<f" << iii << "> ";
            debugPrintNode(out, ng->node(iii), last);
            //out << " | ";
        }

        /*
        out << "\"\n" <<
            "shape = \"record\"\n"
            "];\n";
            */
        out <<
            "label=\"ng#" << static_cast<const void*>(ng) <<
            "\";\nstyle=filled;\n"
            "color=white;\n"
            "node [style=filled, color=lightgrey];\n"
            "}\n";
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrintNode(std::ostream &out, const NodeT &n, bool last) const
    {
        out << "\"" << static_cast<const void*>(&n) << "\" [\n " << "label = \"";
        for (OffsetT iii = 0; iii < n.slotsUsed(); ++iii)
        {
            out << "<f" << iii << "> ";
            out << n.key(iii);
            out << " | ";
        }
        out << "<f" << n.slotsUsed() << "> ";
        out << "\"\n" <<
            "shape = \"record\"\n"
            "];\n";

        if (!last)
        {
            for (OffsetT iii = 0; iii <= n.slotsUsed(); ++iii)
            {
                debugPrintPath(out, &n, iii, &n.childGroup().nodeGroup->node(iii));
            }
        }
        else
        {
            for (OffsetT iii = 0; iii <= n.slotsUsed(); ++iii)
            {
                debugPrintPath(out, &n, iii, &n.childGroup().leafGroup->node(iii));
            }
        }
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrintLeafGroup(std::ostream &out, const LeafGroupT* lg) const
    {
        //out << "\"" << static_cast<const void*>(lg) << "\" [\n " << "label = \"";
        out << "subgraph \"lg" << static_cast<const void*>(lg) << "\" {\n";

        for (OffsetT iii = 0; iii < lg->slotsUsed(); ++iii)
        {
            //out << "<f" << iii << "> ";
            debugPrintLeaf(out, lg->node(iii));
            //out << " | ";
        }

        /*
        out << "\"\n" <<
            "shape = \"record\"\n"
            "];\n";
            */
        out <<
            "label=\"lg#" << static_cast<const void*>(lg) <<
            "\";\nstyle=filled;\n"
            "color=lightgrey;\n"
            "node [style=filled, color=white];\n"
            "}\n";
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrintLeaf(std::ostream &out, const LeafT &l) const
    {
        out << "\"" << static_cast<const void*>(&l) <<
               "\" [\n "
               //"rankdir = LR \n "
               "label = \"";
        for (OffsetT iii = 0; iii < l.slotsUsed(); ++iii)
        {
            out << "<f" << iii << "> ";
            out << l.key(iii) << " " << l.data(iii);
            out << " | ";
        }
        out << "\"\n" <<
            "shape = \"record\"\n"
            "];\n";
    }

    template <typename CFG>
    void BPTree<CFG>::debugPrintPath(std::ostream &out, const void *first,
                                     OffsetT firstSlot, const void *second,
                                     OffsetT secondSlot) const
    {
        out << "\"" << first << "\"" <<
               ":f" << firstSlot <<
               " -> " <<
               "\"" << second << "\"" <<
               ":f" << secondSlot << ";\n";
    }

    template <typename CFG>
    void BPTree<CFG>::verify() const
    {
        if (!mRoot.nodeGroup || !mFirstLeafGroup || !mLastLeafGroup)
        { // Tree is unused.
            BPTreeAssert(!mRoot.nodeGroup);
            BPTreeAssert(!mFirstLeafGroup);
            BPTreeAssert(!mLastLeafGroup);

            BPTreeAssert(mStatistics.empty());

            return;
        }

        if (mHeight == 0)
        { // There is only a single Leaf as the root of this tree.
            BPTreeAssert(mRoot.leafGroup == mFirstLeafGroup);
            BPTreeAssert(mFirstLeafGroup == mLastLeafGroup);

            BPTreeAssert(mFirstLeafGroup->prev() == nullptr);
            BPTreeAssert(mFirstLeafGroup->next() == nullptr);
            BPTreeAssert(mFirstLeafGroup->slotsUsed() == 1);

            LeafT &leaf{mFirstLeafGroup->node(0)};
            BPTreeAssert(mStatistics.numNodeGroups == 0);
            BPTreeAssert(mStatistics.numNodes == 0);
            BPTreeAssert(mStatistics.numNodesFilledKeys == 0);
            BPTreeAssert(mStatistics.numLeafGroups == 1);
            BPTreeAssert(mStatistics.numLeafs == 1);
            BPTreeAssert(mStatistics.numLeafsFilledKeys == leaf.slotsUsed());

            return;
        }

        // Root group should contain only a single Node.
        BPTreeAssert(mRoot.nodeGroup->slotsUsed() == 1);

        struct GroupCrawler
        {
            GroupCrawler(const NodeGroupT *root,
                         const LeafGroupT *first,
                         const LeafGroupT *last,
                         u64 height) :
                mRootNodeGroup{root},
                mHeight{height}
            {
                BPTreeAssert(first && last);

                mLeafGroupNext = first;
                checkLGCapacity = first != last;

                BPTreeAssert(first->prev() == nullptr);
                BPTreeAssert(last->next() == nullptr);
            }

            void operator()(ConstChildGroupPtr grp, u64 height)
            {
                if (height < mHeight)
                { // Inner/Outer NodeGroup
                    const NodeGroupT *ng{grp.nodeGroup};

                    numNodeGroups++;
                    numNodes += ng->slotsUsed();

                    if (ng == mRootNodeGroup)
                    { // We are testing the root NodeGroup.
                        BPTreeAssert(ng->slotsUsed() == 1);
                    }

                    if (height > 1)
                    {
                        BPTreeAssert(!ng->underMinimalCapacity());
                    }

                    const NodeT *beg{ng->nodeBegin()};
                    const NodeT *end{ng->nodeEnd()};
                    for (const NodeT *it = beg; it != end; ++it)
                    {
                        numNodesFilledKeys += it->slotsUsed();

                        if (ng != mRootNodeGroup)
                        {
                            BPTreeAssert(!it->underMinimalCapacity());
                        }

                        if (height < mHeight - 1)
                        { // Inner NodeGroup
                            const NodeGroupT *nodeGroup{it->childGroup().nodeGroup};

                            BPTreeAssert(nodeGroup != nullptr);
                            BPTreeAssert(it->slotsUsed() + 1 == nodeGroup->slotsUsed());

                            OffsetT kBeg{0};
                            OffsetT kEnd{it->slotsUsed()};


                            for (OffsetT kIdx = kBeg; kIdx != kEnd; ++kIdx)
                            { // Check first key in each left-most Leaf.
                                // key(idx) == left-most key in the branch.
                                ConstChildGroupPtr child{it->childGroup()};
                                u64 currHeight{height + 1};

                                // Find the Leaf.
                                child = child.nodeGroup->node(kIdx + 1).childGroup();
                                currHeight++;

                                while (currHeight < mHeight)
                                {
                                    child = child.nodeGroup->node(0).childGroup();
                                    currHeight++;
                                }

                                const LeafT *leaf{&child.leafGroup->node(0)};
                                BPTreeAssert(leaf->key(0) == it->key(kIdx));
                            }
                        }
                        else
                        { // Outer NodeGroup
                            const LeafGroupT *leafGroup{it->childGroup().leafGroup};

                            BPTreeAssert(leafGroup != nullptr);
                            BPTreeAssert(it->slotsUsed() + 1 == leafGroup->slotsUsed());

                            OffsetT kBeg{0};
                            OffsetT kEnd{it->slotsUsed()};

                            for (OffsetT kIdx = kBeg; kIdx != kEnd; ++kIdx)
                            { // Check first key in each Leaf.
                                BPTreeAssert(it->key(kIdx) == leafGroup->node(kIdx + 1).key(0));
                            }
                        }
                    }
                }
                else
                { // LeafGroup
                    const LeafGroupT *lg{grp.leafGroup};

                    numLeafGroups++;
                    numLeafs += lg->slotsUsed();

                    const LeafT *beg{lg->nodeBegin()};
                    const LeafT *end{lg->nodeEnd()};
                    for (const LeafT *it = beg; it != end; ++it)
                    {
                        numLeafsFilledKeys += it->slotsUsed();

                        BPTreeAssert(!it->underMinimalCapacity());
                    }

                    BPTreeAssert(lg->prev() == mLeafGroupBefore);
                    BPTreeAssert(lg == mLeafGroupNext);

                    BPTreeAssert(!checkLGCapacity || !lg->underMinimalCapacity());

                    mLeafGroupBefore = lg;
                    mLeafGroupNext = lg->next();
                }
            }

            void operator()(ChildGroupPtr*, OffsetT)
            {
                BPTreeAssert(false);
            }

            u64 numNodeGroups{0};
            u64 numNodes{0};
            u64 numNodesFilledKeys{0};

            u64 numLeafGroups{0};
            u64 numLeafs{0};
            u64 numLeafsFilledKeys{0};

        private:
            const NodeGroupT *mRootNodeGroup{nullptr};
            const LeafGroupT *mLeafGroupBefore{nullptr};
            const LeafGroupT *mLeafGroupNext{nullptr};
            bool checkLGCapacity{false};
            u64 mHeight{0};
        } crawler(mRoot.nodeGroup, mFirstLeafGroup, mLastLeafGroup, mHeight);

        ConstChildGroupPtr root{mRoot};

        /**
         * Traverse the tree, check Nodes on
         * the way and build a list of LeafGroups.
         */
        travPostOrder(root, mHeight, crawler);

        // Check, if the counts are right.
        BPTreeAssert(mStatistics.numNodeGroups == crawler.numNodeGroups);
        BPTreeAssert(mStatistics.numNodes == crawler.numNodes);
        BPTreeAssert(mStatistics.numNodesFilledKeys == crawler.numNodesFilledKeys);

        BPTreeAssert(mStatistics.numLeafGroups == crawler.numLeafGroups);
        BPTreeAssert(mStatistics.numLeafs == crawler.numLeafs);
        BPTreeAssert(mStatistics.numLeafsFilledKeys == crawler.numLeafsFilledKeys);
    }

#endif

    template <typename Config>
    template <typename... Args>
    inline
    auto BPTree<Config>::insert(const KeyT &key, Args... args)
    -> Iterator
    {
        checkCreateFirst();

        bool created{false};
        Iterator it{findCreateElement(key, created)};
        EleT *ptr{&((*it).second)};
        if (!created)
        {
            ptr->~EleT();
        }

        new (ptr) EleT(std::forward<Args...>(args)...);

        return it;
    }

    template <typename Config>
    inline
    bool BPTree<Config>::remove(const KeyT &key)
    {
        return findRemoveElement(key);
    }

    template <typename Config>
    template <typename... Args>
    inline
    auto BPTree<Config>::searchOrInsert(const KeyT &key, Args... args)
    -> Iterator
    {
        checkCreateFirst();

        bool created{false};
        Iterator it{findCreateElement(key, mRoot, mHeight, created)};
        EleT *ptr{&((*it).second)};
        if (created)
        {
            new (ptr) EleT(std::forward<Args...>(args)...);
        }

        return it;
    }

    template <typename Config>
    inline
    auto BPTree<Config>::begin()
    -> Iterator
    {
        return Iterator(mFirstLeafGroup, 0, 0);
    }

    template <typename Config>
    inline
    auto BPTree<Config>::end()
    -> Iterator
    {
        return Iterator();
    }

    template <typename Config>
    inline
    auto BPTree<Config>::rBegin()
    -> Iterator
    {
        OffsetT lastLeafSlot{mLastLeafGroup->slotsUsed() - 1};
        LeafT &lastLeaf{mLastLeafGroup->leaf(lastLeafSlot)};
        return Iterator(mLastLeafGroup, lastLeafSlot, lastLeaf.slotsUsed() - 1);
    }

    template <typename Config>
    inline
    auto BPTree<Config>::rEnd()
    -> Iterator
    {
        return Iterator();
    }

    template <typename Config>
    BPTree<Config>::BPTree() :
        mHeight{0},
        mRoot{nullptr},
        mFirstLeafGroup{nullptr},
        mLastLeafGroup{nullptr}
    {
        reset();
    }

    template <typename Config>
    BPTree<Config>::~BPTree()
    {
        freeTree(mRoot, mHeight);
    }

    template <typename Config>
    template <typename RootT,
              typename FunctorT>
    void BPTree<Config>::travPostOrder(RootT root,
                                       u64 height,
                                       FunctorT &fun) const
    {
        if (root.nodeGroup == nullptr)
        {
            return;
        }
        else if (height == 0)
        { // There is only a single LeafGroup.
            fun(root, 0);
        }
        else if (height == 1)
        { // A single node level, no need for keeping the path.
            auto *nodeGroup{root.nodeGroup};
            auto *beg{nodeGroup->nodeBegin()};
            auto *end{nodeGroup->nodeEnd()};

            for (auto *it = beg; it != end; ++it)
            { // Go through all the nodes.
                fun(RootT{it->childGroup().group}, 1);
            }

            fun(root, 0);
        }
        else
        { // We have a tree with root Node inside the root NodeGroup.
            // Path accumulator.
            mem::List<std::pair<decltype(root.nodeGroup), OffsetT>> path(height);
            auto *nodeGroup{root.nodeGroup};
            // Offset within the NodeGroup
            OffsetT offset{0};
            u64 currHeight{0};

            path[0].first = nodeGroup;
            path[0].second = offset;

            while (true)
            {
                if (currHeight < height - 1)
                { // We are on an inner Node level.
                    nodeGroup = path[currHeight].first;
                    offset = path[currHeight].second;

                    if (offset < nodeGroup->slotsUsed())
                    { // More children available.
                        // Move to the next child.
                        path[currHeight + 1].first = nodeGroup->node(offset).childGroup().nodeGroup;
                        path[currHeight + 1].second = 0;

                        path[currHeight].second++;
                        currHeight++;
                    }
                    else
                    { // No more children.
                        fun(RootT(nodeGroup), currHeight);

                        if (currHeight > 0)
                        { // We did not just delete the root.
                            currHeight--;
                        }
                        else
                        { // We are finished.
                            break;
                        }
                    }
                }
                else
                { // We are one level above leaf level.
                    nodeGroup = path[currHeight].first;

                    auto *beg{nodeGroup->nodeBegin()};
                    auto *end{nodeGroup->nodeEnd()};

                    for (auto *it = beg; it != end; ++it)
                    { // Go through all the nodes.
                        fun(RootT(it->childGroup()), height);
                    }

                    RootT ptr{nullptr};
                    ptr.nodeGroup = nodeGroup;
                    fun(ptr, currHeight);

                    currHeight--;
                }
            }
        }
    }

    template <typename Config>
    inline
    void BPTree<Config>::reset()
    {
        // Free the existing tree.
        freeTree(mRoot, mHeight);

        // Reset the internal structure.
        mHeight = 0;
        mRoot.nodeGroup = nullptr;
        mFirstLeafGroup = nullptr;
        mLastLeafGroup = nullptr;
        mStatistics.reset();
    }

    template <typename Config>
    inline
    void BPTree<Config>::checkCreateFirst()
    {
        if (mRoot.nodeGroup == nullptr)
        { // We need to create the first Leaf.
            // Create the first LeafGroup.
            mRoot.leafGroup = newLeafGroup(nullptr);
            mFirstLeafGroup = mRoot.leafGroup;
            mLastLeafGroup = mFirstLeafGroup;
            mHeight = 0;

            // Create the first Leaf.
            mFirstLeafGroup->createFirst();

            mStatistics.numLeafs++;
        }
    }
    template <typename Config>
    inline
    auto BPTree<Config>::createRootNode()
    -> NodeGroupT*
    {
        NodeGroupT *ptr{newNodeGroup(nullptr)};
        ptr->createFirst();

        return ptr;
    }

    template <typename Config>
    inline
    auto BPTree<Config>::newNodeGroup(const NodeGroupT *locality)
    -> NodeGroupT*
    {
        NodeGroupT *ptr{mNodeGroupAllocator.allocate(1, locality)};

        mNodeGroupAllocator.construct(ptr);

        mStatistics.numNodeGroups++;

        return ptr;
    }

    template <typename Config>
    inline
    void BPTree<Config>::delNodeGroup(NodeGroupT *ptr)
    {
        mNodeGroupAllocator.deallocate(ptr, 1);

        mStatistics.numNodeGroups--;
    }

    template <typename Config>
    inline
    auto BPTree<Config>::newLeafGroup(const LeafGroupT *locality)
    -> LeafGroupT*
    {
        LeafGroupT *ptr{mLeafGroupAllocator.allocate(1, locality)};

        mLeafGroupAllocator.construct(ptr);

        mStatistics.numLeafGroups++;

        return ptr;
    }

    template <typename Config>
    inline
    void BPTree<Config>::delLeafGroup(LeafGroupT *ptr)
    {
        mLeafGroupAllocator.deallocate(ptr, 1);

        mStatistics.numLeafGroups--;
    }

    template <typename Config>
    inline
    void BPTree<Config>::freeTree(ChildGroupPtr root, u64 height)
    {
        GroupDeleter groupDeleter(this);
        travPostOrder(root, height, groupDeleter);
    }

    template <typename Config>
    inline
    auto BPTree<Config>::findInt(const KeyT &key)
    -> Iterator
    {
        // Find the leaf, where given key should be located.
        LeafGroupIndexPair leaf{findLeaf(key)};

        // Try to locate the key inside the leaf.
        return findElement(key, leaf);
    }

    template <typename Config>
    inline
    auto BPTree<Config>::findLeaf(const KeyT &key)
    -> LeafGroupIndexPair
    {
        // Start with the root group.
        ChildGroupPtr group{mRoot};
        // Start with the given node position.
        OffsetT nodeSlot{0};
        // Start with the root.
        u64 currHeight{0};
        // The height of the leafs.
        const u64 maxHeight{mHeight};

        while (currHeight != maxHeight)
        { // While we are not at the leaf level.
            // Get the current node from its NodeGroup.
            NodeT &node{group.nodeGroup->node(nodeSlot)};

            // Get the child group of current node.
            group = node.childGroup();
            // Find the correct slot for given key.
            nodeSlot = node.find(key);

            // We moved one level up.
            currHeight++;
        }

        // group now contains a ptr to a LeafGroup.
        // nodeSlot now contains an offset in the LeafGroup.

        return LeafGroupIndexPair(group.leafGroup, nodeSlot);
    }

    template <typename Config>
    inline
    auto BPTree<Config>::findElement(const KeyT &key,
                                     LeafGroupIndexPair leaf)
    -> Iterator
    {
        LeafT &leafRef{getLeaf(leaf)};

        OffsetT index{leafRef.find(key)};

        if (leafRef.key(index) == key)
        { // We found it!
            return Iterator(leaf, index);
        }
        else
        { // Key not found, return invalid iterator.
            return Iterator();
        }
    }

    template <typename Config>
    auto BPTree<Config>::findCreateElement(const KeyT &key,
                                           bool &created)
    -> Iterator
    {
        // Start with the root group.
        ChildGroupPtr group{mRoot};
        // Start with the given node position.
        OffsetT nodeSlot{0};
        // Start with the root.
        u64 currHeight{0};
        // The height of the leafs.
        const u64 maxHeight{mHeight};

        // Array to hold the path to the leaf.
        //mem::List<NodeRecord> path(maxHeight);
        NodeRecord path[maxHeight];

        while (currHeight != maxHeight)
        { // Search for the leaf and remember the path taken.
            // Get the current record in the path array.
            NodeRecord &record{path[currHeight]};

            // Get the current node from its NodeGroup.
            NodeT &node{group.nodeGroup->node(nodeSlot)};

            // Fill out the record for current height.
            record.first = group.nodeGroup;
            record.second = nodeSlot;
            // Find the correct slot for given key.
            record.third = node.find(key);

            // Set the node slot for the next height.
            nodeSlot = record.third;

            // Get the child group for the next height.
            group = node.childGroup();

            // We moved one level up.
            currHeight++;
        }

        /**
         * Path is now recorded and:
         *   group contains LeafGroup
         *   nodeSlot contains leafSlot
         */

        LeafGroupT *leafGroup{group.leafGroup};
        OffsetT leafSlot{nodeSlot};
        LeafT &leaf{leafGroup->node(leafSlot)};
        OffsetT elementIndex{leaf.find(key)};

        // Check, if the element already exists.
        if (elementIndex < leaf.slotsUsed() &&
            leaf.key(elementIndex) == key)
        { // We found it!
            created = false;
            return Iterator(leafGroup, leafSlot, elementIndex);
        }

        // We did not find the element, we need to create it!
        created = true;

        /**
         * A) Create a new Element:
         *   1) If there is enough space in the
         *      Leaf, create it and we are done.
         *   2) Else continue with (B).
         * B) Create a new Leaf:
         *   1) If there is enough space in the
         *      LeafGroup, create it and go to (A).
         *   2) Else continue with (C).
         * C) Create a new LeafGroup:
         *   1) Create the new LeafGroup and go to (B).
         *
         * If we got to (A), nothing more needs to happen.
         * If we got to (B), we need to create a new key
         *   in the Node above.
         * If we got to (C), we need to create a new node
         *   in the NodeGroup above.
         */

        bool split{false};
        OffsetT splitSlot{0};
        KeyT newKey;

        bool groupSplit{false};
        //bool splitRight{false};
        ChildGroupPtr leftGroupPtr{nullptr};
        ChildGroupPtr rightGroupPtr{nullptr};

        if (!leaf.full())
        {
            // Insert the new element into the leaf.
            leafGroup->node(leafSlot).addKeyNoFind(key, elementIndex);
            mStatistics.numLeafsFilledKeys++;

            return Iterator(leafGroup, leafSlot, elementIndex);
        }
        else
        { // We need to split the Leaf.
            split = true;

            if (leafGroup->full())
            { // We need to split the LeafGroup.
                groupSplit = true;

                leftGroupPtr.leafGroup = leafGroup;

                // Create the right group.
                rightGroupPtr.leafGroup = newLeafGroup(leafGroup);

                // Split the LeafGroup.
                OffsetT middle{leafGroup->split(*rightGroupPtr.leafGroup, leafSlot)};

                // Set the pointers.
                rightGroupPtr.leafGroup->next() = leftGroupPtr.leafGroup->next();
                rightGroupPtr.leafGroup->prev() = leftGroupPtr.leafGroup;
                leftGroupPtr.leafGroup->next() = rightGroupPtr.leafGroup;

                // Fix the last LeafGroup ptr.
                if (leftGroupPtr.leafGroup == mLastLeafGroup)
                {
                    mLastLeafGroup = rightGroupPtr.leafGroup;
                }

                // Determine, which half contains the leaf.
                if (leafSlot < middle)
                { // Leaf is in the left LeafGroup.
                    splitSlot = leafSlot;
                    // splitRight = false;
                    // leafGroup = leftGroupPtr.leafGroup;
                }
                else
                {  // Leaf is in the right LeafGroup.
                    splitSlot = leafSlot - middle;
                    //splitRight = true;
                    leafGroup = rightGroupPtr.leafGroup;
                }
            }
            else
            { // There is enough space in the LeafGroup.
                // groupSplit = false;
                splitSlot = leafSlot;
                leftGroupPtr.leafGroup = leafGroup;
            }

            // Create the new Leaf.
            OffsetT newSplitSlot{leafGroup->makeSpace(splitSlot)};
            BPTreeAssert(newSplitSlot == splitSlot + 1);
            mStatistics.numLeafs++;

            // Split the Leaf.
            LeafT &leftLeaf{leafGroup->node(splitSlot)};
            LeafT &rightLeaf{leafGroup->node(splitSlot + 1)};

            OffsetT middle{leftLeaf.split(rightLeaf, elementIndex)};

            // Determine which half should we insert into.
            if (elementIndex < middle)
            { // Insert into the left half.
                leafSlot = splitSlot;
            }
            else
            { // Insert into the right half.
                leafSlot = splitSlot + 1;
                elementIndex -= middle;
            }
        }

        // Insert the new element into the leaf.
        leafGroup->node(leafSlot).addKeyNoFind(key, elementIndex);
        mStatistics.numLeafsFilledKeys++;

        // Get the left-most key.
        newKey = leafGroup->node(splitSlot + 1).key(0);

        if (maxHeight != 0)
        {
            currHeight = maxHeight - 1;

            while (split)
            {
                NodeRecord &rec{path[currHeight]};

                NodeGroupT *nodeGroup{rec.first};
                OffsetT nodeIndex{rec.second};
                OffsetT keyIndex{rec.third};

                ChildGroupPtr newLeftGroupPtr{nodeGroup};
                ChildGroupPtr newRightGroupPtr{nullptr};

                if (groupSplit)
                { // If the group below split, we need to split a node.
                    //newSplit = true;
                    split = true;

                    if (nodeGroup->full())
                    { // This group will have to split too.
                        groupSplit = true;

                        // Create a new NodeGroup.
                        newRightGroupPtr.nodeGroup = newNodeGroup(nodeGroup);

                        // Split this nodeGroup.
                        OffsetT middle{nodeGroup->split(*newRightGroupPtr.nodeGroup, nodeIndex)};

                        if (nodeIndex < middle)
                        { // The Node is in the left half.
                            splitSlot = nodeIndex;
                        }
                        else
                        { // The Node is in the right half.
                            splitSlot = nodeIndex - middle;
                            //newSplitRight = true;
                            nodeGroup = newRightGroupPtr.nodeGroup;
                        }
                    }
                    else
                    {
                        groupSplit = false;

                        splitSlot = nodeIndex;
                    }

                    // Create the new Node.
                    OffsetT makeSpaceTest{nodeGroup->makeSpace(splitSlot)};
                    mStatistics.numNodes++;
                    BPTreeAssert(makeSpaceTest == splitSlot + 1);

                    // Split the Node.
                    NodeT &leftNode{nodeGroup->node(splitSlot)};
                    NodeT &rightNode{nodeGroup->node(splitSlot + 1)};

                    if (keyIndex == leftNode.middleOffset())
                    { // We already have the middle key, no need to insert it.
                        //newKey = newKey;
                        OffsetT newMiddle{leftNode.split(rightNode, true, keyIndex)};
                    }
                    else
                    { // We do not have the middle key.
                        OffsetT middle{leftNode.split(rightNode, false, keyIndex)};

                        if (keyIndex > middle)
                        {
                            nodeSlot = splitSlot + 1;
                            keyIndex -= middle;
                        }
                        else
                        {
                            nodeSlot = splitSlot;
                        }

                        BPTreeAssert(nodeSlot < nodeGroup->slotsUsed());
                        // Insert the new key.
                        nodeGroup->node(nodeSlot).addKeyNoFind(newKey, keyIndex);
                        //mStatistics.numNodesFilledKeys++;
                        newKey = leftNode.key(middle);
                    }

                    // Set the pointers.
                    leftNode.childGroup() = leftGroupPtr;
                    rightNode.childGroup() = rightGroupPtr;
                }
                else
                {
                    split = false;

                    nodeSlot = nodeIndex;

                    BPTreeAssert(nodeSlot < nodeGroup->slotsUsed());
                    // Insert the new key.
                    nodeGroup->node(nodeSlot).addKeyNoFind(newKey, keyIndex);
                    mStatistics.numNodesFilledKeys++;
                }

                leftGroupPtr = newLeftGroupPtr;
                rightGroupPtr = newRightGroupPtr;

                if (currHeight > 0)
                {
                    currHeight--;
                }
                else
                {
                    break;
                }
            }
        }

        BPTreeAssert(!groupSplit);

        // If there was a Node split in the root, we need to make a new one.
        if (split)
        {
            NodeGroupT* newRoot{createRootNode()};
            mRoot.nodeGroup = newRoot;
            mHeight++;

            NodeT &newRootNode{newRoot->node(0)};
            newRootNode.addKeyNoFind(newKey, 0);
            newRootNode.childGroup() = leftGroupPtr;

            mStatistics.numNodes++;
            mStatistics.numNodesFilledKeys++;
        }

        return Iterator(leafGroup, leafSlot, elementIndex);
    }

    template <typename Config>
    bool BPTree<Config>::findRemoveElement(const KeyT &key)
    {
        if (mRoot.group == nullptr)
        {
            return false;
        }

        /**
         * A) Find the path to the element to be removed.
         * B) Remove the element.
         * C) Restore the tree invariants.
         *   1a) Check, if the left/right neighbouring Leafs
         *     have more than minimal number of keys, if they
         *     do, then redistribute these elements between
         *     the 2 Leafs.
         *   1b) Check, if the left/right neighbouring Leafs
         *     have exactly minimal number of keys, if they
         *     do, then combine 2 Leafs into one.
         *   2) Propagate the change upwards using the path
         *     we found in (A) .
         */

        // Start with the root group.
        ChildGroupPtr group{mRoot};
        // Start with the given node position.
        OffsetT nodeSlot{0};
        // Start with the root.
        u64 currHeight{0};
        // The height of the leafs.
        const u64 maxHeight{mHeight};

        // Array to hold the path to the leaf.
        //mem::List<NodeRecord> path(maxHeight);
        NodeRecord path[maxHeight];

        while (currHeight != maxHeight)
        { // Search for the leaf and remember the path taken.
            // Get the current record in the path array.
            NodeRecord &record{path[currHeight]};

            // Get the current node from its NodeGroup.
            NodeT &node{group.nodeGroup->node(nodeSlot)};

            // Fill out the record for current height.
            record.first = group.nodeGroup;
            record.second = nodeSlot;
            // Find the correct slot for given key.
            record.third = node.find(key);

            // Set the node slot for the next height.
            nodeSlot = record.third;

            // Get the child group for the next height.
            group = node.childGroup();

            // We moved one level up.
            currHeight++;
        }

        /**
         * Path is now recorded and:
         *   group contains LeafGroup
         *   nodeSlot contains leafSlot
         */

        LeafGroupT *leafGroup{group.leafGroup};
        OffsetT leafSlot{nodeSlot};
        const OffsetT lastLeafSlot{leafGroup->slotsUsed() - 1};
        LeafT *leaf{&leafGroup->node(leafSlot)};
        OffsetT elementIndex{leaf->find(key)};

        // Check, if the element doesn't exists.
        if (elementIndex >= leaf->slotsUsed() ||
            leaf->key(elementIndex) != key)
        { // Element doesn't exist, nothing to remove.
            return false;
        }

        leaf->removeKeyNoFind(elementIndex);
        mStatistics.numLeafsFilledKeys--;

        if (mHeight == 0 && mRoot.leafGroup->node(0).empty())
        { // The current root is a LeafGroup and it's empty.
            BPTreeAssert(mRoot.leafGroup == mFirstLeafGroup);
            BPTreeAssert(mFirstLeafGroup == mLastLeafGroup);

            delLeafGroup(mFirstLeafGroup);

            mRoot.group = nullptr;
            mFirstLeafGroup = nullptr;
            mLastLeafGroup = nullptr;

            mStatistics.numLeafs--;
            mHeight--;

            return true;
        }

        // If elementIndex is 0, that means, we need to repair key values.
        if ((!leaf->underMinimalCapacity() && elementIndex != 0) || maxHeight == 0)
        { // No need to rearrange Leafs.
            return true;
        }

        // We need to restore trees invariants.
        BPTreeAssert(maxHeight != 0);
        currHeight--;
        BPTreeAssert(currHeight == maxHeight - 1);

        // If the deleted element was on 0th position, we need to correct it.
        // The new key is at leaf.key(0)
        bool repairKey{elementIndex == 0};

        // What operation was performed on the Node.
        RemOp op{RemOp::None};
        // What operation was performed on the *Group.
        RemOp groupOp{RemOp::None};
        // Which neighbour group was chosen.
        OffsetT groupOpOffset{0};

        // Index of the Node/Leaf on the left.
        OffsetT leftIndex{0};
        // Index of the chosen Node/Leaf.
        OffsetT chosenIndex{0};

        BPTreeAssert(!leafGroup->underMinimalCapacity() || mHeight == 1);

        if (leaf->underMinimalCapacity())
        { // Merge or rebalance is needed.
            NodeRecord rec{path[currHeight]};
            leftIndex = remChooseNeighbour(leafGroup, leafSlot, chosenIndex);

            if (leafGroup->node(chosenIndex).atMinimalCapacity())
            {
                op = RemOp::Merge;
                leafGroup->merge(leafSlot, chosenIndex);
                mStatistics.numLeafs--;

                OffsetT newLeafOffset{leftIndex};

                /**
                 * If there will be Node merging/balancing above, we
                 * should merge/balance the LeafGroups now.
                 * Special scenario, when height of the tree
                 * is <= 1, in which case there are no
                 * other LeafGroups to merge/balance with.
                 */
                if (leafGroup->underMinimalCapacity() && mHeight > 1)
                {
                    OffsetT numLeafs{leafGroup->slotsUsed()};
                    groupOpOffset = remGroupMinCap(leafGroup, path[currHeight], groupOp);
                    BPTreeAssert(groupOp != RemOp::None);

                    if (groupOp == RemOp::Merge)
                    {
                        LeafGroupT *deletedLeafGroup{nullptr};
                        NodeT *affectedNode{nullptr};
                        OffsetT newKeyIndex{0};

                        if (groupOpOffset < path[currHeight].second)
                        { // Merged into the LeafGroup on groupOpOffset.
                            deletedLeafGroup = leafGroup;
                            affectedNode = &rec.first->node(groupOpOffset);
                            leafGroup = affectedNode->childGroup().leafGroup;

                            newKeyIndex = leafGroup->slotsUsed() - numLeafs;
                            newLeafOffset = newKeyIndex + leftIndex;
                        }
                        else
                        { // Merged into the current LeafGroup.
                            affectedNode = &rec.first->node(rec.second);
                            deletedLeafGroup = rec.first->
                                node(groupOpOffset).childGroup().leafGroup;

                            newKeyIndex = numLeafs;
                            //newLeafOffset = leftIndex;
                        }

                        // Add the new key to the affectedNode.
                        affectedNode->addKeyNoFind(leafGroup->node(newKeyIndex).key(0),
                                                   affectedNode->slotsUsed());
                        mStatistics.numNodesFilledKeys++;

                        // Fix the prev and next pointers.
                        leafGroup->next() = deletedLeafGroup->next();
                        if (leafGroup->next() == nullptr)
                        { // We deleted the last LeafGroup.
                            BPTreeAssert(mLastLeafGroup == deletedLeafGroup);
                            mLastLeafGroup = leafGroup;
                        }
                        else
                        {
                            leafGroup->next()->prev() = leafGroup;
                        }

                        // Delete the LeafGroup.
                        delLeafGroup(deletedLeafGroup);
                    }
                    else
                    {
                        if (groupOpOffset < path[currHeight].second)
                        { // Balanced from the left.
                            newLeafOffset = leftIndex + leafGroup->slotsUsed() - numLeafs;
                        }
                    }
                }
                else
                {
                    //groupOp = Operation::None;
                }

                leaf = &leafGroup->node(newLeafOffset);
            }
            else
            {
                op = RemOp::Balance;
                leafGroup->balance(leafSlot, chosenIndex);

                // Update the key in the Node above.
                rec.first->node(rec.second).key(leftIndex) =
                    leafGroup->node(leftIndex + 1).key(0);
                // If we rotated key to the right, we already fixed the key.
                repairKey = repairKey && leafSlot < chosenIndex;
            }
        }

        BPTreeAssert(!leaf->underMinimalCapacity());
        BPTreeAssert(!leafGroup->underMinimalCapacity() || mHeight == 1);

        // Fix the node above
        if (repairKey && path[currHeight].third)
        { // We found the position, where the key needs to be changed.
            NodeGroupT *aboveNodeGroup{path[currHeight].first};
            OffsetT aboveNodeOffset{path[currHeight].second};
            OffsetT aboveKeyOffset{path[currHeight].third};
            NodeT &aboveNode{aboveNodeGroup->node(aboveNodeOffset)};
            BPTreeAssert(key == aboveNode.key(aboveKeyOffset - 1));
            // The new key has been move to the changed Leaf.
            aboveNode.key(aboveKeyOffset - 1) = leaf->key(0);
            // Only one key needs to be repaired.
            repairKey = false;
        }

        // Repair the rest of the tree.

        while (!(op == RemOp::None && groupOp == RemOp::None && !repairKey))
        { // While we are not fixing the root.
            // Trace the path back.
            NodeRecord &record{path[currHeight]};

            NodeGroupT *nodeGroup{record.first};
            OffsetT nodeOffset{record.second};
            OffsetT keyOffset{record.third};

            NodeT *node{&nodeGroup->node(nodeOffset)};

            //if (repairKey && keyOffset != 0 && currHeight != 0 && path[currHeight - 1].third)
            if (repairKey && currHeight != 0 && path[currHeight - 1].third)
            { // We found the position, where the key needs to be changed.
                NodeGroupT *aboveNodeGroup{path[currHeight - 1].first};
                OffsetT aboveNodeOffset{path[currHeight - 1].second};
                OffsetT aboveKeyOffset{path[currHeight - 1].third};
                NodeT &aboveNode{aboveNodeGroup->node(aboveNodeOffset)};
                BPTreeAssert(key == aboveNode.key(aboveKeyOffset - 1));
                // The new key has been move to the changed Leaf.
                aboveNode.key(aboveKeyOffset - 1) = leaf->key(0);
                // Only one key needs to be repaired.
                repairKey = false;
            }

            /**
             * Depending, on what happened on
             * the level below, we need to take
             * different actions :
             * 1) Node/Leafs operations :
             *   A) None -> nothing needs to be done.
             *   B) Balanced -> correct the key value.
             *   C) Merged -> delete a key.
             * 2) Group operations - only, if (1-B):
             *   A) None -> no multi-node operations required.
             *   B) Balanced -> simple left/right rotation.
             *   C) Merged -> merge Nodes.
             */
            if (op == RemOp::Balance)
            { // 1-B
                op = RemOp::None;
                groupOp = RemOp::None;
            }
            else if (op == RemOp::Merge)
            { // 1-C
                // Key on index slotChanged needs to be deleted.
                node->removeKeyNoFind(leftIndex);
                mStatistics.numNodesFilledKeys--;

                // Nothing else can be done for the root Node.
                if (currHeight == 0)
                {
                    break;
                }

                if (groupOp == RemOp::Balance)
                { // 2-B
                    OffsetT keysUsed{node->slotsUsed()};

                    NodeRecord &rec{path[currHeight - 1]};
                    nodeGroup->balance(nodeOffset, groupOpOffset);
                    op = RemOp::Balance;

                    leftIndex = nodeOffset < groupOpOffset ? nodeOffset : groupOpOffset;

                    // Update the key in the Node above.
                    if (nodeOffset < groupOpOffset)
                    {
                        std::swap(rec.first->node(rec.second).key(leftIndex),
                                  node->key(node->slotsUsed() - 1));

                        OffsetT lastElement{node->slotsUsed() - 1};
                        if (lastElement != keysUsed)
                        {
                            //std::swap(node->key(node->slotsUsed() - 1),
                            //          node->key(keysUsed));
                            node->moveElement(lastElement, -(lastElement - keysUsed));
                        }
                    }
                    else
                    {
                        std::swap(rec.first->node(rec.second).key(leftIndex),
                                  node->key(0));

                        OffsetT firstOriginal{node->slotsUsed() - keysUsed};
                        if (firstOriginal != 1)
                        {
                            //std::swap(node->key(0),
                            //          node->key(firstOriginal - 1));
                            node->moveElement(0, firstOriginal - 1);
                        }
                    }
                    //std::swap(rec.first->node(rec.second).key(leftIndex),
                    //          node->key(node->slotsUsed() - 1));
                    //rec.first->node(rec.second).key(leftIndex) =
                    //    node.key(node.slotsUsed() - 1);
                    //rec.first->node(rec.second).key(leftIndex) =
                    //    leafGroup->node(leftIndex + 1).key(0);

                    // If we rotated key to the right, we already fixed the key.
                    repairKey = repairKey && nodeOffset < groupOpOffset;
                }
                else if (groupOp == RemOp::Merge)
                { // 2-C
                    leftIndex = nodeOffset < groupOpOffset ? nodeOffset : groupOpOffset;
                    nodeGroup->merge(nodeOffset, groupOpOffset);
                    op = RemOp::Merge;
                    mStatistics.numNodes--;

                    node = &nodeGroup->node(leftIndex);
                }
                else
                {
                    // No op in the Node above.
                    op = RemOp::None;
                }

                /**
                 * If there will be Node merging/balancing above, we
                 * should merge/balance the LeafGroups now.
                 * Again, the special scenario for the root, there
                 * are no other NodeGroups on the same level as the
                 * root NodeGroup.
                 */
                if (nodeGroup->underMinimalCapacity())
                { // We cannot do anything more for root.
                    if (currHeight == 1)
                    { // We can't merge/balance root!
                        groupOp = RemOp::None;
                    }
                    else
                    {
                        OffsetT numNodes{nodeGroup->slotsUsed()};
                        groupOpOffset = remGroupMinCap(nodeGroup, path[currHeight - 1], groupOp);

                        if (groupOp == RemOp::Merge)
                        {
                            NodeRecord &rec{path[currHeight - 1]};
                            NodeGroupT *deletedNodeGroup{nullptr};
                            NodeT *affectedNode{nullptr};
                            LeafGroupT *rmLeafGroup{nullptr};

                            if (groupOpOffset < rec.second)
                            { // Merged into the NodeGroup on groupOpOffset.
                                deletedNodeGroup = nodeGroup;
                                affectedNode = &rec.first->node(groupOpOffset);
                                nodeGroup = affectedNode->childGroup().nodeGroup;

                                rmLeafGroup = remFindLeftMostLeafGroup(
                                    /*nodeGroup->node(leftIndex +
                                                nodeGroup->slotsUsed() - numNodes),
                                                */
                                    nodeGroup->node(nodeGroup->slotsUsed() - numNodes),
                                    currHeight, mHeight);
                            }
                            else
                            { // Merged into the current NodeGroup.
                                affectedNode = &rec.first->node(rec.second);
                                deletedNodeGroup = rec.first->
                                    node(groupOpOffset).childGroup().nodeGroup;
                                rmLeafGroup = remFindRightMostLeafGroup(
                                    //nodeGroup->node(leftIndex + 1),
                                    nodeGroup->node(numNodes - 1),
                                    currHeight, mHeight)->next();
                            }

                            // Add the new key to the affectedNode.
                            affectedNode->addKeyNoFind(rmLeafGroup->node(0).key(0),
                                                       affectedNode->slotsUsed());
                            mStatistics.numNodesFilledKeys++;

                            // Change the position of leafGroup to the correct group.

                            // Delete the NodeGroup.
                            delNodeGroup(deletedNodeGroup);
                        }
                    }
                }
                else
                {
                    groupOp = RemOp::None;
                }

                BPTreeAssert((op == RemOp::None && groupOp == RemOp::None) ||
                             (op != RemOp::None));
            }
            else
            {
                //op = Operation::None;
                //groupOp = Operation::None;
                BPTreeAssert(op == RemOp::None);
                BPTreeAssert(groupOp == RemOp::None);
            }

            BPTreeAssert(currHeight < 2 || !nodeGroup->underMinimalCapacity());

            if (currHeight != 0)
            {
                currHeight--;
            }
            else
            {
                break;
            }
        }

        BPTreeAssert(mHeight >= 1);
        // Check, if the current root Node is not empty.
        if (mRoot.nodeGroup->node(0).empty())
        {
            if (mHeight > 1)
            { // The new root is the NodeGroup below current root.
                delNodeGroup(mRoot.nodeGroup);
                mRoot.nodeGroup = path[1].first;
                // New root should have only a single Node.
                BPTreeAssert(mRoot.nodeGroup->slotsUsed() == 1);
            }
            else
            { // The new root is the first LeafGroup.
                BPTreeAssert(mFirstLeafGroup->slotsUsed() == 1);
                delNodeGroup(mRoot.nodeGroup);
                mRoot.leafGroup = mFirstLeafGroup;
            }

            mStatistics.numNodes--;
            mHeight--;
        }

        return true;
    }

    template <typename Config>
    template <typename GroupT>
    inline
    auto BPTree<Config>::remChooseNeighbour(GroupT *group,
                                            OffsetT slot,
                                            OffsetT &nIndex)
    -> OffsetT
    {
        bool hasLeftNeighbour{group->hasLNeighbour(slot)};
        bool hasRightNeighbour{group->hasRNeighbour(slot)};

        /**
         * There should never be a single Leaf/Node inside a
         * non-root *Group, when mHeight != 0.
         */
        BPTreeAssert(hasLeftNeighbour || hasRightNeighbour);

        if (hasLeftNeighbour && hasRightNeighbour)
        { // If Leaf has both neighbours, choose the one with more keys.
            if (group->node(slot - 1).slotsUsed() >
                group->node(slot + 1).slotsUsed())
            { // |left.keys| > |right.keys|
                nIndex = slot - 1;
                return nIndex;
            }
            else
            { // |right.keys| >= |left.keys|
                nIndex = slot + 1;
                return slot;
            }
        }
        else
        { // Only one neighbour present.
            if (hasRightNeighbour)
            {
                nIndex = slot + 1;
                return slot;
            }
            else
            {
                nIndex = slot - 1;
                return nIndex;
            }
        }
    }

    template <typename Config>
    template <typename GroupT>
    inline
    auto BPTree<Config>::remGroupMinCap(GroupT *group,
                                        NodeRecord &rec,
                                        RemOp &groupOp)
    -> OffsetT
    {
        NodeGroupT *&nodeGroup{rec.first};
        OffsetT &nodeOffset{rec.second};

        OffsetT newGroupIndex{0};
        OffsetT leftGroupIndex{remChooseNeighbour(nodeGroup, nodeOffset, newGroupIndex)};

        if (nodeGroup->node(newGroupIndex).atMinimalCapacity())
        { // We need to merge.
            GroupT *leftGroup{static_cast<GroupT*>(
                                  nodeGroup->node(leftGroupIndex).childGroup().group)};
            GroupT *rightGroup{static_cast<GroupT*>(
                                   nodeGroup->node(leftGroupIndex + 1).childGroup().group)};
            leftGroup->combine(*rightGroup);

            BPTreeAssert(!leftGroup->underMinimalCapacity());
            groupOp = RemOp::Merge;
        }
        else
        { // There is enough elements to rebalance.
            GroupT *toBalance{static_cast<GroupT*>(
                                  nodeGroup->node(nodeOffset).childGroup().group)};
            GroupT *balanced{static_cast<GroupT*>(
                                   nodeGroup->node(newGroupIndex).childGroup().group)};
            toBalance->balanceFrom(*balanced, newGroupIndex > nodeOffset);

            BPTreeAssert(!toBalance->underMinimalCapacity());
            BPTreeAssert(!balanced->underMinimalCapacity());
            groupOp = RemOp::Balance;
        }


        return newGroupIndex;
    }

    template <typename Config>
    inline
    auto BPTree<Config>::remFindRightMostLeafGroup(NodeT &node, u64 nodeHeight,
                                                   u64 maxHeight)
    -> LeafGroupT*
    {
        BPTreeAssert(nodeHeight < maxHeight);

        ChildGroupPtr group{node.childGroup()};
        u64 currHeight{nodeHeight + 1};
        OffsetT idx{node.slotsUsed()};

        while (currHeight < maxHeight)
        {
            BPTreeAssert(!group.nodeGroup->empty());

            NodeT &currNode{group.nodeGroup->node(idx)};
            idx = currNode.slotsUsed();
            group = currNode.childGroup();

            currHeight++;
        }

        BPTreeAssert(!group.leafGroup->empty());

        return group.leafGroup;
    }

    template <typename Config>
    inline
    auto BPTree<Config>::remFindLeftMostLeafGroup(NodeT &node, u64 nodeHeight,
                                                   u64 maxHeight)
    -> LeafGroupT*
    {
        BPTreeAssert(nodeHeight < maxHeight);

        ChildGroupPtr group{node.childGroup()};
        u64 currHeight{nodeHeight + 1};
        OffsetT idx{0};

        while (currHeight < maxHeight)
        {
            BPTreeAssert(!group.nodeGroup->empty());

            NodeT &currNode{group.nodeGroup->node(idx)};
            group = currNode.childGroup();

            currHeight++;
        }

        BPTreeAssert(!group.leafGroup->empty());

        return group.leafGroup;
    }

#undef BPTreeDebug
#undef BPTreeAssert
}

#endif //MEM_BPTREE_H
