/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

// Sizes in bytes.
static constexpr u64 ITER_NUM{4};
static constexpr u64 EMPTY_NUM{100000};
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

struct EmptySystem : public Universe::SystemT
{
    using Require = ent::Require<>;
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

    EmptySystem *emptySystem{nullptr};
    SmallSystem *smallSystem{nullptr};
    MediumSystem *mediumSystem{nullptr};
    LargeSystem *largeSystem{nullptr};

    TU_Setup
    {
        PROF_SCOPE("Initialization");

        universe.registerComponent<SmallComponent>();
        universe.registerComponent<MediumComponent>();
        universe.registerComponent<LargeComponent>();

        emptySystem = universe.addSystem<EmptySystem>();
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
            PROF_SCOPE("Creating empty Entities");
            for (u64 iii = 0; iii < EMPTY_NUM; ++iii)
            {
                universe.createEntity();
            }
        }

        {
            PROF_SCOPE("Adding small");
            for (u64 iii = 0; iii < SMALL_NUM; ++iii)
            {
                universe.createEntity().add<SmallComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh adding small");
            universe.refresh();
        }

        {
            PROF_SCOPE("Removing small");
            for (Universe::EntityT &e : smallSystem->foreach())
            {
                e.remove<SmallComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh removing small");
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
            PROF_SCOPE("Removing medium");
            for (Universe::EntityT &e : mediumSystem->foreach())
            {
                e.remove<MediumComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh removing medium");
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

        {
            PROF_SCOPE("Removing large");
            for (Universe::EntityT &e : largeSystem->foreach())
            {
                e.remove<LargeComponent>();
            }
        }

        {
            PROF_SCOPE("Refresh removing large");
            universe.refresh();
        }

        for (Universe::EntityT &e : emptySystem->foreach())
        {
            e.add<SmallComponent>();
        }

        universe.refresh();

        {
            for (u64 iii = 0; iii < ITER_NUM; ++iii)
            {
                PROF_SCOPE("Iteration");

                PROF_BLOCK("Small system");
                std::cout << "Small: " << smallSystem->foreach().size() << std::endl;
                for (Universe::EntityT &e : smallSystem->foreach())
                {
                    e.remove<SmallComponent>();
                    e.add<MediumComponent>();
                }
                PROF_BLOCK_END();

                PROF_BLOCK("Medium system");
                std::cout << "Medium: " << mediumSystem->foreach().size() << std::endl;
                for (Universe::EntityT &e : mediumSystem->foreach())
                {
                    e.remove<MediumComponent>();
                    e.add<LargeComponent>();
                }
                PROF_BLOCK_END();

                PROF_BLOCK("Large system");
                std::cout << "Large: " << largeSystem->foreach().size() << std::endl;
                for (Universe::EntityT &e : largeSystem->foreach())
                {
                    e.remove<LargeComponent>();
                    e.add<SmallComponent>();
                }
                PROF_BLOCK_END();

                PROF_BLOCK("Refresh");
                universe.refresh();
                PROF_BLOCK_END();
            }
        }
    }

    TU_Case(Performance1, "Testing the performance of the ECS")
    {

    }

TU_End(EntropyPerformance)

int main(int argc, char* argv[])
{
    // Pure data size in bytes.
    u64 dataSize{
        SMALL_NUM * sizeof(SmallComponent) +
        MEDIUM_NUM * sizeof(MediumComponent) +
        LARGE_NUM * sizeof(LargeComponent)
    };

    std::cout << "Small entities: " << SMALL_NUM << std::endl;
    std::cout << "Small size: " << sizeof(SmallComponent) << std::endl;
    std::cout << "Medium entities: " << MEDIUM_NUM << std::endl;
    std::cout << "Medium size: " << sizeof(MediumComponent) << std::endl;
    std::cout << "Large entities: " << LARGE_NUM << std::endl;
    std::cout << "Large size: " << sizeof(LargeComponent) << std::endl;
    std::cout << "Number of Entities: " << EMPTY_NUM + SMALL_NUM + MEDIUM_NUM + LARGE_NUM << std::endl;
    std::cout << "Pure data [kB]: " << dataSize / 1000 << std::endl;

    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

