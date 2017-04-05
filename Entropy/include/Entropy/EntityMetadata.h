/**
 * @file Entropy/EntityMetadata.h
 * @author Tomas Polasek
 * @brief Classes for handling of Entity metadata.
 */

#ifndef ECS_FIT_ENTITYMETADATA_H
#define ECS_FIT_ENTITYMETADATA_H

/// Main Entropy namespace
namespace ent
{
    /**
     * Holder for Entity metadata.
     * Structure:
     *   Each Entity is represented by one row in the table.
     *   Each Entity has current value of its generation number.
     *   Each Entity has one bit representing its activity.
     *   Each Entity has a number of bits representing if it
     *     has a Component of specified type present.
     *   Each Entity has a number of bits representing its
     *     presence in EntityGroups.
     * Bits are stored in bitsets, where multiple Entities
     * share (specified in ENT_BITSET_GROUP_SIZE) a bitset
     * with multiple other Entities.
     * This means the bitsets span multiple Entities, representing
     * blocks of bits for multiple rows in the table.
     * This allows parallel access to different bitset groups
     * and bitset columns.
     */
    class EntityMetadata
    {
    public:
    private:
    protected:
    }; // class EntityMetadata
} // namespace ent

#endif //ECS_FIT_ENTITYMETADATA_H
