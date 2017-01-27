/**
 * @file Entropy/Universe.h
 * @author Tomas Polasek
 * @brief Universe is the wrapper around all other parts of the Entropy ECS.
 */

#ifndef ECS_FIT_UNIVERSE_H
#define ECS_FIT_UNIVERSE_H

#include "System.h"
#include "Component.h"
#include "Entity.h"
#include "Group.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * Universe is the main wrapper class around all other
     * functionality of Entropy ECS.
     * It contains management classes for different
     * aspects of the system and methods to access them.
     * @tparam N Used for distinguishing between Universes.
     */
    template <u64 N>
    class Universe : NonCopyable
    {
    public:
        using ThisType = Universe<N>;

        /**
         * Universe default constructor.
         */
        Universe() = default;

        /**
         * Register given System for this universe.
         * @tparam SystemT Type of the system, has to inherit from base ent::System.
         * @tparam CArgTs System constructor arguments types.
         * @param args System constructor arguments, passed to the System on construction.
         * @return Returns reference to the newly registered System.
         */
        template <typename SystemT,
                  typename... CArgTs>
        SystemT& addSystem(CArgTs... args);

        /**
         * Remove a registered System.
         * @tparam SystemT Type of the System.
         */
        template <typename SystemT>
        void removeSystem();
    private:
        /// Used for managing Systems.
        SystemManager<ThisType> mSM;
        /// Used for managing EntityGroups.
        GroupManager<ThisType> mGM;
        /// Used for managing ComponentHolders.
        ComponentManager<ThisType> mCM;
        /// Used for managing Entities.
        EntityManager<ThisType> mEM;
    protected:
    }; // Universe

    template <u64 N>
    template <typename SystemT,
              typename... CArgTs>
    SystemT &Universe<N>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<System, SystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<SystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");
        ENT_WARNING("Called unfinished method!");

        static SystemT sys(args...);
        return sys;
    }

    template <u64 N>
    template<typename SystemT>
    void Universe<N>::removeSystem()
    {
        static_assert(std::is_base_of<System, SystemT>::value, "The System has to inherit from ent::System!");
        ENT_WARNING("Called unfinished method!");
    }

} // namespace ent

#endif //ECS_FIT_UNIVERSE_H
