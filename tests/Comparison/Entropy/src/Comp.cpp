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

    std::cout << "Entities\tEntropy\tEntropyPerEnt" << std::endl;

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < ATTEMPTS; ++attempt)
        {
            Universe u;
            u.registerComponent<PositionCMain>();
            u.registerComponent<MovementCMain>();
            u.init();

            Timer t;
            for (std::size_t idx = 0; idx < creating; idx++)
            {
                Universe::EntityT e = u.createEntity();
                e.add<PositionCMain>(1.0f, 2.0f);
                e.add<MovementCMain>(1.0f, 2.0f);
            }
            //u.refresh();

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

    std::cout << "InUse\tEntropy" << std::endl;

    for (std::size_t usage = start;
         usage <= max;
         usage += increment)
    {
        Universe u;
        u.registerComponent<PositionCMain>();
        u.registerComponent<MovementCMain>();
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
                e.add<PositionCMain>(1.0f, 2.0f);
                e.add<MovementCMain>(1.0f, 2.0f);
            }
        }

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            for (auto &e : ms->foreach())
            {
                PositionCMain *p{e.get<PositionCMain>()};
                MovementCMain *m{e.get<MovementCMain>()};

                p->x += m->dX;
                p->y += m->dY;
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

    std::cout << "Change\tEntropy" << std::endl;

    for (std::size_t change = start;
         change <= max;
         change += increment)
    {
        Universe u;
        u.registerComponent<PositionCMain>();
        u.registerComponent<MovementCMain>();
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
            e.add<PositionCMain>(1.0f, 2.0f);
            e.add<MovementCMain>(1.0f, 2.0f);
        }

        u64 counter1{0u};
        u64 counter2{0u};

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            for (auto &e : ms->foreach())
            {
                PositionCMain *p{e.get<PositionCMain>()};
                MovementCMain *m{e.get<MovementCMain>()};

                p->x += m->dX;
                p->y += m->dY;
            }

            for (auto &e : ps->foreach())
            {
                if (uniform(rng) < percentage)
                {
                    if (e.has<MovementCMain>())
                    {
                        counter1++;
                        e.remove<MovementCMain>();
                    }
                    else
                    {
                        counter2++;
                        e.add<MovementCMain>(1.0f, 2.0f);
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

//#define USE_THREAD_POOL
#ifdef USE_THREAD_POOL
class ThreadPool
{
public:
    class Job
    {
    public:
        Job(std::function<void()> &&job) :
            mJob{job}
        { }

        inline void run()
        { mJob(); }
    private:
        std::function<void()> mJob;
    protected:
    }; // class Job

    ThreadPool(std::size_t numThreads) :
        mRunning{true}, mBusy{0u}
    {
        for (u64 iii = 0; iii < numThreads; ++iii)
        {
            mThreads.emplace_back([this] () { workerLoop(); });
        }
    }

    inline void addJob(Job *job)
    {
        std::lock_guard<std::mutex> g(mWorkMutex);
        mJobs.emplace_back(job);
        mWorkCV.notify_one();
    }

    inline void waitUntilAllFinished()
    {
        std::unique_lock<std::mutex> lock(mWorkMutex);
        mWorkFinishedCV.wait(lock, [this] () { return mJobs.empty() && mBusy == 0u; });
    }

    ~ThreadPool()
    {
        std::unique_lock<std::mutex> lock(mWorkMutex);
        mRunning = false;
        mWorkCV.notify_all();
        lock.unlock();

        for (auto &t : mThreads)
        {
            t.join();
        }
    }
private:
    inline void workerLoop()
    {
        while (mRunning)
        {
            std::unique_lock<std::mutex> lock(mWorkMutex);
            mWorkCV.wait(lock, [this] { return mJobs.size() || !mRunning; });
            if (!mRunning)
            {
                return;
            }
            Job *myJob{mJobs.back()};
            mJobs.pop_back();
            mBusy++;
            lock.unlock();

            myJob->run();
            delete myJob;

            lock.lock();
            mBusy--;
            mWorkFinishedCV.notify_one();
        }
    }

    std::vector<std::thread> mThreads;
    std::mutex mWorkMutex;
    std::condition_variable mWorkCV;
    std::condition_variable mWorkFinishedCV;
    std::atomic_bool mRunning;
    std::vector<Job*> mJobs;
    std::size_t mBusy;
protected:
}; // class ThreadPool
#endif

inline void parProcess(Universe::EntityT &e)
{
    PositionC *p{e.get<PositionC>()};
    MovementC *m{e.get<MovementC>()};

    p->x += m->dX;
    p->y += m->dY;
}

void parallelEntity(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 6);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t start{static_cast<size_t>(atol(argv[4]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[5]))};
    const std::size_t max{static_cast<size_t>(atol(argv[6]))};

    ASSERT_FATAL(start >= 1);

    std::cout << "Threads\tEntropy" << std::endl;

#ifdef USE_THREAD_POOL
    ThreadPool tp(max);
#endif

    for (std::size_t threads = start;
         threads <= max;
         threads += increment)
    {
        Universe u;
        u.registerComponent<PositionC>();
        u.registerComponent<MovementC>();
        u.init();
        MovementSystem *ms{u.addSystem<MovementSystem>()};
        u.refresh();

#ifndef USE_THREAD_POOL
        std::vector<std::thread> threadList;
#endif

        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            Universe::EntityT e = u.createEntity();
            e.add<PositionC>(1.0f, 2.0f);
            e.add<MovementC>(1.0f, 2.0f);
        }
        u.refresh();

        Timer t;
        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            if (threads > 1)
            {
                auto parForeach = ms->foreachP(threads);

                for (u64 iii = 1; iii < threads; ++iii)
                {
#ifdef USE_THREAD_POOL
                    tp.addJob(new ThreadPool::Job([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            parProcess(e);
                        }
                    }));
#else
                    threadList.emplace_back([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            parProcess(e);
                        }
                    });
#endif
                }

                for (auto &e : parForeach.forThread(0))
                {
                    parProcess(e);
                }


#ifdef USE_THREAD_POOL
                tp.waitUntilAllFinished();
#else
                for (std::thread &th : threadList)
                {
                    th.join();
                }

                threadList.clear();
#endif
            }
            else
            {
                for (auto &e : ms->foreach())
                {
                    parProcess(e);
                }
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << threads << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
                  << std::endl;
    }
}

void parProcessCSB(Universe::EntityT &e)
{
    PositionC *p{e.get<PositionC>()};
    MovementC *m{e.get<MovementC>()};

    p->x += m->dX;
    p->y += m->dY;
}

void parProcessCSP(Universe::EntityT &e)
{
    PositionC *pos{e.get<PositionC>()};
    MovementC *mov{e.get<MovementC>()};
    e.addD<PositionC>(pos->x + mov->dX, pos->y + mov->dY);
}

#undef USE_THREAD_POOL

void parallelChangeset(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 6);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t start{static_cast<size_t>(atol(argv[4]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[5]))};
    const std::size_t max{static_cast<size_t>(atol(argv[6]))};

    ASSERT_FATAL(start >= 1);

    std::cout << "Threads\tEntropy" << std::endl;

#ifdef USE_THREAD_POOL
    ThreadPool tp(max);
#endif

    for (std::size_t threads = start;
         threads <= max;
         threads += increment)
    {
        Universe u;
        u.registerComponent<PositionC>();
        u.registerComponent<MovementC>();
        u.init();
        MovementSystem *ms{u.addSystem<MovementSystem>()};
        u.refresh();

#ifndef USE_THREAD_POOL
        std::vector<std::thread> threadList;
#endif

        for (std::size_t idx = 0; idx < numEntities; idx++)
        {
            Universe::EntityT e = u.createEntity();
            e.add<PositionC>(1.0f, 2.0f);
            e.add<MovementC>(1.0f, 2.0f);
        }
        u.refresh();

        Timer t;
        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            // Refresh is not finished yet.
            //u.refresh();

            if (threads > 1)
            {
                auto parForeach = ms->foreachP(threads);

                for (u64 iii = 1; iii < threads; ++iii)
                {
#ifdef USE_THREAD_POOL
                    tp.addJob(new ThreadPool::Job([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            parProcessCSP(e);
                        }
                    }));
#else
                    threadList.emplace_back([iii, &parForeach, &u] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            parProcessCSP(e);

                        }
                        u.commitChangeSet();
                    });
#endif
                }

                for (auto &e : parForeach.forThread(0))
                {
                    parProcessCSP(e);
                }
                u.commitChangeSet();


#ifdef USE_THREAD_POOL
                tp.waitUntilAllFinished();
#else
                for (std::thread &th : threadList)
                {
                    th.join();
                }

                threadList.clear();
#endif
            }
            else
            {
                for (auto &e : ms->foreach())
                {
                    parProcessCSP(e);
                }
                u.commitChangeSet();
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << threads << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
                  << std::endl;
    }
}

template <typename ComponentT>
void holdersHelper(int argc, char *argv[])
{
    const std::size_t start{static_cast<size_t>(atol(argv[2]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[3]))};
    const std::size_t max{static_cast<size_t>(atol(argv[4]))};
    const std::size_t numRepeats{static_cast<size_t>(atol(argv[5]))};
    const std::size_t operations{static_cast<size_t>(atol(argv[6]))};
    const std::size_t doRand{static_cast<size_t>(atol(argv[8]))};

    std::cout << "Entities\tEntropy\tEntropyPerEnt" << std::endl;

    std::mt19937_64 rng;
    std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

    rng.seed(RANDOM_SEED);

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t total{0u};

        for (std::size_t attempt = 0; attempt < numRepeats; ++attempt)
        {
            Universe u;
            u.registerComponent<ComponentT>();
            u.init();

            Timer t;
            for (std::size_t idx = 0; idx < creating; idx++)
            {
                Universe::EntityT e = u.createEntity();
                e.add<ComponentT>();
            }
            //u.refresh();

            std::size_t idx;
            for (std::size_t iii = 0; iii < creating; iii++)
            {
                if (doRand)
                {
                    idx = static_cast<std::size_t>(uniform(rng) * creating);
                }
                else
                {
                    idx = iii;
                }
                ComponentT *c{u.getComponent<ComponentT>(ent::EntityId(idx + 1u, 0u))};
                for (u64 iii = 0; iii < operations; ++iii)
                {
                    c->data[iii] = iii;
                }
            }
            total += t.nanoseconds();
        }

        std::size_t nanoseconds{total / numRepeats};

        std::cout << creating << "\t"
                  << nanoseconds << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / creating)
                  << std::endl;
    }
}

using HoldersCmpFun = void(*)(int argc, char *argv[]);

std::pair<const char*, HoldersCmpFun> holderCallArray[] = {
    {"sm", holdersHelper<SmallCM>},
    {"mm", holdersHelper<MediumCM>},
    {"lm", holdersHelper<LargeCM>},

    {"sml", holdersHelper<SmallCML>},
    {"mml", holdersHelper<MediumCML>},
    {"lml", holdersHelper<LargeCML>},

    {"sl", holdersHelper<SmallCL>},
    {"ml", holdersHelper<MediumCL>},
    {"ll", holdersHelper<LargeCL>},
};

void holders(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 9);
    const char *type{argv[7]};

    for (auto &p : holderCallArray)
    {
        if (!strcmp(type, p.first))
        {
            p.second(argc, argv);
            return;
        }
    }
}

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

