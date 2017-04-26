/**
 * @file tests/TestUniverse.h
 * @author Tomas Polasek
 * @brief Testing Entropy Universe.
 */

#ifndef ECS_FIT_TESTUNIVERSE_H
#define ECS_FIT_TESTUNIVERSE_H

#define ENT_STATS_ASSERT
#include "Entropy/Entropy.h"

class FirstUniverse : public ent::Universe<FirstUniverse>
{
};

class SecondUniverse : public ent::Universe<SecondUniverse>
{
};

class RealUniverse1 : public ent::Universe<RealUniverse1>
{
};

class RealUniverse2 : public ent::Universe<RealUniverse2>
{
};

class RealUniverse3 : public ent::Universe<RealUniverse3>
{
};

#endif //ECS_FIT_TESTUNIVERSE_H
