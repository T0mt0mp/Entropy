/**
 * @file MovementS.inl
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_MOVEMENTS_INL
#define ECS_FIT_MOVEMENTS_INL

#include "MovementS.h"

void MovementS::doMove(f32 deltaT)
{
    for (auto &e : foreach())
    {
        VelocityC *vel{e.get<VelocityC>()};
        PositionC *pos{e.get<PositionC>()};

        glm::vec3 newPos = pos->pos + deltaT * vel->vel;

        float l1{glm::length(newPos - pos->pos)};
        float l2{glm::length(vel->targetPos - pos->pos)};

        if (l1 > l2)
        {
            pos->pos = vel->targetPos;
            e.remove<VelocityC>();
        }
        else
        {
            pos->pos = newPos;
        }

        //pos->pos += deltaT * vel->vel;
    }
}

#endif //ECS_FIT_MOVEMENTS_INL
