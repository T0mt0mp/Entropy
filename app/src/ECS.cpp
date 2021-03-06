/**
 * @file app/ECS.cpp
 * @author Tomas Polasek
 * @brief Configuration of the Entropy ECS.
 */

#include "ECS.h"

template class ent::Universe<Universe>;

void Universe::initialize()
{
    registerComponent<PositionC>();
    registerComponent<VelocityC>();
    registerComponent<RotationVelocityC>();
    registerComponent<TransformC>();

    init();
}
