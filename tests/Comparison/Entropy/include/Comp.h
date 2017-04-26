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

struct MovementSystem : public Universe::SystemT
{
    using Require = ent::Require<PositionC, MovementC>;
};

#endif //COMP_H
