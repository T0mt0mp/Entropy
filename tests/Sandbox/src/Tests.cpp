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

struct OpStruct1
{
    struct OpStructCmp
    {
        bool operator()(const u64 &val, const OpStruct1 &rhs)
        { return val < rhs.val; }
        bool operator()(const OpStruct1 &rhs, const u64 &val)
        { return rhs.val < val; }
        bool operator()(const OpStruct1 &lhs, const OpStruct1 &rhs)
        { return lhs.val < rhs.val; }
    };

    OpStruct1(u64 value) :
        val{value}
    { }

    u64 val;
    u64 data[10];
};

struct OpStruct2
{
    struct OpStructCmp
    {
        bool operator()(const u64 &val, const OpStruct2 &rhs)
        { return val < rhs.val; }
        bool operator()(const OpStruct2 &rhs, const u64 &val)
        { return rhs.val < val; }
        bool operator()(const OpStruct2 &lhs, const OpStruct2 &rhs)
        { return lhs.val < rhs.val; }
    };

    OpStruct2(u64 value, bool operation) :
        val{value}, op{operation}
    { }

    u64 val;
    u64 data[10];
    bool op;
};

void codeBloat1(i64 &result, i64 val)
{
    static constexpr u64 ARR_SIZE{50};
    static f64 workArray[ARR_SIZE];
    f64 thing{0.0f};
    thing = sin(val);
    thing += cos(val);
    for (u64 index = 0; index < ARR_SIZE; ++index)
    {
        u64 swVal{(index + val) % 10};
        switch (swVal)
        {
            case 0:
            {
                thing += swVal * 12.0f;
                break;
            }

            case 1:
            {
                thing = val * thing + swVal;
                break;
            }

            case 2:
            {
                thing = sin(thing);
                break;
            }

            case 3:
            {
                thing = cos(thing);
                break;
            }

            case 6:
            {
                thing = cos(thing) * sin(thing);
                break;
            }

            default:
            {
                thing *= swVal / 10.0f;
                break;
            }
        }
        workArray[index] = thing;
    }

    result += val + thing * 0.0f;
}

void codeBloat2(i64 &result, i64 val)
{
    static constexpr u64 ARR_SIZE{20};
    static f64 workArray[ARR_SIZE];
    f64 thing{0.0f};
    thing = sin(val);
    thing += cos(val);
    for (u64 index = 0; index < ARR_SIZE; ++index)
    {
        u64 swVal{(index + val) % 10};
        switch (swVal)
        {
            case 0:
            {
                thing += swVal * 16.0f;
                break;
            }

            case 1:
            {
                thing = val * thing + swVal;
                break;
            }

            case 2:
            {
                thing = cos(thing);
                break;
            }

            case 3:
            {
                thing = sin(thing);
                break;
            }

            case 6:
            {
                thing = cos(thing) * sin(thing);
                break;
            }

            default:
            {
                thing *= swVal / 20.0f;
                break;
            }
        }
        workArray[index] = thing;
    }

    result += val + thing * 0.0f;
}

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

    TU_Case(Sandbox5, "Sandbox5")
    {
        static constexpr u64 ELEMENTS{1000000};
        static constexpr u64 NUM_ATTEMPTS{10};
        static constexpr f64 THRESHOLD{0.1};
        static constexpr f64 REPEAT_THRESHOLD{0.005};

        std::random_device rd;

        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        for (u64 attempt = 0; attempt < NUM_ATTEMPTS; ++attempt)
        {
            u64 randomSeed{rd()};

            {
                rng.seed(randomSeed);
                PROF_SCOPE("2 lists");

                ent::SortedList<OpStruct1, OpStruct1::OpStructCmp> op1;
                ent::SortedList<u64> op2;

                PROF_BLOCK("Fill");
                for (u64 iii = 0; iii < ELEMENTS;)
                {
                    if (uniform(rng) > THRESHOLD)
                    { // OP1
                        op2.erase(iii);
                        op1.insertUnique(iii, iii);
                    }
                    else
                    { // OP2
                        op1.erase(iii);
                        op2.insertUnique(iii);
                    }

                    if (uniform(rng) > REPEAT_THRESHOLD)
                    {
                        ++iii;
                    }
                }
                PROF_BLOCK_END();

                i64 result{0u};

                PROF_BLOCK("Do Op");
                for (OpStruct1 &val : op1)
                {
                    //result += val;
                    codeBloat1(result, val.val);
                }

                for (u64 val : op2)
                {
                    //result -= val;
                    codeBloat2(result, -val);
                }
                PROF_BLOCK_END();

                std::cout << op1.size() << " " << op2.size() << std::endl;
                std::cout << op1.size() + op2.size() << std::endl;
                std::cout << "Result 2 lists: " << result << std::endl;
            }

            {
                rng.seed(randomSeed);
                PROF_SCOPE("1 list");

                ent::SortedList<OpStruct2, OpStruct2::OpStructCmp> op;

                PROF_BLOCK("Fill");
                for (u64 iii = 0; iii < ELEMENTS;)
                {
                    if (uniform(rng) > THRESHOLD)
                    { // OP1
                        op.replaceUnique(iii, OpStruct2{iii, true});
                    }
                    else
                    { // OP2
                        op.replaceUnique(iii, OpStruct2{iii, false});
                    }

                    if (uniform(rng) > REPEAT_THRESHOLD)
                    {
                        ++iii;
                    }
                }
                PROF_BLOCK_END();

                i64 result{0u};

                PROF_BLOCK("Do Op");
                for (OpStruct2 &opStruct : op)
                {
                    if (opStruct.op)
                    {
                        codeBloat1(result, opStruct.val);
                    }
                    else
                    {
                        codeBloat2(result, -opStruct.val);
                    }
                }
                PROF_BLOCK_END();

                std::cout << op.size() << std::endl;
                std::cout << "Result 1 list: " << result << std::endl;
            }
        }
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

