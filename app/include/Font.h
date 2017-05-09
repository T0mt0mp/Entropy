/**
 * @file Font.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef TTF_TEST_FONT_H
#define TTF_TEST_FONT_H

#include "Types.h"

namespace ttf
{
    struct FontCoord
    {
        FontCoord(i16 posX, i16 posY, bool isOnCurve);

        i16 x;
        i16 y;
        bool onCurve;
    };

    struct ContourFontCoord
    {
        ContourFontCoord();

        void setTo(FontCoord coord);

        i16 x;
        i16 y;
        bool onCurve;
        bool newContour;
    };

    struct FontMapping
    {
        std::size_t contourIndex;
        i16 numContours;
        std::size_t coordinateIndex;
        bool isMapped;
    };

    struct FontHolder
    {
        FontMapping mapping[CHAR_MAPPING];
        std::vector<u16> contours;
        std::vector<FontCoord> coordinates;
    };

    class FontLetterIterator
    {
    public:
        /// End iterator
        FontLetterIterator();
        /// Begin iterator
        FontLetterIterator(i16 numContours, u16 *contourBegin, FontCoord *coordBegin);

        /// Move to next coordinate.
        FontLetterIterator &operator++();

        const ContourFontCoord &operator*() const;
        const ContourFontCoord *operator->() const;

        /// Comparison for it == end.
        bool operator==(const FontLetterIterator &rhs) const;
        bool operator!=(const FontLetterIterator &rhs) const;
    private:
        i16 mNumContours;
        u16 *mContourIterator;
        FontCoord *mCoordIterator;

        u16 mNumCoords;
        i16 mRemainingContours;
        u16 mRemainingCoords;
        ContourFontCoord mCoord;
    protected:
    };

    class FontLetter
    {
    public:
        FontLetter(i16 numContours, u16 *contourBegin, FontCoord *coordBegin);
        FontLetterIterator begin() const;
        FontLetterIterator end() const;

        /// Returns true, if the letter is mapped.
        bool valid() const;
    private:
        i16 mNumContrours;
        u16 *mContourBegin;
        FontCoord *mCoordBegin;
    protected:
    };

    class Font
    {
    public:
        friend class FontParser;

        Font();

        /// Get iterator for given character.
        FontLetter letter(char character);

        friend std::ostream &operator<<(std::ostream &out, const Font &font);
    private:
        FontHolder &holder();
        const FontHolder &holder() const;

        FontHolder mHolder;
    protected:
    };
}

#endif //TTF_TEST_FONT_H
