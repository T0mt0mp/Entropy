/**
 * @file app/App.h
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#ifndef SIMPLE_GAME_APP_H
#define SIMPLE_GAME_APP_H

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Entropy/Entropy.h"
#include "testing/Testing.h"

class FpsCounter
{
public:
    FpsCounter()
    { }

    void start()
    {
        mNumFrames = 0u;
        mStart = clock::now();
    }

    f64 seconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - mStart).count() /
            static_cast<f64>(MS_IN_S);
    }

    void frame()
    {
        mNumFrames++;
    }

    f64 fps()
    {
        return mNumFrames / seconds();
    }
private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;

    static constexpr u64 MS_IN_S{1000u};

    time_point mStart;
    u64 mNumFrames{0u};
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
    // GLFW window.
    /// GLFW window instance.
    GLFWwindow *mWindow{nullptr};
    /// Window width.
    u64 mWindowWidth{640u};
    /// Window height.
    u64 mWindowHeight{480u};
    /// Window title.
    std::string mWindowTitle{"Hello GLFW"};

    // Application variable.
    /// Are we running?
    bool mRunning;
    /// Counting game FPS.
    FpsCounter mFpsCounter;
protected:
};

#endif //SIMPLE_GAME_APP_H
