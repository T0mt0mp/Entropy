/**
 * @file app/App.h
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#ifndef SIMPLE_GAME_APP_H
#define SIMPLE_GAME_APP_H

#include "Types.h"

#include "Timer.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "Gamepad.h"

#include "GLSLProgram.h"
#include "Camera.h"

#include "Entropy/Entropy.h"
#include "testing/Testing.h"

/// Renderable triangle.
class Triangle
{
public:
    /// Construct the Triangle and all the buffers.
    Triangle() :
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

    /// Render the triangle
    void render()
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

    /// Destroy the buffers.
    ~Triangle()
    { destroy(); }
private:
    /// Destroy all the GL buffers.
    void destroy()
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

    /// Vertex data for triangle.
    static const GLfloat VERTEX_BUFFER_DATA[3 * 3];
    /*{
        // x      y     z
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };*/

    /// The vertex array ID.
    GLuint mVAId;
    /// The vertex buffer ID.
    GLuint mVBId;
protected:
}; // class Triangle

/// Renderable cube.
class Cube
{
public:
    /// Construct the Cube and all the buffers.
    Cube() :
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

    void bind()
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

    void renderInstanced(GLsizei count)
    {
        // Draw the cube.
        glDrawArraysInstanced(GL_TRIANGLES, 0, VERTEXES, count);
    }

    void render()
    {
        // Draw the cube.
        glDrawArrays(GL_TRIANGLES, 0, VERTEXES);
    }

    void unbind()
    {
        // Disable the vertex array.
        glBindVertexArray(0);
    }

    /// Destroy the buffers.
    ~Cube()
    { destroy(); }
private:
    /// Destroy all the GL buffers.
    void destroy()
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

/// Position component.
struct PositionC
{
    using HolderT = ent::ComponentHolderList<PositionC>;

    glm::vec3 p;
    glm::vec3 r;
}; // struct PositionC

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

class MovementS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC, VelocityC>;

    /**
     * Move Entities.
     * @param deltaT Time passed in seconds.
     */
    void run(f32 deltaT)
    {
        for (auto &e : foreach())
        {
            e.get<PositionC>()->p += deltaT * e.get<VelocityC>()->v;
            if (e.get<PositionC>()->p.y > 2.0f || e.get<PositionC>()->p.y < -2.0f)
            {
                e.get<VelocityC>()->v *= -1.0;
            }
        }
    }
private:
protected:
};

class RotationS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC, RotSpeedC>;

    /**
     * Rotate Entities.
     * @param deltaT Time passed in seconds.
     */
    void run(f32 deltaT)
    {
        for (auto &e : foreach())
        {
            e.get<PositionC>()->r += deltaT * e.get<RotSpeedC>()->rs;
        }
    }
private:
protected:
};

class RenderS : public Universe::SystemT
{
public:
    using Require = ent::Require<PositionC>;

    /**
     * Render each Entity with Position component as triangle.
     * @param c Current camera.
     * @param cube Cube used in rendering.
     * @param p Shader program used.
     */
    void render(Camera &c, Cube &cube, GLSLProgram &p)
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

        PROF_BLOCK("Matrix loop");

        if (foreach().size() > PARALLEL_THRESHOLD)
        {
            u64 index{0};
            for (auto &e : foreach())
            {
                modelMatrices[index++] = glm::translate(glm::mat4(1.0f), e.get<PositionC>()->p) *
                                         glm::mat4_cast(glm::quat(e.get<PositionC>()->r));
            }
        }
        else
        {
            std::vector<std::thread> threads;

            u64 numThreads{std::thread::hardware_concurrency()};
            if (numThreads == 0)
            { // Unable to detect number of threads...
                numThreads = 2;
            }

            u64 perThread{foreach().size() / numThreads};
            auto beginIterator{foreach().begin()};
            auto endIterator{foreach().end()};

            for (u64 iii = 0; iii < numThreads; ++iii)
            {
                threads.emplace_back([iii, perThread, numThreads, &beginIterator, &endIterator, &modelMatrices] () {
                    u64 index{iii * perThread};
                    u64 endIndex{(iii + 1) * perThread};
                    auto iterator{beginIterator + index};
                    auto end{(iii == (numThreads - 1)) ? (beginIterator + endIndex) : endIterator};

                    for (; iterator != end; ++iterator)
                    {
                        modelMatrices[index] = glm::translate(glm::mat4(1.0f), iterator->get<PositionC>()->p) *
                                               glm::mat4_cast(glm::quat(iterator->get<PositionC>()->r));
                        index++;
                    }
                });
            }

            for (auto &th : threads)
            {
                th.join();
            }
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
private:
    /// Minimal number of Entities to use parallelism for.
    static constexpr u64 PARALLEL_THRESHOLD{5000};
protected:
};

/// Main application class.
class App
{
public:
    /// Constructor.
    App();

    /// Destructor.
    ~App();

    /// Main application loop.
    void run();

    /**
     * GLFW callback, called when error occurs.
     * @param error Error code.
     * @param desc Error description.
     */
    static void glfwErrorCallback(int error, const char *desc);
private:
    /// Milliseconds in one second.
    static constexpr f64 MS_IN_S{1000.0};
    /// Microseconds in one millisecond.
    static constexpr f64 US_IN_MS{1000.0};

    /// Milliseconds until next information print.
    static constexpr u64 MS_PER_INFO{1000u};

    /// Target number of updates per second.
    static constexpr u64 TARGET_UPS{60u};
    /// Once in how many milliseconds should an update occur.
    static constexpr f64 MS_PER_UPDATE{MS_IN_S / TARGET_UPS};

    /// Target number of frames rendered per second.
    static constexpr u64 TARGET_FPS{60u};
    /// Once in how many milliseconds should a frame be rendered.
    static constexpr f64 MS_PER_FRAME{MS_IN_S / TARGET_FPS};

    /// Reload shaders and recompile shader program.
    void reloadShaders();

    // GLFW window.
    /// GLFW window instance.
    GLFWwindow *mWindow{nullptr};
    /// Window width.
    u64 mWindowWidth{1024u};
    /// Window height.
    u64 mWindowHeight{768u};
    /// Window title.
    std::string mWindowTitle{"Hello GLFW"};

    // Application variable.
    /// Are we running?
    bool mRunning;
    /// Shader program used for rendering.
    GLSLProgram mProgram;
protected:
}; // class App

#endif //SIMPLE_GAME_APP_H
