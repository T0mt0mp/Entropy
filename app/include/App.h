/**
 * @file app/App.h
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#ifndef SIMPLE_GAME_APP_H
#define SIMPLE_GAME_APP_H

#include <iostream>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <utility>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Entropy/Entropy.h"
#include "testing/Testing.h"

class Timer
{
private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
public:
    using seconds = std::chrono::seconds;
    using milliseconds = std::chrono::milliseconds;
    using microseconds = std::chrono::microseconds;
    using nanoseconds = std::chrono::nanoseconds;
    /// Initialize the timer and start it.
    Timer()
    { reset(); }

    /// Get the current time.
    time_point now() const
    { return clock::now(); }

    /// Start the timer.
    void reset()
    { mStart = now(); }

    /**
     * Get the elapsed time and reset the timer.
     * @tparam UnitT Unit, in which the time will be returned (e.g. Timer::seconds).
     * @return Elapsed time from the last reset.
     */
    template <typename UnitT>
    u64 elapsedReset()
    {
        u64 elapsedTime{elapsed<UnitT>()};
        reset();
        return elapsedTime;
    }

    /**
     * Get how many time elapsed from the start time.
     * @tparam UnitT Unit, in which the time will be returned (eg. Timer::seconds).
     * @return Returns elapsed time in requested units.
     */
    template <typename UnitT>
    u64 elapsed() const
    { return std::chrono::duration_cast<UnitT>(now() - mStart).count(); }
private:
    /// Time, when the timer started.
    time_point mStart;
protected:
};

/// Information about a shader to be loaded.
class GLSLShaderInfo
{
public:
    /**
     * Get shader source from given file.
     * @param shaderType Type of the shader (e.g. GL_VERTEX_SHADER)
     * @param filename File, where the shader source is located.
     */
    GLSLShaderInfo(GLenum shaderType, const std::string &filename) :
        mShaderType{shaderType}
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Error, unable to open shader file \"") + filename + "\"");
        }

        // Reserve the string size upfront.
        file.seekg(0, std::ios::end);
        mSource.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        // Read the whole file into the string.
        mSource.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    /**
     * Get shader source from supplied string.
     * @param shaderType Type of the shader (e.g. GL_VERTEX_SHADER).
     * @param src Source of the shader.
     */
    GLSLShaderInfo(GLenum shaderType, const char *src) :
        mShaderType{shaderType}, mSource{src} { }

    /**
     * Shader type getter.
     * @return Returns shader type (e.g. GL_VERTEX_SHADER).
     */
    GLenum type() const
    { return mShaderType; }

    /**
     * Shader source code getter.
     * @return Returns shader code.
     */
    const std::string &source() const
    { return mSource; }
private:
    /// Type of the shader (e.g. GL_VERTEX_SHADER).
    GLenum mShaderType;
    /// Source for the shader.
    std::string mSource;
protected:
};

/// OpenGL shader compilation.
class GLSLShader
{
public:
    /**
     * Create a shader from given information.
     * @param info Information about the requested shader.
     */
    GLSLShader(const GLSLShaderInfo &info) :
        mShaderId{glCreateShader(info.type())}
    {
        GLint result{GL_FALSE};
        GLint logLength{0};
        const char *src{info.source().c_str()};

        if (!mShaderId)
        {
            throw std::runtime_error("Unable to glCreateShader!");
        }

        // So far only 1 source file for each shader is supported.
        glShaderSource(mShaderId, 1, &src, nullptr);
        glCompileShader(mShaderId);

        // Get information about the compilation.
        glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            deleteShader();
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mShaderId, logLength, nullptr, &errorMessage[0]);
            throw std::runtime_error(std::string(&errorMessage[0]));
        }
    }

    /// Copying is not allowed.
    GLSLShader(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader(GLSLShader &&rhs) :
        mShaderId{0}
    { std::swap(mShaderId, rhs.mShaderId); }

    /// Copying is not allowed.
    GLSLShader &operator=(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader &operator=(GLSLShader rhs)
    { std::swap(mShaderId, rhs.mShaderId); return *this; }

    /// Shader ID getter.
    GLuint id() const
    { return mShaderId; }

    /// Destructor DOES delete the shader object!
    ~GLSLShader()
    { deleteShader(); }
private:
    /// Delete the shader inside.
    void deleteShader()
    {
        if (mShaderId)
        {
            glDeleteShader(mShaderId);
            mShaderId = 0;
        }
    }

    /// ID of the shader.
    GLuint mShaderId;
protected:
};

/// OpenGL shader program compilation.
class GLSLProgram
{
public:
    /**
     * Create GLSL program, from given list specifying the shaders.
     * @param list List specifying the shaders (e.g. {GL_VERTEX_SHADER, "shader.vert"}).
     */
    GLSLProgram(std::initializer_list<GLSLShaderInfo> list) :
        mProgramId{glCreateProgram()}
    {
        GLint result{GL_FALSE};
        GLint logLength{0};
        std::vector<GLSLShader> shaders;

        // Prepare the shaders.
        try
        {
            for (auto &info : list)
            {
                GLSLShader shader(info);
                glAttachShader(mProgramId, shader.id());
                shaders.emplace_back(std::move(shader));
            }
        } catch (...)
        {
            glDeleteProgram(mProgramId);
            throw;
        }

        // Link the program.
        glLinkProgram(mProgramId);

        // Get information about program compilation.
        glGetShaderiv(mProgramId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(mProgramId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            glDeleteProgram(mProgramId);
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mProgramId, logLength, nullptr, &errorMessage[0]);
            throw std::runtime_error(std::string(&errorMessage[0]));
        }

        // Detach the shaders.
        for (auto &shader : shaders)
        {
            glDetachShader(mProgramId, shader.id());
        }

        // Auto-delete shaders.
    }

    /// Destructor DOES delete the program!
    ~GLSLProgram()
    { deleteProgram(); }

    /// Use this program.
    void use()
    { glUseProgram(mProgramId); }
private:
    /// Delete the program.
    void deleteProgram()
    {
        if (mProgramId)
        {
            glDeleteProgram(mProgramId);
            mProgramId = 0;
        }
    }

    /// ID of the shader program.
    GLuint mProgramId;
protected:
};

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
};

class App
{
public:
    App();
    ~App();

    void run();

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
protected:
};

#endif //SIMPLE_GAME_APP_H
