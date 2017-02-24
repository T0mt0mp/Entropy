/**
 * @file app/App.cpp
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#include "App.h"

App::App()
{
    glfwSetErrorCallback(App::glfwErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("glfwInit failed!");
    }

    // 4x antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 4);
    // OpenGL 3.3 .
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // MacOS fix.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // Only new OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mWindowTitle.c_str(), nullptr, nullptr);
    if (!mWindow)
    {
        throw std::runtime_error("glfwCreateWindow failed!");
    }

    glfwMakeContextCurrent(mWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("glewInit failed!");
    }

    mRunning = true;
}

App::~App()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void App::run()
{
    mFpsCounter.start();

    while (mRunning)
    {
        glfwSwapBuffers(mWindow);

        glfwPollEvents();

        mRunning = (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS) && !glfwWindowShouldClose(mWindow);

        mFpsCounter.frame();

        glfwSetWindowTitle(mWindow, std::to_string(mFpsCounter.fps()).c_str());
    }
}

void App::glfwErrorCallback(int error, const char *desc)
{
    std::cerr << "GLFW error callback : " << error << " " << desc << std::endl;
}

int main(int argc, char* argv[])
{
    try
    {
        App app;
        app.run();
    } catch (std::runtime_error &e)
    {
        std::cout << e.what() << std::endl;
    } catch (...)
    {
        std::cout << "Unknown exception occurred" << std::endl;
    }

    prof::PrintCrawler pc;
    PROF_DUMP(pc);
}

