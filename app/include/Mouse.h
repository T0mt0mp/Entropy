/**
 * @file app/Mouse.h
 * @author Tomas Polasek
 * @brief Mouse handling class.
 */

#ifndef SIMPLE_GAME_MOUSE_H
#define SIMPLE_GAME_MOUSE_H

#include "Types.h"

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

#endif //SIMPLE_GAME_MOUSE_H
