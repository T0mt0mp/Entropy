/**
 * @file app/Gamepad.h
 * @author Tomas Polasek
 * @brief Gamepad handling class.
 */

#ifndef SIMPLE_GAME_GAMEPAD_H
#define SIMPLE_GAME_GAMEPAD_H

#include "Types.h"

/// IDs of joysticks and button for DualShock4 controller.
namespace DS4Mapping
{
    enum Joystick
    {
        // Right analog stick.
        // Left = -1, Right = +1.
        RX = 0,
        // Up = -1, Down = +1.
        RY = 1,

        // Left analog stick.
        // Left = -1, Right = +1.
        LX = 2,
        // Up = -1, Down = +1.
        LY = 5,

        // Analog value of triggers. Not pressed = -1, Max pressed = +1.
        LT_ANALOG = 3,
        RT_ANALOG = 4,

        // Dpad directional buttons.
        // Left = -1, Right = +1.
        DPADX = 6,
        // Up = -1, Down = +1.
        DPADY = 7,

        // Shaking the controller, differential values.
        // UP = +1, DOWN = -1. Rotating around x-axis.
        SHAKEX = 8,
        // LEFT = +1, RIGHT = -1. Rotating around y-axis.
        SHAKEY = 9,
        // LEFT = +1, RIGHT = -1. Rotating around z-axis.
        SHAKEZ = 10,

        // Rotating controller, scalar values.
        // LEFT = +1, RIGHT = -1. Maximal value is for 90 degrees.
        ROTZ = 11,
        // FRONT = +1, UPSIDE_DOWN = -1. 90 degrees up or down is == 0.
        ROTX = 12,
        // FONT = 0, UP = -1, DOWN = 1.
        ROTX_HALF = 13,

        UNK1 = 14,

        // Touchpad.
        // LEFT = -1, RIGHT = +1.
        LAST_TOUCHX = 15,
        // UP = -1, DOWN = +1.
        LAST_TOUCHY = 16,

        UNK2 = 17,
    };

    enum Button
    {
        SQUARE = 0,
        CROSS = 1,
        CIRCLE = 2,
        TRIANGLE = 3,
        LB = 4,
        RB = 5,
        LT = 6,
        RT = 7,
        SHARE = 8,
        OPTIONS = 9,
        LS = 10,
        RS = 11,
        PS = 12,
        TOUCHPAD = 13,
    };
} // namespce DS4Mapping

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
    void pollEvents();

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
    static void checkAlreadyConnected();

    /**
     * Get number of joysticks for given gamepad.
     * @param gamepadId ID of the gamepad.
     * @return Number of joysticks.
     */
    static u64 getNumberOfJoysticks(u16 gamepadId);

    /**
     * Get number of buttons for given gamepad.
     * @param gamepadId ID of the gamepad.
     * @return Number of buttons.
     */
    static u64 getNumberOfButtons(u16 gamepadId);
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
        void pollEvents();

        /**
         * Get the number of joysticks.
         * @return Number of joysticks.
         */
        u64 numberOfJoysticks() const
        { return mJoystickValues.size(); }

        /**
         * Get the number of buttons.
         * @return Number of buttons.
         */
        u64 numberOfButtons() const
        { return mButtonValues.size(); }

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
        void initVectors();

        /**
         * Checks, if gamepad is connected and that the number of axes on joysticks or buttons
         * hasn't changed. Throws runtime_error, if anything goes wrong.
         * @param joystickValues Output, set to array filled with joystick values.
         * @param buttonValues Output, set to array filled with button values.
         */
        void getValues(const float **joystickValues, const unsigned char **buttonValues);

        /// Is this gamepad disconnected?
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
    static void gamepadCallback(int gamepad, int event);

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
