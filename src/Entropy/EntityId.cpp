/**
 * @file Entropy/EntityId.cpp
 * @author Tomas Polasek
 * @brief Entity ID class.
 */

#include "Entropy/EntityId.h"

/// Main Entropy namespace
namespace ent
{
    std::ostream &operator<<(std::ostream &out, const EntityId &id)
    {
        out << "E(" << id.index() << ":" << id.generation() << ")";
        return out;
    }

} // namespace ent
