/**
 * @file CompSys.h
 * @author Tomas Polasek
 * @brief Comparison system functions.
 */

#ifndef COMPSYS_H
#define COMPSYS_H

#include <random>
#include <thread>
#include <condition_variable>
#include <functional>

using CompFun = void(*)(int argc, char *argv[]);
using ParamCheckFun = void(*)(int argc, char *argv[]);

/// Number used as a random seed
static constexpr std::size_t RANDOM_SEED{4007};

/// How hard is the computation task.
static constexpr std::size_t TASK_HARDNESS{30};

/*
 * Prototypes of called comparison functions.
 */

// 3 parameters for number of entities created - start, delta and max
void createEntities(int argc, char *argv[]);
void createEntitiesCheck(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cout << "Need 3 parameters: starting number, delta and max entities" << std::endl;
        std::exit(1);
    }
}

// 5 parameters number of entities, number of repeats
// and percentage in use - start, delta and max
void movementSystem(int argc, char *argv[]);
void movementSystemCheck(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cout << "Need 5 parameters: number of entities, number of repeats, "
                  << "starting number, delta and max usage %" << std::endl;
        std::exit(1);
    }
}

// 5 parameters number of entities, number of repeats
// and percentage to change - start, delta and max
void advancedMovementSystem(int argc, char *argv[]);
void advancedMovementSystemCheck(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cout << "Need 5 parameters: number of entities, number of repeats, "
                  << "starting number, delta and max change %" << std::endl;
        std::exit(1);
    }
}

// 5 parameters number of entities, number of repeats
// start number of threads, delta and max
void parallelEntity(int argc, char *argv[]);
void parallelEntityCheck(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cout << "Need 5 parameters: number of entities, number of repeats, "
                  << "starting number, delta and max threads" << std::endl;
        std::exit(1);
    }
}

// 5 parameters number of entities, number of repeats
// start number of threads, delta and max
void parallelChangeset(int argc, char *argv[]);
void parallelChangesetCheck(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cout << "Need 5 parameters: number of entities, number of repeats, "
                  << "starting number, delta and max threads" << std::endl;
        std::exit(1);
    }
}

// 7 parameters number of entities - start delta and max, repeats, operation size and type of component/holder
void holders(int argc, char *argv[]);
void holdersCheck(int argc, char *argv[])
{
    if (argc != 9)
    {
        std::cout << "Need 6 parameters: number of entities (start, delta and max), repeats, operation size"
                  << " and component/holder type: sm, mm, lm, sml, mml, lml, sl, ml, ll; random"  << std::endl;
        std::exit(1);
    }
}

// 6 parameters number of entities, number of repeats, threads,
// and percentage in use - start, delta and max
void movementSystemP(int argc, char *argv[]);
void movementSystemPCheck(int argc, char *argv[])
{
    if (argc != 8)
    {
        std::cout << "Need 6 parameters: number of entities, number of repeats, threads, "
                  << "starting number, delta and max usage %" << std::endl;
        std::exit(1);
    }
}

// 6 parameters number of entities, number of repeats, threads,
// and percentage to change - start, delta and max
void advancedMovementSystemP(int argc, char *argv[]);
void advancedMovementSystemPCheck(int argc, char *argv[])
{
    if (argc != 8)
    {
        std::cout << "Need 6 parameters: number of entities, number of repeats, threads, "
                  << "starting number, delta and max change %" << std::endl;
        std::exit(1);
    }
}

// Mapping for the functions.
static constexpr CompFun functions[] = {
    createEntities,
    movementSystem,
    advancedMovementSystem,
    parallelEntity,
    parallelChangeset,
    holders,
    movementSystemP,
    advancedMovementSystemP
};
static constexpr ParamCheckFun checkFunctions[] = {
    createEntitiesCheck,
    movementSystemCheck,
    advancedMovementSystemCheck,
    parallelEntityCheck,
    parallelChangesetCheck,
    holdersCheck,
    movementSystemPCheck,
    advancedMovementSystemPCheck
};
static constexpr std::size_t numFunctions()
{ return sizeof(functions) / sizeof(CompFun); }
static_assert(numFunctions() == (sizeof(checkFunctions) / sizeof(ParamCheckFun)));
// End of prototypes.

void printUsage()
{
    std::cout << "Usage: \n"
              << "  executable CompNum [Other Parameters]"
              << std::endl;
}

void runComparison(int argc, char *argv[])
{
    if (argc < 2)
    {
        printUsage();
        return;
    }

    size_t testNum{static_cast<size_t>(std::atol(argv[1]))};

    if (testNum >= numFunctions())
    {
        std::cout << "Comparison function index is out of range!" << std::endl;
        printUsage();
        return;
    }

    checkFunctions[testNum](argc, argv);
    functions[testNum](argc, argv);
}

/// Simple timer.
class Timer
{
private:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
public:
    Timer() :
        mStart{getTime()}
    { }

    TimePoint getTime()
    {
        return std::chrono::high_resolution_clock::now();
    }

    std::size_t nanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
        (getTime() - mStart).count();
    }

private:
    /// Starting time.
    TimePoint mStart;
protected:
};

#endif //COMPSYS_H
