/**
 * @file Entropy/System.h
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#ifndef ECS_FIT_SYSTEM_H
#define ECS_FIT_SYSTEM_H

#include "Types.h"
#include "Util.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * SystemManager base class containing code which does not need to be templated.
     */
    class SystemManagerBase : NonCopyable
    {
    public:
    private:
    protected:
    }; // SystemManagerBase

    /**
     * SystemManager is a part of Entropy ECS Universe.
     * Its main purpose is to manage Systems within a one Universe.
     * It contains methods for adding/removing systems, their
     * initialization and refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class SystemManager final : public SystemManagerBase
    {
    public:
    private:
    protected:
    }; // SystemManager

    /**
     * System in Entropy ECS contains the means to iterate
     * over its EntityGroup and other utility methods.
     * It is meant to be inherited by the System implementation
     * which can contain the logic.
     */
    class System : NonCopyable
    {
    public:
    private:
    protected:
    }; // System
} // namespace ent

#endif //ECS_FIT_SYSTEM_H
