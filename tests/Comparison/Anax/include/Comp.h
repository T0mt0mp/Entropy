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

#include "anax/anax.hpp"

struct PositionC : anax::Component
{
    PositionC(float posX, float posY) :
        x{posX}, y{posY}
    { }

    float x;
    float y;
};

struct MovementC : anax::Component
{
    MovementC(float movX, float movY) :
        dX{movX}, dY{movY}
    { }

    float dX;
    float dY;
};

class MovementSystem : public anax::System<anax::Requires<PositionC, MovementC>>
{
public:
    const std::vector<anax::Entity>& foreach()
    { return getEntities(); }
private:
protected:
};

class PositionSystem : public anax::System<anax::Requires<PositionC>>
{
public:
    const std::vector<anax::Entity> &foreach()
    { return getEntities(); }
private:
protected:
};

#endif //COMP_H
