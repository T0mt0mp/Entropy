/**
 * @file Comp.h
 * @author Tomas Polasek
 * @brief Main comparison file.
 */

#ifndef COMP_H
#define COMP_H

#include <iostream>

#include "testing/Testing.h"
#include "CompSys.h"

#define NO_ENT_WARNING
#define NDEBUG
#include <Entropy/Entropy.h>

// Sizes in bytes.
static constexpr std::size_t SMALL_COMP_SIZE{16u};
static constexpr std::size_t MEDIUM_COMP_SIZE{64u};
static constexpr std::size_t LARGE_COMP_SIZE{128u};

struct SmallCM
{
    using HolderT = ent::ComponentHolder<SmallCM>;
    char data[SMALL_COMP_SIZE];
};

struct MediumCM
{
    using HolderT = ent::ComponentHolder<MediumCM>;
    char data[MEDIUM_COMP_SIZE];
};

struct LargeCM
{
    using HolderT = ent::ComponentHolder<LargeCM>;
    char data[LARGE_COMP_SIZE];
};

struct SmallCML
{
    using HolderT = ent::ComponentHolderMapList<SmallCML>;
    char data[SMALL_COMP_SIZE];
};

struct MediumCML
{
    using HolderT = ent::ComponentHolderMapList<MediumCML>;
    char data[MEDIUM_COMP_SIZE];
};

struct LargeCML
{
    using HolderT = ent::ComponentHolderMapList<LargeCML>;
    char data[LARGE_COMP_SIZE];
};

struct SmallCL
{
    using HolderT = ent::ComponentHolderList<SmallCL>;
    char data[SMALL_COMP_SIZE];
};

struct MediumCL
{
    using HolderT = ent::ComponentHolderList<MediumCL>;
    char data[MEDIUM_COMP_SIZE];
};

struct LargeCL
{
    using HolderT = ent::ComponentHolderList<LargeCL>;
    char data[LARGE_COMP_SIZE];
};

/*
struct PositionCMain
{
    using HolderT = ent::ComponentHolderList<PositionCMain>;

    PositionCMain() = default;
    PositionCMain(float posX, float posY) :
        x{posX}, y{posY}
    { }

    float x;
    float y;
};

struct MovementCMain
{
    using HolderT = ent::ComponentHolderList<MovementCMain>;

    MovementCMain() = default;
    MovementCMain(float movX, float movY) :
        dX{movX}, dY{movY}
    { }

    float dX;
    float dY;
};
 */

struct PositionC
{
    using HolderT = ent::ComponentHolderList<PositionC>;

    PositionC() = default;
    PositionC(float posX, float posY) :
        x{posX}, y{posY}
    { }

    float x;
    float y;
};

struct MovementC
{
    using HolderT = ent::ComponentHolderList<MovementC>;

    MovementC() = default;
    MovementC(float movX, float movY) :
        dX{movX}, dY{movY}
    { }

    float dX;
    float dY;
};

class Universe : public ent::Universe<Universe>
{
public:
private:
protected:
};

struct PositionSystem : public Universe::SystemT
{
    using Require = ent::Require<PositionC>;
};

struct MovementSystem : public Universe::SystemT
{
    using Require = ent::Require<PositionC, MovementC>;
};

#endif //COMP_H
