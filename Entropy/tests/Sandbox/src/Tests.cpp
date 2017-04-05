/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <vector>
#include <thread>
#include <mutex>
#include <functional>

struct ThreadTester
{
    ThreadTester()
    {
        std::lock_guard<std::mutex> lock(sMutex);

        std::cout << "ThreadTester() from " << std::this_thread::get_id() << std::endl;
        thId = std::this_thread::get_id();
        sThreads.push_back(std::this_thread::get_id());
        sAllocated.push_back(new int[100]);
    }

    static void printIds()
    {
        std::lock_guard<std::mutex> lock(sMutex);

        std::cout << "Printing thread ids: " << std::endl;
        for (auto &id : sThreads)
        {
            std::cout << "Id : " << id << std::endl;
        }
        std::cout << std::endl;
    }

    ~ThreadTester()
    {
        std::lock_guard<std::mutex> lock(sMutex);

        std::cout << "~ThreadTester() from " << std::this_thread::get_id() << std::endl;
    }

    std::thread::id thId;

    static std::mutex sMutex;
    static std::vector<std::thread::id> sThreads;
    static std::vector<int*> sAllocated;
};

std::mutex ThreadTester::sMutex;
std::vector<std::thread::id> ThreadTester::sThreads;
std::vector<int*> ThreadTester::sAllocated;

template <typename T>
thread_local ThreadTester threadTester;

struct Tester
{
    template <typename T>
    static ThreadTester &get()
    {
        static thread_local ThreadTester t;
        return t;
    }
};

struct TesterStruct
{
    TesterStruct(ent::EIdType gen) :
        id{0u, gen}
    { }

    ent::EntityId id;
    u8 data[8];
};

struct FunTester
{
    template <typename T>
    void fun(T&)
    {
        std::cout << sizeof(T) << std::endl;
    }
};

struct DestructTester
{
    virtual ~DestructTester()
    {
        std::cout << "Base destruct" << std::endl;
    }
};

struct DestructTesterSpec : public DestructTester
{
    virtual ~DestructTesterSpec()
    {
        std::cout << "Spec destruct" << std::endl;
    }
};

struct TestBase
{
    void say()
    { std::cout << "Hello from TestBase" << std::endl; }
    virtual TestBase *give()
    { return this;}
};

template <typename T>
struct TestSpec : public TestBase
{
    void say()
    { std::cout << "Hello from Spec" << std::endl; }
    virtual TestSpec *give()
    { return this; }
};

TU_Begin(EntropySandbox)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

    TU_Case(Sandbox0, "Sandbox0")
    {
        ThreadTester::printIds();

        std::cout << Tester::get<int>().thId << std::endl;

        std::vector<std::thread> threads;

        for (u64 iii = 0; iii < 10; ++iii)
        {
            threads.emplace_back([] () {
                std::cout << Tester::get<int>().thId << std::endl;
            });
        }

        for (auto &th : threads)
        {
            th.join();
        }

        ThreadTester::printIds();

        for (int *mem : ThreadTester::sAllocated)
        {
            delete[] mem;
        }
    }

    TU_Case(Sandbox1, "Sandbox1")
    {
        static constexpr u64 SIZE{1000000};
        static constexpr f64 THRESHOLD{0.9f};
        static constexpr u64 NUM_ATTEMPTS{1};

        std::random_device rd;

        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        for (u64 attempt = 0; attempt < NUM_ATTEMPTS; ++attempt)
        {
            u64 randomSeed{rd()};
            std::cout << "Random seed : " << randomSeed << std::endl;

            {
                ent::List<TesterStruct> list1;

                rng.seed(randomSeed);

                {
                    PROF_SCOPE("One list - fill");
                    for (u64 iii = 0; iii < SIZE; ++iii)
                    {
                        if (uniform(rng) < THRESHOLD)
                        {
                            list1.emplaceBack(0u);
                        }
                        else
                        {
                            list1.emplaceBack(ent::EntityId::TEMP_ENTITY_GEN);
                        }
                    }
                }

                std::cout << "One list size : " << list1.size() << std::endl;

                u64 normal{0u};
                u64 temp{0u};
                u64 jumble{0u};

                {
                    PROF_SCOPE("One list - process");

                    for (const TesterStruct &ts : list1)
                    {
                        if (ts.id.generation() == ent::EntityId::TEMP_ENTITY_GEN)
                        {
                            temp++;
                            jumble += ts.data[0u];
                        }
                        else
                        {
                            normal++;
                            jumble += ts.data[1u];
                        }
                    }
                }
                std::cout << "Two list results : " << normal << " "
                          << temp << " " << jumble << std::endl;
            }

            {
                ent::List<TesterStruct> list1;
                ent::List<TesterStruct> list2;

                rng.seed(randomSeed);

                {
                    PROF_SCOPE("Two lists - fill");
                    for (u64 iii = 0; iii < SIZE; ++iii)
                    {
                        if (uniform(rng) < THRESHOLD)
                        {
                            list1.emplaceBack(0u);
                        }
                        else
                        {
                            list2.emplaceBack(ent::EntityId::TEMP_ENTITY_GEN);
                        }
                    }
                }

                std::cout << "Two lists sizes : " << list1.size() << " " << list2.size()
                          << " (" << list1.size() + list2.size() << " total)" << std::endl;

                u64 normal{0u};
                u64 temp{0u};
                u64 jumble{0u};

                {
                    PROF_SCOPE("two lists - process");

                    for (const TesterStruct &ts : list2)
                    {
                        temp++;
                        jumble += ts.data[0u];
                    }

                    for (const TesterStruct &ts : list1)
                    {
                        normal++;
                        jumble += ts.data[1u];
                    }
                }
                std::cout << "One list results : " << normal << " "
                          << temp << " " << jumble << std::endl;
            }
        }
    }

    TU_Case(Sandbox2, "Sandbox2")
    {
        auto l = [] (auto *ptr, auto val) {
            ptr->fun(val);
        };

        FunTester fun;

        l(&fun, 0);
        l(&fun, 0.0f);
        l(&fun, std::string{});
    }

    TU_Case(Sandbox3, "Sandbox3")
    {
        DestructTester *bPtr = new DestructTester;
        delete bPtr;
        bPtr = new DestructTesterSpec;
        delete bPtr;
    }

    TU_Case(Sandbox4, "Sandbox4")
    {
        TestBase b;
        TestSpec<u64> s;
        TestBase *bPtr{&b};
        TestSpec<u64> *sPtr{&s};
        TestBase *bsPtr{&s};
        b.say();
        s.say();
        bPtr->give()->say();
        sPtr->give()->say();
        bsPtr->give()->say();
    }
TU_End(EntropySandbox)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

