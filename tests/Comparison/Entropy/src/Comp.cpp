/**
 * @file Comp.cpp
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#include "Comp.h"

inline void computation(Universe::EntityT &e)
{
    PositionC *p{e.get<PositionC>()};
    MovementC *m{e.get<MovementC>()};

    p->x += m->dX;
    p->y += m->dY;
    for (u64 iii = 0; iii < TASK_HARDNESS; ++iii)
    {
        p->x += cos(p->x + m->dX);
        p->y += sin(p->y + m->dY);
    }
}

inline void computationPar(Universe::EntityT &e)
{
    PositionC *pos{e.get<PositionC>()};
    MovementC *mov{e.get<MovementC>()};

    PositionC tmp;

    tmp.x = pos->x + mov->dX;
    tmp.y = pos->y + mov->dY;
    for (u64 iii = 0; iii < TASK_HARDNESS; ++iii)
    {
        tmp.x += cos(tmp.x + mov->dX);
        tmp.y += sin(tmp.y + mov->dY);
    }

    e.addD<PositionC>(tmp);
}

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
                computation(e);
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

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            for (auto &e : ms->foreach())
            {
                computation(e);
            }

            for (auto &e : ps->foreach())
            {
                if (uniform(rng) < percentage)
                {
                    if (e.has<MovementC>())
                    {
                        e.remove<MovementC>();
                    }
                    else
                    {
                        e.add<MovementC>(1.0f, 2.0f);
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

//#define USE_THREAD_POOL

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
#else
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
                            computation(e);
                        }
                    }));
#else
                    threadList.emplace_back([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            computation(e);
                        }
                    });
#endif
                }

                for (auto &e : parForeach.forThread(0))
                {
                    computation(e);
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
                    computation(e);
                }
            }
        }

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << threads << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
                  << std::endl;
    }
}

//#define USE_THREAD_POOL

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
#else
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
            u.refresh();

            if (threads > 1)
            {
                auto parForeach = ms->foreachP(threads);

                for (u64 iii = 1; iii < threads; ++iii)
                {
#ifdef USE_THREAD_POOL
                    tp.addJob(new ThreadPool::Job([iii, &parForeach, &u] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            computationPar(e);
                        }
                        u.commitChangeSet();
                    }));
#else
                    threadList.emplace_back([iii, &parForeach, &u] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            computationPar(e);
                        }
                        u.commitChangeSet();
                    });
#endif
                }

                for (auto &e : parForeach.forThread(0))
                {
                    computationPar(e);
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
                    computationPar(e);
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

#undef USE_THREAD_POOL

template <typename ComponentT>
void holdersHelper(int argc, char *argv[])
{
    const std::size_t start{static_cast<size_t>(atol(argv[2]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[3]))};
    const std::size_t max{static_cast<size_t>(atol(argv[4]))};
    const std::size_t numRepeats{static_cast<size_t>(atol(argv[5]))};
    const std::size_t operations{static_cast<size_t>(atol(argv[6]))};
    const std::size_t doRand{static_cast<size_t>(atol(argv[8]))};

    std::cout << "Entities\tEntropyPerEnt" << std::endl;

    std::mt19937_64 rng;
    std::uniform_real_distribution<f64> uniform(0.0f, 1.0f);

    rng.seed(RANDOM_SEED);

    for (std::size_t creating = start;
         creating <= max;
         creating += increment)
    {
        std::size_t creation{0u};
        std::size_t access{0u};

        for (std::size_t attempt = 0; attempt < numRepeats; ++attempt)
        {
            Universe u;
            u.registerComponent<ComponentT>();
            u.init();

            for (std::size_t idx = 0; idx < creating; idx++)
            {
                Universe::EntityT e = u.createEntity();
            }

            Timer t1;
            for (std::size_t iii = 0; iii < creating; iii++)
            {
                u.addComponent<ComponentT>(ent::EntityId(iii + 1u, 0u));
            }
            creation += static_cast<double>(t1.nanoseconds()) / creating;

            Timer t2;
            if (doRand)
            {
                std::size_t idx;

                for (std::size_t iii = 0; iii < creating * operations; iii++)
                {
                    idx = static_cast<std::size_t>(uniform(rng) * creating);
                    ComponentT *c{u.getComponent<ComponentT>(ent::EntityId(idx + 1u, 0u))};
                    c->data[0] = 1;
                }
            }
            else
            {
                for (u64 opId = 0; opId < operations; ++opId)
                {
                    for (std::size_t iii = 0; iii < creating; iii++)
                    {
                        ComponentT *c{u.getComponent<ComponentT>(ent::EntityId(iii + 1u, 0u))};
                        c->data[0] = 1;
                    }
                }
            }
            access += static_cast<double>(t2.nanoseconds()) / operations / creating;
        }

        creation /= numRepeats;
        access /= numRepeats;


        std::cout << creating << "\t"
                  << creation + access
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

//#define USE_THREAD_POOL

void movementSystemP(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 7);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t threads{static_cast<size_t>(atol(argv[4]))};
    const std::size_t start{static_cast<size_t>(atol(argv[5]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[6]))};
    const std::size_t max{static_cast<size_t>(atol(argv[7]))};

    std::cout << "InUse\tEntropy" << std::endl;

#ifdef USE_THREAD_POOL
    ThreadPool tp(threads);
#endif

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

#ifndef USE_THREAD_POOL
        std::vector<std::thread> threadList;
#endif

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

            auto parForeach = ms->foreachP(threads);

            for (u64 iii = 1; iii < threads; ++iii)
            {
#ifdef USE_THREAD_POOL
                tp.addJob(new ThreadPool::Job([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            computation(e);
                        }
                    }));
#else
                threadList.emplace_back([iii, &parForeach, &u] () {
                    auto foreach = parForeach.forThread(iii);
                    for (auto &e : foreach)
                    {
                        computation(e);
                    }
                });
#endif
            }

            for (auto &e : parForeach.forThread(0))
            {
                computation(e);
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

        std::size_t nanoseconds{t.nanoseconds()};
        std::cout << usage << "\t"
                  << static_cast<std::size_t>(static_cast<double>(nanoseconds) / repeats)
                  << std::endl;
    }
}

void advancedMovementSystemP(int argc, char *argv[])
{
    ASSERT_FATAL(argc >= 7);

    const std::size_t numEntities{static_cast<size_t>(atol(argv[2]))};
    const std::size_t repeats{static_cast<size_t>(atol(argv[3]))};
    const std::size_t threads{static_cast<size_t>(atol(argv[4]))};
    const std::size_t start{static_cast<size_t>(atol(argv[5]))};
    const std::size_t increment{static_cast<size_t>(atol(argv[6]))};
    const std::size_t max{static_cast<size_t>(atol(argv[7]))};

    std::cout << "Change\tEntropy" << std::endl;

#ifdef USE_THREAD_POOL
    ThreadPool tp(max);
#endif

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

#ifndef USE_THREAD_POOL
        std::vector<std::thread> threadList;
#endif

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

        for (std::size_t rep = 0; rep < repeats; ++rep)
        {
            u.refresh();

            auto parForeach = ms->foreachP(threads);

            for (u64 iii = 1; iii < threads; ++iii)
            {
#ifdef USE_THREAD_POOL
                tp.addJob(new ThreadPool::Job([iii, &parForeach] () {
                        auto foreach = parForeach.forThread(iii);
                        for (auto &e : foreach)
                        {
                            computation(e);
                        }
                    }));
#else
                threadList.emplace_back([iii, &parForeach, &u] () {
                    auto foreach = parForeach.forThread(iii);
                    for (auto &e : foreach)
                    {
                        computation(e);
                    }
                });
#endif
            }

            for (auto &e : parForeach.forThread(0))
            {
                computation(e);
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

            for (auto &e : ps->foreach())
            {
                if (uniform(rng) < percentage)
                {
                    if (e.has<MovementC>())
                    {
                        e.remove<MovementC>();
                    }
                    else
                    {
                        e.add<MovementC>(1.0f, 2.0f);
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

int main(int argc, char* argv[])
{
    runComparison(argc, argv);
}

