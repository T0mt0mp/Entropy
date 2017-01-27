/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"
#include <x86intrin.h>
#include <chrono>
#include "testing/Testing.h"

u64 tscTicks()
{
    /*
    u32 lo;
    u32 hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((u64)hi << 32) | lo;
     */
    return __rdtsc();
}

std::pair<void(*)(), std::string> functions[] = {
    std::make_pair([](){
        volatile auto t = std::chrono::high_resolution_clock::now();
        t;
    }, "Chrono high resolution clock"),
    std::make_pair([](){
        volatile auto t = std::chrono::system_clock::now();
        t;
    }, "Chrono system clock"),
    std::make_pair([](){
        volatile auto t = std::chrono::steady_clock::now();
        t;
    }, "Chrono steady clock"),
    std::make_pair([](){
        volatile auto t = _rdtsc();
        t;
    }, "RDTSC"),
    std::make_pair([](){
        u32 c;
        volatile auto t = _rdtscp(&c);
        t;
    }, "RDTSCP"),
    std::make_pair([](){
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts;
    }, "clock_gettime(CLOCK_REALTIME...)"),
};

class Test
{
    using ThisType = Test;
    static std::vector<void(Test::*)()> mFuns;

    void fun1() {fun2();};

    Test()
    {
        for (auto f : mFuns)
        {
            (this->*f)();
        }
    }
    struct ClFun1
    {
        ClFun1()
        {
            mFuns.push_back(&ThisType::fun1);
        }
    } a;

    void fun2(){};
};

TU_Begin(MyTestUnit)
    TU_Setup
    {
        std::cout << "Hello from Setup!" << std::endl;
    }

    TU_Teardown
    {
        std::cout << "Hello from Teardown!" << std::endl;
    }

    TU_Case(Case1, "Testing the basic commands [ 2P / 1F / 3T ] { 0E }")
    {
        std::cout << "Hello from TC1!" << std::endl;
        TC_Check(true);
        TC_Require(true);
        TC_Require(false);
        TC_Check(false);

        TC_Fail("Should not get to here!");
    }

    TU_Case(Case2, "Testing Check [ 1P / 3F / 4T ] { 0E }")
    {
        std::cout << "Hello from TC2!" << std::endl;

        TC_CheckEqual(2 + 2, 3 + 1);
        TC_CheckMessage(false, "This is a constructed message " << (2 + 1));
        TC_Error("Testing error message");
        TC_Fail("Successful failure");

        TC_Fail("Should not get to here!");
    }

    TU_Case(Case3, "Testing fail1 [ 0P / 1F / 1T ] { 0E }")
    {
        TC_RequireEqual(1 + 3, 2 + 3);

        TC_Fail("Should not get to here!");
    }

    TU_Case(Case4, "Testing fail2 [ 0P / 1F / 1T ] { 0E }")
    {
        TC_RequireMessage(false, "Constructed message");

        TC_Fail("Should not get to here!");
    }

    TU_Case(Case5, "Testing fail3 [ 2P / 2F / 4T ] { 0E }")
    {
        TC_CheckException(1 + 1, std::runtime_error);
        TC_CheckException(throw std::runtime_error("Hi"), std::runtime_error);
        TC_RequireException(throw std::runtime_error("Hi"), std::runtime_error);
        TC_RequireException(1 + 1, std::runtime_error);

        TC_Fail("Should not get to here!");
    }

TU_End(MyTestUnit)

TU_Begin(MySecondTestUnit)

    TU_Case(CaseOne, "Hello")
    {
        TC_Fail("Hey there");
    }

TU_End(MySecondTestUnit)

auto time()
{
    using inClock = std::chrono::high_resolution_clock;
    return inClock::now();
}

int main(int argc, char* argv[])
{
    static constexpr u64 US_IN_S{1000000};

    u64 ticksB{0};
    u64 ticksA{0};

    decltype(time()) tpL{time()};
    decltype(time()) tpN{tpL};

    for (auto &p : functions)
    {
        static constexpr u64 SAMPLE_NUM{5};
        double sum{0.0};

        for (u64 iii = 0; iii < SAMPLE_NUM; ++iii)
        {
            ticksB = __rdtsc();

            p.first();

            ticksA = __rdtsc();

            if (ticksA < ticksB)
            {
                std::cout << "Warning, negative delta!" << std::endl;
            }

            sum += ticksA - ticksB;

            usleep(US_IN_S / 100);
        }

        std::cout << "Avg ticks for " << p.second << " : " << sum / SAMPLE_NUM << std::endl;
    }

    /*
    std::vector<std::thread> threads;
    thr::SpinLock lock;
     */

    u64 lastTicks{tscTicks()};
    u64 newTicks{lastTicks};
    u64 tscWarps{0};

    for (u64 iii = 0; iii < 100; ++iii)
    {
        newTicks = tscTicks();

        if (lastTicks > newTicks)
        {
            tscWarps++;
        }

        lastTicks = newTicks;

        /*
        for (u64 iii = 0; iii < 5; ++iii)
        {
            threads.emplace_back([iii, &lock] () {
                lock.lock();
                lock.unlock();

                usleep(US_IN_S);
            });
        }
        threads.clear();
         */

        usleep(US_IN_S / 100);
    }

    if (tscWarps)
    {
        std::cout << "TSC warped : " << tscWarps << " times!" << std::endl;
    }
    else
    {
        std::cout << "TSC seems to be stable!" << std::endl;
    }

    TCC_Run();
    TCC_Report();

    TCC_Reset();

    TCC_Run();
    TCC_Report();
}

