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
    MovementSystem(artemis::World *world);
    virtual void initialize();
    virtual void processEntity(artemis::Entity &e);
private:
    artemis::ComponentMapper<PositionC> mPosMapper;
    artemis::ComponentMapper<MovementC> mMovMapper;
protected:
};

class PositionSystem : public artemis::EntityProcessingSystem
{
public:
    PositionSystem(artemis::World *world, std::mt19937_64 &rng,
                   std::uniform_real_distribution<f64> &uniform, float percentage);
    virtual void initialize();
    virtual void processEntity(artemis::Entity &e);

    std::size_t counter1{0u};
    std::size_t counter2{0u};
private:
    artemis::ComponentMapper<PositionC> mPosMapper;
    artemis::ComponentMapper<MovementC> mMovMapper;

    const float mPercentage;
    std::mt19937_64 &mRng;
    std::uniform_real_distribution<f64> &mUniform;
protected:
};

#endif //COMP_H
