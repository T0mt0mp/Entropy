/**
 * @file FontFile.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "FontFile.h"

namespace ttf
{
    FontFile::FontFile(const char *filename)
    {
        std::ifstream f(filename, std::ios::in | std::ios::binary | std::ios::ate);
        if (!f)
        {
            throw std::runtime_error("Unable to open the font file!");
        }

        std::size_t size{static_cast<std::size_t>(f.tellg())};
        mData = new u8[size];
        if (!mData)
        {
            throw std::runtime_error("Unable to allocate data buffer!");
        }
        mDataSize = size;

        f.seekg(0, std::ios::beg);
        f.read(reinterpret_cast<char*>(mData), size);

        f.close();
    }

    FontFile::~FontFile()
    {
        if (mData)
        {
            delete[] mData;
        }
    }

    FontFile::FontFile(const FontFile &rhs)
    { copy(rhs); }
    FontFile::FontFile(FontFile &&rhs)
    { swap(std::forward<FontFile>(rhs)); }

    FontFile &FontFile::operator=(const FontFile &rhs)
    { copy(rhs); return *this; }
    FontFile &FontFile::operator=(FontFile &&rhs)
    { swap(std::forward<FontFile>(rhs)); return *this; }

    void FontFile::copy(const FontFile &rhs)
    {
        if (mData)
        {
            delete[] mData;
        }

        mData = new u8[rhs.mDataSize];
        if (!mData)
        {
            throw std::runtime_error("Unable to allocate font data buffer!");
        }
        std::memcpy(mData, rhs.mData, rhs.mDataSize);
        mDataSize = rhs.mDataSize;
    }

    void FontFile::swap(FontFile &&rhs)
    {
        std::swap(mData, rhs.mData);
        std::swap(mDataSize, rhs.mDataSize);
    }
}
