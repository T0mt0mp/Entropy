/**
 * @file Entropy/System.cpp
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#include "Entropy/System.h"

/// Main Entropy namespace
namespace ent
{
    // System implementation.
    void System::setGroup(EntityGroup *grp)
    {
        mGroup = grp;
        mInitialized = true;
    }
    // System implementation end.
} // namespace ent
