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
     * @tparam T Used for distinguishing between Universes.
     */
    template <typename T>
    class Universe : NonCopyable
    {
    private:
    public:
        using UniverseT = Universe<T>;

        /**
         * Singleton instance getter.
         * @return Instance of the Universe.
         */
        static UniverseT &instance()
        {
            static UniverseT u;
            return u;
        }

        /**
         * Register given System for this universe.
         * @tparam SystemT Type of the system, has to inherit from base ent::System.
         * @tparam CArgTs System constructor argument types.
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

        /**
         * Register given Component and its ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs ComponentHolder constructor argument types.
         * @param args ComponentHolder constructor arguments, passed to the ComponentHolder on construction.
         * @return Returns ID of the Component.
         */
        template <typename ComponentT,
                  typename... CArgTs>
        u64 registerComponent(CArgTs... args);
    private:
        /**
         * Universe default constructor.
         */
        Universe();

        /// Used for managing Systems.
        SystemManager<UniverseT> mSM;
        /// Used for managing EntityGroups.
        GroupManager<UniverseT> mGM;
        /// Used for managing ComponentHolders.
        ComponentManager<UniverseT> mCM;
        /// Used for managing Entities.
        EntityManager<UniverseT> mEM;
    protected:
    }; // Universe

    template <typename T>
    template <typename SystemT,
              typename... CArgTs>
    SystemT &Universe<T>::addSystem(CArgTs... args)
    {
        static_assert(std::is_base_of<System, SystemT>::value,
                      "The System has to inherit from ent::System!");
        static_assert(std::is_constructible<SystemT, CArgTs...>::value,
                      "Unable to construct System with given constructor parameters!");
        ENT_WARNING("Called unfinished method!");

        static SystemT sys(args...);
        return sys;
    }

    template <typename T>
    template<typename SystemT>
    void Universe<T>::removeSystem()
    {
        static_assert(std::is_base_of<System, SystemT>::value, "The System has to inherit from ent::System!");
        ENT_WARNING("Called unfinished method!");
    }

    template <typename T>
    template <typename ComponentT,
              typename... CArgTs>
    u64 Universe<T>::registerComponent(CArgTs... args)
    {
        static bool registered{false};
        // Check for multiple calls for single Component.
        if (registered)
        {
            ENT_WARNING("registerComponent called multiple times!");
        }

        static const u64 cId{mCM.registerComponent<ComponentT>(std::forward<CArgTs>(args)...)};

        registered = true;

        return cId;
    }

    template <typename T>
    Universe<T>::Universe()
    {

    }

} // namespace ent

#endif //ECS_FIT_UNIVERSE_H
