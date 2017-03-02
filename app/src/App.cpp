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

const Keyboard Keyboard::sEmptyKeyboard;
const Keyboard *Keyboard::sSelected{&Keyboard::sEmptyKeyboard};

const Mouse Mouse::sEmptyMouse;
const Mouse *Mouse::sSelected{&Mouse::sEmptyMouse};

std::vector<Gamepad::GamepadData> Gamepad::sConnected;
const Gamepad::CallbackInformation Gamepad::sDefaultCallbacks;
const Gamepad::CallbackInformation *Gamepad::sSelectedCallbacks{&Gamepad::sDefaultCallbacks};

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
    glfwSetInputMode(mWindow, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);

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
    Universe u;

    u.registerComponent<PositionC>();
    u.registerComponent<VelocityC>();

    MovementS *movementSystem{u.addSystem<MovementS>()};
    TriangleRenderS *triangleRenderSystem{u.addSystem<TriangleRenderS>()};

    u.init();

    Universe::EntityT e{u.createEntity()};
    e.add<PositionC>()->p = glm::vec3(0.0f, 0.0f, 0.0f);
    e.add<VelocityC>()->v = glm::vec3(0.0f, 0.5f, 0.0f);

    e = u.createEntity();
    e.add<PositionC>()->p = glm::vec3(1.0f, 0.0f, 0.0f);
    e.add<VelocityC>()->v = glm::vec3(0.0f, 0.5f, 0.0f);

    e = u.createEntity();
    e.add<PositionC>()->p = glm::vec3(-1.0f, 0.0f, 0.0f);
    e.add<VelocityC>()->v = glm::vec3(0.0f, 0.5f, 0.0f);

    u.refresh();

    Keyboard kb;

    kb.setDefaultAction([] (GLFWwindow *window, int key, int scancode, int action, int mods) {
        std::cout << "Received keyboard event from : " << window << " ;\n key= "
                  << key << "; scancode= " << scancode << "; action= " << action
                  << "; mods= " << mods << std::endl;
    });
    kb.setAction(GLFW_KEY_SPACE, 0, GLFW_PRESS, [&] () {
        this->reloadShaders();
    });
    kb.setAction(GLFW_KEY_ESCAPE, 0, GLFW_PRESS, [&] () {
        mRunning = false;
    });

    kb.select();
    kb.setCallback(mWindow);

    Mouse mouse;

    mouse.setDefaultAction([] (GLFWwindow *window, int key, int action, int mods, double x, double y) {
        std::cout << "Received mouse event from : " << window << " (" << x << " " << y << ") ;\n key= "
                  << key << "; action= " << action << "; mods= " << mods << std::endl;
    });

    f64 xScroll{0};
    f64 yScroll{0};
    mouse.setScrollAction([&] (f64 xOffset, f64 yOffset, f64 xPos, f64 yPos) {
        xScroll += xOffset;
        yScroll += yOffset;
    });

    mouse.select();
    mouse.setCallback(mWindow);

    Gamepad gamepad;
    gamepad.setJoystickDeadzone(0.0f);
    u32 usedJoystick{0};

    static constexpr double MAX_ANGULAR_SPEED_X{1.5f};
    double xAngle{0.0f};
    double xAngleGoal{0.0f};
    static constexpr double MAX_ANGULAR_SPEED_Z{1.5f};
    double zAngle{0.0f};
    double zAngleGoal{0.0f};

    gamepad.setDefaultAction([] (u16 gamepadId, u32 key, int action) {
        std::cout << "Received gamepad event from gamepad : " << gamepadId << ";\n key= "
                  << key << "; action= " << action << std::endl;
    });
    gamepad.setJoystickAction(usedJoystick, [usedJoystick] (u16 gamepadId, double value) {
        std::cout << "Received gamepad event from gamepad : " << gamepadId << ";\n joystick= "
                  << usedJoystick << "; value= " << value << std::endl;
    });

    gamepad.setJoystickAction(DS4Mapping::Joystick::ROTX_HALF, [&xAngleGoal] (u16 gamepadId, double value) {
        double newGoal{-value * 90.0f};
        if (std::abs(newGoal - xAngleGoal) > MAX_ANGULAR_SPEED_X)
        {
            xAngleGoal = -value * 90.0f;
        }
    });

    gamepad.setJoystickAction(DS4Mapping::Joystick::ROTZ, [&zAngleGoal] (u16 gamepadId, double value) {
        double newGoal{-value * 90.0f};
        if (std::abs(newGoal - zAngleGoal) > MAX_ANGULAR_SPEED_Z)
        {
            zAngleGoal = -value * 90.0f;
        }
    });

    gamepad.select();
    gamepad.setCallback(mWindow);

    kb.setAction(GLFW_KEY_UP, 0, GLFW_PRESS, [&] () {
        std::cout << "Moving to the next joystick." << std::endl;
        gamepad.resetJoystickAction(usedJoystick);
        ++usedJoystick;
        gamepad.setJoystickAction(usedJoystick, [usedJoystick] (u16 gamepadId, double value) {
            std::cout << "Received gamepad event from gamepad : " << gamepadId << ";\n joystick= "
                      << usedJoystick << "; value= " << value << std::endl;
        });
    });

    kb.setAction(GLFW_KEY_DOWN, 0, GLFW_PRESS, [&] () {
        if (usedJoystick)
        {
            std::cout << "Moving to the previous joystick." << std::endl;
            gamepad.resetJoystickAction(usedJoystick);
            --usedJoystick;
            gamepad.setJoystickAction(usedJoystick, [usedJoystick] (u16 gamepadId, double value) {
                std::cout << "Received gamepad event from gamepad : " << gamepadId << ";\n joystick= "
                          << usedJoystick << "; value= " << value << std::endl;
            });
        }
    });

    // In ms.
    f64 lag{0.0};
    // Counter for frames rendered.
    u64 frameCounter{0};
    // Counter for updates.
    u64 updateCounter{0};

    reloadShaders();

    Camera camera(45.0f, static_cast<float>(mWindowWidth) / mWindowHeight, 0.1f, 100.0f);
    camera.setPos({0.0f, 0.0f, 3.0f});

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

            std::cout << "To scroll : " << xScroll << " " << yScroll << std::endl;

            frameCounter = 0;
            updateCounter = 0;
        }

        glfwPollEvents();
        gamepad.pollEvents();

        while (lag >= MS_PER_UPDATE)
        {
            updateCounter++;

            // Update...
            movementSystem->run(static_cast<f32>(MS_PER_UPDATE / MS_IN_S));

            u.refresh();

            lag -= MS_PER_UPDATE;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render...
        mProgram.use();

        glm::mat4 model(1.0f);

        if (xAngleGoal > xAngle)
        {
            xAngle = std::min(xAngle + std::min(xAngleGoal - xAngle, MAX_ANGULAR_SPEED_X), xAngleGoal);
        }
        else
        {
            xAngle = std::max(xAngle + std::max(xAngleGoal - xAngle, -MAX_ANGULAR_SPEED_X), xAngleGoal);
        }

        if (zAngleGoal > zAngle)
        {
            zAngle = std::min(zAngle + std::min(zAngleGoal - zAngle, MAX_ANGULAR_SPEED_Z), zAngleGoal);
        }
        else
        {
            zAngle = std::max(zAngle + std::max(zAngleGoal - zAngle, -MAX_ANGULAR_SPEED_Z), zAngleGoal);
        }

        camera.setRot({-glm::radians(xAngle), 0.0f, -glm::radians(zAngle)});
        camera.recalculate();

        /*
        glm::mat4 mvp{camera.viewProjectionMatrix() * model};
        GLint mvpLocation{mProgram.getUniformLocation("mvp")};
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
        triangle.render();
         */

        triangleRenderSystem->render(camera, triangle, mProgram);

        glfwSwapBuffers(mWindow);

        frameCounter++;
    }
}

void App::glfwErrorCallback(int error, const char *desc)
{
    std::cerr << "GLFW error callback : " << error << " " << desc << std::endl;
}

void App::reloadShaders()
{
    try {
        /*
        GLSLProgram newProgram{
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
         */

        GLSLProgram newProgram{
            {GL_VERTEX_SHADER, std::string("shaders/basic.vert")},
            {GL_FRAGMENT_SHADER, std::string("shaders/basic.frag")}
        };

        mProgram = std::move(newProgram);
    } catch(std::runtime_error &e)
    {
        std::cerr << "Unable to reload shaders!\n" << e.what() << std::endl;
    } catch(...)
    {
        std::cerr << "Unable to reload shaders!\nUnknown exception has been thrown!" << std::endl;
    }
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
