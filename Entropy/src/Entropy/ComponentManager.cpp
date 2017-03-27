/**
 * @file Entropy/Component.cpp
 * @author Tomas Polasek
 * @brief Component is a basic data holding structure in Entropy ECS. Each Entity can have one of
 *        each type of Component.
 */

#include "Entropy/ComponentManager.h"

/// Main Entropy namespace
namespace ent
{
    std::ostream &operator<<(std::ostream &out, const ComponentFilter &rhs)
    {
        out << "rq: " << rhs.mRequire << "; msk: " << rhs.mMask;
        return out;
    }
} // namespace ent

