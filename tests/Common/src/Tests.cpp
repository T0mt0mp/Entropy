/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

#include <unordered_map>
#include <map>

namespace tst {

    TEST_UNIT_BEG(TestingTheTest, 1)

        TEST_UNIT_SETUP_BEG
        TEST_UNIT_SETUP_END

        TEST_UNIT_TEARDOWN_BEG
        TEST_UNIT_TEARDOWN_END

        TEST_UNIT_MAIN_BEG
#if 0
            {
                static constexpr u64 SIZE{1000};
                u64 myArray[1000][1000];

                for (u64 iii = 0; iii < SIZE; ++iii)
                {
                    for (u64 jjj = 0; jjj < SIZE; ++jjj)
                    {
                        myArray[iii][jjj] = iii * jjj;
                    }
                }
            }
#endif

#if 0
            {
                using Type = u64;
                using MapType = std::unordered_map<u64, u64>;

                static constexpr u64 NUM_BUCKETS{128};
                static constexpr u64 BUCKET_SIZE{8192};
                static constexpr u64 TOTAL_SIZE{NUM_BUCKETS * BUCKET_SIZE};

                Type *buckets[NUM_BUCKETS]{nullptr, };
                std::unordered_map<u64, Type> testMap;
                testMap.rehash(TOTAL_SIZE);
                testMap.reserve(TOTAL_SIZE);

                MapType largeMap;
                MapType *smallMaps[NUM_BUCKETS]{nullptr, };

                u32 *smallArrayMaps[BUCKET_SIZE];

                for (u64 iii = 0; iii < NUM_BUCKETS; ++iii)
                {
                    buckets[iii] = new Type[BUCKET_SIZE];

                    smallMaps[iii] = new MapType;
                    smallMaps[iii]->rehash(BUCKET_SIZE);
                    smallMaps[iii]->reserve(BUCKET_SIZE);

                    smallArrayMaps[iii] = new u32[BUCKET_SIZE];
                }

                largeMap.rehash(TOTAL_SIZE);
                largeMap.reserve(TOTAL_SIZE);

                for (u64 iii = 0; iii < NUM_BUCKETS; ++iii)
                {
                    for (u32 jjj = 0; jjj < BUCKET_SIZE; ++jjj)
                    {
                        const u64 index{iii * BUCKET_SIZE + jjj};

                        {
                            PROF_SCOPE(largeMap_insert);
                            largeMap[index] = jjj;
                        }
                    }
                }

                for (u64 iii = 0; iii < NUM_BUCKETS; ++iii)
                {
                    for (u32 jjj = 0; jjj < BUCKET_SIZE; ++jjj)
                    {
                        const u64 index{iii * BUCKET_SIZE + jjj};

                        {
                            PROF_SCOPE(smallMaps_insert);
                            (*(smallMaps[iii]))[index] = jjj;
                        }
                    }
                }

                for (u64 iii = 0; iii < NUM_BUCKETS; ++iii)
                {
                    for (u32 jjj = 0; jjj < BUCKET_SIZE; ++jjj)
                    {
                        //const u64 index{iii * BUCKET_SIZE + jjj};

                        {
                            PROF_SCOPE(smallArrayMaps_insert);
                            smallArrayMaps[iii][jjj] = jjj;
                        }
                    }
                }

                for (u64 iii = 0; iii < NUM_BUCKETS; ++iii)
                {
                    for (u32 jjj = 0; jjj < BUCKET_SIZE; ++jjj)
                    {
                        const u64 index{iii * BUCKET_SIZE + jjj};

                        {
                            PROF_SCOPE(testMap_insert);
                            testMap[index] = jjj;
                        }
                    }
                }

                for (u64 iii = 0; iii < TOTAL_SIZE; ++iii)
                {
                    u64 bucket{iii / BUCKET_SIZE};
                    UNUSED(bucket);

                    {
                        PROF_SCOPE(largeMap_lookup);
                        u64 indexL{largeMap[iii]};
                        UNUSED(indexL);
                    }
                }

                for (u64 iii = 0; iii < TOTAL_SIZE; ++iii)
                {
                    u64 bucket{iii / BUCKET_SIZE};

                    {
                        PROF_SCOPE(smallMaps_lookup);
                        u64 indexS{(*(smallMaps[bucket]))[iii]};
                        UNUSED(indexS);
                    }
                }

                for (u64 iii = 0; iii < TOTAL_SIZE; ++iii)
                {
                    u64 bucket{iii / BUCKET_SIZE};

                    {
                        PROF_SCOPE(smallArrayMaps_lookup);
                        u64 indexA{smallArrayMaps[bucket][iii % BUCKET_SIZE]};
                        UNUSED(indexA);
                    }
                }

                for (u64 iii = 0; iii < TOTAL_SIZE; ++iii)
                {
                    u64 bucket{iii / BUCKET_SIZE};
                    UNUSED(bucket);

                    {
                        PROF_SCOPE(testMap_lookup);
                        u64 indexTM{testMap[iii]};
                        UNUSED(indexTM);
                    }
                }

                for (u64 iii = NUM_BUCKETS - 1; iii > 0; --iii)
                {
                    delete[] buckets[iii];
                    delete smallMaps[iii];
                    delete[] smallArrayMaps[iii];
                }

#if 0

                Type *array{new Type[TOTAL_SIZE]};

                for (u64 iii = 0; iii < TOTAL_SIZE; ++iii)
                {
                    array[iii] = iii;
                }

                delete[] array;
#endif
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

