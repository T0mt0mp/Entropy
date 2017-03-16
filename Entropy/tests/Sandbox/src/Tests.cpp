/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <vector>
#include <thread>
#include <mutex>

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

TU_End(EntropySandbox)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

