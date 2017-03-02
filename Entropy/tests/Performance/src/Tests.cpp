/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

// Sizes in bytes.
template <typename T>
using SmallContainer = ent::ComponentHolderList<T>;
static constexpr u64 SMALL_SIZE{4};
static constexpr u64 SMALL_NUM{100000};
template <typename T>
using MediumContainer = ent::ComponentHolderList<T>;
static constexpr u64 MEDIUM_SIZE{20};
static constexpr u64 MEDIUM_NUM{100000};
template <typename T>
using LargeContainer = ent::ComponentHolderList<T>;
static constexpr u64 LARGE_SIZE{40};
static constexpr u64 LARGE_NUM{100000};

struct SmallComponent
{
    using HolderT = SmallContainer<SmallComponent>;
    u8 values[SMALL_SIZE];
};

struct MediumComponent
{
    using HolderT = MediumContainer<MediumComponent>;
    u8 values[MEDIUM_SIZE];
};

struct LargeComponent
{
    using HolderT = LargeContainer<LargeComponent>;
    u8 values[LARGE_SIZE];
};

struct SmallSystem : public Universe::SystemT
{
    using Require = ent::Require<SmallComponent>;
};

struct MediumSystem : public Universe::SystemT
{
    using Require = ent::Require<MediumComponent>;
};

struct LargeSystem : public Universe::SystemT
{
    using Require = ent::Require<LargeComponent>;
};

TU_Begin(EntropyPerformance)

    /// Universe instance used in these tests.
    Universe universe;

    SmallSystem *smallSystem{nullptr};
    MediumSystem *mediumSystem{nullptr};
    LargeSystem *largeSystem{nullptr};

    TU_Setup
    {
        PROF_SCOPE("Initialization");

        universe.registerComponent<SmallComponent>();
        universe.registerComponent<MediumComponent>();
        universe.registerComponent<LargeComponent>();

        smallSystem = universe.addSystem<SmallSystem>();
        mediumSystem = universe.addSystem<MediumSystem>();
        largeSystem = universe.addSystem<LargeSystem>();

        universe.init();
    }

    TU_Teardown
    {

    }

    TU_Case(Performance0, "Testing the performance of the ECS")
    {
        {
            PROF_SCOPE("Adding small");
            for (u64 iii = 0; iii < SMALL_NUM; ++iii)
            {
                universe.createEntity().add<SmallComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh small");
            universe.refresh();
        }

        {
            PROF_SCOPE("Adding medium");
            for (u64 iii = 0; iii < MEDIUM_NUM; ++iii)
            {
                universe.createEntity().add<MediumComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh medium");
            universe.refresh();
        }

        {
            PROF_SCOPE("Adding large");
            for (u64 iii = 0; iii < LARGE_NUM; ++iii)
            {
                universe.createEntity().add<LargeComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh large");
            universe.refresh();
        }
    }

    TU_Case(Performance1, "Testing the performance of the ECS")
    {

    }

TU_End(EntropyPerformance)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

