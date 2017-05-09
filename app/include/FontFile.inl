/**
 * @file FontFile.inl
 * @author Tomas Polasek
 * @brief 
 */

#ifndef TTF_TEST_FONTFILE_INL
#define TTF_TEST_FONTFILE_INL

#include "FontFile.h"

namespace ttf
{
    size_t FontFile::size() const
    {
        return mDataSize;
    }

    void FontFile::pushCursor()
    {
        mCursorMemory = mCursor;
    }

    void FontFile::popCursor()
    {
        mCursor = mCursorMemory;
    }

    void FontFile::moveTo(std::size_t newPosition)
    {
        mCursor = newPosition;
    }

    void FontFile::moveBy(std::size_t addPosition)
    {
        mCursor += addPosition;
    }

    std::size_t FontFile::cursorPos() const
    {
        return mCursor;
    }

    template <typename T>
    void FontFile::readFTo(std::size_t from, T &to, std::size_t size)
    {
        u8 *resArray{reinterpret_cast<u8*>(&to)};
        for (std::size_t iii = 0; iii < size; ++iii)
        {
            resArray[size - iii - 1] = mData[from + iii];
        }
    }

    template <typename T>
    void FontFile::readFToWR(std::size_t from, T &to, std::size_t size)
    {
        u8 *resArray{reinterpret_cast<u8*>(&to)};
        for (std::size_t iii = size; iii > 0; --iii)
        {
            resArray[iii] = mData[from + iii];
        }
    }

    template <typename T>
    void FontFile::readTo(T &to, std::size_t size)
    {
        u8 *resArray{reinterpret_cast<u8*>(&to)};
        for (std::size_t iii = 0; iii < size; ++iii)
        {
            resArray[size - iii - 1] = mData[mCursor + iii];
        }
        mCursor += size;
    }

    template <typename T>
    void FontFile::readToWR(T &to, std::size_t size)
    {
        u8 *resArray{reinterpret_cast<u8*>(&to)};
        for (std::size_t iii = 0; iii < size; ++iii)
        {
            resArray[iii] = mData[mCursor + iii];
        }
        mCursor += size;
    }

    template<typename T>
    void FontFile::readFloat(float &to, std::size_t bias)
    {
        T input;
        readTo(input);
        to = static_cast<float>(input) / (1 << bias);
    }

    template<typename T>
    void FontFile::readFloatF(std::size_t from, float &to, std::size_t bias)
    {
        T input;
        readTo(from, input);
        to = static_cast<float>(input) / (1 << bias);
    }

    void FontFile::readShortFracTo(ShortFrac &to)
    {
        readFloat<i16>(to, SHORT_FRAC_BIAS);
    }

    void FontFile::readShortFracFTo(std::size_t from, ShortFrac &to)
    {
        readFloatF<i16>(from, to, SHORT_FRAC_BIAS);
    }

    void FontFile::readFixedTo(Fixed &to)
    {
        readFloat<i32>(to, FIXED_BIAS);
    }

    void FontFile::readFixedFTo(std::size_t from, Fixed &to)
    {
        readFloatF<i32>(from, to, FIXED_BIAS);
    }

    void FontFile::readF2Dot14To(F2Dot14 &to)
    {
        readFloat<i16>(to, F2DOT14_BIAS);
    }

    void FontFile::readF2Dot14FTo(std::size_t from, F2Dot14 &to)
    {
        readFloatF<i16>(from, to, F2DOT14_BIAS);
    }

    template <typename T>
    T FontFile::read()
    {
        T tmp;
        readTo(tmp);
        return tmp;
    }

    template <typename T>
    inline T FontFile::read(std::size_t from)
    {
        T tmp;
        readFTo(from, tmp);
        return tmp;
    }

    void FontFile::readFTo(std::size_t from, u8 &to)
    {
        to = mData[from++];
    }

    void FontFile::readFTo(std::size_t from, u8 *to)
    {
        *to = mData[from++];
    }

    void FontFile::readTo(u8 &to)
    {
        to = mData[mCursor++];
    }

    void FontFile::readTo(u8 *to)
    {
        *to = mData[mCursor++];
    }

    u8 FontFile::read8u()
    { return read<u8>(); }
    u8 FontFile::read8u(std::size_t from)
    { return read<u8>(from); }
    i8 FontFile::read8i()
    { return read<i8>(); }
    i8 FontFile::read8i(std::size_t from)
    { return read<i8>(from); }

    u16 FontFile::read16u()
    { return read<u16>(); }
    u16 FontFile::read16u(std::size_t from)
    { return read<u16>(from); }
    i16 FontFile::read16i()
    { return read<i16>(); }
    i16 FontFile::read16i(std::size_t from)
    { return read<i16>(from); }

    u32 FontFile::read32u()
    { return read<u32>(); }
    u32 FontFile::read32u(std::size_t from)
    { return read<u32>(from); }
    i32 FontFile::read32i()
    { return read<i32>(); }
    i32 FontFile::read32i(std::size_t from)
    { return read<i32>(from); }
}

#endif //TTF_TEST_FONTFILE_INL
