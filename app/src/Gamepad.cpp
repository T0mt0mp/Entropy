/**
 * @file Gamepad.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Gamepad.h"

std::vector<Gamepad::GamepadData> Gamepad::sConnected;
const Gamepad::CallbackInformation Gamepad::sDefaultCallbacks{};
const Gamepad::CallbackInformation *Gamepad::sSelectedCallbacks{&Gamepad::sDefaultCallbacks};

void Gamepad::pollEvents()
{
    sConnected.erase(std::remove_if(sConnected.begin(), sConnected.end(),
                                    [] (const GamepadData &gamepad){ return !gamepad.isConnected(); }),
                     sConnected.end());

    for (GamepadData &gamepad : sConnected)
    {
        try
        {
            gamepad.pollEvents();
        } catch (std::runtime_error &e)
        {
            std::cout << "Gamepad event polling failed : \n" << e.what() << std::endl;
        }
    }
}

void Gamepad::checkAlreadyConnected()
{
    for (u16 gamepadId = GLFW_JOYSTICK_1; gamepadId <= GLFW_JOYSTICK_LAST; ++gamepadId)
    {
        if (glfwJoystickPresent(gamepadId))
        {
            gamepadCallback(gamepadId, GLFW_CONNECTED);
        }
    }
}

u64 Gamepad::getNumberOfJoysticks(u16 gamepadId)
{
    auto findIt{findGamepad(gamepadId)};
    if (findIt != sConnected.end())
    {
        return findIt->numberOfJoysticks();
    }

    return 0;
}

u64 Gamepad::getNumberOfButtons(u16 gamepadId)
{
    auto findIt{findGamepad(gamepadId)};
    if (findIt != sConnected.end())
    {
        return findIt->numberOfButtons();
    }

    return 0;
}

void Gamepad::GamepadData::pollEvents()
{
    const float *joystickValues{nullptr};
    const unsigned char *buttonValues{nullptr};
    const CallbackInformation &callbackInfo{callbacks()};

    getValues(&joystickValues, &buttonValues);

    // Process joystick events.
    for (u32 iii = 0; iii < mJoystickValues.size(); ++iii)
    {
        if (std::abs(joystickValues[iii]) > callbackInfo.joystickDeadzone)
        {
            decltype(callbackInfo.joystickMapping.begin()) search{callbackInfo.joystickMapping.find(iii)};
            if (search != callbackInfo.joystickMapping.end())
            { // We found an action!
                search->second(mId, joystickValues[iii]);
            }
            else if (callbackInfo.defaultJoystickAction)
            {
                callbackInfo.defaultJoystickAction(mId, iii, joystickValues[iii]);
            }
        }
    }

    // Process button events.
    for (u32 iii = 0; iii < mButtonValues.size(); ++iii)
    {
        int newState{buttonValues[iii]};
        if (newState != mButtonValues[iii])
        {
            decltype(callbackInfo.mapping.begin()) search{callbackInfo.mapping.find({iii, newState})};
            if (search != callbackInfo.mapping.end())
            { // We found an action!
                search->second(mId);
            }
            else if (callbackInfo.defaultAction)
            {
                callbackInfo.defaultAction(mId, iii, newState);
            }
        }
    }

    copyJoystickValues(joystickValues);
    copyButtonValues(buttonValues);
}

void Gamepad::GamepadData::initVectors()
{
    assertPresence();

    i32 count{0};

    // Get joystick values.
    const float *joystickAxes{glfwGetJoystickAxes(mId, &count)};
    if (count < 0)
    {
        throw std::runtime_error("Number of joystick axes on gamepad is negative!");
    }
    mJoystickValues.resize(static_cast<u32>(count));
    copyJoystickValues(joystickAxes);

    // Get button values.
    const unsigned char *buttons{glfwGetJoystickButtons(mId, &count)};
    if (count < 0)
    {
        throw std::runtime_error("Number of buttons on gamepad is negative!");
    }
    mButtonValues.resize(static_cast<u32>(count));
    copyButtonValues(buttons);
}

void Gamepad::GamepadData::getValues(const float **joystickValues, const unsigned char **buttonValues)
{
    i32 joystickCount{0};
    i32 buttonCount{0};

    assertPresence();
    *joystickValues = glfwGetJoystickAxes(mId, &joystickCount);
    *buttonValues = glfwGetJoystickButtons(mId, &buttonCount);

    if (static_cast<u32>(joystickCount) != mJoystickValues.size() ||
        static_cast<u32>(buttonCount) != mButtonValues.size())
    {
        throw std::runtime_error("Number of joystick axes or button axes on gamepad has changed!");
    }
}

void Gamepad::gamepadCallback(int gamepad, int event)
{
    std::cout << "Gamepad : " << gamepad << " ";
    if (event == GLFW_CONNECTED)
    {
        std::cout << "has been connected!" << std::endl;
        auto findIt{findGamepad(gamepad)};
        if (findIt == sConnected.end())
        { // Add gamepad to list of connected gamepads.
            sConnected.emplace_back(gamepad, &sSelectedCallbacks);
            std::cout << "Gamepad has been added, it has " << getNumberOfJoysticks(gamepad) << " joysticks and "
                      << getNumberOfButtons(gamepad) << " buttons: " << sConnected.size() << " gamepads connected" << std::endl;
        } else
        {
            std::cout << "But it was already connected?!?" << std::endl;
        }
    }
}
