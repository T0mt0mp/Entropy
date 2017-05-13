/**
 * @file RotationS.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_ROTATIONS_H
#define ECS_FIT_ROTATIONS_H

#include "Types.h"

class RotationS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC, RotationVelocityC>;

    /**
     * Rotate Entities.
     * @param deltaT Time passed in seconds.
     */
    void doRot(f32 deltaT);
private:
protected:
}; // class RotationS

#include "RotationS.inl"

#endif //ECS_FIT_ROTATIONS_H
