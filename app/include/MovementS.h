/**
 * @file MovementS.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_MOVEMENTS_H
#define ECS_FIT_MOVEMENTS_H

#include "Types.h"

class MovementS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC, VelocityC>;

    /**
     * Move Entities.
     * @param deltaT Time passed in seconds.
     */
    void doMove(f32 deltaT);
private:
protected:
}; // class MovementS

#include "MovementS.inl"

#endif //ECS_FIT_MOVEMENTS_H
