/**
 * @file RenderS.inl
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_RENDERS_INL
#define ECS_FIT_RENDERS_INL

#include "RenderS.h"

void RenderS::render(Camera &c, Cube &cube, GLSLProgram &p)
{
    const GLsizei cubes{static_cast<GLsizei>(foreach().size())};
    const glm::mat4 &vp{c.viewProjectionMatrix()};

    GLint vpHandle{p.getUniformLocation("vp")};
    GLint modelHandle{p.getAttribLocation("model")};

    GLuint modelMatrixBuffer;
    glGenBuffers(1, &modelMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * cubes, nullptr, GL_STATIC_DRAW);
    glm::mat4 *modelMatrices{static_cast<glm::mat4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY))};

    PROF_BLOCK("Transform copy");
    u64 index{0};
    for (auto &e : foreach())
    {
        modelMatrices[index++] = e.get<TransformC>()->modelMatrix;
    }
    PROF_BLOCK_END();

    glUnmapBuffer(GL_ARRAY_BUFFER);

    cube.bind();

    glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
    for (u16 idx = 0; idx < 4; ++idx)
    { // Mat4 takes 4 locations.
        glEnableVertexAttribArray(modelHandle + idx);
        glVertexAttribPointer(
            modelHandle + idx, // Mat4 takes up 4 positions.
            4,                 // 4 floats in vec4.
            GL_FLOAT, GL_FALSE,
            sizeof(glm::mat4),
            reinterpret_cast<void*>(sizeof(glm::vec4) * idx)
        );
        // Make the matrix instanced.
        glVertexAttribDivisor(modelHandle + idx, 1);
    }

    glUniformMatrix4fv(vpHandle, 1, GL_FALSE, &vp[0][0]);

    PROF_BLOCK("Render");
    cube.renderInstanced(cubes);
    PROF_BLOCK_END();

    cube.unbind();

    glDeleteBuffers(1, &modelMatrixBuffer);
}

#endif //ECS_FIT_RENDERS_INL
