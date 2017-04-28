/**
 * @file CompSys.h
 * @author Tomas Polasek
 * @brief Comparison system functions.
 */

#ifndef COMPSYS_H
#define COMPSYS_H

#include <random>

using CompFun = void(*)(int argc, char *argv[]);

/// Number used as a random seed
static constexpr std::size_t RANDOM_SEED{4007};

/*
 * Prototypes of called comparison functions.
 */

// 3 parameters for number of entities created - start, delta and max
void createEntities(int argc, char *argv[]);

// 5 parameters number of entities, number of repeats
// and percentage in use - start, delta and max
void movementSystem(int argc, char *argv[]);

// 5 parameters number of entities, number of repeats
// and percentage to change - start, delta and max
void advancedMovementSystem(int argc, char *argv[]);

// Mapping for the functions.
static constexpr CompFun functions[] = {
    createEntities,
    movementSystem,
    advancedMovementSystem
};
static constexpr std::size_t numFunctions()
{ return sizeof(functions) / sizeof(CompFun); }
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
