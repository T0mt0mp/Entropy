/**
 * @file app/App.cpp
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#include "App.h"

const GLfloat Triangle::VERTEX_BUFFER_DATA[] =
    {
        // x      y     z
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f
    };

Keyboard Keyboard::sEmptyKeyboard;
Keyboard *Keyboard::sSelected{&Keyboard::sEmptyKeyboard};

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

    glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);

    mRunning = true;
}

App::~App()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void App::run()
{
    Keyboard kb;
    kb.setDefaultAction([] (GLFWwindow *window, int key, int scancode, int action, int mods) {
        std::cout << "Received key event from : " << window << " ;\n key= "
                  << key << "; scancode= " << scancode << "; action= " << action
                  << "; mods= " << mods << std::endl;
    });
    kb.setAction(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, [&] () {
        mRunning = false;
    });

    kb.select();

    glfwSetKeyCallback(mWindow, kb.callback());

    // In ms.
    f64 lag{0.0};
    // Counter for frames rendered.
    u64 frameCounter{0};
    // Counter for updates.
    u64 updateCounter{0};

    GLSLProgram program{
        {GL_VERTEX_SHADER,
            "#version 330 core\n"
            "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
            "void main()\n"
            "{\n"
            "   gl_Position.xyz = vertexPosition_modelspace;\n"
            "   gl_Position.w = 1.0;\n"
            "}"
        },
        {GL_FRAGMENT_SHADER,
            "#version 330 core\n"
            "out vec3 color;\n"
            "void main()\n"
            "{\n"
            "   color = vec3(0.0, 0.4, 0.0);\n"
            "}"
        }
    };

    Triangle triangle;

    Timer updateTimer;
    Timer printTimer;

    while (mRunning && !glfwWindowShouldClose(mWindow))
    {
        lag += updateTimer.elapsedReset<Timer::microseconds>() / US_IN_MS;

        u64 sincePrint{printTimer.elapsed<Timer::milliseconds>()};
        if (sincePrint >= MS_PER_INFO)
        {
            printTimer.reset();

            std::cout << "FrameTime[ms] : " << static_cast<f64>(sincePrint) / frameCounter
                      << "\nFrames : " << frameCounter << "; Updates : " << updateCounter << std::endl;

            frameCounter = 0;
            updateCounter = 0;
        }

        glfwPollEvents();

        while (lag >= MS_PER_UPDATE)
        {
            updateCounter++;

            // Update...

            lag -= MS_PER_UPDATE;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render...
        program.use();
        triangle.render();

        glfwSwapBuffers(mWindow);

        frameCounter++;
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
