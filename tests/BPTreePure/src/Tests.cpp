/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

int main(int argc, char* argv[])
{
    static constexpr u64 MAX_ELE{1000000};
    static constexpr u64 JUMPY_SEED{997};
#ifndef NDEBUG
    std::cout << "NDEBUG is not set!" << std::endl;
#endif

    {
        PROF_SCOPE("CSBP");

        //using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>, mem::DefaultAllocator<u64>, 64, 16, 16>;
        using CFG = mem::BPTreeConfig<u64, u64, std::less<u64>, std::allocator<u64>, 128>;
        using NewTreeT = mem::BPTree<CFG>;
        NewTreeT newTree;

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            PROF_SCOPE("CSBP_Insert1M");
            newTree.insert(iii, iii);
        }

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            PROF_SCOPE("CSBP_Search1M");
            newTree.search(iii);
        }

        std::srand(JUMPY_SEED);

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            u64 randNum{std::rand() % MAX_ELE};
            PROF_SCOPE("CSBP_RandomSearch1M");
            newTree.search(randNum);
        }

        u64 jumpyIndex{0};
        while (jumpyIndex < MAX_ELE)
        {
            PROF_SCOPE("CSBP_JumpySearch");
            newTree.search(jumpyIndex);
            jumpyIndex += std::rand() % 8;
        }

        mem::List<u64> list(MAX_ELE, 0);
        ASSERT_SLOW(list.size() == MAX_ELE);
        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            list[iii] = iii;
            ASSERT_SLOW(list[iii] == iii);
        }

        ASSERT_SLOW(list.size() == MAX_ELE);

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            u64 randNum{std::rand() % list.size()};
            u64 remEle{list[randNum]};
            list.remove(randNum);
            PROF_SCOPE("CSBP_RandomRemove");
            bool res = newTree.remove(remEle);
            ASSERT_SLOW(res);
        }

        ASSERT_SLOW(list.empty());
        ASSERT_SLOW(newTree.empty());
    }

    /*
    newTree.debugPrintStats(std::cout);
    */

    {
        PROF_SCOPE("STX");

        using MapType = stx::btree<u64, u64,
            std::pair<u64, u64>,
            std::less<u64>,
            stx::btree_default_map_traits<u64, u64>,
            false>;
        MapType stxTree;

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            PROF_SCOPE("STX_Insert1M");
            stxTree.insert(iii, iii);
        }

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            PROF_SCOPE("STX_Search1M");
            stxTree.find(iii);
        }

        std::srand(JUMPY_SEED);

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            u64 randNum{std::rand() % MAX_ELE};
            PROF_SCOPE("STX_RandomSearch1M");
            stxTree.find(randNum);
        }

        u64 jumpyIndex{0};
        while (jumpyIndex < MAX_ELE)
        {
            PROF_SCOPE("STX_JumpySearch");
            stxTree.find(jumpyIndex);
            jumpyIndex += std::rand() % 8;
        }

        mem::List<u64> list(MAX_ELE, 0);
        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            list[iii] = iii;
        }

        for (u64 iii = 0; iii < MAX_ELE; ++iii)
        {
            u64 randNum{std::rand() & list.size()};
            u64 remEle{list[randNum]};
            list.remove(randNum);
            PROF_SCOPE("STX_RandomRemove");
            stxTree.erase(remEle);
        }

        ASSERT_SLOW(list.empty());
        ASSERT_SLOW(stxTree.empty());

        /*
        std::cout << "Inner nodes : " << newTree.get_stats().innernodes << std::endl;
        std::cout << "Inner slots : " << newTree.get_stats().innerslots << std::endl;
        std::cout << "Item count : " << newTree.get_stats().itemcount << std::endl;
        std::cout << "Leaf slots : " << newTree.get_stats().leafslots << std::endl;
        std::cout << "Leafs : " << newTree.get_stats().leaves << std::endl;
        std::cout << "Nodes : " << newTree.get_stats().nodes() << std::endl;
        std::cout << "Node size : " << sizeof(MapType::inner_node) << std::endl;
        std::cout << "Leaf size : " << sizeof(MapType::leaf_node) << std::endl;
        */
    }

    prof::PrintCrawler pc;
    PROF_DUMP(pc);
}

