/**
 * @file app/Gamepad.h
 * @author Tomas Polasek
 * @brief Gamepad handling class.
 */

#ifndef SIMPLE_GAME_GAMEPAD_H
#define SIMPLE_GAME_GAMEPAD_H

#include "Types.h"

/// Gamepad handling class.
class Gamepad
{
public:
    /// GLFW keyboard callback function type.
    using GLFWGamepadCallbackT = void(*)(int, int);
    /// Type of the action function, called when corresponding key is pressed.
    using ActionFun = std::function<void(u16)>;
    /// Type of the default action, arguments - gamepad, key, action.
    using DefaultActionFun = std::function<void(u16, u32, int)>;
    /// Type of the action function, called when joystick values are above deadzone value.
    using JoystickActionFun = std::function<void(u16, double)>;
    /// Type of the default action for joystick, arguments - gamepad, joystick, value.
    using DefaultJoystickActionFun = std::function<void(u16, u32, double)>;

    /// Default value for joystick deadzone setting.
    static constexpr double DEFAULT_JOYSTICK_DEADZONE{0.1f};

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
    void setAction(u32 key, int action, ActionFun fun)
    { mCallbacks.mapping[{key, action}] = fun; }

    /**
     * Reset action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     */
    void setAction(u32 key, int action)
    { mCallbacks.mapping.erase({key, action}); }

    /**
     * Set deadzone of the joystick (when event occurs).
     * @param deadzone New deadzone.
     */
    void setJoystickDeadzone(double deadzone)
    { mCallbacks.joystickDeadzone = deadzone; }

    /**
     * Set callback called, when joystick value is above deadzone value.
     * @param joystick Which joystick.
     * @param fun Function to be called, gamepad and value are passed.
     */
    void setJoystickAction(u32 joystick, JoystickActionFun fun)
    { mCallbacks.joystickMapping[joystick] = fun; }

    /**
     * Reset callback called, when joystick value is above deadzone value.
     * @param joystick Which joystick.
     */
    void resetJoystickAction(u32 joystick)
    { mCallbacks.joystickMapping.erase(joystick); }

    /**
     * Set the default action called, when no other mapping is found.
     * @param fun Function called - parameters gamepad, joystick ID and value are passsed.
     */
    void setDefaultJoystickAction(DefaultJoystickActionFun fun)
    { mCallbacks.defaultJoystickAction = fun; }

    /**
     * Reset the default action called, when no other mapping is found.
     */
    void resetDefaultJoystickAction()
    { mCallbacks.defaultJoystickAction = nullptr; }

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
    { mCallbacks.defaultAction = fun; }

    /**
     * Reset the default action.
     */
    void resetDefaultAction()
    { mCallbacks.defaultAction = nullptr; }

    /**
     * Poll gamepad events and call corresponding callbacks.
     */
    void pollEvents()
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

    /**
     * Select this gamepad mapping as current.
     */
    void select()
    { sSelectedCallbacks = &mCallbacks; }

    /**
     * Reset selected gamepad mapping to the default one.
     */
    void resetSelection()
    { sSelectedCallbacks = &sDefaultCallbacks; }

    /// Get gamepad callback, compatible with glfwSetJoystickCallback.
    static GLFWGamepadCallbackT callback()
    { return gamepadCallback; }

    /**
     * Set gamepad callback for given window.
     * @param window Window which should use this keyboard.
     */
    static void setCallback(GLFWwindow *window)
    { glfwSetJoystickCallback(callback()); checkAlreadyConnected(); }

    /**
     * Check, if any of the controllers were already connected and
     * no connected events will be sent.
     */
    static void checkAlreadyConnected()
    {
        for (u16 gamepadId = GLFW_JOYSTICK_1; gamepadId <= GLFW_JOYSTICK_LAST; ++gamepadId)
        {
            if (glfwJoystickPresent(gamepadId))
            {
                gamepadCallback(gamepadId, GLFW_CONNECTED);
            }
        }
    }
private:
    /// Helper structure for searching in map.
    struct KeyCombination
    {
        KeyCombination(u32 keyV, int actionV) :
            key{keyV}, action{actionV} { }

        /// Keycode.
        u32 key;
        /// Action - e.g. GLFW_PRESS.
        int action;

        /// Comparison operator.
        bool operator<(const KeyCombination &rhs) const
        { return (key < rhs.key) || ((key == rhs.key) && (action < rhs.action)); }
        /// Comparison equal operator.
        bool operator==(const KeyCombination &rhs) const
        { return (key == rhs.key) && (action == rhs.action); }
    };

    /// Structure containing information about how to handle callbacks.
    struct CallbackInformation
    {
        /// Mapping from keys to actions.
        std::map<KeyCombination, ActionFun> mapping;
        /// Default action, called when no mapping is found.
        DefaultActionFun defaultAction{nullptr};
        /// Function called, when joystick value is above the deadzone value.
        std::map<u32, JoystickActionFun> joystickMapping;
        /// Default function for joystick.
        DefaultJoystickActionFun defaultJoystickAction{nullptr};
        /// Deadzone for joysticks.
        double joystickDeadzone{DEFAULT_JOYSTICK_DEADZONE};
    };

    /// Information about connected gamepad.
    class GamepadData
    {
    public:
        /**
         * Create object for given gamepad ID.
         * @param gamepadId ID of the connected gamepad.
         */
        GamepadData(u16 gamepadId, const CallbackInformation **callbacksPtr) :
            mId{gamepadId},
            mCallbacks(callbacksPtr)
        {
            initVectors();
        }

        /// Get values from gamepad and call corresponding callbacks.
        void pollEvents()
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

        /**
         * Is this gamepad still connected?
         * @return Returns true, if the gamepad is still connected.
         */
        bool isConnected() const
        { return glfwJoystickPresent(mId) != 0; }

        /// Comparison operator.
        bool operator==(const GamepadData &rhs) const
        { return mId == rhs.mId; }
        /// Comparison operator.
        bool operator==(u64 id) const
        { return mId == id; }
    protected:
    private:
        /// Get currently used callback information.
        const CallbackInformation &callbacks() const
        { return *(*mCallbacks); }

        /// Make sure, gamepad is still connected, throw runtime_error, if it is not.
        void assertPresence()
        {
            if (!isConnected())
            {
                throw std::runtime_error("Unable to use GamepadData without connected gamepad!");
            }
        }

        /**
         * Copy input array to joystick vector.
         * @param joystickValues Input array of values.
         */
        void copyJoystickValues(const float *joystickValues)
        {
            std::memcpy(mJoystickValues.data(), joystickValues, sizeof(float) * mJoystickValues.size());
        }

        /**
         * Copy input array to button vector.
         * @param buttonValues Input array of values.
         */
        void copyButtonValues(const unsigned char *buttonValues)
        {
            std::memcpy(mButtonValues.data(), buttonValues, sizeof(unsigned char) * mButtonValues.size());
        }

        /// Initialize inner vectors.
        void initVectors()
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

        /**
         * Checks, if gamepad is connected and that the number of axes on joysticks or buttons
         * hasn't changed. Throws runtime_error, if anything goes wrong.
         * @param joystickValues Output, set to array filled with joystick values.
         * @param buttonValues Output, set to array filled with button values.
         */
        void getValues(const float **joystickValues, const unsigned char **buttonValues)
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

        ///
        bool mDisconnected{false};
        /// ID of the gamepad.
        u16 mId;
        /// List of joystick values, each is <-1; 1>.
        std::vector<f32> mJoystickValues;
        /// List of button values, values : GLFW_PRESS, GLFW_RELEASE.
        std::vector<u8> mButtonValues;
        /// Information about which callback should be called.
        const CallbackInformation **mCallbacks;
    protected:
    };

    /**
     * Callback called, when joystick is connected or disconnected.
     * @param joystick ID of the joystick.
     * @param event Event - GLFW_CONNECTED or GLFW_DISCONNECTED.
     */
    static void gamepadCallback(int gamepad, int event)
    {
        std::cout << "Gamepad : " << gamepad << " ";
        if (event == GLFW_CONNECTED)
        {
            std::cout << "has been connected!" << std::endl;
            auto findIt{findGamepad(gamepad)};
            if (findIt == sConnected.end())
            { // Add gamepad to list of connected gamepads.
                sConnected.emplace_back(gamepad, &sSelectedCallbacks);
                std::cout << "Gamepad has been added: " << sConnected.size() << " gamepads connected" << std::endl;
            } else
            {
                std::cout << "But it was already connected?!?" << std::endl;
            }
        }
        /*
        else if (event == GLFW_DISCONNECTED)
        {
            std::cout << "has been disconnected!" << std::endl;

            auto findIt{findGamepad(gamepad)};
            if (findIt != sConnected.end())
            { // Remove gamepad from list of connected gamepads.
                sConnected.erase(findIt);
                std::cout << "Gamepad has been removed: " << sConnected.size()<< " gamepads connected" << std::endl;
            } else
            {
                std::cout << "But it was not connected?!?" << std::endl;
            }
        }
        else
        {
            std::cout << "unknown event!" << std::endl;
        }
         */
    }

    /**
     * Search for given gamepad amongst the connected gamepads.
     * @param gamepad ID of the gamepad.
     * @return Iterator in the sConnected vector.
     */
    static std::vector<GamepadData>::iterator findGamepad(int gamepad)
    { return std::find(sConnected.begin(), sConnected.end(), gamepad); }

    /// Information about how to handle callbacks.
    CallbackInformation mCallbacks;
    /// List of connected gamepads.
    static std::vector<GamepadData> sConnected;
    /// Default callbacks information.
    static const CallbackInformation sDefaultCallbacks;
    /// Currently selected callback information.
    static const CallbackInformation *sSelectedCallbacks;
protected:
}; // class Gamepad


#endif //SIMPLE_GAME_GAMEPAD_H
