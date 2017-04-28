/**
 * @file Comp.cpp
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#include "Comp.h"

void createEntities(int argc, char *argv[])
{
    static constexpr std::size_t ATTEMPTS{10};

    ASSERT_FATAL(argc >= 4);

    const std::size_t start{static_cast<size_t>(atol(argv[2]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[3]))};
    const std::size_t max{static_cast<size_t>(atol(argv[4]))};

    std::cout << "Entities\tEntropy\tEntropyPerEnt" << std::endl;

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < ATTEMPTS; ++attempt)
        {
            Universe u;
            u.registerComponent<PositionC>();
            u.registerComponent<MovementC>();
            u.init();

            Timer t;
            for (std::size_t idx = 0; idx < creating; idx++)
            {
                Universe::EntityT e = u.createEntity();
                e.add<PositionC>(1.0f, 2.0f);
                e.add<MovementC>(1.0f, 2.0f);
            }

            total += t.nanoseconds();
        }

        std::size_t nanoseconds{total / ATTEMPTS};

        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / creating) << "\t"
                  << std::endl;
    }
}

void movementSystem(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 6);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t start{static_cast<size_t>(atol(argv[4]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[5]))};
    const std::size_t max{static_cast<size_t>(atol(argv[6]))};

    std::cout << "InUse\tEntropy" << std::endl;

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        Universe u;
        u.registerComponent<PositionC>();
        u.registerComponent<MovementC>();
        u.init();
        MovementSystem *ms{u.addSystem<MovementSystem>()};
        u.refresh();

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            Universe::EntityT e = u.createEntity();
            if (uniform(rng) < percentage)
            {
                e.add<PositionC>(1.0f, 2.0f);
                e.add<MovementC>(1.0f, 2.0f);
            }
        }

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            for (auto &e : ms->foreach())
            {
                PositionC *p{e.get<PositionC>()};
                MovementC *m{e.get<MovementC>()};

                p->x += m->dX;
                p->y += m->dY;
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << usage << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats) << "\t"
                  << std::endl;
    }
}

void advancedMovementSystem(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 6);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t start{static_cast<size_t>(atol(argv[4]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[5]))};
    const std::size_t max{static_cast<size_t>(atol(argv[6]))};

    std::cout << "Change\tEntropy" << std::endl;

    for (std::size_t change = start;
         change <= max;
         change += increment)
    {
        Universe u;
        u.registerComponent<PositionC>();
        u.registerComponent<MovementC>();
        u.init();
        PositionSystem *ps{u.addSystem<PositionSystem>()};
        MovementSystem *ms{u.addSystem<MovementSystem>()};
        u.refresh();

        const float percentage{change / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            Universe::EntityT e = u.createEntity();
            e.add<PositionC>(1.0f, 2.0f);
            e.add<MovementC>(1.0f, 2.0f);
        }

        u64 counter1{0u};
        u64 counter2{0u};

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            for (auto &e : ms->foreach())
            {
                PositionC *p{e.get<PositionC>()};
                MovementC *m{e.get<MovementC>()};

                p->x += m->dX;
                p->y += m->dY;
            }

            for (auto &e : ps->foreach())
            {
                if (uniform(rng) < percentage)
                {
                    if (e.has<MovementC>())
                    {
                        counter1++;
                        e.remove<MovementC>();
                    }
                    else
                    {
                        counter2++;
                        e.add<MovementC>(1.0f, 2.0f);
                    }
                }
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << change << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats) << "\t"
                  << counter1 << "\t" << counter2
                  << std::endl;
    }
}

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

