/**
 * @file Font.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Font.h"

namespace ttf
{
    FontCoord::FontCoord(i16 posX, i16 posY, bool isOnCurve) :
        x{posX}, y{posY}, onCurve{isOnCurve}
    { }

    ContourFontCoord::ContourFontCoord() :
        x{0}, y{0}, onCurve{false}
    { }

    void ContourFontCoord::setTo(FontCoord coord)
    {
        x = coord.x;
        y = coord.y;
        onCurve = coord.onCurve;
    }

    FontLetterIterator::FontLetterIterator() :
        FontLetterIterator(0u, nullptr, nullptr)
    { }

    FontLetterIterator::FontLetterIterator(i16 numContours, u16 *contourBegin, FontCoord *coordBegin) :
        mNumContours{numContours}, mContourIterator{contourBegin}, mCoordIterator{coordBegin},
        mNumCoords{0u}, mRemainingContours{numContours}, mRemainingCoords{0u}
    {
        if (mRemainingContours)
        {
            mRemainingContours--;
            u16 newNumCoords = *(mContourIterator++) + 1;
            mRemainingCoords = newNumCoords - mNumCoords;
            mNumCoords = newNumCoords;
            mCoord.newContour = true;

            if (mRemainingCoords == 0u)
            {
                throw std::runtime_error("Contour with zero coordinates!");
            }

            mRemainingCoords--;
            mCoord.setTo(*(mCoordIterator++));
        }
    }

    FontLetterIterator &FontLetterIterator::operator++()
    {
        mCoord.newContour = false;

        if (mRemainingCoords == 0u)
        { // No more coords in current contour.
            if (mRemainingContours)
            { // There are more contours.
                mRemainingContours--;
                u16 newNumCoords = *(mContourIterator++) + 1;
                mRemainingCoords = newNumCoords - mNumCoords;
                mNumCoords = newNumCoords;
                mCoord.newContour = true;

                if (mRemainingCoords == 0u)
                {
                    throw std::runtime_error("Contour with zero coordinates!");
                }
            }
            else
            { // No more contours.
                mNumContours = 0u;
                mContourIterator = nullptr;
                mCoordIterator = nullptr;

                return *this;
            }
        }

        mRemainingCoords--;
        mCoord.setTo(*(mCoordIterator++));

        return *this;
    }

    const ContourFontCoord &FontLetterIterator::operator*() const
    { return mCoord; }
    const ContourFontCoord *FontLetterIterator::operator->() const
    { return &mCoord;}

    bool FontLetterIterator::operator==(const FontLetterIterator &rhs) const
    {
        return mNumContours == rhs.mNumContours
               && mContourIterator == rhs.mContourIterator
               && mCoordIterator == rhs.mCoordIterator;
    }

    bool FontLetterIterator::operator!=(const FontLetterIterator &rhs) const
    { return !(*this == rhs); }

    FontLetter::FontLetter(i16 numContours, u16 *contourBegin, FontCoord *coordBegin) :
        mNumContrours{numContours}, mContourBegin{contourBegin}, mCoordBegin{coordBegin}
    { }

    FontLetterIterator FontLetter::begin() const
    {
        return FontLetterIterator(mNumContrours, mContourBegin, mCoordBegin);
    }

    FontLetterIterator FontLetter::end() const
    {
        return FontLetterIterator();
    }

    bool FontLetter::valid() const
    { return mNumContrours > 0; }

    Font::Font()
    {
        memset(mHolder.mapping, 0, sizeof(mHolder.mapping));
    }

    FontLetter Font::letter(char character)
    {
        if (character < 0 || character > CHAR_MAPPING)
        {
            return FontLetter(0, nullptr, nullptr);
        }

        FontMapping &mapping = mHolder.mapping[character];

        return FontLetter(mapping.numContours, &mHolder.contours[mapping.contourIndex],
                          &mHolder.coordinates[mapping.coordinateIndex]);
    }

    std::ostream &operator<<(std::ostream &out, const Font &font)
    {
        out << "Font info: \n";
        for (std::size_t index = 0; index < CHAR_MAPPING; ++index)
        {
            const FontMapping &mapping = font.holder().mapping[index];
            out << "  Character " << index << "(\"" << static_cast<char>(index) << "\")\n"
                << "    Mapped : " << (mapping.isMapped ? "yes" : "no") << "\n";
            if (mapping.isMapped)
            {
                out << "    Number of contours: " << mapping.numContours << "\n"
                    << "    Contour index: " << mapping.contourIndex << "\n";
                for (std::size_t contourIndex = 0; contourIndex < mapping.numContours; ++contourIndex)
                {
                    out << "      End point: " << font.holder().contours[mapping.contourIndex + contourIndex] << "\n";
                }
                out << "    Coord index: " << mapping.coordinateIndex << "\n";
                std::size_t coordinate{mapping.coordinateIndex};
                for (std::size_t contourIndex = 0; contourIndex < mapping.numContours; ++contourIndex)
                {
                    u16 endpoint{font.holder().contours[mapping.contourIndex + contourIndex]};
                    std::size_t endCoordinate{mapping.coordinateIndex + endpoint};
                    for (;coordinate < endCoordinate; ++coordinate)
                    {
                        const FontCoord &fc = font.holder().coordinates[coordinate];
                        out << "      Coord: " << fc.x << " " << fc.y << " : "
                            << (fc.onCurve ? "On curve" : "Not on curve") << "\n";
                    }
                }
            }
        }

        return out;
    }

    FontHolder &Font::holder()
    { return mHolder; }

    const FontHolder &Font::holder() const
    { return mHolder; }
}
