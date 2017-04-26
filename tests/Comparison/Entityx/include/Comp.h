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

#include "entityx/entityx.h"

struct PositionC
{
    PositionC(float posX, float posY) :
        x{posX}, y{posY}
    { }

    float x;
    float y;
};

struct MovementC
{
    MovementC(float movX, float movY) :
        dX{movX}, dY{movY}
    { }

    float dX;
    float dY;
};

#endif //COMP_H
