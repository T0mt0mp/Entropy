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
};

class SecondUniverse : public ent::Universe<SecondUniverse>
{
};

#endif //ECS_FIT_TESTUNIVERSE_H
