/**
 * @file testing/HashNode.h
 * @author Tomas Polasek
 * @brief Hash node tree.
 */

#ifndef UTIL_HASHNODE_H
#define UTIL_HASHNODE_H

#include <vector>

#include "math/Math.h"

namespace util
{
    /**
     * Single node of the call stack.
     * Each node has its own hash table.
     * Hash table always has at least one space empty.
     * Size of the hash table is always a power of 2.
     * Conflict resolution is linear probing.
     */
    template <typename DataType, u32 START_SIZE = 4u>
    class HashNode
    {
    public:
        using InnerNode = HashNode<DataType, START_SIZE>;
        using HashTable = std::vector<InnerNode*>;

        /**
         * @param name Name of the node.
         * @param parent Ptr to the parent node.
         */
        HashNode(const char *name, HashNode *parent) :
            mChildren(START_SIZE),
            mName(name),
            mParent(parent)
        {
            // Make sure, the startSize is power of 2.
            ASSERT_SLOW( (START_SIZE & (START_SIZE - 1)) == 0 );
        }

        /**
         * Use given functor on each node.
         * @param functor Function object.
         */
        template<typename FunObj>
        void foreach(FunObj &functor)
        {
            for (InnerNode *node : mChildren)
            {
                functor(node);
            }
        }

        /**
         * Use given functor on each node != nullptr.
         * @param functor Function object.
         */
        template<typename FunObj>
        void foreachnn(FunObj &functor)
        {
            for (InnerNode *node : mChildren)
            {
                if (node != nullptr)
                    functor(node);
            }
        }

        /**
         * Get already existing child, or create a new one.
         * @param name Name of the child.
         * @return Returns ptr to child with given name.
         */
        HashNode *getCreateChild(const char *name)
        {
            // Try to find matching child first.
            u32 tableSize{getCurrentSize(mChildren)};

            tableSize = assureTwoEmpty(tableSize, mNumChildren, mChildren);

            u32 index{getIndex(name, tableSize)};

            u32 circleMask{tableSize - 1};
            // Search the hash table.
            for (InnerNode *nodePtr{nullptr};
                 (nodePtr = mChildren[index]);
                 index = (index + 1) & circleMask
                )
            {
                if (nodePtr->name() == name)
                    return nodePtr;
            }

            // We did not find the matching child.
            // index now points to the first empty space
            // in the hash table.

            InnerNode *newNode = new InnerNode(name, this);
            mChildren[index] = newNode;
            mNumChildren++;

            ASSERT_SLOW(newNode->name() == name);
            return newNode;
        }

        /**
         * Print debug information about this node.
         * @param recursive If set to true, debugPrint will be
         * called on all the children too.
         */
        void debugPrint(i8 recursive)
        {
            debugPrintIndent(recursive, 0);
        }

        /**
         * Get name pointer for this node
         * @param name pointer identifying this node.
         */
        const char *name()
        {
            return mName;
        }

        /**
         * Get measurement data structure for this node.
         * @return Structure containing measurement data.
         */
        DataType &data()
        {
            return mData;
        }

        /**
         * Get ptr to this nodes parent.
         * @return Pointer to parent the node.
         */
        HashNode *parent()
        {
            return mParent;
        }

        ~HashNode()
        {
            for (InnerNode *node : mChildren)
            {
                if (node)
                    delete node;
            }
        }
    private:
        /**
         * Debug printing helper function, includes indent.
         * @param recursive Should the printing be recursive?
         * @param indent Indent level.
         */
        void debugPrintIndent(i8 recursive, u32 indent)
        {
            printf("%*s%s %u\n", indent, "", mName, getCurrentSize(mChildren));
            if (recursive)
            {
                for (InnerNode *node : mChildren)
                {
                    if (node)
                        node->debugPrintIndent(recursive, indent + 1u);
                }
            }
        }

        /**
         * Find first empty slot for given name.
         * @param name Identifier.
         * @param table Hash table to search through.
         * @return Index of the first empty slot.
         */
        u32 findEmpty(const char *name, HashTable &table)
        {
            u32 tableSize{getCurrentSize(table)};
            u32 index{getIndex(name, tableSize)};

            u32 circleMask{tableSize - 1};

            while (table[index])
            {
                index = (index + 1) & circleMask;
            }

            return index;
        }

        /**
         * Make sure at least two spaces in the hash table are empty.
         * One is for required for search and the other is to be sure,
         * that after insert, the integrity won't be violated.
         * @param size Current size of the hash table.
         * @param numChildren Current number of children.
         * @param table The hash table.
         * @return Returns the new size.
         */
        u32 assureTwoEmpty(u32 size, u32 numChildren, HashTable &table)
        {
            if (size > numChildren + 1)
                return size;

            u32 newSize = size << 1;
            resize(newSize, table);
            ASSERT_SLOW(newSize == getCurrentSize(table));
            return newSize;
        }

        /**
         * Make sure that given table has the given size.
         * @param size Requested size.
         * @param table Hash table to work on.
         */
        void assureSize(u32 size, HashTable &table)
        {
            table.resize(size);
        }

        /**
         * Swap contents of 2 hash tables.
         * @param table1 The first table.
         * @param table2 The second table.
         */
        void swapTables(HashTable &table1, HashTable &table2)
        {
            table1.swap(table2);
        }

        /**
         * Get the current size of the hash table.
         * Return value from this function is always a
         * power of 2 (non-zero).
         * @param table The hash table.
         * @return The current size of the hash table.
         */
        u32 getCurrentSize(HashTable &table)
        {
            u32 size{static_cast<u32>(table.size())};
            // Assert that the size is power of 2.
            ASSERT_SLOW( (size & (size - 1)) == 0 );
            return size;
        }

        /**
         * Resize the hash table to at least specified size.
         * Does nothing, if the size of the hash table is at
         * least given size.
         * If resizing is required, the new size will always
         * be a power of 2.
         * @param newSize Requested size of the hash table.
         * @param table The hash table.
         */
        void resize(u32 newSize, HashTable &table)
        {
            u32 currentSize{getCurrentSize(table)};
            u32 pow2NewSize{math::pow2RoundUp(newSize)};

            if (currentSize < pow2NewSize)
            {
                // We need to resize the hash table.
                HashTable newTable;
                assureSize(pow2NewSize, newTable);
                // Insert nodes to the new table.
                repopulateTable(mChildren, newTable);
                swapTables(mChildren, newTable);
            }
        }

        /**
         * Repopulate new hash table with the old values.
         * @param src The source hash table.
         * @param dest The destination hash table.
         */
        void repopulateTable(HashTable &src, HashTable &dest)
        {
            ASSERT_SLOW(getCurrentSize(dest) >= getCurrentSize(src));

            for (InnerNode *node : src)
            {
                if (node)
                {
                    u32 index = findEmpty(node->name(), dest);
                    dest[index] = node;
                }
            }
        }

        /**
         * Calculate index for given name and size of the hash table.
         * @param name Identifying name.
         * @param size Size of the hash table.
         * @return Index to the hash table.
         */
        static inline u32 getIndex(const char *name, u32 size)
        {
            u64 ptr{reinterpret_cast<u64>(name)};
            return (static_cast<u32>(ptr)) % (size);
        }
    protected:
        /// Number of children.
        u32 mNumChildren{0};

        /// Contains child nodes.
        HashTable mChildren{4u};

        /// Name with unique pointer.
        const char *mName{nullptr};

        HashNode *mParent{nullptr};

        /// Data for this node
        DataType mData;
    };

#endif //UTIL_HASHNODE_H
}
