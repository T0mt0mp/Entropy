/**
 * @file Keyboard.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Keyboard.h"

const Keyboard Keyboard::sEmptyKeyboard;
const Keyboard *Keyboard::sSelected{&Keyboard::sEmptyKeyboard};

void Keyboard::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) const
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
