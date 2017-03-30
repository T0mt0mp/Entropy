/**
 * @file tests/Tests.h
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#ifndef Tests_H
#define Tests_H

#include <iostream>

#include "testing/Testing.h"

#define NO_ENT_WARNING
#define NDEBUG
#include <Entropy/Entropy.h>

class Universe : public ent::Universe<Universe>
{
public:
private:
protected:
};

#endif //Tests_H
