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

#include "Artemis/Artemis.h"

struct PositionC : public artemis::Component
{
    PositionC(float posX, float posY) :
        x{posX}, y{posY}
    { }

    float x;
    float y;
};

struct MovementC : public artemis::Component
{
    MovementC(float movX, float movY) :
        dX{movX}, dY{movY}
    { }

    float dX;
    float dY;
};

class MovementSystem : public artemis::EntityProcessingSystem
{
public:
    MovementSystem();
    virtual void initialize();
    virtual void processEntity(artemis::Entity &e);

    static void setWorld(artemis::World *world);

    static std::size_t sCounter;
private:
    artemis::ComponentMapper<PositionC> mPosMapper;
    artemis::ComponentMapper<MovementC> mMovMapper;

    static artemis::World *sWorld;
protected:
};

#endif //COMP_H
