/**
 * @file app/ECS.h
 * @author Tomas Polasek
 * @brief Configuration of the Entropy ECS.
 */

#ifndef SIMPLE_GAME_ECS_H
#define SIMPLE_GAME_ECS_H

#include <glm/glm.hpp>

#include "testing/Testing.h"

#define NO_ENT_WARNING
#include <Entropy/Entropy.h>

/// Camera position.
struct CameraInfo
{
    glm::vec3 pos;
    glm::vec3 rot;
}; // struct CameraInfo

/// Position component.
struct PositionC
{
    using HolderT = ent::ComponentHolderList<PositionC>;

    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scale;
}; // struct PositionC

/// Transform component.
struct TransformC
{
    using HolderT = ent::ComponentHolderList<PositionC>;

    glm::mat4 modelMatrix;
}; // struct TransformC

/// Velocity component.
struct VelocityC
{
    glm::vec3 vel;
    glm::vec3 targetPos;
}; // struct VelocityC

/// Rotation momentum component.
struct RotationVelocityC
{
    using HolderT = ent::ComponentHolderList<RotationVelocityC>;

    glm::vec3 rv;
}; // struct RotSpeedC

class Universe : public ent::Universe<Universe>
{
public:
    void initialize();
private:
protected:
};

//extern template class ent::Universe<Universe>;

#endif //SIMPLE_GAME_ECS_H
