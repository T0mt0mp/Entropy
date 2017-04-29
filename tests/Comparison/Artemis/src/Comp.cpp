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

    std::cout << "Entities\tArtemis\tArtemisPerEnt" << std::endl;

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < ATTEMPTS; ++attempt)
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

            total += t.nanoseconds();
        }

        std::size_t nanoseconds{total / ATTEMPTS};

        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / creating)
                  << std::endl;
    }
}

MovementSystem::MovementSystem(artemis::World *world)
{
    setWorld(world);
    addComponentType<PositionC>();
    addComponentType<MovementC>();
}

void MovementSystem::initialize()
{
    mPosMapper.init(*world);
    mMovMapper.init(*world);
}

void MovementSystem::processEntity(artemis::Entity &e)
{
    mPosMapper.get(e)->x += mMovMapper.get(e)->dX;
    mPosMapper.get(e)->y += mMovMapper.get(e)->dY;
}

PositionSystem::PositionSystem(artemis::World *world,
                               std::mt19937_64 &rng,
                               std::uniform_real_distribution<f64> &uniform,
                               float percentage) :
    mPercentage{percentage}, mRng{rng}, mUniform{uniform}
{
    setWorld(world);
    addComponentType<PositionC>();
}

void PositionSystem::initialize()
{
    mPosMapper.init(*world);
    mMovMapper.init(*world);
}

void PositionSystem::processEntity(artemis::Entity &e)
{
    static int compId = artemis::ComponentTypeManager::getId<MovementC>();
    if (mUniform(mRng) < mPercentage)
    {
        //if (e.getTypeBits().test(compId))
        if (mMovMapper.get(e))
        {
            counter1++;
            e.removeComponent<MovementC>();
        }
        else
        {
            counter2++;
            e.addComponent(new MovementC(1.0f, 2.0f));
        }
        e.refresh();
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

    std::cout << "InUse\tArtemis" << std::endl;

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        artemis::World world;
        artemis::SystemManager *sm = world.getSystemManager();
        artemis::EntityManager *em = world.getEntityManager();

        MovementSystem *ms = dynamic_cast<MovementSystem*>(sm->setSystem(new MovementSystem(&world)));

        sm->initializeAll();

        const float percentage{usage / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        rng.seed(RANDOM_SEED);

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

    std::cout << "Change\tArtemis" << std::endl;

    for (std::size_t change = start;
         change <= max;
         change += increment)
    {
        artemis::World world;
        artemis::SystemManager *sm = world.getSystemManager();
        artemis::EntityManager *em = world.getEntityManager();

        const float percentage{change / 100.0f};
        std::mt19937_64 rng;
        std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

        MovementSystem *ms = dynamic_cast<MovementSystem*>(sm->setSystem(new MovementSystem(&world)));
        PositionSystem *ps = dynamic_cast<PositionSystem*>(sm->setSystem(new PositionSystem(&world, rng, uniform, percentage)));

        sm->initializeAll();

        rng.seed(RANDOM_SEED);

        Timer t;
        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            artemis::Entity &e = em->create();
            e.addComponent(new PositionC(1.0f, 2.0f));
            e.addComponent(new MovementC(1.0f, 2.0f));
            e.refresh();
        }

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            world.loopStart();
            ms->process();
            ps->process();
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

