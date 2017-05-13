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

    glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(mWindow, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    mRunning = true;
}

App::~App()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void App::run()
{
    PROF_BLOCK("Init");

    ttf::FontFile ff("res/font.ttf");
    ttf::FontParser fp(ff);
    ttf::Font font = fp.parse();

    Universe u;

    u.initialize();

    MovementS *movementSystem{u.addSystem<MovementS>()};
    RotationS *rotationSystem{u.addSystem<RotationS>()};
    TransformS *transformSystem{u.addSystem<TransformS>()};
    RenderS *renderSystem{u.addSystem<RenderS>()};
    TextS *textSystem{u.addSystem<TextS>(&font)};
    textSystem->addText("Entropy");
    textSystem->addText("Entity");
    textSystem->addText("Component");
    textSystem->addText("System");

    u.refresh();

    PROF_BLOCK("Creating cubes");
    textSystem->initializeCubes(10);
    u.refresh();
    PROF_BLOCK_END();

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

    f64 xMousePos{0};
    f64 yMousePos{0};
    mouse.setMousePosAction([&] (f64 posX, f64 posY)
    {
        xMousePos = posX;
        yMousePos = posY;
    });

    mouse.select();
    mouse.setCallback(mWindow);

    Gamepad gamepad;
    gamepad.setJoystickDeadzone(0.0f);
    u32 usedJoystick{0};

    bool cameraEnabled{false};
    CameraInfo cameraInfo;

    static constexpr double MAX_ANGULAR_SPEED_X{1.5f};
    double xAngleGoal{0.0f};
    //static constexpr double MAX_ANGULAR_SPEED_Y{1.5f};
    //double yAngleGoal{0.0f};
    static constexpr double MAX_ANGULAR_SPEED_Z{1.5f};
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

    kb.setAction(GLFW_KEY_C, 0, GLFW_PRESS, [&] () {
        static bool disabled{false};
        if (disabled)
        {
            cameraEnabled = false;
            disabled = false;
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            cameraEnabled = true;
            disabled = true;
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    });

    kb.setAction(GLFW_KEY_T, 0, GLFW_PRESS, [&] () {
        textSystem->nextText();
    });

    // In ms.
    f64 lag{0.0};
    // Counter for frames rendered.
    u64 frameCounter{0};
    // Counter for updates.
    u64 updateCounter{0};

    reloadShaders();

    Camera camera(45.0f, static_cast<float>(mWindowWidth) / mWindowHeight, 0.1f, 100.0f);

    Triangle triangle;
    Cube cube;

    PROF_BLOCK_END();

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
        gamepad.pollEvents();

        textSystem->updateText();

        PROF_BLOCK("Update loop");
        while (lag >= MS_PER_UPDATE)
        {
            PROF_SCOPE("Update");
            updateCounter++;

            // Update...
            PROF_BLOCK("Movement");
            movementSystem->doMove(static_cast<f32>(MS_PER_UPDATE / MS_IN_S));
            PROF_BLOCK_END();

            PROF_BLOCK("Rotation");
            rotationSystem->doRot(static_cast<f32>(MS_PER_UPDATE / MS_IN_S));
            PROF_BLOCK_END();

            PROF_BLOCK("Refresh");
            u.refresh();
            PROF_BLOCK_END();

            /*
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
             */

            if (cameraEnabled)
            {
                cameraInfo.rot.x = (yMousePos - mWindowHeight / 2.0f) / 400.0f;
                if (cameraInfo.rot.x > PI_8)
                {
                    cameraInfo.rot.x = PI_8;
                }
                if (cameraInfo.rot.x < -PI_8)
                {
                    cameraInfo.rot.x = -PI_8;
                }
                cameraInfo.rot.y = (xMousePos - mWindowWidth / 2.0f) / 400.0f;
                if (cameraInfo.rot.y > PI_4)
                {
                    cameraInfo.rot.y = PI_4;
                }
                if (cameraInfo.rot.y < -PI_4)
                {
                    cameraInfo.rot.y = -PI_4;
                }
            }
            else
            {
                cameraInfo.rot.x = 0.0f;
                cameraInfo.rot.y = 0.0f;
            }
            cameraInfo.rot.z = 0.0f;

            glm::vec3 spherePoint;
            spherePoint.x = glm::cos(cameraInfo.rot.x) * glm::sin(cameraInfo.rot.y);
            spherePoint.y = glm::sin(cameraInfo.rot.x) * glm::cos(cameraInfo.rot.y);
            spherePoint.z = glm::cos(cameraInfo.rot.y);

            /*
            std::cout << "Rot: "
                      << glm::degrees(cameraInfo.rot.x) << " "
                      << glm::degrees(cameraInfo.rot.y) << " "
                      << glm::degrees(cameraInfo.rot.z) << std::endl;

            std::cout << "Dir: "
                      << (spherePoint.x) << " "
                      << (spherePoint.y) << " "
                      << (spherePoint.z) << std::endl;
                      */

            camera.setPos(cameraInfo.pos);
            camera.setRot(cameraInfo.rot);

            lag -= MS_PER_UPDATE;
        }
        camera.recalculate();

        PROF_BLOCK("Transform system");
        transformSystem->doTransform();
        PROF_BLOCK_END();

        PROF_BLOCK_END();

        PROF_BLOCK("glClear");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        PROF_BLOCK_END();

        // Render...
        PROF_BLOCK("Render");
        mProgram.use();

        renderSystem->render(camera, cube, mProgram);

        PROF_BLOCK_END();

        PROF_BLOCK("Swap");
        glfwSwapBuffers(mWindow);
        PROF_BLOCK_END();

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
        GLSLProgram newProgram{
            {GL_VERTEX_SHADER, std::string("res/basic.vert")},
            {GL_FRAGMENT_SHADER, std::string("res/basic.frag")}
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
