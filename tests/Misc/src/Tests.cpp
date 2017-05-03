/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <vector>
#include <cmath>

class Test : public ent::Universe<Test>
{};

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

    TU_Case(TripleMergeSort, "Testing triple merge sort")
    {
        PROF_BLOCK("TripleMergeSort");

        std::vector<u64> input1{2, 3, 4};
        std::vector<u64> input2{1, 4, 5, 6, 7};
        std::vector<u64> input3{0, 4, 7, 8, 9};

        std::vector<u64> output;
        output.resize(input1.size() + input2.size() + input3.size());

        auto iit1 = input1.cbegin();
        auto ieit1 = input1.cend();
        auto iit2 = input2.cbegin();
        auto ieit2 = input2.cend();
        auto iit3 = input3.cbegin();
        auto ieit3 = input3.cend();

        auto oit = output.begin();
        auto oeit = output.end();

        for (;oit != oeit && (iit1 != ieit1 || iit2 != ieit2 || iit3 != ieit3);
             ++oit)
        {
            /*
             *                   A < B
             *           T                   F
             *         A < C               B < C
             *    T            F    T               F
             *    |            |    |               |
             *    A            C    B               C
             *                 +    +               +
             *            ?C == A ?B == A         ?C == B
             *                                    ?B == A
             */

            /*
            // A < B
            if ((iit1 != ieit1) && ((iit2 != ieit2) || (*iit1 < *iit2)))
            { // A < B
                // A < C
                if ((iit3 != ieit3) || (*iit1 < *iit3))
                { // A < C
                    // -> A
                    *oit = *iit1;
                    ++iit1;
                }
                else
                { // C <= A
                    // -> C + ?C == A?
                    *oit = *iit3;
                    ++iit3;
                }
            }
            else
            { // B >= A
                // B < C
                if ((iit2 != ieit2) && ((iit3 != ieit3) || (*iit2 < *iit3)))
                { // B < C
                    // -> B + ?B == A?
                    *oit = *iit2;
                    ++iit2;
                }
                else
                { // C >= B >= A
                    // -> C + ?C == B? + ?B == A?
                    *oit = *iit3;
                    ++iit3;
                }
            }
             */

            bool a{iit1 != ieit1};
            ENT_UNUSED(a);
            bool b{iit2 != ieit2};
            ENT_UNUSED(b);
            bool c{iit3 != ieit3};
            ENT_UNUSED(c);
            *oit = (iit1 != ieit1) && ((iit2 == ieit2) || (*iit1 < *iit2)) ?
                   (iit3 == ieit3) || (*iit1 < *iit3) ?
                   (*iit1) : (*iit3)
                                                                           :
                   (iit2 != ieit2) && ((iit3 == ieit3) || (*iit2 < *iit3)) ?
                   (*iit2) : (*iit3);

            while ((iit1 != ieit1) && (*iit1 == *oit))
            {
                ++iit1;
            }
            while ((iit2 != ieit2) && (*iit2 == *oit))
            {
                ++iit2;
            }
            while ((iit3 != ieit3) && (*iit3 == *oit))
            {
                ++iit3;
            }
        }

        u64 finalSize{static_cast<u64>(oit - output.begin())};
        TC_RequireEqual(finalSize, 10u);
        output.resize(finalSize);

        u64 counter{0};
        for (u64 &el : output)
        {
            TC_RequireEqual(el, counter);
            counter++;
        }

        TC_RequireEqual(counter, 10u);
    }

    TU_Case(InfoBitset0, "Testing the InfoBitset class")
    {
        PROF_SCOPE("Bitsets");

        {
            PROF_SCOPE("Bitset size 199.");

            static ENT_CONSTEXPR_EXPR u64 SIZE{199u};
            using Bitset = ent::InfoBitset<SIZE>;
            TC_RequireEqual(Bitset::size(), SIZE);
            TC_RequireEqual(sizeof(Bitset), 4 * sizeof(u64));
            TC_RequireEqual(Bitset::excess(), 256u - SIZE);

            Bitset bitset1;
            TC_Require(bitset1.none());

            bitset1.set(SIZE - 1);
            TC_Require(bitset1.any());
            TC_RequireEqual(bitset1.count(), 1u);
            TC_Require(bitset1.test(SIZE - 1));

            bitset1.set();
            TC_Require(bitset1.all());

            bitset1.reset();
            TC_Require(!bitset1.all());
            TC_Require(!bitset1.any());
            TC_Require(bitset1.none());

            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                bitset1.set(iii);
            }
            TC_Require(bitset1.all());
            TC_Require(bitset1.any());
            TC_Require(!bitset1.none());
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                TC_Require(bitset1.test(iii));
            }
            TC_RequireEqual(bitset1.count(), SIZE);

            static constexpr u64 POS{4};
            Bitset bitset2{1u << POS};
            TC_Require(!bitset2.all());
            TC_Require(bitset2.any());
            TC_Require(!bitset2.none());
            TC_Require(bitset2.test(POS));

            TC_RequireNEqual(bitset1, bitset2);
            TC_RequireNEqual(bitset2, bitset1);

            std::string test(SIZE, '0');
            test[4] = '1';
            TC_RequireEqual(bitset2.toString(), test);

            Bitset bitset3(bitset1);
            TC_Require(bitset3.all());
            TC_Require(bitset3.any());
            TC_Require(!bitset3.none());

            TC_RequireEqual(bitset1, bitset3);
            TC_RequireEqual(bitset3, bitset1);

            bitset2.copy(bitset3);
            TC_RequireEqual(bitset2, bitset3);
            TC_RequireEqual(bitset2, bitset1);

            Bitset bitset4;
            TC_RequireNEqual(bitset4, bitset1);

            bitset2.swap(bitset4);
            TC_Require(!bitset2.all());
            TC_Require(!bitset2.any());
            TC_Require(bitset2.none());
            TC_RequireEqual(bitset4, bitset1);

            TC_RequireEqual(bitset2 | bitset4, bitset4);
            TC_RequireEqual(Bitset{62u} & bitset4, Bitset{62u});
        }

        {
            PROF_SCOPE("Bitset size 128.");

            static ENT_CONSTEXPR_EXPR u64 SIZE{128u};
            using Bitset = ent::InfoBitset<SIZE>;
            TC_RequireEqual(Bitset::size(), SIZE);
            TC_RequireEqual(sizeof(Bitset), 2 * sizeof(u64));
            TC_RequireEqual(Bitset::excess(), 0u);

            Bitset bitset1;
            TC_Require(bitset1.none());

            bitset1.set(SIZE - 1);
            TC_Require(bitset1.any());
            TC_RequireEqual(bitset1.count(), 1u);
            TC_Require(bitset1.test(SIZE - 1));

            bitset1.set();
            TC_Require(bitset1.all());

            bitset1.reset();
            TC_Require(!bitset1.all());
            TC_Require(!bitset1.any());
            TC_Require(bitset1.none());

            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                bitset1.set(iii);
            }
            TC_Require(bitset1.all());
            TC_Require(bitset1.any());
            TC_Require(!bitset1.none());
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                TC_Require(bitset1.test(iii));
            }
            TC_RequireEqual(bitset1.count(), SIZE);

            static constexpr u64 POS{4};
            Bitset bitset2{1u << POS};
            TC_Require(!bitset2.all());
            TC_Require(bitset2.any());
            TC_Require(!bitset2.none());
            TC_Require(bitset2.test(POS));

            TC_RequireNEqual(bitset1, bitset2);
            TC_RequireNEqual(bitset2, bitset1);

            std::string test(SIZE, '0');
            test[4] = '1';
            TC_RequireEqual(bitset2.toString(), test);

            Bitset bitset3(bitset1);
            TC_Require(bitset3.all());
            TC_Require(bitset3.any());
            TC_Require(!bitset3.none());

            TC_RequireEqual(bitset1, bitset3);
            TC_RequireEqual(bitset3, bitset1);

            bitset2.copy(bitset3);
            TC_RequireEqual(bitset2, bitset3);
            TC_RequireEqual(bitset2, bitset1);

            Bitset bitset4;
            TC_RequireNEqual(bitset4, bitset1);

            bitset2.swap(bitset4);
            TC_Require(!bitset2.all());
            TC_Require(!bitset2.any());
            TC_Require(bitset2.none());
            TC_RequireEqual(bitset4, bitset1);

            TC_RequireEqual(bitset2 | bitset4, bitset4);
            TC_RequireEqual(Bitset{62u} & bitset4, Bitset{62u});
        }

        {
            PROF_SCOPE("Bitset size 64.");

            static ENT_CONSTEXPR_EXPR u64 SIZE{64u};
            using Bitset = ent::InfoBitset<SIZE>;
            TC_RequireEqual(Bitset::size(), SIZE);
            TC_RequireEqual(sizeof(Bitset), sizeof(u64));
            TC_RequireEqual(Bitset::excess(), 0u);

            Bitset bitset1;
            TC_Require(bitset1.none());

            bitset1.set(SIZE - 1);
            TC_Require(bitset1.any());
            TC_RequireEqual(bitset1.count(), 1u);
            TC_Require(bitset1.test(SIZE - 1));

            bitset1.set();
            TC_Require(bitset1.all());

            bitset1.reset();
            TC_Require(!bitset1.all());
            TC_Require(!bitset1.any());
            TC_Require(bitset1.none());

            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                bitset1.set(iii);
            }
            TC_Require(bitset1.all());
            TC_Require(bitset1.any());
            TC_Require(!bitset1.none());
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                TC_Require(bitset1.test(iii));
            }
            TC_RequireEqual(bitset1.count(), SIZE);

            static constexpr u64 POS{4};
            Bitset bitset2{1u << POS};
            TC_Require(!bitset2.all());
            TC_Require(bitset2.any());
            TC_Require(!bitset2.none());
            TC_Require(bitset2.test(POS));

            TC_RequireNEqual(bitset1, bitset2);
            TC_RequireNEqual(bitset2, bitset1);

            std::string test(SIZE, '0');
            test[4] = '1';
            TC_RequireEqual(bitset2.toString(), test);

            Bitset bitset3(bitset1);
            TC_Require(bitset3.all());
            TC_Require(bitset3.any());
            TC_Require(!bitset3.none());

            TC_RequireEqual(bitset1, bitset3);
            TC_RequireEqual(bitset3, bitset1);

            bitset2.copy(bitset3);
            TC_RequireEqual(bitset2, bitset3);
            TC_RequireEqual(bitset2, bitset1);

            Bitset bitset4;
            TC_RequireNEqual(bitset4, bitset1);

            bitset2.swap(bitset4);
            TC_Require(!bitset2.all());
            TC_Require(!bitset2.any());
            TC_Require(bitset2.none());
            TC_RequireEqual(bitset4, bitset1);

            TC_RequireEqual(bitset2 | bitset4, bitset4);
            TC_RequireEqual(Bitset{62u} & bitset4, Bitset{62u});
        }

        {
            PROF_SCOPE("Bitset size 32.");

            static ENT_CONSTEXPR_EXPR u64 SIZE{32u};
            using Bitset = ent::InfoBitset<SIZE>;
            TC_RequireEqual(Bitset::size(), SIZE);
            TC_RequireEqual(sizeof(Bitset), sizeof(u64));
            TC_RequireEqual(Bitset::excess(), 32u);

            Bitset bitset1;
            TC_Require(bitset1.none());

            bitset1.set(SIZE - 1);
            TC_Require(bitset1.any());
            TC_RequireEqual(bitset1.count(), 1u);
            TC_Require(bitset1.test(SIZE - 1));

            bitset1.set();
            TC_Require(bitset1.all());

            bitset1.reset();
            TC_Require(!bitset1.all());
            TC_Require(!bitset1.any());
            TC_Require(bitset1.none());

            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                bitset1.set(iii);
            }
            TC_Require(bitset1.all());
            TC_Require(bitset1.any());
            TC_Require(!bitset1.none());
            for (u64 iii = 0; iii < SIZE; ++iii)
            {
                TC_Require(bitset1.test(iii));
            }
            TC_RequireEqual(bitset1.count(), SIZE);

            static constexpr u64 POS{4};
            Bitset bitset2{1u << POS};
            TC_Require(!bitset2.all());
            TC_Require(bitset2.any());
            TC_Require(!bitset2.none());
            TC_Require(bitset2.test(POS));

            TC_RequireNEqual(bitset1, bitset2);
            TC_RequireNEqual(bitset2, bitset1);

            std::string test(SIZE, '0');
            test[4] = '1';
            TC_RequireEqual(bitset2.toString(), test);

            Bitset bitset3(bitset1);
            TC_Require(bitset3.all());
            TC_Require(bitset3.any());
            TC_Require(!bitset3.none());

            TC_RequireEqual(bitset1, bitset3);
            TC_RequireEqual(bitset3, bitset1);

            bitset2.copy(bitset3);
            TC_RequireEqual(bitset2, bitset3);
            TC_RequireEqual(bitset2, bitset1);

            Bitset bitset4;
            TC_RequireNEqual(bitset4, bitset1);

            bitset2.swap(bitset4);
            TC_Require(!bitset2.all());
            TC_Require(!bitset2.any());
            TC_Require(bitset2.none());
            TC_RequireEqual(bitset4, bitset1);

            TC_RequireEqual(bitset2 | bitset4, bitset4);
            TC_RequireEqual(Bitset{62u} & bitset4, Bitset{62u});
        }
    }

    TU_Case(List0, "Testing the List class")
    {
        PROF_SCOPE("List0");

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
        PROF_SCOPE("List1");

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
        PROF_SCOPE("List2");

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
        PROF_SCOPE("ListPerformance");

        using Type = ent::EntityId;
        using List = ent::List<Type>;
        using Vector = std::vector<Type>;
        static ENT_CONSTEXPR_EXPR u64 SIZE{1000};
        static ENT_CONSTEXPR_EXPR ent::EntityId VALUE{0};

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
        PROF_SCOPE("SortedList");

        static constexpr u64 NUM_ELEMENTS{1000};
        ent::SortedList<u64> list;

        for (u64 iii = NUM_ELEMENTS; iii > 0; --iii)
        {
            list.insertUnique(iii);
        }
        for (u64 iii = NUM_ELEMENTS; iii > 0; --iii)
        {
            list.insertUnique(iii);
        }
        for (u64 iii = 1; iii <= NUM_ELEMENTS; ++iii)
        {
            list.insertUnique(iii);
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

    TU_Case(MetadataGroup0, "Testing the MetadataGroup class")
    {
        TC_RequireEqual(ent::pow2RoundUp(0u), 0u);
        TC_RequireEqual(ent::pow2RoundUp(1u), 1u);

        ent::MetadataGroup mg1;
        ent::MetadataGroup mg2(1);
        ent::MetadataGroup mg3(1, 1);
        ent::MetadataGroup mg4(10, 10);
        ent::MetadataGroup mg5(10, 64);
        ent::MetadataGroup mg6(10, 65);

        TC_RequireEqual(mg1.columns(), 0u);
        TC_RequireEqual(mg1.rows(), 0u);
        TC_RequireEqual(mg1.capacity(), 0u);
        TC_RequireEqual(mg2.columns(), 1u);
        TC_RequireEqual(mg2.rows(), 0u);
        TC_RequireEqual(mg2.capacity(), 0u);
        TC_RequireEqual(mg3.columns(), 1u);
        TC_RequireEqual(mg3.rows(), 1u);
        TC_RequireEqual(mg3.capacity(), ent::ENT_BITSET_GROUP_SIZE * 1u);
        TC_RequireEqual(mg4.columns(), 10u);
        TC_RequireEqual(mg4.rows(), 10u);
        TC_RequireEqual(mg4.capacity(), ent::ENT_BITSET_GROUP_SIZE * 1u);
        TC_RequireEqual(mg5.columns(), 10u);
        TC_RequireEqual(mg5.rows(), 64u);
        TC_RequireEqual(mg5.capacity(), ent::ENT_BITSET_GROUP_SIZE * 1u);
        TC_RequireEqual(mg6.columns(), 10u);
        TC_RequireEqual(mg6.rows(), 65u);
        TC_RequireEqual(mg6.capacity(), ent::ENT_BITSET_GROUP_SIZE * 3u);

        for (u64 col = 0; col < mg6.columns(); ++col)
        {
            for (ent::MetadataBitset *it = mg6.begin(col); it != mg6.end(col); ++it)
            {
                TC_Require(it->none());
            }

            u64 pos{0u};

            ent::MetadataBitset &bs1(mg6.bitsetBit(pos, col, 0u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs1.testAndSet(pos, true), false);
            ent::MetadataBitset &bs2(mg6.bitsetBit(pos, col, 1u));
            TC_RequireEqual(pos, 1u);
            TC_RequireEqual(bs2.testAndSet(pos, true), false);
            ent::MetadataBitset &bs3(mg6.bitsetBit(pos, col, 64u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs3.testAndSet(pos, true), false);
            ent::MetadataBitset &bs4(mg6.bitsetBit(pos, col, 127u));
            TC_RequireEqual(pos, 63u);
            TC_RequireEqual(bs4.testAndSet(pos, true), false);
            ent::MetadataBitset &bs5(mg6.bitsetBit(pos, col, 128u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs5.testAndSet(pos, true), false);

            TC_Require(&bs1 == &bs2);
            TC_Require(&bs1 != &bs3);
            TC_Require(&bs3 == &bs4);
            TC_Require(&bs3 != &bs5);

            TC_RequireEqual(bs1.count(), 2u);
            TC_RequireEqual(bs3.count(), 2u);
            TC_RequireEqual(bs5.count(), 1u);
        }

        mg6.pushBackRow();
        TC_RequireEqual(mg6.columns(), 10u);
        TC_RequireEqual(mg6.rows(), 66u);
        TC_RequireEqual(mg6.capacity(), ent::ENT_BITSET_GROUP_SIZE * 3u);

        for (u64 col = 0; col < mg6.columns(); ++col)
        {
            u64 pos{0u};

            ent::MetadataBitset &bs1(mg6.bitsetBit(pos, col, 0u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs1.testAndSet(pos, true), true);
            ent::MetadataBitset &bs2(mg6.bitsetBit(pos, col, 1u));
            TC_RequireEqual(pos, 1u);
            TC_RequireEqual(bs2.testAndSet(pos, false), true);
            ent::MetadataBitset &bs3(mg6.bitsetBit(pos, col, 64u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs3.testAndSet(pos, true), true);
            ent::MetadataBitset &bs4(mg6.bitsetBit(pos, col, 127u));
            TC_RequireEqual(pos, 63u);
            TC_RequireEqual(bs4.testAndSet(pos, false), true);
            ent::MetadataBitset &bs5(mg6.bitsetBit(pos, col, 128u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs5.testAndSet(pos, true), true);

            TC_Require(&bs1 == &bs2);
            TC_Require(&bs1 != &bs3);
            TC_Require(&bs3 == &bs4);
            TC_Require(&bs3 != &bs5);

            TC_RequireEqual(bs1.count(), 1u);
            TC_RequireEqual(bs3.count(), 1u);
            TC_RequireEqual(bs5.count(), 1u);
        }

        mg6.setColumns(12u);
        TC_RequireEqual(mg6.columns(), 12u);
        TC_RequireEqual(mg6.rows(), 66u);
        TC_RequireEqual(mg6.capacity(), ent::ENT_BITSET_GROUP_SIZE * 2u);

        for (u64 col = 0; col < mg6.columns(); ++col)
        {
            u64 pos{0u};

            ent::MetadataBitset &bs1(mg6.bitsetBit(pos, col, 0u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs1.testAndSet(pos, true), col < 10);
            ent::MetadataBitset &bs2(mg6.bitsetBit(pos, col, 1u));
            TC_RequireEqual(pos, 1u);
            TC_RequireEqual(bs2.testAndSet(pos, true), false);
            ent::MetadataBitset &bs3(mg6.bitsetBit(pos, col, 64u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs3.testAndSet(pos, true), col < 10);
            ent::MetadataBitset &bs4(mg6.bitsetBit(pos, col, 127u));
            TC_RequireEqual(pos, 63u);
            TC_RequireEqual(bs4.testAndSet(pos, true), false);

            TC_Require(&bs1 == &bs2);
            TC_Require(&bs1 != &bs3);
            TC_Require(&bs3 == &bs4);

            TC_RequireEqual(bs1.count(), 2u);
            TC_RequireEqual(bs3.count(), 2u);
        }

        mg6.setColumns(1u);
        TC_RequireEqual(mg6.columns(), 1u);
        TC_RequireEqual(mg6.rows(), 66u);
        TC_RequireEqual(mg6.capacity(), ent::ENT_BITSET_GROUP_SIZE * 2u);

        for (u64 col = 0; col < mg6.columns(); ++col)
        {
            u64 pos{0u};

            ent::MetadataBitset &bs1(mg6.bitsetBit(pos, col, 0u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs1.testAndSet(pos, true), true);
            ent::MetadataBitset &bs2(mg6.bitsetBit(pos, col, 1u));
            TC_RequireEqual(pos, 1u);
            TC_RequireEqual(bs2.testAndSet(pos, true), true);
            ent::MetadataBitset &bs3(mg6.bitsetBit(pos, col, 64u));
            TC_RequireEqual(pos, 0u);
            TC_RequireEqual(bs3.testAndSet(pos, true), true);
            ent::MetadataBitset &bs4(mg6.bitsetBit(pos, col, 127u));
            TC_RequireEqual(pos, 63u);
            TC_RequireEqual(bs4.testAndSet(pos, true), true);

            TC_Require(&bs1 == &bs2);
            TC_Require(&bs1 != &bs3);
            TC_Require(&bs3 == &bs4);

            TC_RequireEqual(bs1.count(), 2u);
            TC_RequireEqual(bs3.count(), 2u);
        }
    }

    TU_Case(Parallel0, "Testing parallel access 1")
    {
        PROF_SCOPE("Parallel0");
        using Universe = ParUniverse0::UniverseT;
        using Entity = ParUniverse0::EntityT;
        using TempEntity = ParUniverse0::TempEntityT;

        static constexpr u64 NUM_MAX_THREADS{8u};
        static constexpr u64 NUM_THREADS{4u};
        static constexpr u64 NUM_ENTITIES{1000u};
        static constexpr const char *THREAD_NAMES[NUM_MAX_THREADS + 1u] = {
            "Thread 0",
            "Thread 1",
            "Thread 2",
            "Thread 3",
            "Thread 4",
            "Thread 5",
            "Thread 6",
            "Thread 7"
        };

        Universe u;
        u.registerComponent<Position>();
        u.registerComponent<Velocity>();

        u.init();

        PositionSystem *ps{u.addSystem<PositionSystem>()};
        VelocitySystem *vs{u.addSystem<VelocitySystem>()};
        PosVelSystem *pvs{u.addSystem<PosVelSystem>()};

        u.refresh();

        std::vector<std::thread> threads;

        for (u64 entNum = 0; entNum < NUM_ENTITIES; ++entNum)
        {
            Entity e = u.createEntity();
            e.add<Position>(0.0f, 0.0f);
            e.add<Velocity>(0.0f, 0.0f);
        }

        u.refresh();

        PROF_BLOCK("Work");

        auto parallelIterator{pvs->foreachP(NUM_THREADS)};
        for (u64 iii = 1; iii <= NUM_THREADS; ++iii)
        {
            threads.emplace_back([&, iii] () {
                auto foreach = parallelIterator.forThread(iii - 1u);
                for (auto &e : foreach)
                {
                    Position* pos{e.get<Position>()};
                    Velocity* vel{e.get<Velocity>()};
                    pos->x += vel->x;
                    pos->y += vel->y;

                    for (u64 iii = 0; iii < 100; ++iii)
                    {
                        pos->x += cos(pos->x + vel->x);
                        pos->y += sin(pos->y + vel->y);
                    }

                }
            });
        }

        for (u64 iii = 0; iii < NUM_THREADS; ++iii)
        {
            threads[iii].join();
        }

        threads.clear();

        PROF_BLOCK_END();
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

