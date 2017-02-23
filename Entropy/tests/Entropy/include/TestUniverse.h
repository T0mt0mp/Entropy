/**
 * @file tests/TestUniverse.h
 * @author Tomas Polasek
 * @brief Testing Entropy Universe.
 */

#ifndef ECS_FIT_TESTUNIVERSE_H
#define ECS_FIT_TESTUNIVERSE_H

#include "Entropy/Entropy.h"

class FirstUniverse : public ent::Universe<FirstUniverse>
{
public:
    static UniverseT &instance()
    {
        static UniverseT u;
        return u;
    }
};

class SecondUniverse : public ent::Universe<SecondUniverse>
{
public:
    static UniverseT &instance()
    {
        static UniverseT u;
        return u;
    }
};

class RealUniverse1 : public ent::Universe<RealUniverse1>
{
public:
    static UniverseT &instance()
    {
        static UniverseT u;
        return u;
    }
};

class RealUniverse2 : public ent::Universe<RealUniverse2>
{
public:
    static UniverseT &instance()
    {
        static UniverseT u;
        return u;
    }
};

#endif //ECS_FIT_TESTUNIVERSE_H
