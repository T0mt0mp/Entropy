/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <vector>

struct ListTester
{
    ListTester() { constructed++; }
    ListTester(u64 num) : mData{num} { constructed++; }
    ListTester(const ListTester &rhs) { copyConstructed++; }
    ListTester(ListTester &&rhs) { moveConstructed++; }
    ListTester &operator=(const ListTester &rhs) { copyAssigned++; return *this; }
    ListTester &operator=(ListTester &&rhs) { moveAssigned++; return *this; }
    ~ListTester() { destructed++; }

    u64 mData;

    static u64 constructed;
    static u64 copyConstructed;
    static u64 moveConstructed;
    static u64 copyAssigned;
    static u64 moveAssigned;
    static u64 destructed;
};

u64 ListTester::constructed;
u64 ListTester::copyConstructed;
u64 ListTester::moveConstructed;
u64 ListTester::copyAssigned;
u64 ListTester::moveAssigned;
u64 ListTester::destructed;

TU_Begin(EntropyMisc)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

    TU_Case(List0, "Testing the List class")
    {
        ent::List<u64> list0;

        TC_RequireEqual(list0.size(), 0u);
        TC_RequireEqual(list0.capacity(), 0u);
        TC_Require(list0.begin() == nullptr);
        TC_Require(list0.end() == nullptr);
        TC_Require(list0.data() == nullptr);
        TC_RequireException(list0.at(0), std::out_of_range);

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            list0.pushBack(iii);
        }

        TC_RequireEqual(list0.size(), 1000u);
        TC_RequireEqual(list0.capacity(), 1024u);
        TC_Require(list0.begin() != nullptr);
        TC_Require(list0.end() != nullptr);
        TC_Require(list0.data() != nullptr);

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            TC_RequireEqual(list0[iii], list0.at(iii));
            TC_RequireEqual(list0[iii], iii);
        }

        auto oldPtr{list0.data()};

        list0.shrinkToFit();

        TC_RequireEqual(list0.size(), 1000u);
        TC_RequireEqual(list0.capacity(), 1000u);
        TC_Require(list0.data() != oldPtr);

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            TC_RequireEqual(list0[iii], list0.at(iii));
            TC_RequireEqual(list0[iii], iii);
        }

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            list0.popBack();
        }

        TC_RequireEqual(list0.size(), 0u);
        TC_RequireEqual(list0.capacity(), 1000u);

        list0.shrinkToFit();

        TC_RequireEqual(list0.size(), 0u);
        TC_RequireEqual(list0.capacity(), 0u);
        TC_Require(list0.data() == nullptr);
    }

    TU_Case(List1, "Testing the List class")
    {
        ent::List<ListTester> list1(1000);

        TC_RequireEqual(list1.size(), 1000u);
        TC_RequireEqual(list1.capacity(), 1000u);

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            list1.set(iii, iii);
        }

        TC_RequireEqual(ListTester::constructed, 1000u);

        for (u64 iii = 0; iii < 1000; ++iii)
        {
            TC_RequireEqual(list1[iii].mData, list1.at(iii).mData);
            TC_RequireEqual(list1[iii].mData, iii);
        }

        {
            ent::List<ListTester> list11(list1);
            for (u64 iii = 0; iii < 1000; ++iii)
            {
                TC_RequireEqual(list11[iii].mData, iii);
            }
        }

        {
            ent::List<ListTester> list12(1000, ListTester(42));
            for (u64 iii = 0; iii < 1000; ++iii)
            {
                TC_RequireEqual(list12[iii].mData, 42u);
            }
        }

        {
            std::vector<u64> vec;
            for (u64 iii = 0; iii < 1000; ++iii)
            {
                vec.push_back(iii);
            }
            ent::List<ListTester> list13(vec.begin(), vec.end());
            for (u64 iii = 0; iii < 1000; ++iii)
            {
                TC_RequireEqual(list13[iii].mData, iii);
            }
        }

        {
            ent::List<ListTester> list14({0, 1, 2, 3, 4, 5});
            for (u64 iii = 0; iii < 6; ++iii)
            {
                TC_RequireEqual(list14[iii].mData, iii);
            }

            ent::List<ListTester> list15(std::move(list14));
            for (u64 iii = 0; iii < 6; ++iii)
            {
                TC_RequireEqual(list15[iii].mData, iii);
            }

            ent::List<ListTester> list16;
            list16 = list15;
            for (u64 iii = 0; iii < 6; ++iii)
            {
                TC_RequireEqual(list16[iii].mData, iii);
            }

            list16 = std::move(list15);
            for (u64 iii = 0; iii < 6; ++iii)
            {
                TC_RequireEqual(list16[iii].mData, iii);
            }
        }

        TC_CheckEqual(ListTester::constructed, 2007u);
        TC_CheckEqual(ListTester::copyConstructed, 0u);
        TC_CheckEqual(ListTester::copyAssigned, 0u);
        TC_CheckEqual(ListTester::moveConstructed, 0u);
        TC_CheckEqual(ListTester::moveAssigned, 0u);
        TC_CheckEqual(ListTester::destructed, 2007u);
    }

    TU_Case(List2, "Testing the List class")
    {
        ent::List<u64> list2(1000u, 1u);

        ent::List<u64> list21;
        list21.insert(list21.begin(), 42u);

        TC_RequireEqual(list21.size(), 1u);
        TC_RequireEqual(list21.at(0), 42u);

        list21.insert(list21.begin(), 10, 0);
        for (u64 iii = 0; iii < 10; ++iii)
        {
            TC_RequireEqual(list21.at(iii), 0u);
        }
        TC_RequireEqual(list21.at(10), 42u);

        list21.insert(list21.end(), list2.begin() + 1, list2.end() - 1);
        TC_RequireEqual(list21.size(), 11u + 998u);
        u64 counter{0};
        for (u64 &e : list21)
        {
            u64 expected{0};
            if (counter == 10)
            {
                expected = 42;
            }
            else if (counter > 10)
            {
                expected = 1;
            }

            TC_RequireEqual(e, expected);

            counter++;
        }
        TC_RequireEqual(counter, 11u + 998u);
    }

    TU_Case(ListPerformance, "Testing the List performance")
    {
        using Type = ent::EntityId;
        using List = ent::List<Type>;
        using Vector = std::vector<Type>;
        static constexpr u64 SIZE{1000000};
        static constexpr ent::EntityId VALUE{0};

        {
            PROF_SCOPE("Construct empty");

            PROF_BLOCK("List");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                List list;
            }
            PROF_BLOCK_END();

            PROF_BLOCK("Vector");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                Vector vector;
            }
            PROF_BLOCK_END();
        }

        {
            PROF_SCOPE("Construct size");

            PROF_BLOCK("List");
            List list(SIZE);
            PROF_BLOCK_END();

            PROF_BLOCK("Vector");
            Vector vector(SIZE);
            PROF_BLOCK_END();
        }

        {
            PROF_SCOPE("Construct size value");

            PROF_BLOCK("List");
            List list(SIZE, VALUE);
            PROF_BLOCK_END();

            PROF_BLOCK("Vector");
            Vector vector(SIZE, VALUE);
            PROF_BLOCK_END();
        }

        {
            PROF_SCOPE("Push pop");

            PROF_BLOCK("List");
            List list;

            PROF_BLOCK("Push");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                list.pushBack(VALUE);
            }
            PROF_BLOCK_END();

            PROF_BLOCK("Pop");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                list.popBack();
            }
            PROF_BLOCK_END();
            PROF_BLOCK_END();

            PROF_BLOCK("Vector");
            Vector vector;
            PROF_BLOCK("Push");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                vector.push_back(VALUE);
            }
            PROF_BLOCK_END();

            PROF_BLOCK("Pop");
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                vector.pop_back();
            }
            PROF_BLOCK_END();
            PROF_BLOCK_END();
        }

        {
            PROF_SCOPE("Push pop, with reserve");

            PROF_BLOCK("List");
            List list;
            list.reserve(SIZE);
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                list.pushBack(VALUE);
            }
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                list.popBack();
            }
            PROF_BLOCK_END();

            PROF_BLOCK("Vector");
            Vector vector;
            vector.reserve(SIZE);
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                vector.push_back(VALUE);
            }
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                vector.pop_back();
            }
            PROF_BLOCK_END();
        }
    }

    TU_Case(SortedList, "Testing the SortedList class")
    {
        static constexpr u64 NUM_ELEMENTS{100000};
        ent::SortedList<u64> list;

        for (u64 iii = NUM_ELEMENTS; iii > 0; --iii)
        {
            list.insert(iii);
        }
        for (u64 iii = NUM_ELEMENTS; iii > 0; --iii)
        {
            list.insert(iii);
        }
        for (u64 iii = 1; iii <= NUM_ELEMENTS; ++iii)
        {
            list.insert(iii);
        }

        TC_RequireEqual(list.size(), NUM_ELEMENTS);

        for (u64 iii = 1; iii <= NUM_ELEMENTS; ++iii)
        {
            TC_RequireEqual(list[iii - 1], iii);
        }

        for (u64 iii = 1; iii <= NUM_ELEMENTS / 2; ++iii)
        {
            list.erase(iii);
        }
        TC_RequireEqual(list.size(), NUM_ELEMENTS / 2);

        for (u64 iii = 0; iii <= NUM_ELEMENTS / 2 - 1; ++iii)
        {
            TC_RequireEqual(list[iii], NUM_ELEMENTS / 2 + iii + 1);
        }
    }

TU_End(EntropyMisc)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

