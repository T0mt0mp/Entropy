/**
 * @file Entropy/Util.inl
 * @author Tomas Polasek
 * @brief Utility functions/classes used in Entropy ECS.
 */

#include "Util.h"

/// Main Entropy namespace
namespace ent
{
    static ENT_CONSTEXPR_FUN u64 pow2RoundUp(u64 value)
    {
        --value;
        value |= value >> 0b1;
        value |= value >> 0b10;
        value |= value >> 0b100;
        value |= value >> 0b1000;
        value |= value >> 0b10000;
        value |= value >> 0b100000;

        return value + 1;
    }

    // InfoBitset implementation.
    template <u64 N>
    bool InfoBitset<N>::all() const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != BLOCK_ONE)
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the last block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) !=
                (BLOCK_ONE & partlyUsedBlockMask()))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 N>
    void InfoBitset<N>::copy(const InfoBitset &other)
    {
        std::memcpy(mMemory, other.mMemory, sizeof(mMemory));
    }

    template <u64 N>
    void InfoBitset<N>::swap(InfoBitset &other)
    {
        for (u64 block = 0; block < NUM_BLOCKS; ++block)
        {
            std::swap(mMemory[block], other.mMemory[block]);
        }
    }

    template <u64 N>
    std::string InfoBitset<N>::toString() const
    {
        std::string out;
        out.resize(NUM_BITS);

        for (u64 bit = 0u; bit < NUM_BITS; ++bit)
        {
            out[bit] = testImpl(bit) ? '1' : '0';
        }

        return out;
    }

    template <u64 N>
    auto InfoBitset<N>::operator&(const InfoBitset &rhs) const -> InfoBitset
    {
        InfoBitset result;

        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            result.getBlock(block) = getBlock(block) & rhs.getBlock(block);
        }

        return result;
    }

    template <u64 N>
    auto InfoBitset<N>::operator&=(const InfoBitset &rhs) -> InfoBitset&
    {
        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            getBlock(block) &= rhs.getBlock(block);
        }

        return *this;
    }

    template <u64 N>
    auto InfoBitset<N>::operator|(const InfoBitset &rhs) const -> InfoBitset
    {
        InfoBitset result;

        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            result.getBlock(block) = getBlock(block) | rhs.getBlock(block);
        }

        return result;
    }

    template <u64 N>
    auto InfoBitset<N>::operator|=(const InfoBitset &rhs) -> InfoBitset&
    {
        for (u64 block = 0u; block < NUM_BLOCKS; ++block)
        {
            getBlock(block) &= rhs.getBlock(block);
        }

        return *this;
    }

    template <u64 N>
    bool InfoBitset<N>::operator==(const InfoBitset &rhs) const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != rhs.getBlock(block))
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) !=
                (rhs.getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 M>
    std::ostream &operator<<(std::ostream &out, const InfoBitset<M> &rhs)
    {
        out << rhs.toString();
        return out;
    }

    template <u64 N>
    template <typename T>
    void InfoBitset<N>::fromVal(const T &val)
    {
        std::memcpy(mMemory, &val, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::setImpl()
    {
        std::memset(mMemory, BYTE_ONE, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::setImpl(u64 pos)
    {
        getBlock(memBlock(pos)) |= bitMask(pos);
    }

    template <u64 N>
    void InfoBitset<N>::resetImpl()
    {
        std::memset(mMemory, BYTE_ZERO, NUM_USED_BYTES);
    }

    template <u64 N>
    void InfoBitset<N>::resetImpl(u64 pos)
    {
        getBlock(memBlock(pos)) &= ~bitMask(pos);
    }

    template <u64 N>
    u64 InfoBitset<N>::countImpl() const
    {
        u64 result{0u};

        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            result += popcount64(getBlock(block));
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            result += popcount64(getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask());
        }

        return result;
    }

    template <u64 N>
    bool InfoBitset<N>::noneImpl() const
    {
        for (u64 block = 0u; block < NUM_WHOLE_BLOCKS; ++block)
        {
            if (getBlock(block) != static_cast<BMBType>(0u))
            {
                return false;
            }
        }

        if (PARTLY_USED)
        { // Take care of the partly used memory block.
            if ((getBlock(NUM_BLOCKS - 1u) & partlyUsedBlockMask()) != static_cast<BMBType>(0u))
            {
                return false;
            }
        }

        return true;
    }

    template <u64 N>
    ENT_CONSTEXPR_FUN bool InfoBitset<N>::testImpl(u64 pos) const
    {
        return getBlock(memBlock(pos)) & bitMask(pos);
    }

    template <u64 N>
    bool InfoBitset<N>::testAndSetImpl(std::size_t pos, bool val)
    {
        BMBType &block(getBlock(memBlock(pos)));
        bool old{static_cast<bool>(block & bitMask(pos))};
        //     | reset the tested bit | -> | set the requested value |
        block = (block & (~bitMask(pos))) | (static_cast<BMBType>(val) << pos);

        return old;
    }
    // InfoBitset implementation end.
} // namespace ent
