/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <new>

namespace tst {
    struct Tester
    {
        static u64 globCounter;
        u64 counter{1234};

        Tester()
        {
            counter = globCounter++;
            std::cout << "Default constructor " << counter << std::endl;
        }

        ~Tester()
        {
            std::cout << "Destructor " << counter << std::endl;
        }

        Tester(const Tester &other)
        {
            counter = globCounter++;
            std::cout << "Copy constructor " << counter <<
                " <- " << other.counter << std::endl;
        }

        Tester(Tester &&other)
        {
            counter = globCounter++;
            std::cout << "Move constructor " << counter <<
                " <- " << other.counter << std::endl;
        }

        Tester &operator=(const Tester &rhs)
        {
            std::cout << "Copy assignment operator " << counter <<
                " <- " << rhs.counter << std::endl;

            return *this;
        }

        Tester &operator=(Tester &&rhs)
        {
            std::cout << "Move assignment operator " << counter <<
                " <- " << rhs.counter << std::endl;

            return *this;
        }
    };

    u64 Tester::globCounter{0};

    constexpr bool testFun(bool a, bool b) {
        constexpr std::logical_or<bool> orer;
        return orer(a, b);
    }

    TEST_UNIT_BEG(TestingTheTest, 1)

        TEST_UNIT_SETUP_BEG
        TEST_UNIT_SETUP_END

        TEST_UNIT_TEARDOWN_BEG
        TEST_UNIT_TEARDOWN_END

        TEST_UNIT_MAIN_BEG
            /*
            using CmpType1 = mp_list<int, double, float>;
            using CmpType2 = std::tuple<int, double, float>;
            static_assert(std::is_same<mp_rename_t<CmpType1,
                                                 std::tuple>,
                                       CmpType2>::value);

            static_assert(std::is_same<CmpType1, CmpType1>::value);

            static_assert(mp_is_specialization_v<mp_list, mp_list<int>>);

            static_assert(mp_are_specializations_v<mp_list,
                mp_list<int>>);
            static_assert(mp_are_specializations_v<mp_list,
                mp_list<int>,
                mp_list<int, float>,
                mp_list<float, double, std::vector<double>>>);
            static_assert(!mp_are_specializations_v<mp_list,
                mp_list<int>,
                mp_list<int, float>,
                mp_list<float, double>,
                std::vector<double>>);

            struct CA {};
            struct CB {};
            struct CC {};
            using CompTypes = ecs::ComponentList<CA, CB, CC>;
            static_assert(mp_contains_v<CA, CompTypes>);
            static_assert(mp_contains_v<CB, CompTypes>);
            static_assert(mp_contains_v<CC, CompTypes>);
            static_assert(mp_size_v<CompTypes> == 3);

            struct TA {};
            struct TB {};
            struct TC {};
            using TagTypes = ecs::TagList<TA, TB, TC>;
            static_assert(mp_contains_v<TA, TagTypes >);
            static_assert(mp_contains_v<TB, TagTypes >);
            static_assert(mp_contains_v<TC, TagTypes >);
            static_assert(mp_size_v<TagTypes> == 3);
            static_assert(mp_index_of_v<TA, TagTypes> == 0);
            static_assert(mp_index_of_v<TB, TagTypes> == 1);
            static_assert(mp_index_of_v<TC, TagTypes> == 2);

            using SA = ecs::Signature<CA, CC, TC>;
            using SB = ecs::Signature<CB, TB>;
            using SC = ecs::Signature<CB, CC, TA>;
            using Signatures = ecs::SignatureList<SA, SB, SC>;
            static_assert(mp_contains_v<SA, Signatures >);
            static_assert(mp_contains_v<ecs::Signature<CA, CC, TC>, Signatures >);
            static_assert(mp_contains_v<SB, Signatures >);
            static_assert(mp_contains_v<SC, Signatures >);
            static_assert(mp_size_v<SA> == 3);
            static_assert(mp_size_v<SB> == 2);
            static_assert(mp_size_v<SC> == 3);
            static_assert(mp_index_of_v<ecs::Signature<CB, TB>, Signatures> == 1);

            static_assert(!mp_contains_v<TA, CompTypes>);
            static_assert(!mp_contains_v<CA, TagTypes>);
            static_assert(!mp_contains_v<ecs::Signature<CA, CC, TA>, Signatures >);

            using CFG = ecs::Config<CompTypes, TagTypes, Signatures>;

            static_assert(CFG::componentCount() == 3);
            static_assert(CFG::tagCount() == 3);
            static_assert(CFG::signatureCount() == 3);

            static_assert(CFG::isComponent<CA>());
            static_assert(!CFG::isComponent<TA>());
            static_assert(!CFG::isComponent<SA>());
            static_assert(CFG::isTag<TB>());
            static_assert(!CFG::isTag<CB>());
            static_assert(!CFG::isTag<SB>());
            static_assert(CFG::isSignature<SC>());
            static_assert(!CFG::isSignature<CC>());
            static_assert(!CFG::isSignature<TC>());

            static_assert(CFG::componentId<CA>() == 0);
            static_assert(CFG::componentId<CB>() == 1);
            static_assert(CFG::componentId<CC>() == 2);
            static_assert(CFG::tagId<TA>() == 0);
            static_assert(CFG::tagId<TB>() == 1);
            static_assert(CFG::tagId<TC>() == 2);
            static_assert(CFG::signatureId<SA>() == 0);
            static_assert(CFG::signatureId<SB>() == 1);
            static_assert(CFG::signatureId<SC>() == 2);

            TC_AssertTrue(CFG::mask<CA>() == 1);
            TC_AssertTrue(CFG::mask<CB>() == 2);
            TC_AssertTrue(CFG::mask<CC>() == 4);
            TC_AssertTrue(CFG::mask<TA>() == 8);
            TC_AssertTrue(CFG::mask<TB>() == 16);
            TC_AssertTrue(CFG::mask<TC>() == 32);

            std::cout << "CA : " << CFG::mask<CA>() << std::endl;
            std::cout << "CB : " << CFG::mask<CB>() << std::endl;
            std::cout << "CC : " << CFG::mask<CC>() << std::endl;
            std::cout << "TA : " << CFG::mask<TA>() << std::endl;
            std::cout << "TB : " << CFG::mask<TB>() << std::endl;
            std::cout << "TC : " << CFG::mask<TC>() << std::endl;

            TC_AssertTrue(CFG::mask<SA>() == 37);
            TC_AssertTrue(CFG::mask<SB>() == 18);
            TC_AssertTrue(CFG::mask<SC>() == 14);

            std::cout << "SA : " << CFG::mask<SA>() << std::endl;
            std::cout << "SB : " << CFG::mask<SB>() << std::endl;
            std::cout << "SC : " << CFG::mask<SC>() << std::endl;
            */

            /*
            struct C0 {
            };
            struct C1 {
            };
            struct C2 {
            };
            struct C3 {
            };
            struct C4 {
            };
            struct C5 {
            };
            struct C6 {
            };
            struct C7 {
            };
            struct C8 {
            };
            struct C9 {
            };
            struct C10 {
            };
            struct C11 {
            };
            struct C12 {
            };
            struct C13 {
            };
            struct C14 {
            };
            struct C15 {
            };
            struct C16 {
            };
            struct C17 {
            };
            struct C18 {
            };
            struct C19 {
            };
            struct C20 {
            };
            struct C21 {
            };
            struct C22 {
            };
            struct C23 {
            };
            struct C24 {
            };
            struct C25 {
            };
            struct C26 {
            };
            struct C27 {
            };
            struct C28 {
            };
            struct C29 {
            };
            struct C30 {
            };
            struct C31 {
            };
            struct C32 {
            };
            struct C33 {
            };
            struct C34 {
            };
            struct C35 {
            };
            struct C36 {
            };
            struct C37 {
            };
            struct C38 {
            };
            struct C39 {
            };
            struct C40 {
            };
            struct C41 {
            };
            struct C42 {
            };
            struct C43 {
            };
            struct C44 {
            };
            struct C45 {
            };
            struct C46 {
            };
            struct C47 {
            };
            struct C48 {
            };
            struct C49 {
            };
            using CompTypes = ecs::ComponentList<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49>;
            struct T0 {
            };
            struct T1 {
            };
            struct T2 {
            };
            struct T3 {
            };
            struct T4 {
            };
            struct T5 {
            };
            struct T6 {
            };
            struct T7 {
            };
            struct T8 {
            };
            struct T9 {
            };
            struct T10 {
            };
            struct T11 {
            };
            struct T12 {
            };
            struct T13 {
            };
            using TagTypes = ecs::TagList<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>;
            using S = ecs::Signature<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>;
            using Signatures = ecs::SignatureList<S>;
            using CFG = ecs::Config<CompTypes, TagTypes, Signatures>;
            std::cout << "Last Comp : " << CFG::mask<C49>() << std::endl;
            std::cout << "Last Tag : " << CFG::mask<T13>() << std::endl;
            std::cout << "Full Sig : " << CFG::mask<S>() << std::endl;

            constexpr ecs::EntityHandle id1;
            constexpr ecs::EntityHandle id2(1);
            constexpr ecs::EntityHandle id3(1, 1);
            constexpr ecs::EntityHandle id4(1, 256);
            static_assert(id1.index() == 0 && id1.generation() == 0);
            static_assert(id1.id() == 0);
            static_assert(id2.id() == 1);
            static_assert(id3.index() == id2.index());
            static_assert(id3.generation() == 1);
            static_assert(id4.id() == id2.id());

            constexpr std::bitset<sizeof(ecs::EntityHandle::IdType) * 8> bsId3(id3.id());
            std::cout << bsId3 << std::endl;

            ecs::EntityManager<CFG> mgr1;
            std::deque<ecs::EntityHandle> entities1;

            mgr1.testIter<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43, C44, C45, C46, C47, C48, C49, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>();

            {
                PROF_SCOPE(EntitiesInOrder);
                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    PROF_BLOCK(EntityCreate);
                    auto entity = mgr1.create();
                    PROF_BLOCK_END();
                    entities1.push_back(entity);
                }

                for (u64 iii = 0; iii < 1000000; ++iii)
                {
                    auto entity = entities1.front();
                    entities1.pop_front();
                    PROF_BLOCK(EntityDestroy);
                    mgr1.destroy(entity);
                    PROF_BLOCK_END();
                }
            }

            ecs::EntityManager<CFG> mgr2;
            std::deque<ecs::EntityHandle> entities2;

            {
                PROF_SCOPE(EntitiesOutOfOrder);
                u64 toCreate{1000000};

                std::cout << "Creating " << toCreate << std::endl;
                while (toCreate > 0)
                {
                    u64 choice = std::rand() % 100;

                    if (choice < 80 || entities2.size() < 10)
                    {
                        PROF_BLOCK(EntityCreateOOO);
                        auto entity = mgr2.create();
                        PROF_BLOCK_END();
                        entities2.push_back(entity);
                        toCreate--;
                        if (toCreate % 10000 == 0)
                            std::cout << "Remaining : " << toCreate << std::endl;
                    }
                    else
                    {
                        //u64 index = std::rand() % entities2.size();
                        //auto entity = entities2[index];
                        //entities2.erase(entities2.begin() + index);
                        auto entity = entities2.front();
                        entities2.pop_front();
                        PROF_BLOCK(EntityDestroyOOO);
                        mgr2.destroy(entity);
                        PROF_BLOCK_END();
                    }
                    if (toCreate > 1000000)
                        std::cout << "a" << std::endl;
                }

                std::cout << "Deleting the rest " << entities2.size() << std::endl;

                while (!entities2.empty())
                {
                    auto entity = entities2.front();
                    entities2.pop_front();
                    PROF_BLOCK(EntityDestroyOOO);
                    mgr2.destroy(entity);
                    PROF_BLOCK_END();
                }
            }
            */

            mem::List<u64> normList;

            normList.pushBack(1);
            normList.pushBack(2);
            normList.pushBack(3);
            normList.pushBack(4);

            const mem::List<u64> &constList = normList;

            for (const u64 &element : constList)
            {
                std::cout << element << std::endl;
            }

            auto it1 = begin(normList);
            UNUSED(it1);
            auto it2 = end(normList);
            UNUSED(it2);
            auto it3 = begin(constList);
            UNUSED(it3);
            auto it4 = end(constList);
            UNUSED(it4);

            mem::List<Tester> list1;
            std::vector<Tester> vec1;

            for (u64 iii = 0; iii < 10; ++iii)
            {
                std::cout << "Push back # " << iii << std::endl;
                std::cout << "List: " << std::endl;
                list1.pushBack(Tester());
                for (const auto &el : list1)
                    std::cout << el.counter << " ";
                std::cout << std::endl;
                std::cout << "Vector: " << std::endl;
                vec1.push_back(Tester());
                for (const auto &el : vec1)
                    std::cout << el.counter << " ";
                std::cout << std::endl;
            }

            mem::List<Tester> list2;
            std::vector<Tester> vec2;

            for (u64 iii = 0; iii < 10; ++iii)
            {
                std::cout << "Emplace back # " << iii << std::endl;
                std::cout << "List: " << std::endl;
                list2.emplaceBack();
                std::cout << "Vector: " << std::endl;
                vec2.emplace_back();
            }

            std::cout << "List copy constructor" << std::endl;
            mem::List<Tester> copyList(list1);
            TC_AssertTrue(list1.size() == copyList.size());

            std::cout << "Vector copy constructor" << std::endl;
            std::vector<Tester> copyVec(vec1);

            std::cout << "List move constructor" << std::endl;
            mem::List<Tester> moveList(std::move(list1));
            TC_AssertTrue(list1.size() == 0);

            std::cout << "Vector move constructor" << std::endl;
            std::vector<Tester> moveVec(std::move(vec1));

            std::cout << "Vector random delete" << std::endl;
            while (moveList.size() > 1)
            {
                u64 index{std::rand() % moveList.size()};
                std::cout << "Removing element on index " << index << std::endl;
                TC_AssertTrue(moveList.remove(index));
            }

            std::cout << "Removing last element" << std::endl;
            TC_AssertTrue(moveList.remove(0));
            TC_AssertTrue(moveList.empty());

            static constexpr u64 sampleSize{1000000};
            using ElementT = std::string;

            PROF_BLOCK("vec_push_out");

            std::vector<ElementT> profVecPush;

            for (u64 iii = 0; iii < sampleSize; ++iii)
            {
                profVecPush.push_back("test");
            }

            PROF_BLOCK_END();

            PROF_BLOCK("vec_emplace_out");

            std::vector<ElementT> profVecEmplace;

            for (u64 iii = 0; iii < sampleSize; ++iii)
            {
                profVecEmplace.emplace_back("test");
            }

            PROF_BLOCK_END();

            PROF_BLOCK("list_push_out");

            mem::List<ElementT> profListPush;

            for (u64 iii = 0; iii < sampleSize; ++iii)
            {
                profListPush.pushBack("test");
            }

            PROF_BLOCK_END();

            PROF_BLOCK("list_emplace_out");

            mem::List<ElementT> profListEmplace;

            for (u64 iii = 0; iii < sampleSize; ++iii)
            {
                profListEmplace.emplaceBack("test");
            }

            PROF_BLOCK_END();

            PROF_BLOCK("list_iter");

            for (const auto &el : profListPush)
            {
                el.size();
            }

            PROF_BLOCK_END();

            PROF_BLOCK("vector_iter");

            for (const auto &el : profVecPush)
            {
                el.size();
            }

            PROF_BLOCK_END();

            TC_AssertTrue(profListPush.size() == profVecPush.size());
            for (u64 iii = 0; iii < profListPush.size(); ++iii)
            {
                TC_AssertTrue(profListPush[iii] == profVecPush[iii]);
            }

            PROF_BLOCK("list_pop");

            while (!profListPush.empty())
            {
                profListPush.popBack();
            }

            TC_AssertTrue(profListPush.size() == 0);

            PROF_BLOCK_END();

            PROF_BLOCK("vec_pop");

            while (!profVecPush.empty())
            {
                profVecPush.pop_back();
            }

            TC_AssertTrue(profVecPush.size() == 0);

            PROF_BLOCK_END();

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

