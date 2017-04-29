/**
 * @file Comp.cpp
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#include "Comp.h"

void createEntities(int argc, char *argv[])
{
    static constexpr std::size_t ATTEMPTS{20};

    ASSERT_FATAL(argc >= 4);

    const std::size_t start{static_cast<size_t>(atol(argv[2]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[3]))};
    const std::size_t max{static_cast<size_t>(atol(argv[4]))};

    std::cout << "Entities\tAnax\tAnaxPerEnt" << std::endl;

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < ATTEMPTS; ++attempt)
        {
            anax::World world;

            Timer t;
            for (std::size_t idx = 0; idx < creating; idx++)
            {
                anax::Entity e = world.createEntity();
                e.addComponent<PositionC>(1.0f, 2.0f);
                e.addComponent<MovementC>(1.0f, 2.0f);
                e.activate();
            }

            total += t.nanoseconds();
        }

        std::size_t nanoseconds{total / ATTEMPTS};

        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / creating)
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

    std::cout << "InUse\tAnax" << std::endl;

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        anax::World world;

        MovementSystem ms;
        world.addSystem(ms);

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            anax::Entity e = world.createEntity();
            if (uniform(rng) < percentage)
            {
                e.addComponent<PositionC>(1.0f, 2.0f);
                e.addComponent<MovementC>(1.0f, 2.0f);
            }
            e.activate();
        }

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            world.refresh();
            auto entities = ms.foreach();

            for (auto &e : entities)
            {
                PositionC &pos = e.getComponent<PositionC>();
                MovementC &mov = e.getComponent<MovementC>();

                pos.x += mov.dX;
                pos.y += mov.dY;
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << usage << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
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

    std::cout << "Change\tAnax" << std::endl;

    for (std::size_t change = start;
         change <= max;
         change += increment)
    {
        anax::World world;

        MovementSystem ms;
        world.addSystem(ms);
        PositionSystem ps;
        world.addSystem(ps);

        const float percentage{change / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            anax::Entity e = world.createEntity();
            e.addComponent<PositionC>(1.0f, 2.0f);
            e.addComponent<MovementC>(1.0f, 2.0f);
            e.activate();
        }

        u64 counter1{0u};
        u64 counter2{0u};

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            world.refresh();
            auto entitiesMs = ms.foreach();
            auto entitiesPs = ps.foreach();

            for (auto &e : entitiesMs)
            {
                PositionC &pos = e.getComponent<PositionC>();
                MovementC &mov = e.getComponent<MovementC>();

                pos.x += mov.dX;
                pos.y += mov.dY;
            }

            for (auto &e : entitiesPs)
            {
                if (uniform(rng) < percentage)
                {
                    if (e.hasComponent<MovementC>())
                    {
                        counter1++;
                        e.removeComponent<MovementC>();
                        e.activate();
                    }
                    else
                    {
                        counter2++;
                        e.addComponent<MovementC>(1.0f, 2.0f);
                        //e.activate();
                    }
                }
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << change << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
                  << std::endl;
    }
}

void parallelEntity(int argc, char *argv[])
{ std::cout << "Not supported" << std::endl; }
void parallelChangeset(int argc, char *argv[])
{ std::cout << "Not supported" << std::endl; }
void holders(int argc, char *argv[])
{ std::cout << "Not implemented" << std::endl; }

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

