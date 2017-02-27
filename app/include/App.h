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
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
    static const GLfloat VERTEX_BUFFER_DATA[3 * 3 * sizeof(GLfloat)];
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
