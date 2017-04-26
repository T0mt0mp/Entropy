/**
 * @file Comp.cpp
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#include "Comp.h"

void createEntities(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 4);

    const std::size_t start{static_cast<size_t>(atol(argv[2]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[3]))};
    const std::size_t max{static_cast<size_t>(atol(argv[4]))};

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        entityx::EntityX ex;

        Timer t;
        for (std::size_t idx = 0; idx < creating; idx++)
        {
            entityx::Entity e = ex.entities.create();
            e.assign<PositionC>(1.0f, 2.0f);
            e.assign<MovementC>(1.0f, 2.0f);
        }
        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<double>(nanoseconds) / creating
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

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        entityx::EntityX ex;

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(4007);

        std::size_t counter{0u};

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
            ex.entities.each<PositionC, MovementC>([&counter]
                                                      (entityx::Entity entity,
                                                       PositionC &pos,
                                                       MovementC &mov) {
                counter++;
                pos.x += mov.dX;
                pos.y += mov.dY;
            });
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << usage << "\t"
                  << static_cast<double>(nanoseconds) / repeats << "\t"
                  << counter
                  << std::endl;
    }
}

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

