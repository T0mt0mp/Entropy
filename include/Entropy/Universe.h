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
    private:
        /// Component ID generator.
        class ComponentIdGenerator : public ClassIdGenerator<ComponentIdGenerator> {};
        /// Used for extracting Component type.
        template <typename HolderT>
        struct ComponentTypeExtractor;
        /// Used for extracting Component type.
        template <template <typename, typename...> typename HolderTT,
            typename... Other,
            typename ComponentTT>
        struct ComponentTypeExtractor<HolderTT<ComponentTT, Other...>>
        {
            using ComponentT = ComponentTT;
            using HolderT = HolderTT<ComponentTT, Other...>;
        };
    public:
        using ThisType = Universe<N>;

        /**
         * Singleton instance getter.
         * @return Instance of the Universe.
         */
        static Universe &instance()
        {
            static Universe u;
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
         * @tparam HolderT Type of the ComponentHolder.
         * @tparam ComponentT Type of the Component.
         * @tparam CArgTs ComponentHolder constructor argument types.
         * @tparam B Used for inner template generation hacks.
         * @tparam ID Used for inner template generation hacks.
         * @param args ComponentHolder constructor arguments, passed to the ComponentHolder on construction.
         */
        template <typename HolderT,
                  typename ComponentT = typename ComponentTypeExtractor<HolderT>::ComponentT,
                  bool B = ComponentIdGenerator::template generated<ComponentT>(),
                  u64 ID = ComponentIdGenerator::template getId<ComponentT>(),
                  typename... CArgTs>
        void registerComponent(CArgTs... args);
    private:
        /**
         * Universe default constructor.
         */
        Universe();

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

    std::ostream &operator<<(std::ostream &out, const EntityId &id)
    {
        out << "E(" << id.index() << ":" << id.generation() << ")";
        return out;
    }

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

    template <u64 N>
    template <typename HolderT,
              typename ComponentT,
              bool B,
              u64 ID,
              typename... CArgTs>
    void Universe<N>::registerComponent(CArgTs... args)
    {
        static_assert(!B, "Each component type can have only one holder!");
        constexpr u64 cId{ID};
        ENT_UNUSED(cId);
        ENT_WARNING("Called unfinished method!");
    }

    template <u64 N>
    Universe<N>::Universe()
    {

    }

} // namespace ent

#endif //ECS_FIT_UNIVERSE_H
