/**
 * @file Comp.cpp
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#include "Comp.h"

artemis::World *MovementSystem::sWorld{nullptr};
std::size_t MovementSystem::sCounter{0u};

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
        artemis::World world;
        artemis::SystemManager *sm = world.getSystemManager();
        artemis::EntityManager *em = world.getEntityManager();

        sm->initializeAll();

        Timer t;
        for (std::size_t idx = 0; idx < creating; idx++)
        {
            artemis::Entity &e = em->create();
            e.addComponent(new PositionC(1.0f, 2.0f));
            e.addComponent(new MovementC(1.0f, 2.0f));
            e.refresh();
        }
        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<double>(nanoseconds) / creating
                  << std::endl;
    }
}

MovementSystem::MovementSystem()
{
    addComponentType<PositionC>();
    addComponentType<MovementC>();
}

void MovementSystem::initialize()
{
    mPosMapper.init(*sWorld);
    mMovMapper.init(*sWorld);
}

void MovementSystem::processEntity(artemis::Entity &e)
{
    sCounter++;
    mPosMapper.get(e)->x += mMovMapper.get(e)->dX;
    mPosMapper.get(e)->y += mMovMapper.get(e)->dY;
}

void MovementSystem::setWorld(artemis::World *world)
{
    MovementSystem::sWorld = world;
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
        artemis::World world;
        artemis::SystemManager *sm = world.getSystemManager();
        artemis::EntityManager *em = world.getEntityManager();

        MovementSystem::setWorld(&world);

        MovementSystem *ms = dynamic_cast<MovementSystem*>(sm->setSystem(new MovementSystem()));

        sm->initializeAll();

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(4007);

        MovementSystem::sCounter = 0;

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            artemis::Entity &e = em->create();
            if (uniform(rng) < percentage)
            {
                e.addComponent(new PositionC(1.0f, 2.0f));
                e.addComponent(new MovementC(1.0f, 2.0f));
            }
            e.refresh();
        }

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            world.loopStart();
            ms->process();
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << usage << "\t"
                  << static_cast<double>(nanoseconds) / repeats << "\t"
                  << MovementSystem::sCounter
                  << std::endl;
    }
}

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

