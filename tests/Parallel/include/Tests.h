/**
 * @file tests/Tests.h
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#ifndef Tests_H
#define Tests_H

#include <iostream>

#include "testing/Testing.h"

#define ENT_STATS_ASSERT
#define ENT_ENTITY_VALID
#define ENT_COMP_EXCEPT
#include "Entropy/Entropy.h"

class ParUniverse0 : public ent::Universe<ParUniverse0>
{ };

struct Position
{
    using HolderT = ent::ComponentHolderList<Position>;

    Position() :
        x{0.0f}, y{0.0f}
    { }

    Position(f32 posX, f32 posY) :
        x{posX}, y{posY}
    { }

    f32 x, y;
};

struct Velocity
{
    using HolderT = ent::ComponentHolderList<Velocity>;

    Velocity() :
        x{0.0f}, y{0.0f}
    { }

    Velocity(f32 velX, f32 velY) :
        x{velX}, y{velY}
    { }

    f32 x, y;
};

struct PositionSystem : public ParUniverse0::SystemT
{
    using Require = ent::Require<Position>;
    using Reject = ent::Reject<Velocity>;
};

struct VelocitySystem : public ParUniverse0::SystemT
{
    using Require = ent::Require<Velocity>;
    using Reject = ent::Reject<Position>;
};

struct PosVelSystem : public ParUniverse0::SystemT
{
    using Require = ent::Require<Position, Velocity>;
};

#endif //Tests_H
