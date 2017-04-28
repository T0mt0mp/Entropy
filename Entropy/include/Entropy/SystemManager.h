/**
 * @file Entropy/System.h
 * @author Tomas Polasek
 * @brief System is a part of the Entropy ECS through which the user gains access to Entities.
 */

#ifndef ECS_FIT_SYSTEM_H
#define ECS_FIT_SYSTEM_H

#include "Types.h"
#include "Util.h"
#include "GroupManager.h"

/// Main Entropy namespace
namespace ent
{
    /**
     * SystemManager is a part of Entropy ECS Universe.
     * Its main purpose is to manage Systems within a one Universe.
     * It contains methods for adding/removing systems, their
     * initialization and refreshing.
     * @tparam UniverseT Type of the Universe, where this class is being used.
     */
    template <typename UniverseT>
    class SystemManager final : NonCopyable
    {
    public:
        /**
         * Construct the System manager for given Universe.
         * @param groupMgr Group manager from the same Universe.
         */
        SystemManager();

        /**
         * Destruct the System manager and all System within.
         */
        ~SystemManager();

        /**
         * Destruct all systems.
         */
        void reset();

        /**
         * Add System of given type to the manager. System is constructed
         * with provided constructor parameters.
         * If there already exists System of given type, the old one
         * is destructed and new one constructed in its place.
         * @tparam SystemT Type of the System.
         * @tparam CArgTs Constructor argument types.
         * @param uni Universe ptr.
         * @param cm Used for resolving Component filter.
         * @param gm Used for creating EntityGroups.
         * @param em Used for adding metadata for EntityGroup.
         * @param cArgs Construct arguments.
         * @return Returns ptr to the constructed System.
         */
        template <typename SystemT,
                  typename... CArgTs>
        SystemT *addSystem(UniverseT *uni, const ComponentManager<UniverseT> &cm,
                           EntityManager &em,
                           GroupManager<UniverseT> &gm, CArgTs... cArgs);

        /**
         * Is there System of this type?
         * @tparam SystemT Type of the System.
         * @return Returns true, if there is such a System.
         */
        template <typename SystemT>
        inline bool hasSystem() const;

        /**
         * Get System with given type.
         * @tparam SystemT Type of the System.
         * @return Returns ptr to the System object, or nullptr
         *   if the System has not been added.
         */
        template <typename SystemT>
        SystemT *getSystem() const;

        /**
         * Remove System with given type and decrement the
         * usage counter within its group.
         * @tparam SystemT Type of the System.
         * @return Returns true, if the System has been
         *   removed successfully.
         */
        template <typename SystemT>
        bool removeSystem();
    private:
        /**
         * Container for the System.
         * @tparam SystemT Type of the System.
         */
        template <typename SystemT>
		static ConstructionHandler<SystemT> &system()
		{
			static ConstructionHandler<SystemT> sys;
			return sys;
		}

        /// List of things to destruct, when system reset occurs.
        std::vector<std::function<void()>> mDestructOnReset;
    protected:
    }; // SystemManager

    /**
     * System in Entropy ECS contains the means to iterate
     * over its EntityGroup and other utility methods.
     * It is meant to be inherited by the System implementation
     * which can contain the logic.
     * @tparam UniverseT Type of the Universe.
     */
    template <typename UniverseT>
    class System : NonCopyable
    {
    public:
        friend class SystemManager<UniverseT>;

        /// Destroy System and notify Group, its not being used anymore.
        virtual ~System();

        /// Check, if the System is ready for use.
        bool isInitialized() const;

        /**
         * Iterator for iterating trough Entities within the group.
         * @return Returns iterator for iterating through Entities withing the group.
         */
        EntityList<UniverseT, EntityGroup::EntityListT> foreach();

        /**
         * Get object used for parallel iteration over Entities.
         * @param numThreads How many threads will be used.
         * @return Returns the generator object, which can be used (forThread method) to
         *   get the iteration objects.
         */
        EntityListParallel<UniverseT, EntityGroup::EntityListT, false> foreachP(u64 numThreads);

        /**
         * Iterator for iterating trough Entities which were added since the last refresh.
         * @return Returns iterator for iterating through Entities which were added since the last refresh.
         */
        EntityList<UniverseT, EntityGroup::AddedListT> foreachAdded();

        /**
         * Get object used for parallel iteration over added Entities.
         * @param numThreads How many threads will be used.
         * @return Returns the generator object, which can be used (forThread method) to
         *   get the iteration objects.
         */
        EntityListParallel<UniverseT, EntityGroup::AddedListT, false> foreachAddedP(u64 numThreads);

        /**
         * Iterator for iterating trough Entities which were removed since the last refresh.
         * !!Warning: The Entities withing this Group may not exist anymore!!
         * @return Returns iterator for iterating through Entities which were removed since the last refresh.
         */
        EntityList<UniverseT, EntityGroup::RemovedListT> foreachRemoved();

        /**
         * Get object used for parallel iteration over removed Entities.
         * @param numThreads How many threads will be used.
         * @return Returns the generator object, which can be used (forThread method) to
         *   get the iteration objects.
         */
        EntityListParallel<UniverseT, EntityGroup::RemovedListT, false> foreachRemovedP(u64 numThreads);

        /// Filter getter.
        const EntityFilter &filter() const;

        /// Group ID getter.
        u64 groupId() const;
    private:
        /**
         * Set Entity Group containing Entities which are
         * of interest to this System.
         * @param grp Pointer to the Group.
         */
        void setGroup(EntityGroup *grp);

        /**
         * Set Universe, for this System.
         * @param uni Universe ptr.
         */
        void setUniverse(UniverseT *uni);

        /// Flag used for signifying, that this System is ready for use.
        bool mInitialized;
        /// Entity group containing Entities, which are of interest to this System.
        EntityGroup *mGroup;
        /// Universe this System works for.
        UniverseT *mUniverse;
    protected:
    }; // class System
} // namespace ent

#include "SystemManager.inl"

#endif //ECS_FIT_SYSTEM_H
