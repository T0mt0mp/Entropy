/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

TU_Begin(EntropyEntity)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

    TU_Case(Entity0, "Testing the Entity and TemporaryEntity")
    {
        using Universe = ParUniverse0::UniverseT;
        using Entity = ParUniverse0::EntityT;
        using TempEntity = ParUniverse0::TempEntityT;

        Universe u;
        u.registerComponent<Position>();
        u.registerComponent<Velocity>();

        u.init();

        PositionSystem *ps{u.addSystem<PositionSystem>()};
        VelocitySystem *vs{u.addSystem<VelocitySystem>()};
        PosVelSystem *pvs{u.addSystem<PosVelSystem>()};

        Entity e = u.createEntity();

        TC_Require(e.active());

        e.deactivate();
        TC_Require(!e.active());

        e.activate();
        TC_Require(e.active());

        e.deactivateD();
        TC_Require(e.active());

        e.add<Position>(1.0f, 1.0f);
        e.add<Velocity>(1.0f, 1.0f);

        TC_Require(e.get<Position>());
        TC_Require(e.get<Position>()->x == 1.0f && e.get<Position>()->y == 1.0f);
        TC_Require(static_cast<const Entity>(e).get<Position>()->x == 1.0f);

        TC_Require(e.get<Velocity>());
        TC_Require(e.get<Velocity>()->x == 1.0f && e.get<Velocity>()->y == 1.0f);
        TC_Require(static_cast<const Entity>(e).get<Velocity>()->x == 1.0f);

        TC_Require(e.has<Position>());
        TC_Require(e.has<Velocity>());
        TC_Require(!e.hasD<Position>());
        TC_Require(!e.hasD<Velocity>());

        u.refresh();

        TC_Require(e.active());

        u.commitChangeSet();
        u.refresh();

        TC_Require(!e.active());
        e.activateD();

        TC_RequireEqual(ps->foreach().size(), 0u);
        TC_RequireEqual(vs->foreach().size(), 0u);
        TC_RequireEqual(pvs->foreach().size(), 0u);

        u.refresh();

        TC_Require(!e.active());

        u.commitChangeSet();
        u.refresh();

        TC_Require(e.active());

        TC_RequireEqual(ps->foreach().size(), 0u);
        TC_RequireEqual(vs->foreach().size(), 0u);
        TC_RequireEqual(pvs->foreach().size(), 1u);

        TC_Require(e.get<Position>()->x == 1.0f && e.get<Position>()->y == 1.0f);
        TC_Require(e.get<Velocity>()->x == 1.0f && e.get<Velocity>()->y == 1.0f);

        e.addD<Position>(2.0f, 2.0f);
        e.addD<Velocity>();
        TC_Require(e.hasD<Position>());
        TC_Require(e.hasD<Velocity>());

        TC_Require(e.get<Position>()->x == 1.0f && e.get<Position>()->y == 1.0f);
        TC_Require(e.get<Velocity>()->x == 1.0f && e.get<Velocity>()->y == 1.0f);

        TC_Require(e.getD<Position>()->x == 2.0f && e.getD<Position>()->y == 2.0f);
        TC_Require(e.getD<Velocity>()->x == 0.0f && e.getD<Velocity>()->y == 0.0f);

        e.removeDC<Position>();
        e.removeDC<Velocity>();
        TC_Require(!e.hasD<Position>());
        TC_Require(!e.hasD<Velocity>());

        e.addD<Position>(2.0f, 2.0f);
        e.addD<Velocity>();
        TC_Require(e.hasD<Position>());
        TC_Require(e.hasD<Velocity>());

        TC_Require(e.getD<Position>()->x == 2.0f && e.getD<Position>()->y == 2.0f);
        TC_Require(e.getD<Velocity>()->x == 0.0f && e.getD<Velocity>()->y == 0.0f);

        e.remove<Position>();
        e.remove<Velocity>();
        TC_Require(!e.has<Position>());
        TC_Require(!e.has<Velocity>());

        u.refresh();

        TC_Require(!e.has<Position>());
        TC_Require(!e.has<Velocity>());
        TC_Require(e.hasD<Position>());
        TC_Require(e.hasD<Velocity>());

        u.commitChangeSet();
        u.refresh();

        TC_Require(e.has<Position>());
        TC_Require(e.has<Velocity>());
        TC_Require(!e.hasD<Position>());
        TC_Require(!e.hasD<Velocity>());

        TC_Require(e.get<Position>()->x == 2.0f && e.get<Position>()->y == 2.0f);
        TC_Require(e.get<Velocity>()->x == 0.0f && e.get<Velocity>()->y == 0.0f);

        TC_RequireEqual(ps->foreach().size(), 0u);
        TC_RequireEqual(vs->foreach().size(), 0u);
        TC_RequireEqual(pvs->foreach().size(), 1u);

        e.removeD<Position>();
        e.removeD<Velocity>();

        u.commitChangeSet();
        u.refresh();

        TC_Require(!e.has<Position>());
        TC_Require(!e.has<Velocity>());

        e.add<Velocity>();

        e.removeD<Position>();
        e.addD<Position>();

        e.addD<Velocity>();
        e.removeD<Velocity>();

        u.commitChangeSet();
        u.refresh();

        TC_Require(e.has<Position>());
        TC_Require(!e.has<Velocity>());

        e.addD<Position>();
        e.addD<Velocity>();

        e.destroyD();

        TC_Require(e.valid());

        u.commitChangeSet();
        u.refresh();

        TC_Require(!e.valid());

        TempEntity te = u.createEntityD();

        TC_Require(!te.has<Position>());
        TC_Require(!te.has<Velocity>());

        te.add<Position>(2.0f, 2.0f);
        te.add<Velocity>(2.0f, 2.0f);

        TC_Require(te.has<Position>());
        TC_Require(te.has<Velocity>());

        TC_RequireEqual(te.get<Position>()->x, 2.0f);
        TC_RequireEqual(te.get<Position>()->y, 2.0f);
        TC_RequireEqual(te.get<Velocity>()->x, 2.0f);
        TC_RequireEqual(te.get<Velocity>()->y, 2.0f);

        u.commitChangeSet();
        u.refresh();

        TC_RequireEqual(ps->foreach().size(), 0u);
        TC_RequireEqual(vs->foreach().size(), 0u);
        TC_RequireEqual(pvs->foreach().size(), 1u);

        TC_RequireEqual(pvs->foreach().begin()->get<Position>()->x, 2.0f);
        TC_RequireEqual(pvs->foreach().begin()->get<Position>()->y, 2.0f);
        TC_RequireEqual(pvs->foreach().begin()->get<Velocity>()->x, 2.0f);
        TC_RequireEqual(pvs->foreach().begin()->get<Velocity>()->y, 2.0f);

        te = u.createEntityD();
        te.add<Position>();
        te.deactivate();
        te.activate();

        te = u.createEntityD();
        te.add<Velocity>();
        te.deactivate();

        te = u.createEntityD();
        te.add<Position>();
        te.add<Velocity>();
        te.remove<Position>();

        te = u.createEntityD();
        te.add<Position>();
        te.add<Velocity>();
        te.destroy();

        u.commitChangeSet();
        u.refresh();

        TC_RequireEqual(ps->foreach().size(), 1u);
        TC_RequireEqual(vs->foreach().size(), 1u);
        TC_RequireEqual(pvs->foreach().size(), 1u);
    }

    TU_Case(Parallel0, "Testing parallel access")
    {
        PROF_SCOPE("Parallel0");
        using Universe = ParUniverse0::UniverseT;
        using Entity = ParUniverse0::EntityT;
        using TempEntity = ParUniverse0::TempEntityT;

        static constexpr u64 NUM_THREADS{4u};
        static constexpr u64 NUM_ENTITIES{1000u};
        static constexpr const char *THREAD_NAMES[NUM_THREADS + 1u] = {
            "Thread 0",
            "Thread 1",
            "Thread 2",
            "Thread 3",
            "Thread 4"
        };

        Universe u;
        u.registerComponent<Position>();
        u.registerComponent<Velocity>();

        u.init();

        PositionSystem *ps{u.addSystem<PositionSystem>()};
        VelocitySystem *vs{u.addSystem<VelocitySystem>()};
        PosVelSystem *pvs{u.addSystem<PosVelSystem>()};

        std::vector<std::thread> threads;
        for (u64 iii = 1; iii <= NUM_THREADS; ++iii)
        {
            threads.emplace_back([&, iii] () {
                PROF_THREAD(THREAD_NAMES[iii]);
                for (u64 entNum = 0; entNum < NUM_ENTITIES; ++entNum)
                {
                    PROF_SCOPE("Entity");

                    PROF_BLOCK("Create");
                    TempEntity te = u.createEntityD();
                    PROF_BLOCK_END();

                    PROF_BLOCK("Add Position");
                    te.add<Position>(iii, iii);
                    PROF_BLOCK_END();

                    PROF_BLOCK("Add Velocity");
                    te.add<Velocity>(iii, iii);
                    PROF_BLOCK_END();
                }

                u.commitChangeSet();
            });
        }

        for (u64 entNum = 0; entNum < NUM_ENTITIES; ++entNum)
        {
            Entity e = u.createEntity();
            e.add<Position>(0.0f, 0.0f);
            e.add<Velocity>(0.0f, 0.0f);
        }

        for (u64 iii = 0; iii < NUM_THREADS; ++iii)
        {
            threads[iii].join();
        }

        PROF_BLOCK("Refresh");
        u.refresh();
        PROF_BLOCK_END();

        TC_RequireEqual(ps->foreach().size(), 0u);
        TC_RequireEqual(vs->foreach().size(), 0u);
        TC_RequireEqual(pvs->foreach().size(), (NUM_THREADS + 1u) * NUM_ENTITIES);
    }

TU_End(EntropyEntity)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

