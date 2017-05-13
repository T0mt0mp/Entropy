/**
 * @file RenderS.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_RENDERS_H
#define ECS_FIT_RENDERS_H

#include "Types.h"
#include "GLSLProgram.h"
#include "Cube.h"
#include "Camera.h"

class RenderS : public Universe::SystemT
{
public:
    using Require = ent::Require<TransformC>;

    /**
     * Render each Entity with Position component as triangle.
     * @param c Current camera.
     * @param cube Cube used in rendering.
     * @param p Shader program used.
     */
    void render(Camera &c, Cube &cube, GLSLProgram &p);
private:
protected:
}; // class RenderS


#include "RenderS.inl"

#endif //ECS_FIT_RENDERS_H
