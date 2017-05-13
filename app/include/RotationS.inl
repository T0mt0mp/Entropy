/**
 * @file RotationS.inl
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_ROTATIONS_INL
#define ECS_FIT_ROTATIONS_INL

#include "RotationS.h"

void RotationS::doRot(f32 deltaT)
{
    for (auto &e : foreach())
    {
        e.get<PositionC>()->rot += deltaT * e.get<RotationVelocityC>()->rv;
    }
}

#endif //ECS_FIT_ROTATIONS_INL
