/**
 * @file app/App.h
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#ifndef SIMPLE_GAME_APP_H
#define SIMPLE_GAME_APP_H

#include "Types.h"

#include "Timer.h"

#include "Font.h"
#include "FontFile.h"
#include "FontParser.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "Gamepad.h"

#include "GLSLProgram.h"
#include "Camera.h"

#include "MovementS.h"
#include "RotationS.h"
#include "TransformS.h"
#include "RenderS.h"
#include "TextS.h"

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
    u64 mWindowWidth{1280u};
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
