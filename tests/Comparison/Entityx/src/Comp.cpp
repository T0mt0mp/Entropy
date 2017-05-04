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

    std::cout << "Entities\tEntityX\tEntityXPerEnt" << std::endl;

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < ATTEMPTS; ++attempt)
        {
            entityx::EntityX ex;

            Timer t;
            for (std::size_t idx = 0; idx < creating; idx++)
            {
                entityx::Entity e = ex.entities.create();
                e.assign<PositionC>(1.0f, 2.0f);
                e.assign<MovementC>(1.0f, 2.0f);
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

    std::cout << "InUse\tEntityX" << std::endl;

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        entityx::EntityX ex;

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            entityx::Entity e = ex.entities.create();
            if (uniform(rng) < percentage)
            {
                e.assign<PositionC>(1.0f, 2.0f);
                e.assign<MovementC>(1.0f, 2.0f);
            }
        }

        entityx::ComponentHandle<PositionC> pos;
        entityx::ComponentHandle<MovementC> mov;

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            /*
            for (entityx::Entity e : ex.entities.entities_with_components(pos, mov))
            {
                pos->x += mov->dX;
                pos->y += mov->dY;
            }
            */
            ex.entities.each<PositionC, MovementC>([]
                                                      (entityx::Entity entity,
                                                       PositionC &pos,
                                                       MovementC &mov) {
                pos.x += mov.dX;
                pos.y += mov.dY;

                for (u64 iii = 0; iii < TASK_HARDNESS; ++iii)
                {
                    pos.x += cos(pos.x + mov.dX);
                    pos.y += sin(pos.y + mov.dY);
                }
            });
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

    std::cout << "Change\tEntityX" << std::endl;

    for (std::size_t change = start;
         change <= max;
         change += increment)
    {
        entityx::EntityX ex;

        const float percentage{change / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            entityx::Entity e = ex.entities.create();
            e.assign<PositionC>(1.0f, 2.0f);
            e.assign<MovementC>(1.0f, 2.0f);
        }

        u64 counter1{0u};
        u64 counter2{0u};

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            ex.entities.each<PositionC, MovementC>([&]
                                                   (entityx::Entity e,
                                                    PositionC &pos,
                                                    MovementC &mov) {
                pos.x += mov.dX;
                pos.y += mov.dY;

                for (u64 iii = 0; iii < TASK_HARDNESS; ++iii)
                {
                    pos.x += cos(pos.x + mov.dX);
                    pos.y += sin(pos.y + mov.dY);
                }
            });
            ex.entities.each<PositionC>([&]
                                                   (entityx::Entity e,
                                                    PositionC &pos) {
                if (uniform(rng) < percentage)
                {
                    if (e.has_component<MovementC>())
                    {
                        e.remove<MovementC>();
                    }
                    else
                    {
                        e.assign<MovementC>(1.0f, 2.0f);
                    }
                }
            });
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
void movementSystemP(int argc, char *argv[])
{ std::cout << "Not supported" << std::endl; }
void advancedMovementSystemP(int argc, char *argv[])
{ std::cout << "Not supported" << std::endl; }

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

