/**
 * @file TransformS.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_TRANSFORMS_H
#define ECS_FIT_TRANSFORMS_H

#include "Types.h"

class TransformS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC, TransformC>;

    /**
     * Calculate the model matrix for each entity.
     */
    void doTransform();
private:
protected:
}; // class TransformS

#include "TransformS.inl"

#endif //ECS_FIT_TRANSFORMS_H
