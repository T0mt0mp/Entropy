/**
 * @file Cube.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_CUBE_H
#define ECS_FIT_CUBE_H

#include "Types.h"

/// Renderable cube.
class Cube
{
public:
    /// Construct the Cube and all the buffers.
    Cube();

    /// Bind the required buffer for rendering.
    void bind();

    /// Instanced rendering.
    void renderInstanced(GLsizei count);

    /// Non-instanced rendering.
    void render();

    /// Unbind the buffers.
    void unbind();

    /// Destroy the buffers.
    ~Cube();
private:
    /// Destroy all the GL buffers.
    void destroy();

    static constexpr u64 TRIANGLES{2 * 6};
    static constexpr u64 VERTEXES{TRIANGLES * 3};

    /// Vertex data for Cube.
    static const GLfloat VERTEX_BUFFER_DATA[VERTEXES * 3];
    /// Vertex color data.
    static const GLfloat COLOR_BUFFER_DATA[VERTEXES * 3];

    /// The vertex array ID.
    GLuint mVAId;
    /// The vertex buffer ID.
    GLuint mVBId;
    /// Color buffer ID.
    GLuint mCBId;
protected:
}; // class Cube

/// Renderable triangle.
class Triangle
{
public:
    /// Construct the Triangle and all the buffers.
    Triangle();

    /// Render the triangle
    void render();

    /// Destroy the buffers.
    ~Triangle();
private:
    /// Destroy all the GL buffers.
    void destroy();

    /// Vertex data for triangle.
    static const GLfloat VERTEX_BUFFER_DATA[3 * 3];

    /// The vertex array ID.
    GLuint mVAId;
    /// The vertex buffer ID.
    GLuint mVBId;
protected:
}; // class Triangle

#endif //ECS_FIT_CUBE_H
