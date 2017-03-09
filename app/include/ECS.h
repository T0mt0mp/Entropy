/**
 * @file app/ECS.h
 * @author Tomas Polasek
 * @brief Configuration of the Entropy ECS.
 */

#ifndef SIMPLE_GAME_ECS_H
#define SIMPLE_GAME_ECS_H

#include <glm/glm.hpp>

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

    glm::vec3 p;
    glm::vec3 r;
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
    glm::vec3 v;
}; // struct VelocityC

/// Rotation momentum component.
struct RotSpeedC
{
    using HolderT = ent::ComponentHolderList<RotSpeedC>;

    glm::vec3 rs;
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
