/**
 * @file sys/Types.h
 * @author Tomas Polasek
 * @brief Types used in Entity Component System.
 */

#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#include <functional>
#include <bitset>
#include <vector>
#include <deque>

#include "util/Types.h"
#include "util/Assert.h"
#include "util/Meta.h"

namespace ecs
{
    template <typename... Ts>
    using ComponentList = mp_list<Ts...>;
    template <typename... Ts>
    using TagList = mp_list<Ts...>;
    template <typename... Ts>
    using Signature = mp_list<Ts...>;
    template <typename... Ts>
    using SignatureList = mp_list<Ts...>;

    /// Type for the entity ID.
    using IdType = u32;

    /**
     * Entity Component System configuration.
     * @param TComponents Type list of components.
     * @param TTags Type list of tags.
     * @param TSignatures Type list of signatures.
     */
    template <
        typename TComponents,
        typename TTags,
        typename TSignatures>
    class Config;

    // Template implementation
    template <
        typename TComponents,
        typename TTags,
        typename TSignatures>
    class Config
    {
    public:
        static_assert(mp_are_specializations_v<mp_list, TComponents,
                                               TTags, TSignatures>);

        using ComponentList = TComponents;
        using TagList = TTags;
        using SignatureList = TSignatures;

        /**
         * Get the number of component types.
         * @return The number of component types.
         */
        static constexpr u64 componentCount() noexcept
        {
            return mp_size_v<ComponentList>;
        }

        /**
         * Get the number of tag types.
         * @return The number of tag types.
         */
        static constexpr u64 tagCount() noexcept
        {
            return mp_size_v<TagList>;
        }

        /**
         * Get the number of signature types.
         * @return The number of signature types.
         */
        static constexpr u64 signatureCount() noexcept
        {
            return mp_size_v<SignatureList>;
        }

        /**
         * Helper method for getting index of a bit,
         * for either a component or a tag type.
         * @return Bit index.
         */
        template <typename T>
        static constexpr
        mp_fail_on_false_t<mp_contains_v<T, ComponentList>, u64>
        bitsetTypeBit() noexcept
        {
            return componentBit<T>();
        }

        /**
         * Helper method for getting index of a bit,
         * for either a component or a tag type.
         * @return Bit index.
         */
        template <typename T>
        static constexpr
        mp_fail_on_false_t<mp_contains_v<T, TagList>, u64>
        bitsetTypeBit() noexcept
        {
            return tagBit<T>();
        }

        /**
         * Get the size of the bitset in bits.
         * @return The size of the bitset.
         */
        static constexpr u64 bitsetSize() noexcept
        {
            return tagCount() + componentCount();
        }

        using BitMask = std::bitset<bitsetSize()>;
        static constexpr BitMask EMPTY_MASK{0};

        /**
         * Is given template argument a component type?
         * @return Returns true, if the template argument is a component type.
         */
        template <typename TComponent>
        static constexpr bool isComponent() noexcept
        {
            return mp_contains_v<TComponent, ComponentList>;
        }

        /**
         * Is given template argument a tag type?
         * @return Returns true, if the template argument is a tag type.
         */
        template <typename TTag>
        static constexpr bool isTag() noexcept
        {
            return mp_contains_v<TTag, TagList>;
        }

        /**
         * Is given template argument a signature type?
         * @return Returns true, if the template argument is a signature type.
         */
        template <typename TSignature>
        static constexpr bool isSignature() noexcept
        {
            return mp_contains_v<TSignature, SignatureList>;
        }

        /**
         * Check if given type is a component, tag or signature.
         * @return Returns true, if the given type is one of the above.
         */
        template <typename T>
        static constexpr bool isCTS() noexcept
        {
            return isComponent<T>() || isTag<T>() || isSignature<T>();
        }

        /**
         * Get ID of given component type.
         * Only unique between all component types!
         * @return Unique component type ID.
         */
        template <typename TComponent>
        static constexpr u64 componentId() noexcept
        {
            static_assert(isComponent<TComponent>());
            return mp_index_of_v<TComponent, ComponentList>;
        }

        /**
         * Get ID of given tag type.
         * Only unique between all tag types!
         * @return Unique tag type ID.
         */
        template <typename TTag>
        static constexpr u64 tagId() noexcept
        {
            static_assert(isTag<TTag>());
            return mp_index_of_v<TTag, TagList>;
        }

        /**
         * Get ID of given signature type.
         * Only unique between all signature types!
         * @return Unique signature type ID.
         */
        template <typename TSignature>
        static constexpr u64 signatureId() noexcept
        {
            static_assert(isSignature<TSignature>());
            return mp_index_of_v<TSignature, SignatureList>;
        }

        /**
         * Get the index of a bit, which signifies presence of this
         * type of component.
         * Component bits are located from the LSb.
         * @return Index of the bit.
         */
        template <typename TComponent>
        static constexpr u64 componentBit() noexcept
        {
            static_assert(isComponent<TComponent>());
            return componentId<TComponent>();
        }

        /**
         * Get the index of a bit, which signifies presence of this
         * type of tag.
         * Tag bits are located from the LSb + number of component types.
         * @return Index of the bit.
         */
        template <typename TTag>
        static constexpr u64 tagBit() noexcept
        {
            static_assert(isTag<TTag>());
            return tagId<TTag>() + componentCount();
        }

        /**
         * Get the bit mask of given component or tag.
         * @return Bit mask for given component or tag.
         */
        template <typename T>
        static constexpr
        mp_fail_on_false_t<mp_contains_v<T, ComponentList> |
                           mp_contains_v<T, TagList>, BitMask>
        mask() noexcept
        {
            return BitMask(mBitMask<T>);
        }

        /**
         * Get the mask for given signature.
         * @return Bit mask for given signature.
         */
        template <typename T>
        static constexpr
        mp_fail_on_false_t<mp_contains_v<T, SignatureList>, BitMask>
        mask() noexcept
        {
            using MaskList = mp_transform_t<MaskReader, T>;

            return BitMask(
                mp_accumulate_v<std::bit_or<u64>, MaskList>);
        }
    private:
        /// BitMask storage.
        template <typename T>
        static constexpr u64 mBitMask{
            static_cast<u64>(1) << bitsetTypeBit<T>()
        };

        template <typename T>
        struct MaskReader
        {
            static constexpr u64 value = mBitMask<T>;
        };
    protected:
    };
}

#endif //SYS_TYPES_H
