/**
 * @file Mouse.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Mouse.h"

const Mouse Mouse::sEmptyMouse;
const Mouse *Mouse::sSelected{&Mouse::sEmptyMouse};

void Mouse::mouseCallback(GLFWwindow *window, int key, int action, int mods) const
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

void Mouse::scrollCallback(GLFWwindow *window, double xOffset, double yOffset) const
{
    if (mScrollFun)
    {
        double xPos{0};
        double yPos{0};

        glfwGetCursorPos(window, &xPos, &yPos);

        mScrollFun(xOffset, yOffset, xPos, yPos);
    }
}
