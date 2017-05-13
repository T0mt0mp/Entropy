/**
 * @file Cube.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Cube.h"

const GLfloat Triangle::VERTEX_BUFFER_DATA[] =
    {
        // x      y     z
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f
    };

const GLfloat Cube::VERTEX_BUFFER_DATA[] =
    {
        // x      y     z
        // Left
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        // Front
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        // Right
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,

        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,

        // Back
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,

        // Up
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,

        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        // Down
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,

        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
    };

#define COLOR1 0.24f, 0.19f, 0.46f
#define COLOR2 0.18f, 0.26f, 0.45f
#define COLOR3 0.37f, 0.32f, 0.22f
#define COLOR4 0.21f, 0.28f, 0.22f

#define COLOR5 0.15f, 0.09f, 0.35f
#define COLOR6 0.09f, 0.16f, 0.33f
#define COLOR7 0.07f, 0.22f, 0.18f
#define COLOR8 0.10f, 0.36f, 0.28f

const GLfloat Cube::COLOR_BUFFER_DATA[] =
    {
        // Left
        COLOR1,
        COLOR2,
        COLOR3,

        COLOR1,
        COLOR3,
        COLOR4,

        // Front
        COLOR2,
        COLOR5,
        COLOR6,

        COLOR2,
        COLOR6,
        COLOR3,

        // Right
        COLOR5,
        COLOR7,
        COLOR8,

        COLOR5,
        COLOR8,
        COLOR6,

        // Back
        COLOR7,
        COLOR1,
        COLOR4,

        COLOR7,
        COLOR4,
        COLOR8,

        // Up
        COLOR3,
        COLOR6,
        COLOR8,

        COLOR3,
        COLOR8,
        COLOR4,

        // Down
        COLOR5,
        COLOR2,
        COLOR1,

        COLOR5,
        COLOR1,
        COLOR7,
    };

Cube::Cube() :
    mVAId{0u}
{
    // Create vertex array.
    glGenVertexArrays(1, &mVAId);
    if (!mVAId)
    {
        throw std::runtime_error("Unable to glGenVertexArrays!");
    }
    glBindVertexArray(mVAId);

    // Create vertex buffer object.
    glGenBuffers(1, &mVBId);
    if (!mVBId)
    {
        destroy();
        throw std::runtime_error("Unable to glGenBuffers!");
    }
    glBindBuffer(GL_ARRAY_BUFFER, mVBId);

    // Set the data.
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_BUFFER_DATA), VERTEX_BUFFER_DATA, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create vertex buffer object.
    glGenBuffers(1, &mCBId);
    if (!mCBId)
    {
        destroy();
        throw std::runtime_error("Unable to glGenBuffers!");
    }
    glBindBuffer(GL_ARRAY_BUFFER, mCBId);

    // Set the data.
    glBufferData(GL_ARRAY_BUFFER, sizeof(COLOR_BUFFER_DATA), COLOR_BUFFER_DATA, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Cube::bind()
{
    // Bind the vertex array.
    glBindVertexArray(mVAId);

    // Location 0.
    glEnableVertexAttribArray(0);
    // Bind the data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBId);
    // Set attributes.
    glVertexAttribPointer(
        0,             // Layout location.
        3,             // 3 vertexes for a triangle.
        GL_FLOAT,      // Type of the vertex data.
        GL_FALSE,      // Not normalized.
        0,             // Stride, data are behind each other.
        nullptr        // No offset.
    );

    // Location 1.
    glEnableVertexAttribArray(1);
    // Bind the data.
    glBindBuffer(GL_ARRAY_BUFFER, mCBId);
    // Set attributes.
    glVertexAttribPointer(
        1,             // Layout location.
        3,             // 3 vertexes for a triangle.
        GL_FLOAT,      // Type of the vertex data.
        GL_FALSE,      // Not normalized.
        0,             // Stride, data are behind each other.
        nullptr        // No offset.
    );
}

void Cube::renderInstanced(GLsizei count)
{
    // Draw the cube.
    glDrawArraysInstanced(GL_TRIANGLES, 0, VERTEXES, count);
}

void Cube::render()
{
    // Draw the cube.
    glDrawArrays(GL_TRIANGLES, 0, VERTEXES);
}

void Cube::unbind()
{
    // Disable the vertex array.
    glBindVertexArray(0);
}

Cube::~Cube()
{ destroy(); }

void Cube::destroy()
{
    if (mVAId)
    {
        glDeleteVertexArrays(1, &mVAId);
        mVAId = 0;
    }

    if (mVBId)
    {
        glDeleteBuffers(1, &mVBId);
        mVBId = 0;
    }

    if (mCBId)
    {
        glDeleteBuffers(1, &mCBId);
        mCBId = 0;
    }
}

Triangle::Triangle() :
    mVAId{0u}
{
    // Create vertex array.
    glGenVertexArrays(1, &mVAId);
    if (!mVAId)
    {
        throw std::runtime_error("Unable to glGenVertexArrays!");
    }
    glBindVertexArray(mVAId);

    // Create vertex buffer object.
    glGenBuffers(1, &mVBId);
    if (!mVBId)
    {
        destroy();
        throw std::runtime_error("Unable to glGenBuffers!");
    }
    glBindBuffer(GL_ARRAY_BUFFER, mVBId);

    // Set the data.
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_BUFFER_DATA), VERTEX_BUFFER_DATA, GL_STATIC_DRAW);

    // Unbind the buffer and array.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Triangle::render()
{
    // Bind the vertex array.
    glBindVertexArray(mVAId);
    // Location 0.
    glEnableVertexAttribArray(0);
    // Bind the data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBId);
    // Set attributes.
    glVertexAttribPointer(
        0,             // Layout location.
        3,             // 3 vertexes for a triangle.
        GL_FLOAT,      // Type of the vertex data.
        GL_FALSE,      // Not normalized.
        0,             // Stride, data are behind each other.
        nullptr        // No offset.
    );

    // Draw the triangle, start at vertex 0 and draw 3.
    glDrawArrays(GL_LINES, 0, 3);

    // Disable the attribute.
    glDisableVertexAttribArray(0);
    // Disable the vertex array.
    glBindVertexArray(mVAId);
}

Triangle::~Triangle()
{ destroy(); }

void Triangle::destroy()
{
    if (mVAId)
    {
        glDeleteVertexArrays(1, &mVAId);
        mVAId = 0;
    }

    if (mVBId)
    {
        glDeleteBuffers(1, &mVBId);
        mVBId = 0;
    }
}
