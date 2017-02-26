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
}; // class Timer

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
        mShaderType{shaderType},
        mFilename{filename}
    {
        std::ifstream file(mFilename);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Error, unable to open shader file \"") + mFilename + "\"");
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
        mShaderType{shaderType}, mFilename{"SourceShader"}, mSource{src} { }

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

    /**
     * Filename getter.
     * @return Returns filename, from which the shader has benn loaded.
     */
    const std::string &filename() const
    { return mFilename; }
private:
    /// Type of the shader (e.g. GL_VERTEX_SHADER).
    GLenum mShaderType;
    /// Shader filename, if it exists.
    std::string mFilename;
    /// Source for the shader.
    std::string mSource;
protected:
}; // class GLSLShaderInfo

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
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mShaderId, logLength, nullptr, &errorMessage[0]);
            deleteShader();
            throw std::runtime_error(info.filename() + ": \n" + std::string(&errorMessage[0]));
        }
    }

    /// Copying is not allowed.
    GLSLShader(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader(GLSLShader &&rhs) :
        mShaderId{0}
    { swap(rhs); }

    /// Copying is not allowed.
    GLSLShader &operator=(const GLSLShader &rhs) = delete;

    /// Move shader.
    GLSLShader &operator=(GLSLShader &&rhs)
    { swap(rhs); return *this; }

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

    /// Swap 2 shaders.
    void swap(GLSLShader &other)
    { std::swap(mShaderId, other.mShaderId); }

    /// ID of the shader.
    GLuint mShaderId;
protected:
}; // class GLSLShader

/// OpenGL shader program compilation.
class GLSLProgram
{
public:
    /// Default constructor.
    GLSLProgram() :
        mProgramId{0}
    { }

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
        } catch (std::runtime_error &e)
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
            // Reserve space for the message (+1 for terminating null character).
            std::vector<char> errorMessage(logLength + 1);
            glGetShaderInfoLog(mProgramId, logLength, nullptr, &errorMessage[0]);
            glDeleteProgram(mProgramId);
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

    /// Copying is forbidden.
    GLSLProgram(const GLSLProgram &rhs) = delete;

    /// Move constructor.
    GLSLProgram(GLSLProgram &&rhs) :
        mProgramId{0}
    { swap(rhs); }

    /// Copying is forbidden.
    GLSLProgram &operator=(const GLSLProgram &rhs) = delete;

    /// Move-assign operator.
    GLSLProgram &operator=(GLSLProgram &&rhs)
    { swap(rhs); return *this; }

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

    /// Swap 2 GLSL programs.
    void swap(GLSLProgram &other)
    { std::swap(mProgramId, other.mProgramId); }

    /// ID of the shader program.
    GLuint mProgramId;
protected:
}; // class GLSLProgram

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

/// Keyboard handling class.
class Keyboard
{
public:
    /// GLFW keyboard callback function type.
    using GLFWKeyboardCallbackT = void(*)(GLFWwindow*, int, int, int, int);
    /// Type of the action function, called when corresponding key is pressed.
    using ActionFun = std::function<void()>;
    /// Type of the default action, arguments - window, key, scancode, action, mods.
    using DefaultActionFun = std::function<void(GLFWwindow*, int, int, int, int)>;

    /// Default constructor.
    Keyboard()
    { }

    /// If this keyboard is currently selected, change the selected to empty.
    ~Keyboard()
    {
        if (this == sSelected)
        {
            resetSelection();
        }
    }

    /**
     * Select this Keyboard as the active keyboard.
     */
    void select()
    { sSelected = this; }

    /**
     * Reset the selected Keyboard to the default empty one.
     */
    void resetSelection()
    { sSelected = &sEmptyKeyboard; }

    /**
     * Set action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param mods Modifiers - e.g. GLFW_MOD_SHIFT.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     * @param fun Function to call.
     */
    void setAction(int key, int mods, int action, ActionFun fun)
    { mMapping[{key, mods, action}] = fun; }

    /**
     * Reset action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param mods Modifiers - e.g. GLFW_MOD_SHIFT.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     */
    void setAction(int key, int mods, int action)
    { mMapping.erase({key, mods, action}); }

    /**
     * Set the default action, called when no other mapping is found.
     * Function will be passed following arguments :
     *  GLFWwindow* window - window, where the event originated.
     *  int key - Key pressed, e.g. GLFW_KEY_UP.
     *  int scancode - Scancode of the key.
     *  int action - Action of the key - e.g. GLFW_RELEASE.
     *  int mods - Mods - e.g. GLFW_MOD_SHIFT.
     * @param fun Function to call.
     */
    void setDefaultAction(DefaultActionFun fun)
    { mDefaultAction = fun; }

    /**
     * Reset the default action.
     */
    void resetDefaultAction()
    { mDefaultAction = nullptr; }

    /**
     * Get callback to this class, compatible with glfwSetKeyCallback.
     * @return Callback function.
     */
    GLFWKeyboardCallbackT callback()
    {
        return keyboardCallbackDispatch;
    }

    /**
     * Set this keyboards callback for given window.
     * @param window Window which should use this keyboard.
     */
    void setCallback(GLFWwindow *window)
    { glfwSetKeyCallback(window, callback()); }
private:
    /// Helper structure for searching in map.
    struct KeyCombination
    {
        KeyCombination(int keyV, int modsV, int actionV) :
            key{keyV}, mods{modsV}, action{actionV} { }

        /// Keycode.
        int key;
        /// Modifiers.
        int mods;
        /// Action - e.g. GLFW_PRESS.
        int action;

        /// Comparison operator.
        bool operator<(const KeyCombination &rhs) const
        { return (key < rhs.key) || ((key == rhs.key) && (mods < rhs.mods)) || ((key == rhs.key) && (mods == rhs.mods) && (action < rhs.action)); }
        /// Comparison equal operator.
        bool operator==(const KeyCombination &rhs) const
        { return (key == rhs.key) && (mods == rhs.mods) && (action == rhs.action); }
    };

    /**
     * Callback method called by GLFW.
     * Dispatches the call to the selected Keyboard object.
     * @param window Window, where the event originated.
     * @param key Keyboard key.
     * @param scancode System-specific keycode.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    static void keyboardCallbackDispatch(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        sSelected->keyboardCallback(window, key, scancode, action, mods);
    }

    /**
     * Callback method called by dispatcher.
     * @param window Window, where the event originated.
     * @param key Keyboard key.
     * @param scancode System-specific keycode.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) const
    {
        decltype(mMapping.begin()) search{mMapping.find({key, mods, action})};
        if (search != mMapping.end())
        { // We found an action!
            search->second();
        }
        else if (mDefaultAction)
        {
            mDefaultAction(window, key, scancode, action, mods);
        }
    }

    /// Mapping from keys to actions.
    std::map<KeyCombination, ActionFun> mMapping;
    /// Default action, called when no mapping is found.
    DefaultActionFun mDefaultAction;
    /// Basic empty keyboard.
    static const Keyboard sEmptyKeyboard;
    /// Selected keyboard object, which will receive the callbacks.
    static const Keyboard *sSelected;
protected:
}; // class Keyboard

/// Mouse handling class.
class Mouse
{
public:
    /// GLFW mouse button callback function type.
    using GLFWMouseButtonCallbackT = void(*)(GLFWwindow*, int, int, int);
    /// Type of the action function, called when corresponding key is pressed.
    using ActionFun = std::function<void(double, double)>;
    /// Type of the default action, arguments - window, key, scancode action, mods, xpos and ypos.
    using DefaultActionFun = std::function<void(GLFWwindow*, int, int, int, double, double)>;
    /// GLFW scroll callback function type.
    using GLFWScrollCallbackT = void(*)(GLFWwindow*, double, double);
    /// Action taken, when scroll occurs.
    using ScrollActionFun = std::function<void(double, double, double, double)>;

    /// Default constructor.
    Mouse()
    { }

    /// If this Mouse is currently selected, change the selected to empty.
    ~Mouse()
    {
        if (this == sSelected)
        {
            resetSelection();
        }
    }

    /**
     * Select this Mouse as the active mouse mapping.
     */
    void select()
    { sSelected = this; }

    /**
     * Reset the selected Mouse to the default empty one.
     */
    void resetSelection()
    { sSelected = &sEmptyMouse; }

    /**
     * Set action for given key combination.
     * @param key Key - e.g. GLFW_MOUSE_BUTTON_RIGHT.
     * @param mods Modifiers - e.g. GLFW_MOD_SHIFT.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     * @param fun Function to call, function will receive x and y mouse positions.
     */
    void setAction(int key, int mods, int action, ActionFun fun)
    { mMapping[{key, mods, action}] = fun; }

    /**
     * Reset action for given key combination.
     * @param key Key - e.g. GLFW_MOUSE_BUTTON_RIGHT.
     * @param mods Modifiers - e.g. GLFW_MOD_SHIFT.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     */
    void setAction(int key, int mods, int action)
    { mMapping.erase({key, mods, action}); }

    /**
     * Set the default action, called when no other mapping is found.
     * Function will be passed following arguments :
     *  GLFWwindow* window - window, where the event originated.
     *  int key - Key pressed, e.g. GLFW_KEY_UP.
     *  int action - Action of the key - e.g. GLFW_RELEASE.
     *  int mods - Mods - e.g. GLFW_MOD_SHIFT.
     * @param fun Function to call.
     */
    void setDefaultAction(DefaultActionFun fun)
    { mDefaultAction = fun; }

    /**
     * Reset the default action.
     */
    void resetDefaultAction()
    { mDefaultAction = nullptr; }

    /**
     * Set function called, when scrolling occurs
     * @param fun Function to call, function will receive x and y mouse position and x and y scroll offset.
     */
    void setScrollAction(ScrollActionFun fun)
    { mScrollFun = fun; }

    /**
     * Reset the scroll action.
     */
    void resetScrollAction()
    { mScrollFun = nullptr; }

    /**
     * Get callback to this class, compatible with glfwSetMouseButtonCallback.
     * @return Callback function.
     */
    static GLFWMouseButtonCallbackT callback()
    { return mouseCallbackDispatch; }

    /**
     * Get scrolling callback, compatible with glfwSetScrollCallback
     * @return Callback function.
     */
    static GLFWScrollCallbackT scrollCallback()
    { return scrollCallbackDispatch; }

    /**
     * Set this mouse button callback and scroll callback for given window.
     * @param window Window which should use this mouse.
     */
    static void setCallback(GLFWwindow *window)
    {
        glfwSetMouseButtonCallback(window, callback());
        glfwSetScrollCallback(window, scrollCallback());
    }
private:
    /// Helper structure for searching in map.
    struct KeyCombination
    {
        KeyCombination(int keyV, int modsV, int actionV) :
            key{keyV}, mods{modsV}, action{actionV} { }

        /// Keycode.
        int key;
        /// Modifiers.
        int mods;
        /// Action - e.g. GLFW_PRESS.
        int action;

        /// Comparison operator.
        bool operator<(const KeyCombination &rhs) const
        { return (key < rhs.key) || ((key == rhs.key) && (mods < rhs.mods)) || ((key == rhs.key) && (mods == rhs.mods) && (action < rhs.action)); }
        /// Comparison equal operator.
        bool operator==(const KeyCombination &rhs) const
        { return (key == rhs.key) && (mods == rhs.mods) && (action == rhs.action); }
    };

    /**
     * Callback method called by GLFW.
     * Dispatches the call to the selected Mouse object.
     * @param window Window, where the event originated.
     * @param key Mouse key.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    static void mouseCallbackDispatch(GLFWwindow *window, int key, int action, int mods)
    { sSelected->mouseCallback(window, key, action, mods); }

    /**
     * Callback method called by GLFW.
     * @param window Window, where the event originated.
     * @param xOffset X offset from scrolling.
     * @param yOffset Y offset from scrolling.
     */
    static void scrollCallbackDispatch(GLFWwindow *window, double xOffset, double yOffset)
    { sSelected->scrollCallback(window, xOffset, yOffset); }

    /**
     * Callback method called by dispatcher.
     * @param window Window, where the event originated.
     * @param key Mouse key.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    void mouseCallback(GLFWwindow *window, int key, int action, int mods) const
    {
        decltype(mMapping.begin()) search{mMapping.find({key, mods, action})};

        double xPos{0};
        double yPos{0};

        glfwGetCursorPos(window, &xPos, &yPos);

        if (search != mMapping.end())
        { // We found an action!
            search->second(xPos, yPos);
        }
        else if (mDefaultAction)
        {
            mDefaultAction(window, key, action, mods, xPos, yPos);
        }
    }

    /**
     * Called, when scrolling occurs.
     * @param window Window, where the event originated.
     * @param xOffset X offset of the scrolling.
     * @param yOffset Y offset of the scrolling.
     */
    void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) const
    {
        double xPos{0};
        double yPos{0};

        glfwGetCursorPos(window, &xPos, &yPos);

        mScrollFun(xOffset, yOffset, xPos, yPos);
    }

    /// Mapping from keys to actions.
    std::map<KeyCombination, ActionFun> mMapping;
    /// Function called, when scrolling occurs.
    ScrollActionFun mScrollFun;
    /// Default action, called when no mapping is found.
    DefaultActionFun mDefaultAction;
    /// Basic empty mouse.
    static const Mouse sEmptyMouse;
    /// Selected mouse object, which will receive the callbacks.
    static const Mouse *sSelected;
protected:
}; // class Mouse

/// Gamepad handling class.
class Gamepad
{
public:
    /// GLFW keyboard callback function type.
    using GLFWGamepadCallbackT = void(*)(int, int);
    /// Type of the action function, called when corresponding key is pressed.
    using ActionFun = std::function<void(int)>;
    /// Type of the default action, arguments - gamepad, key, action.
    using DefaultActionFun = std::function<void(int, int, int)>;

    /// Default constructor.
    Gamepad()
    { }

    /// Destructor.
    ~Gamepad()
    { }

    /**
     * Set action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     * @param fun Function to call.
     */
    void setAction(int key, int action, ActionFun fun)
    { mMapping[{key, action}] = fun; }

    /**
     * Reset action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     */
    void setAction(int key, int action)
    { mMapping.erase({key, action}); }

    /**
     * Set the default action, called when no other mapping is found.
     * Function will be passed following arguments :
     *  GLFWwindow* window - window, where the event originated.
     *  int gamepad - ID of the gamepad.
     *  int key - Key pressed, e.g. GLFW_KEY_UP.
     *  int action - Action of the key - e.g. GLFW_RELEASE.
     * @param fun Function to call.
     */
    void setDefaultAction(DefaultActionFun fun)
    { mDefaultAction = fun; }

    /**
     * Reset the default action.
     */
    void resetDefaultAction()
    { mDefaultAction = nullptr; }

    /// Get gamepad callback, compatible with glfwSetJoystickCallback.
    static GLFWGamepadCallbackT callback()
    { return gamepadCallback; }

    /**
     * Set gamepad callback for given window.
     * @param window Window which should use this keyboard.
     */
    static void setCallback(GLFWwindow *window)
    { glfwSetJoystickCallback(callback()); }
private:
    /// Helper structure for searching in map.
    struct KeyCombination
    {
        KeyCombination(int keyV, int actionV) :
            key{keyV}, action{actionV} { }

        /// Keycode.
        int key;
        /// Action - e.g. GLFW_PRESS.
        int action;

        /// Comparison operator.
        bool operator<(const KeyCombination &rhs) const
        { return (key < rhs.key) || ((key == rhs.key) && (action < rhs.action)); }
        /// Comparison equal operator.
        bool operator==(const KeyCombination &rhs) const
        { return (key == rhs.key) && (action == rhs.action); }
    };

    /**
     * Callback called, when joystick is connected or disconnected.
     * @param joystick ID of the joystick.
     * @param event Event - GLFW_CONNECTED or GLFW_DISCONNECTED.
     */
    static void gamepadCallback(int joystick, int event)
    {
        std::cout << "Joystick : " << joystick << " ";
        if (event == GLFW_CONNECTED)
        {
            std::cout << "has been connected!" << std::endl;
        }
        else if (event == GLFW_DISCONNECTED)
        {
            std::cout << "has been disconnected!" << std::endl;
        }
        else
        {
            std::cout << "unknown event!" << std::endl;
        }
    }

    /// List of connected gamepads.
    std::vector<int> mConnected;
    /// Mapping from keys to actions.
    std::map<KeyCombination, ActionFun> mMapping;
    /// Default action, called when no mapping is found.
    DefaultActionFun mDefaultAction;
protected:
}; // class Gamepad

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
