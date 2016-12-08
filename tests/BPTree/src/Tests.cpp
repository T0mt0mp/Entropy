/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <fstream>

#include "mem/BPTree.h"

namespace tst {
    struct KeyProbe
    {
        ~KeyProbe()
        {
            if (mPrint)
            std::cout << "KeyProbe destructor" << mMyId << "," << mKey << std::endl;
        }

        KeyProbe() :
            mKey{0},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "Default KeyProbe constructor " << mMyId << std::endl;
        }

        KeyProbe(u16 key) :
            mKey{key},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "KeyProbe constructor " << mMyId << " " << key << std::endl;
        }

        KeyProbe(const KeyProbe &other) :
            mKey{other.mKey},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "Copy KeyProbe constructor from " <<
                other.mMyId << "," << other.mKey << " I am " << mMyId <<
                "," << mKey << std::endl;
        }

        KeyProbe(KeyProbe &&other) :
            mKey{other.mKey},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "Move KeyProbe constructor from " <<
                other.mMyId << "," << other.mKey << " I am " << mMyId <<
                "," << mKey << std::endl;
        }

        KeyProbe &operator=(const KeyProbe &other)
        {
            mKey = other.mKey;
            if (mPrint)
            std::cout << "Copy KeyProbe operator= from " <<
                other.mMyId << "," << other.mKey << " I am " << mMyId <<
                "," << mKey << std::endl;
            return *this;
        }

        KeyProbe &operator=(KeyProbe &&other)
        {
            mKey = other.mKey;
            if (mPrint)
            std::cout << "Move KeyProbe operator= from " <<
                other.mMyId << "," << other.mKey << " I am " << mMyId <<
                "," << mKey << std::endl;
            return *this;
        }

        bool operator==(const KeyProbe &other) const
        {
            return mKey == other.mKey;
        }
        bool operator!=(const KeyProbe &other) const
        {
            return mKey != other.mKey;
        }

        bool operator<(const KeyProbe &other) const
        {
            return mKey < other.mKey;
        }
        bool operator<=(const KeyProbe &other) const
        {
            return mKey <= other.mKey;
        }
        bool operator>(const KeyProbe &other) const
        {
            return mKey > other.mKey;
        }
        bool operator>=(const KeyProbe &other) const
        {
            return mKey >= other.mKey;
        }

        friend std::ostream &operator<<(std::ostream &out, const KeyProbe &probe)
        {
            out << "kp#" << probe.mMyId << "," << probe.mKey;
            return out;
        }

        u16 mKey;

        u16 mMyId;
        static u16 mNum;
        static bool mPrint;

        static void setPrint()
        { mPrint = true; }
        static void resetPrint()
        { mPrint = false; }
    };

    u16 KeyProbe::mNum{0};
    bool KeyProbe::mPrint{true};

    struct DataProbe
    {
        ~DataProbe()
        {
            if (mPrint)
            std::cout << "DataProbe destructor" << mMyId << "," << mData << std::endl;
        }

        DataProbe() :
            mData{0},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "Default DataProbe constructor " << mMyId << std::endl;
        }

        DataProbe(u16 data) :
            mData{data},
            mMyId{mNum++}
        {
            if (mPrint)
            std::cout << "DataProbe constructor " << mMyId << "," << mData << std::endl;
        }

        DataProbe(const DataProbe &other) :
            mMyId{mNum++}
        {
            mData = other.mData;
            if (mPrint)
            std::cout << "Copy DataProbe constructor from " <<
                other.mMyId << "," << other.mData << " I am " << mMyId <<
                "," << mData << std::endl;
        }

        DataProbe(DataProbe &&other) :
            mMyId{mNum++}
        {
            mData = other.mData;
            if (mPrint)
            std::cout << "Move DataProbe constructor from " <<
                other.mMyId << "," << other.mData << " I am " << mMyId <<
                "," << mData << std::endl;
        }

        DataProbe &operator=(const DataProbe &other)
        {
            mData = other.mData;
            if (mPrint)
            std::cout << "Copy DataProbe operator= from " <<
                other.mMyId << "," << other.mData << " I am " << mMyId <<
                "," << mData << std::endl;
            return *this;
        }

        DataProbe &operator=(DataProbe &&other)
        {
            mData = other.mData;
            if (mPrint)
            std::cout << "Move DataProbe operator= from " <<
                other.mMyId << "," << other.mData << " I am " << mMyId <<
                "," << mData << std::endl;
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &out, const DataProbe &probe)
        {
            out << "dp#" << probe.mMyId << "," << probe.mData;
            return out;
        }

        u16 mData;
        u16 mMyId;
        static u16 mNum;
        static bool mPrint;

        static void setPrint()
        { mPrint = true; }
        static void resetPrint()
        { mPrint = false; }
    };

    u16 DataProbe::mNum{0};
    bool DataProbe::mPrint{true};

    template <typename KeyT,
              u16 N>
    struct TestNode
    {
        TestNode()
        {
            mem.slotsUsed = mp_memory_filler_v<u16>;
            mem.childGroup = reinterpret_cast<void*>(0x123456789ABCDEF0);

            for (u16 iii = 0; iii < N; ++iii)
            {
                mem.keys[iii] = mp_memory_filler_v<KeyT>;
            }

            for (u16 iii = 0; iii <= N; ++iii)
            {
                mem.offsets[iii] = mp_memory_filler_v<u16>;
            }
        }

        void printStats()
        {
            u64 total{sizeof(TestNode)};
            std::cout << "Size of node [B] : " << total << std::endl;

            u64 sum{sizeof(MemLayout::childGroup) + sizeof(MemLayout::slotsUsed) +
                    sizeof(MemLayout::keys) + sizeof(MemLayout::offsets)};
            std::cout << "Sum of members [B] : " << sum << std::endl;

            float usefulness{(static_cast<float>(sum) / total) * 100.0f};
            std::cout << "Usefulness [%] : " << usefulness << std::endl;

            std::cout << "Alignment [B] : " << alignof(TestNode) << std::endl;
            std::cout << "childGroup [B/B] : " << sizeof(MemLayout::childGroup) <<
                " / " << alignof(decltype(MemLayout::childGroup)) << std::endl;
            std::cout << "slotsUsed [B/B] : " << sizeof(MemLayout::slotsUsed) <<
                " / " << alignof(decltype(MemLayout::slotsUsed)) << std::endl;
            std::cout << "keys [B/B] : " << sizeof(MemLayout::keys) <<
                " / " << alignof(decltype(MemLayout::keys)) << std::endl;
            std::cout << "offsets [B/B] : " << sizeof(MemLayout::offsets) <<
                " / " << alignof(decltype(MemLayout::offsets)) << std::endl;

            std::cout << "Size of array of 10 nodes [B] : " <<
                sizeof(TestNode[10]) << std::endl;
        }

        struct LayoutA
        {
            void *childGroup;
            u16 slotsUsed;
            KeyT keys[N];
            u16 offsets[N + 1];
        };

        struct LayoutB
        {
            void *childGroup;
            KeyT keys[N];
            u16 slotsUsed;
            u16 offsets[N + 1];
        };

        struct LayoutC
        {
            void *childGroup;
            KeyT keys[N];
            u16 offsets[N + 1];
            u16 slotsUsed;
        };

        struct LayoutD
        {
            KeyT keys[N];
            u16 offsets[N + 1];
            u16 slotsUsed;
            void *childGroup;
        };

        struct LayoutE
        {
            KeyT keys[N];
            u16 offsets[N + 1];
            void *childGroup;
            u16 slotsUsed;
        };

        struct LayoutF
        {
            KeyT keys[N];
            void *childGroup;
            u16 slotsUsed;
            u16 offsets[N + 1];
        };

        /*
        static_assert(alignof(LayoutA) == alignof(LayoutB));
        static_assert(alignof(LayoutB) == alignof(LayoutC));
        static constexpr u64 ALIGNMENT{alignof(LayoutA)};

        using MemLayout1 = mp_choose_more_t<
            mp_list<LayoutA, LayoutB, LayoutC>,
            sizeof(void *) + sizeof(u16) <= ALIGNMENT,
            sizeof(KeyT) + sizeof(u16) <= ALIGNMENT,
            true
        >;

        using MemLayout2 = mp_choose_more_t<
            mp_list<LayoutA, LayoutB, LayoutC>,
            sizeof(LayoutA) <= sizeof(LayoutB) &&
                sizeof(LayoutA) <= sizeof(LayoutC),
            sizeof(LayoutB) <= sizeof(LayoutC),
            true
        >;

        //static_assert(mp_is_same_v<MemLayout1, MemLayout2>);

         */

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
        MemLayout mem;
    };

    template <typename KeyT,
              typename DataT,
              u16 M>
    struct TestLeaf
    {
        TestLeaf()
        {
            mem.slotsUsed = mp_memory_filler_v<u16>;
            for (u16 iii = 0; iii < M; ++iii)
            {
                mem.keys[iii] = mp_memory_filler_v<KeyT>;
            }

            for (u16 iii = 0; iii < M; ++iii)
            {
                mem.data[iii] = mp_memory_filler_v<DataT>;
            }
        }

        void printStats()
        {
            u64 total{sizeof(TestLeaf)};
            std::cout << "Size of leaf [B] : " << total << std::endl;

            u64 sum{sizeof(MemLayout::slotsUsed) + sizeof(MemLayout::keys) +
                    sizeof(MemLayout::data)};
            std::cout << "Sum of members [B] : " << sum << std::endl;

            float usefulness{(static_cast<float>(sum) / total) * 100.0f};
            std::cout << "Usefulness [%] : " << usefulness << std::endl;

            std::cout << "Alignment [B] : " << alignof(TestLeaf) << std::endl;

            std::cout << "slotsUsed [B/B] : " << sizeof(MemLayout::slotsUsed) <<
                " / " << alignof(decltype(MemLayout::slotsUsed)) << std::endl;
            std::cout << "keys [B/B] : " << sizeof(MemLayout::keys) <<
                " / " << alignof(decltype(MemLayout::keys)) << std::endl;
            std::cout << "data [B/B] : " << sizeof(MemLayout::data) <<
                " / " << alignof(decltype(MemLayout::data)) << std::endl;

            std::cout << "Size of array of 10 leafs [B] : " <<
                sizeof(TestLeaf[10]) << std::endl;
        }

        struct LayoutA
        {
            u16 slotsUsed;
            KeyT keys[M];
            DataT data[M];
        };

        struct LayoutB
        {
            KeyT keys[M];
            u16 slotsUsed;
            DataT data[M];
        };

        struct LayoutC
        {
            KeyT keys[M];
            DataT data[M];
            u16 slotsUsed;
        };

        struct LayoutD
        {
            DataT data[M];
            u16 slotsUsed;
            KeyT keys[M];
        };

        using MemLayout = mp_choose_best_t<
            mp_list<LayoutA, LayoutB, LayoutC, LayoutD>,
            std::less<std::size_t>,
            sizeof(LayoutA),
            sizeof(LayoutB),
            sizeof(LayoutC),
            sizeof(LayoutD)
        >;
        MemLayout mem;
    };

    TEST_UNIT_BEG(TestingTheTest, 1)

        TEST_UNIT_SETUP_BEG
        TEST_UNIT_SETUP_END

        TEST_UNIT_TEARDOWN_BEG
        TEST_UNIT_TEARDOWN_END

        TEST_UNIT_MAIN_BEG
#if 1
            KeyProbe::resetPrint();
            DataProbe::resetPrint();

            /*
            using TreeT = mem::BPTree<mem::BPTreeConfig<KeyProbe, DataProbe,
                std::less<KeyProbe>,
                mem::DefaultAllocator<DataProbe>,
                64, 8, 8>>;
                */
            using TreeT = mem::BPTree<mem::BPTreeConfig<KeyProbe, DataProbe>>;

            std::cout << "Leaf default constructor : " << std::endl;
            TreeT::LeafT leaf;
            std::cout << "End of Leaf default constructor : " << std::endl;

            std::cout << leaf.slotsUsed() << std::endl;
            TC_AssertTrue(leaf.slotsUsed() == 0);

            std::cout << "Adding keys..." << std::endl;

            std::cout << "New" << std::endl;
            DataProbe *ptr{nullptr};
            bool created{false};

            ptr = leaf.getCreateData(KeyProbe(1), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(1);

            ptr = leaf.getCreateData(KeyProbe(2), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(2);

            ptr = leaf.getCreateData(KeyProbe(3), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(3);

            ptr = leaf.getCreateData(KeyProbe(4), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(4);

            ptr = leaf.getCreateData(KeyProbe(5), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(5);

            std::cout << "Id" << std::endl;
            TC_Expect(leaf.data(leaf.find(KeyProbe(1))).mMyId, 0);
            TC_Expect(leaf.data(leaf.find(KeyProbe(2))).mMyId, 1);
            TC_Expect(leaf.data(leaf.find(KeyProbe(3))).mMyId, 2);
            TC_Expect(leaf.data(leaf.find(KeyProbe(4))).mMyId, 3);
            TC_Expect(leaf.data(leaf.find(KeyProbe(5))).mMyId, 4);

            TC_Expect(leaf.slotsUsed(), 5);

            std::cout << "Data" << std::endl;
            TC_AssertTrue(leaf.data(leaf.find(KeyProbe(1))).mData == 1);
            TC_AssertTrue(leaf.data(leaf.find(KeyProbe(2))).mData == 2);
            TC_AssertTrue(leaf.data(leaf.find(KeyProbe(3))).mData == 3);
            TC_AssertTrue(leaf.data(leaf.find(KeyProbe(4))).mData == 4);
            TC_AssertTrue(leaf.data(leaf.find(KeyProbe(5))).mData == 5);

            std::cout << "End of adding keys..." << std::endl;

            std::cout << "Testing find..." << std::endl;

            TC_AssertTrue(leaf.find(KeyProbe(1)) == 0);
            TC_AssertTrue(leaf.find(KeyProbe(2)) == 1);
            TC_AssertTrue(leaf.find(KeyProbe(3)) == 2);
            TC_AssertTrue(leaf.find(KeyProbe(4)) == 3);
            TC_AssertTrue(leaf.find(KeyProbe(5)) == 4);
            TC_AssertTrue(leaf.find(KeyProbe(0)) == 0);
            TC_AssertTrue(leaf.find(KeyProbe(6)) == 5);

            std::cout << "Printing content of leaf : " << std::endl;
            {
                auto keyIt = leaf.keyBegin();
                auto dataIt = leaf.dataBegin();
                for (;keyIt < leaf.keyEnd(); ++keyIt, ++dataIt)
                {
                    std::cout << *keyIt << " " << *dataIt << std::endl;
                }
            }

            {
                TreeT::LeafT rightLeaf;
                std::cout << "Splitting leaf..." << std::endl;

                leaf.split(rightLeaf, 4);

                TC_AssertTrue(leaf.slotsUsed() == 3);
                TC_AssertTrue(leaf.key(0).mKey == 1);
                TC_AssertTrue(leaf.key(1).mKey == 2);
                TC_AssertTrue(leaf.key(2).mKey == 3);
                TC_AssertTrue(leaf.data(0).mData == 1);
                TC_AssertTrue(leaf.data(1).mData == 2);
                TC_AssertTrue(leaf.data(2).mData == 3);

                TC_AssertTrue(rightLeaf.slotsUsed() == 2);
                TC_AssertTrue(rightLeaf.key(0).mKey == 4);
                TC_AssertTrue(rightLeaf.key(1).mKey == 5);
                TC_AssertTrue(rightLeaf.data(0).mData == 4);
                TC_AssertTrue(rightLeaf.data(1).mData == 5);

                std::cout << "Printing content of leafLeaf : " << std::endl;
                {
                    auto keyIt = leaf.keyBegin();
                    auto dataIt = leaf.dataBegin();
                    for (;keyIt < leaf.keyEnd(); ++keyIt, ++dataIt)
                    {
                        std::cout << *keyIt << " " << *dataIt << std::endl;
                    }
                }

                std::cout << "Printing content of rightLeaf : " << std::endl;
                {
                    auto keyIt = rightLeaf.keyBegin();
                    auto dataIt = rightLeaf.dataBegin();
                    for (;keyIt < rightLeaf.keyEnd(); ++keyIt, ++dataIt)
                    {
                        std::cout << *keyIt << " " << *dataIt << std::endl;
                    }
                }

                leaf.combine(rightLeaf);

                TC_Expect(leaf.slotsUsed(), 5);
                TC_Expect(leaf.key(0).mKey, 1);
                TC_Expect(leaf.key(1).mKey, 2);
                TC_Expect(leaf.key(2).mKey, 3);
                TC_Expect(leaf.key(3).mKey, 4);
                TC_Expect(leaf.key(4).mKey, 5);
                TC_Expect(leaf.data(0).mData, 1);
                TC_Expect(leaf.data(1).mData, 2);
                TC_Expect(leaf.data(2).mData, 3);
                TC_Expect(leaf.data(3).mData, 4);
                TC_Expect(leaf.data(4).mData, 5);

                TC_Expect(rightLeaf.slotsUsed(), 0);

                std::cout << "Printing content of leafLeaf : " << std::endl;
                {
                    auto keyIt = leaf.keyBegin();
                    auto dataIt = leaf.dataBegin();
                    for (;keyIt < leaf.keyEnd(); ++keyIt, ++dataIt)
                    {
                        std::cout << *keyIt << " " << *dataIt << std::endl;
                    }
                }

                std::cout << "Printing content of rightLeaf : " << std::endl;
                {
                    auto keyIt = rightLeaf.keyBegin();
                    auto dataIt = rightLeaf.dataBegin();
                    for (;keyIt < rightLeaf.keyEnd(); ++keyIt, ++dataIt)
                    {
                        std::cout << *keyIt << " " << *dataIt << std::endl;
                    }
                }

            }

            std::cout << "Leaf copy construction" << std::endl;
            TreeT::LeafT copyLeaf(leaf);
            TC_AssertTrue(copyLeaf.slotsUsed() == leaf.slotsUsed());
            std::cout << "Copy leaf content" << std::endl;
            {
                auto keyIt = copyLeaf.keyBegin();
                auto dataIt = copyLeaf.dataBegin();
                for (;keyIt < copyLeaf.keyEnd(); ++keyIt, ++dataIt)
                {
                    std::cout << *keyIt << " " << *dataIt << std::endl;
                }
            }

            std::cout << "Leaf move construction" << std::endl;
            TreeT::LeafT moveLeaf(std::move(copyLeaf));
            TC_AssertTrue(moveLeaf.slotsUsed() == leaf.slotsUsed());
            TC_AssertTrue(copyLeaf.slotsUsed() == 0);
            std::cout << "Copy leaf content" << std::endl;
            {
                auto keyIt = copyLeaf.keyBegin();
                auto dataIt = copyLeaf.dataBegin();
                for (;keyIt < copyLeaf.keyEnd(); ++keyIt, ++dataIt)
                {
                    std::cout << *keyIt << " " << *dataIt << std::endl;
                }
            }
            std::cout << "Move leaf content" << std::endl;
            {
                auto keyIt = moveLeaf.keyBegin();
                auto dataIt = moveLeaf.dataBegin();
                for (;keyIt < moveLeaf.keyEnd(); ++keyIt, ++dataIt)
                {
                    std::cout << *keyIt << " " << *dataIt << std::endl;
                }
            }

            std::cout << "LeafGroup default constructor : " << std::endl;
            TreeT::LeafGroupT leafGroup;
            std::cout << "End of LeafGroup default constructor : " << std::endl;

            TC_AssertTrue(leafGroup.slotsUsed() == 0);

            std::cout << "Creating first leaf..." << std::endl;
            leafGroup.createFirst();
            TC_AssertTrue(leafGroup.slotsUsed() == 1);
            TreeT::LeafT &firstLeaf{leafGroup.node(0)};
            TC_AssertTrue(firstLeaf.slotsUsed() == 0);

            std::cout << "Adding 1 to firsLeaf" << std::endl;

            ptr = firstLeaf.getCreateData(KeyProbe(1), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(1);

            std::cout << "MakeSpace after first(1)." << std::endl;

            TC_AssertTrue(leafGroup.makeSpace(0) == 1);
            TC_AssertTrue(leafGroup.slotsUsed() == 2);
            TreeT::LeafT &secondLeaf{leafGroup.node(1)};
            TC_AssertTrue(secondLeaf.slotsUsed() == 0);

            std::cout << "Adding 3 to secondLeaf" << std::endl;

            ptr = secondLeaf.getCreateData(KeyProbe(3), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(3);

            std::cout << "MakeSpace after first(2)." << std::endl;

            TC_AssertTrue(leafGroup.makeSpace(0) == 1);
            TC_AssertTrue(leafGroup.slotsUsed() == 3);

            TreeT::LeafT &newSecondLeaf{leafGroup.node(1)};
            TC_AssertTrue(newSecondLeaf.slotsUsed() == 0);

            std::cout << "Adding 2 to newSecondLeaf" << std::endl;

            ptr = newSecondLeaf.getCreateData(KeyProbe(2), created);
            TC_AssertTrue(ptr && created);
            new (ptr) DataProbe(2);

            TC_AssertTrue(leafGroup.find(KeyProbe(0)) == 0);
            TC_AssertTrue(leafGroup.find(KeyProbe(1)) == 0);
            TC_AssertTrue(leafGroup.find(KeyProbe(2)) == 1);
            TC_AssertTrue(leafGroup.find(KeyProbe(3)) == 2);
            TC_AssertTrue(leafGroup.find(KeyProbe(4)) == 3);

            TreeT::LeafGroupT rightLeafGroup;

            TC_AssertTrue(leafGroup.slotsUsed() == 3);
            TC_AssertTrue(rightLeafGroup.slotsUsed() == 0);

            leafGroup.split(rightLeafGroup, 2);

            TC_AssertTrue(leafGroup.slotsUsed() == 2);
            TC_AssertTrue(rightLeafGroup.slotsUsed() == 1);

            leafGroup.combine(rightLeafGroup);

            TC_AssertTrue(leafGroup.slotsUsed() == 3);
            TC_AssertTrue(rightLeafGroup.slotsUsed() == 0);

            std::cout << "LeafGroup copy constructor" << std::endl;
            TreeT::LeafGroupT copyLeafGroup(leafGroup);
            TC_AssertTrue(copyLeafGroup.slotsUsed() == leafGroup.slotsUsed());

            std::cout << "LeafGroup move constructor" << std::endl;
            TreeT::LeafGroupT moveLeafGroup(std::move(copyLeafGroup));
            TC_AssertTrue(moveLeafGroup.slotsUsed() == leafGroup.slotsUsed());
            TC_AssertTrue(copyLeafGroup.slotsUsed() == 0);

            std::cout << "Node default constructor : " << std::endl;
            TreeT::NodeT node;
            std::cout << "End of Node default constructor : " << std::endl;

            TC_AssertTrue(node.slotsUsed() == 0);

            std::cout << "Adding keys..." << std::endl;

            u32 idBefore = KeyProbe::mNum;
            std::cout << "New" << std::endl;
            TC_Expect(node.addKey(KeyProbe(1)), 0);
            TC_Expect(node.addKey(KeyProbe(2)), 1);
            TC_Expect(node.addKey(KeyProbe(3)), 2);
            TC_Expect(node.addKey(KeyProbe(4)), 3);
            TC_Expect(node.addKey(KeyProbe(5)), 4);

            std::cout << node.slotsUsed() << std::endl;

            TC_AssertTrue(node.slotsUsed() == 5);

            std::cout << "Id" << std::endl;

            TC_AssertTrue(node.key(node.findK(KeyProbe(1))).mMyId = idBefore + 0);
            TC_AssertTrue(node.key(node.findK(KeyProbe(2))).mMyId = idBefore + 1);
            TC_AssertTrue(node.key(node.findK(KeyProbe(3))).mMyId = idBefore + 2);
            TC_AssertTrue(node.key(node.findK(KeyProbe(4))).mMyId = idBefore + 3);
            TC_AssertTrue(node.key(node.findK(KeyProbe(5))).mMyId = idBefore + 4);

            TC_AssertTrue(node.slotsUsed() == 5);

            std::cout << "End of adding keys..." << std::endl;

            std::cout << "Testing find..." << std::endl;

            TC_Expect(node.findK(KeyProbe(1)), 0);
            TC_Expect(node.findK(KeyProbe(2)), 1);
            TC_Expect(node.findK(KeyProbe(3)), 2);
            TC_Expect(node.findK(KeyProbe(4)), 3);
            TC_Expect(node.findK(KeyProbe(5)), 4);
            TC_Expect(node.findK(KeyProbe(0)), 0);
            TC_Expect(node.findK(KeyProbe(6)), 5);

            std::cout << "Printing content of node : " << std::endl;
            {
                auto keyIt = node.keyBegin();
                for (;keyIt < node.keyEnd(); ++keyIt)
                {
                    std::cout << *keyIt << std::endl;
                }
            }

            {
                TreeT::NodeT rightNode;
                std::cout << "Splitting node..." << std::endl;

                node.split(rightNode, true, 5);

                TC_Expect(node.slotsUsed(), 3);
                TC_Expect(node.key(0).mKey, 1);
                TC_Expect(node.key(1).mKey, 2);
                TC_Expect(node.key(2).mKey, 3);

                TC_Expect(rightNode.slotsUsed(), 2);
                TC_Expect(rightNode.key(0).mKey, 4);
                TC_Expect(rightNode.key(1).mKey, 5);

                std::cout << "Printing content of leftNode : " << std::endl;
                {
                    auto keyIt = node.keyBegin();
                    for (;keyIt < node.keyEnd(); ++keyIt)
                    {
                        std::cout << *keyIt << std::endl;
                    }
                }

                std::cout << "Printing content of rightNode : " << std::endl;
                {
                    auto keyIt = rightNode.keyBegin();
                    for (;keyIt < rightNode.keyEnd(); ++keyIt)
                    {
                        std::cout << *keyIt << std::endl;
                    }
                }

                std::cout << "Combining them back into one node..." << std::endl;

                node.combine(rightNode);

                TC_Expect(node.slotsUsed(), 5);
                TC_Expect(node.key(0).mKey, 1);
                TC_Expect(node.key(1).mKey, 2);
                TC_Expect(node.key(2).mKey, 3);
                TC_Expect(node.key(3).mKey, 4);
                TC_Expect(node.key(4).mKey, 5);

                TC_Expect(rightNode.slotsUsed(), 0);

                std::cout << "Printing content of leftNode : " << std::endl;
                {
                    auto keyIt = node.keyBegin();
                    for (;keyIt < node.keyEnd(); ++keyIt)
                    {
                        std::cout << *keyIt << std::endl;
                    }
                }

                std::cout << "Printing content of rightNode : " << std::endl;
                {
                    auto keyIt = rightNode.keyBegin();
                    for (;keyIt < rightNode.keyEnd(); ++keyIt)
                    {
                        std::cout << *keyIt << std::endl;
                    }
                }
            }

            std::cout << "Node copy construction" << std::endl;
            TreeT::NodeT copyNode(node);
            TC_AssertTrue(copyNode.slotsUsed() == node.slotsUsed());
            std::cout << "Copy node content" << std::endl;
            {
                auto keyIt = copyNode.keyBegin();
                for (;keyIt < copyNode.keyEnd(); ++keyIt)
                {
                    std::cout << *keyIt << std::endl;
                }
            }

            std::cout << "Node move construction" << std::endl;
            TreeT::NodeT moveNode(std::move(copyNode));
            TC_AssertTrue(moveNode.slotsUsed() == node.slotsUsed());
            TC_AssertTrue(copyNode.slotsUsed() == 0);
            std::cout << "Copy node content" << std::endl;
            {
                auto keyIt = copyNode.keyBegin();
                for (;keyIt < copyNode.keyEnd(); ++keyIt)
                {
                    std::cout << *keyIt << std::endl;
                }
            }
            std::cout << "Move node content" << std::endl;
            {
                auto keyIt = moveNode.keyBegin();
                for (;keyIt < moveNode.keyEnd(); ++keyIt)
                {
                    std::cout << *keyIt << std::endl;
                }
            }

            std::cout << "NodeGroup default constructor : " << std::endl;
            TreeT::NodeGroupT nodeGroup;
            std::cout << "End of NodeGroup default constructor : " << std::endl;

            TC_Expect(nodeGroup.slotsUsed(), 0);

            std::cout << "Creating first node..." << std::endl;
            nodeGroup.createFirst();
            TC_Expect(nodeGroup.slotsUsed(), 1);
            TreeT::NodeT &firstNode{nodeGroup.node(0)};
            TC_Expect(firstNode.slotsUsed(), 0);

            std::cout << "Adding 1 to firstNode" << std::endl;

            firstNode.addKey(KeyProbe(1));

            std::cout << "MakeSpace after first(1)." << std::endl;

            TC_Expect(nodeGroup.makeSpace(0), 1);
            TC_Expect(nodeGroup.slotsUsed(), 2);
            TreeT::NodeT &secondNode{nodeGroup.node(1)};
            std::cout << secondNode.slotsUsed() << std::endl;
            TC_Expect(secondNode.slotsUsed(), 0);

            std::cout << "Adding 3 to secondNode" << std::endl;

            secondNode.addKey(KeyProbe(3));

            std::cout << "MakeSpace after first(2)." << std::endl;

            TC_Expect(nodeGroup.makeSpace(0), 1);
            TC_Expect(nodeGroup.slotsUsed(), 3);

            TreeT::NodeT &newSecondNode{nodeGroup.node(1)};
            TC_Expect(newSecondNode.slotsUsed(), 0);

            std::cout << "Adding 2 to newSecondNode" << std::endl;

            newSecondNode.addKey(KeyProbe(2));

            TC_Expect(nodeGroup.find(KeyProbe(0)), 0);
            TC_Expect(nodeGroup.find(KeyProbe(1)), 0);
            TC_Expect(nodeGroup.find(KeyProbe(2)), 1);
            TC_Expect(nodeGroup.find(KeyProbe(3)), 2);
            TC_Expect(nodeGroup.find(KeyProbe(4)), 3);

            TreeT::NodeGroupT rightNodeGroup;

            TC_Expect(nodeGroup.slotsUsed(), 3);
            TC_Expect(rightNodeGroup.slotsUsed(), 0);

            nodeGroup.split(rightNodeGroup, 2);

            TC_Expect(nodeGroup.slotsUsed(), 2);
            TC_Expect(rightNodeGroup.slotsUsed(), 1);

            nodeGroup.combine(rightNodeGroup);

            TC_Expect(nodeGroup.slotsUsed(), 3);
            TC_Expect(rightNodeGroup.slotsUsed(), 0);

            std::cout << "NodeGroup copy constructor" << std::endl;
            TreeT::NodeGroupT copyNodeGroup(nodeGroup);
            TC_Expect(copyNodeGroup.slotsUsed(), nodeGroup.slotsUsed());

            std::cout << "NodeGroup move constructor" << std::endl;
            TreeT::NodeGroupT moveNodeGroup(std::move(copyNodeGroup));
            TC_Expect(moveNodeGroup.slotsUsed(), nodeGroup.slotsUsed());
            TC_Expect(copyNodeGroup.slotsUsed(), 0);

            std::cout << "Iterator default constructor : " << std::endl;
            TreeT::Iterator it;
            std::cout << "End of Iterator default constructor : " << std::endl;

            TC_Expect(it.valid(), false);

            std::cout << "ConstIterator default constructor : " << std::endl;
            TreeT::ConstIterator cIt;
            std::cout << "End of ConstIterator default constructor : " << std::endl;

            TC_Expect(cIt.valid(), false);

            std::cout << "Tree default constructor : " << std::endl;
            TreeT tree;
            std::cout << "End of Tree default constructor : " << std::endl;

            u32 dataIdStart{DataProbe::mNum};

            std::cout << "Inserting a new key" << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(1), 1).valid());
            TC_Expect(DataProbe::mNum, dataIdStart + 1);

            std::cout << "Inserting the same key" << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(1), 1).valid());
            TC_Expect(DataProbe::mNum, dataIdStart + 2);

            std::cout << "Linear iteration over the tree" << std::endl;
            for (const auto &pair : tree)
            {
                std::cout << pair.first << " " << pair.second << std::endl;
            }

            std::cout << "Inserting 10 keys" << std::endl;
            for (u32 iii= 0; iii < 20; iii += 2)
            {
                std::cout << "== " << iii << std::endl;
                TC_AssertTrue(tree.insert(KeyProbe(iii), iii).valid());
                std::cout << "!= " << iii << std::endl;
            }

            std::cout << "Out of order insertion" << std::endl;
            std::cout << "== " << 17 << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(17), 17).valid());
            tree.verify();
            std::cout << "!= " << 17 << std::endl;
            std::cout << "== " << 9 << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(9), 9).valid());
            tree.verify();
            std::cout << "!= " << 9 << std::endl;
            std::cout << "== " << 13 << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(13), 13).valid());
            tree.verify();
            std::cout << "!= " << 13 << std::endl;
            std::cout << "== " << 19 << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(13), 13).valid());
            tree.verify();
            std::cout << "!= " << 19 << std::endl;
            std::cout << "== " << 11 << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(11), 11).valid());
            tree.verify();
            std::cout << "!= " << 11 << std::endl;

            std::cout << "Linear iteration over the tree" << std::endl;
            //for (const auto &pair : tree)
            for (auto it = tree.begin(); it != tree.end(); ++it)
            {
                const auto &pair{*it};
                std::cout << pair.first << " " << pair.second << std::endl;
            }

            std::cout << "Triggering Leaf split and new root creation." << std::endl;
            TC_AssertTrue(tree.insert(KeyProbe(42), 42).valid());

            std::cout << "Adding more elements" << std::endl;
            for (u32 iii = 43; iii < 50; ++iii)
            {
                TC_AssertTrue(tree.insert(KeyProbe(iii), iii).valid());
                auto it{tree.search(KeyProbe(iii))};
                TC_AssertTrue(it.valid());
                TC_Expect((*it).first.mKey, iii);
                TC_Expect((*it).second.mData, iii);
                TC_AssertTrue(!(++it).valid());
            }

            std::cout << "Verifying the tree" << std::endl;
            tree.verify();

            std::cout << "Debug tree print" << std::endl;
            std::ofstream myFile("graph.dot");
            tree.debugPrint(myFile);
            myFile.close();

            std::cout << "Removing elements" << std::endl;
            for (u32 iii = 43; iii < 50; ++iii)
            {
                std::cout << "Removing element : " << iii << std::endl;
                TC_AssertTrue(tree.remove(KeyProbe(iii)));

                std::cout << "Verifying the tree" << std::endl;
                tree.verify();
            }

            std::cout << "Removing all elements" << std::endl;
            while (!tree.empty())
            {
                TC_AssertTrue(tree.begin().valid());
                std::cout << "Removing element : " << (*tree.begin()).first << std::endl;
                TC_AssertTrue(tree.remove((*tree.begin()).first));
                tree.verify();
            }

            std::cout << "Debug tree print after remove" << std::endl;
            std::ofstream myFileRem("graphRem.dot");
            tree.debugPrint(myFileRem);
            myFileRem.close();

            tree.debugPrintStats(std::cout);

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 3, 3>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                mem::List<u64> indices(100);
                for (u64 iii = 0; iii < 100; ++iii)
                {
                    indices.pushBack(iii);
                }

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    u64 index{std::rand() % indices.size()};
                    u64 key{indices[index]};
                    TC_AssertTrue(indices.remove(index));

                    TC_AssertTrue(delTree.remove(key));
                    delTree.verify();
                }

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 4, 4>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                mem::List<u64> indices(100);
                for (u64 iii = 0; iii < 100; ++iii)
                {
                    indices.pushBack(iii);
                }

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    u64 index{std::rand() % indices.size()};
                    u64 key{indices[index]};
                    TC_AssertTrue(indices.remove(index));

                    TC_AssertTrue(delTree.remove(key));
                    delTree.verify();
                }

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 8, 8>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                mem::List<u64> indices(100);
                for (u64 iii = 0; iii < 100; ++iii)
                {
                    indices.pushBack(iii);
                }

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    u64 index{std::rand() % indices.size()};
                    u64 key{indices[index]};
                    TC_AssertTrue(indices.remove(index));

                    TC_AssertTrue(delTree.remove(key));
                    delTree.verify();
                }

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 5, 5>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                for (u64 iii = 99; iii > 0; --iii)
                {
                    TC_AssertTrue(delTree.remove(iii));
                    delTree.verify();
                }

                TC_AssertTrue(delTree.remove(0));
                delTree.verify();

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 5, 5>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                for (u64 iii = 0; iii < 100; ++iii)
                {
                    TC_AssertTrue(delTree.remove(iii));
                    /*
                    delTreeFile.open(std::string("delGraph/DelTreeDel") +
                                     std::to_string(iii) + ":" +
                                     std::to_string(key) + ".dot");
                    delTree.debugPrint(delTreeFile);
                    delTreeFile.close();
                     */
                    delTree.verify();
                }

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>,
                    mem::DefaultAllocator<u64>, 64, 12, 12>;
                using DelTreeT = mem::BPTree<CFG>;

                DelTreeT delTree;

                for (u64 iii = 0; iii < 200; ++iii)
                {
                    delTree.insert(iii, iii);
                    delTree.verify();
                }

                std::cout << "DelTree after insert" << std::endl;
                std::ofstream delTreeFile("delTreeIns.dot");
                delTree.debugPrint(delTreeFile);
                delTreeFile.close();

                for (u64 iii = 0; iii < 200; ++iii)
                {
                    TC_AssertTrue(delTree.remove(iii));
                    if (iii > 80)
                    {
                        delTreeFile.open(std::string("delGraph/DelTreeDel") +
                                         std::to_string(iii) + ":" +
                                         std::to_string(iii) + ".dot");
                        delTree.debugPrint(delTreeFile);
                        delTreeFile.close();
                    }
                    delTree.verify();
                }

                TC_AssertTrue(delTree.empty());
            }

            {
                using CFG = mem::BPTreeConfig<u64, u64>;
                using NewTreeT = mem::BPTree<CFG>;
                NewTreeT newTree;

                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    PROF_SCOPE(CSBP_Insert1M);
                    newTree.insert(iii, iii);
                }

                {
                    PROF_SCOPE(CSBP_Verify);
                    newTree.verify();
                }

                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    PROF_SCOPE(CSBP_Search1M);
                    newTree.search(iii);
                }

                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    u64 randNum{std::rand() % 1000000};
                    PROF_SCOPE(CSBP_RandomSearch1M);
                    newTree.search(randNum);
                }

                u64 jumpyIndex{0};
                while (jumpyIndex < 1000000)
                {
                    PROF_SCOPE(CSBP_JumpySearch);
                    newTree.search(jumpyIndex);
                    jumpyIndex += std::rand() % 8;
                }

                {
                    PROF_SCOPE(CSBP_Verify);
                    newTree.verify();
                }

                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    PROF_SCOPE(CSBP_Remove1M);
                    newTree.remove(iii);
                }

                {
                    PROF_SCOPE(CSBP_Verify);
                    newTree.verify();
                }

                newTree.debugPrintStats(std::cout);
            }
#elif 0

            {
                static constexpr u16 BF{4};
                using KeyT = u8;

                using Node = TestNode<KeyT, BF>;

                Node memNode;

                memNode.printStats();

                struct NodeSOA
                {
                    KeyT keys[10 * BF];
                    u16 offsets[10 * (BF + 1)];
                    void *childGroups[10];
                    u16 slotsUsed[10];
                };

                std::cout << "Size of NodeSOA [B] : " <<
                    sizeof(NodeSOA) << std::endl;

                u8 *begin = reinterpret_cast<u8*>(&memNode);
                u8 *end = reinterpret_cast<u8*>(&memNode + 1);

                std::cout << std::hex;
                for (auto it = begin; it != end; ++it)
                {
                    std::cout << static_cast<u16>(*it) << " ";
                }
                std::cout << std::dec << std::endl;
            }

            {
                struct DataS
                {
                    u64 a;
                    u64 b;
                };

                static constexpr u16 BF{4};
                using KeyT = u8;
                using DataT = u64;

                using Leaf = TestLeaf<KeyT, DataT, BF>;

                Leaf memLeaf;

                memLeaf.printStats();

                struct LeafSOA
                {
                    u16 slotsUsed[10];
                    KeyT keys[10 * BF];
                    DataT data[10 * BF];
                };

                std::cout << "Size of LeafSOA [B] : " <<
                    sizeof(LeafSOA) << std::endl;

                u8 *begin = reinterpret_cast<u8*>(&memLeaf);
                u8 *end = reinterpret_cast<u8*>(&memLeaf + 1);

                std::cout << std::hex;
                for (auto it = begin; it != end; ++it)
                {
                    std::cout << static_cast<u16>(*it) << " ";
                }
                std::cout << std::dec << std::endl;
            }
#endif
        TEST_UNIT_MAIN_END

    TEST_UNIT_END(TestingTheTest)
}

int main(int argc, char* argv[])
{
    TC_Run();
    TC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);
}

