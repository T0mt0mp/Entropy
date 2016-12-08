/**
 * @file ecs/EntityManager.h
 * @author Tomas Polasek
 * @brief Manager class for managing entities.
 */

#ifndef ECS_ENTITYMANAGER_H
#define ECS_ENTITYMANAGER_H

#include <iostream>

#include "ecs/Types.h"

namespace ecs
{
    /**
     * Unique ID representing an entity in the
     * Entity Component System.
     */
    class EntityHandle
    {
    public:
        /// Type of the packed id.
        using IdType = ecs::IdType;
        /// Starting generation number.
        static constexpr IdType START_GEN{1};

        /**
         * Compose an entity ID from index and generation number.
         * @param index Index part of the ID.
         * @param generation Which generation is this entity.
         */
        inline constexpr EntityHandle(IdType index = 0, IdType generation = 0);

        /**
         * Get packed identifier.
         * @return Packed identifier.
         */
        inline constexpr IdType id() const;

        /**
         * Get the index part of this identifier.
         * @return Index starting at the LSb.
         */
        inline constexpr IdType index() const;

        /**
         * Get the generation part of this identifier.
         * @return Generation starting at the LSb.
         */
        inline constexpr IdType generation() const;
    private:
        /**
         * Move the generation bits to the left of the IdType
         * @param rGen Generation with information bits on the right.
         * @return Generation bits shifted to the MSbits.
         */
        static constexpr IdType rGenToLGen(IdType rGen)
        {
            return rGen << INDEX_BITS;
        }

        /**
         * Move the generation bits to the right of the IdType
         * @param rGen Generation with information bits on the left.
         * @return Generation bits shifted to the LSbits.
         */
        static constexpr IdType lGenToRGen(IdType lGen)
        {
            return lGen >> INDEX_BITS;
        }

        /**
         * Get the index part of the packed identifier.
         * @param packedId Packed identifier
         * @return Index part starting at the LSb.
         */
        static constexpr IdType indexPart(IdType packedId)
        {
            return packedId & INDEX_MASK;
        }

        /**
         * Get the generation part of the packed identifier.
         * @param packedId Packed identifier
         * @return Generation part starting at the LSb.
         */
        static constexpr IdType genPart(IdType packedId)
        {
            return lGenToRGen(packedId);
        }

        /**
         * Combine index and generation information into one IdType.
         * @param index Index of this entity identifier.
         * @param gen Generation of this entity, starts on the LSb.
         */
        static constexpr IdType combineGenIndex(IdType index, IdType rGen)
        {
            return rGenToLGen(rGen) | indexPart(index);
        }

        /**
         * Packed identifier information :
         *  GEN_BITS on the left,
         *  INDEX_BITS on the right.
         */
        IdType mId;

        /// Zero represented in as the IdType.
        static constexpr IdType ZERO{0};

        /**
         * Number of bits representing the index part of the ID.
         *  24b - index (~16M)
         */
        static constexpr u64 INDEX_BITS{24};
        /**
         * Number of bits representing the generation part of the ID.
         *  8b  - generation (256)
         */
        static constexpr u64 GEN_BITS{sizeof(IdType) * 8 - INDEX_BITS};
        static_assert((INDEX_BITS + GEN_BITS) == sizeof(IdType) * 8);

        /// Mask used for masking out the index bits.
        static constexpr IdType GEN_MASK{~(ZERO) << INDEX_BITS};
        /// Mask used for masking out the generation bits.
        static constexpr IdType INDEX_MASK{~GEN_MASK};
    protected:
    public:
        /// Maximum number of entities.
        static constexpr IdType MAX_ENTITIES{INDEX_MASK};
        /// Maximum generations.
        static constexpr IdType MAX_GENS{GEN_MASK};
    };

    /**
     * Management class for storing information about entities.
     */
    template <typename Config>
    class EntityManager
    {
    public:
        /// Type for the bitmask.
        using BitMask = typename Config::BitMask;

        EntityManager(const EntityManager&) = delete;
        EntityManager &operator=(const EntityManager&) = delete;

        EntityManager();
        ~EntityManager();

        /**
         * Check if given EntityHandle is valid.
         * @return Returns true, if the EntityHandle is valid.
         */
        inline bool valid(EntityHandle handle)
        {
            return entityValid(handle);
        }

        /**
         * Create a new entity and return a handle to it.
         * @return Handle to crated entity.
         */
        inline EntityHandle create()
        {
            return createEntity();
        }

        /**
         * Delete given entity.
         * @param handle Specifies which entity to delete.
         */
        inline void destroy(EntityHandle handle)
        {
            return deleteEntity(handle);
        }

        /**
         * Get the number of entities.
         * @return The current number of entities.
         */
        inline u64 numEntities() const
        {
            return mEC.gens.size();
        }

        template <typename... CompTs>
        friend class EntityIterator;

        /// Used to iterate through entities.
        template <typename... CompTs>
        class EntityIterator
        {
        public:
            using CompTsList = mp_list<CompTs...>;
            using Signature = mp_rename_t<
                mp_transform_t<mp_remove_const_t, CompTsList>,
                ecs::Signature>;

            EntityIterator(EntityManager *mgr)
            {
            }
        private:
            static constexpr BitMask MASK{Config::template mask<Signature>()};
        protected:
        };

        template <typename... CompTs>
        auto testIter()
        {
            EntityIterator<CompTs...> it(this);
            return it;
        }
    private:
        /**
         * Holds information about entities.
         * Structure of arrays.
         * Entity is and imaginary object with following
         * members:
         *  index/generation
         *  bitset indicating which components are present
         */
        struct EntityContainer
        {
            std::vector<EntityHandle::IdType> gens;
            std::vector<BitMask> masks;
            std::deque<EntityHandle::IdType> free;
        };

        /**
         * Is given entity handle a valid handle?
         * @return Returns true, iff the EntityHandle is valid.
         */
        inline bool entityValid(EntityHandle handle);

        /**
         * Create a new entity and return its handle.
         * @return Handle for the newly created entity.
         */
        inline EntityHandle createEntity();

        /**
         * Delete entity represented by given EntityHandle.
         * @param handle EntityHandle representing entity to be deleted.
         */
        inline void deleteEntity(EntityHandle handle);
        
        /// Contains entity data.
        EntityContainer mEC;
        /// Minimum number of free indices, before they are used.
        static constexpr u64 MIN_FREE{8};
    protected:
    };

    inline constexpr EntityHandle::EntityHandle(IdType index, IdType generation) :
        mId{combineGenIndex(index, generation)}
    {

    }

    constexpr EntityHandle::IdType EntityHandle::id() const
    {
        return mId;
    }

    constexpr EntityHandle::IdType EntityHandle::index() const
    {
        return indexPart(mId);
    }

    constexpr EntityHandle::IdType EntityHandle::generation() const
    {
        return genPart(mId);
    }

    template <typename Config>
    EntityManager<Config>::EntityManager()
    {

    }

    template <typename Config>
    EntityManager<Config>::~EntityManager()
    {

    }

    template <typename Config>
    bool EntityManager<Config>::entityValid(EntityHandle handle)
    {
        auto index = handle.index();
        if (index < mEC.gens.size())
        {
            return mEC.gens[index] == handle.generation();
        }
        else
        {
            return false;
        }
    }

    template <typename Config>
    EntityHandle EntityManager<Config>::createEntity()
    {
        EntityHandle::IdType index{0};
        EntityHandle::IdType gen{0};

        // If there are not enough free indices, use a new one.
        if (mEC.free.size() > MIN_FREE)
        {
            index = mEC.free.front();
            mEC.free.pop_front();
            gen = mEC.gens[index];
        }
        else
        {
            ASSERT_SLOW(numEntities() + 1 < EntityHandle::MAX_ENTITIES);
            mEC.gens.push_back(EntityHandle::START_GEN);
            mEC.masks.push_back(BitMask(0));
            // TODO - assure maximum size.
            index = static_cast<EntityHandle::IdType>(mEC.gens.size() - 1);
            gen = EntityHandle::START_GEN;
        }

        return EntityHandle(index, gen);
    }

    template <typename Config>
    void EntityManager<Config>::deleteEntity(EntityHandle handle)
    {
        bool valid = entityValid(handle);

        if (valid)
        {
            EntityHandle::IdType index = handle.index();

            ASSERT_SLOW(mEC.gens[index] < EntityHandle::MAX_GENS);
            mEC.gens[index]++;

            mEC.masks[index] = 0;

            mEC.free.push_back(index);
        }
        else
        {
            return;
        }
    }
}

#endif //ECS_ENTITYMANAGER_H
