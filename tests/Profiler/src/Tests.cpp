/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <new>

constexpr char *THREAD_NAMES[] = {
    "Thread 1",
    "Thread 2",
    "Thread 3",
    "Thread 4",
    "Thread 5",
    "Thread 6",
    "Thread 7",
    "Thread 8",
    "Thread 9",
    "Thread 10"
};

namespace tst {

    TEST_UNIT_BEG(TestingTheTest, 1)

        TEST_UNIT_SETUP_BEG
        TEST_UNIT_SETUP_END

        TEST_UNIT_TEARDOWN_BEG
        TEST_UNIT_TEARDOWN_END

        TEST_UNIT_MAIN_BEG
            PROF_BLOCK("First");

            std::vector<std::thread> threads;

            double sum{0.0};
            thr::SpinLock lock;

            for (uint64_t iii = 0; iii < 10; ++iii)
            {
                threads.emplace_back([&, iii] () {
                    PROF_THREAD(THREAD_NAMES[iii]);

                    for (uint64_t jjj = 0; jjj < 1e5; ++jjj)
                    {
                        PROF_BLOCK("Lock");
                        lock.lock();
                        {
                            PROF_SCOPE("SecureSection");
                            sum += 1e-6;
                        }
                        lock.unlock();
                        PROF_BLOCK_END();
                    }
                });
            }

            PROF_BLOCK("Wait_for_threads");
            for (auto &thread : threads)
            {
                thread.join();
            }
            PROF_BLOCK_END();

            PROF_BLOCK_END();

            std::cout << sum << std::endl;
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

