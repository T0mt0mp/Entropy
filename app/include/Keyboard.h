/**
 * @file app/Keyboard.h
 * @author Tomas Polasek
 * @brief Keyboard handling class.
 */

#ifndef SIMPLE_GAME_KEYBOARD_H
#define SIMPLE_GAME_KEYBOARD_H

#include "Types.h"

/// Keyboard handling class.
class Keyboard
{
public:
    /// GLFW keyboard callback function type.
    using GLFWKeyboardCallbackT = void(*)(GLFWwindow*, int, int, int, int);
    /// Type of the action function, called when corresponding key is pressed.
    using ActionFun = std::function<void()>;
    /// Type of the default action, arguments - window, key, scancode, action, mods.
    using DefaultActionFun = std::function<void(GLFWwindow*, int, int, int, int)>;

    /// Default constructor.
    Keyboard()
    { }

    /// If this keyboard is currently selected, change the selected to empty.
    ~Keyboard()
    {
        if (this == sSelected)
        {
            resetSelection();
        }
    }

    /**
     * Select this Keyboard as the active keyboard.
     */
    void select()
    { sSelected = this; }

    /**
     * Reset the selected Keyboard to the default empty one.
     */
    void resetSelection()
    { sSelected = &sEmptyKeyboard; }

    /**
     * Set action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
     * @param mods Modifiers - e.g. GLFW_MOD_SHIFT.
     * @param action Action taken with the key - e.g. GLFW_RELEASE.
     * @param fun Function to call.
     */
    void setAction(int key, int mods, int action, ActionFun fun)
    { mMapping[{key, mods, action}] = fun; }

    /**
     * Reset action for given key combination.
     * @param key Key - e.g. GLFW_KEY_UP.
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
     *  int scancode - Scancode of the key.
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
     * Get callback to this class, compatible with glfwSetKeyCallback.
     * @return Callback function.
     */
    GLFWKeyboardCallbackT callback()
    {
        return keyboardCallbackDispatch;
    }

    /**
     * Set this keyboards callback for given window.
     * @param window Window which should use this keyboard.
     */
    void setCallback(GLFWwindow *window)
    { glfwSetKeyCallback(window, callback()); }
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
     * Dispatches the call to the selected Keyboard object.
     * @param window Window, where the event originated.
     * @param key Keyboard key.
     * @param scancode System-specific keycode.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    static void keyboardCallbackDispatch(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        sSelected->keyboardCallback(window, key, scancode, action, mods);
    }

    /**
     * Callback method called by dispatcher.
     * @param window Window, where the event originated.
     * @param key Keyboard key.
     * @param scancode System-specific keycode.
     * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param mods Key modifiers.
     */
    void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) const
    {
        decltype(mMapping.begin()) search{mMapping.find({key, mods, action})};
        if (search != mMapping.end())
        { // We found an action!
            search->second();
        }
        else if (mDefaultAction)
        {
            mDefaultAction(window, key, scancode, action, mods);
        }
    }

    /// Mapping from keys to actions.
    std::map<KeyCombination, ActionFun> mMapping;
    /// Default action, called when no mapping is found.
    DefaultActionFun mDefaultAction;
    /// Basic empty keyboard.
    static const Keyboard sEmptyKeyboard;
    /// Selected keyboard object, which will receive the callbacks.
    static const Keyboard *sSelected;
protected:
}; // class Keyboard


#endif //SIMPLE_GAME_KEYBOARD_H
