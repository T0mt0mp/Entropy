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
        FWord xMin;
        FWord yMin;
        FWord xMax;
        FWord yMax;
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
        FontLetterIterator(i16 numContours, const u16 *contourBegin, const FontCoord *coordBegin);

        /// Move to next coordinate.
        FontLetterIterator &operator++();

        const ContourFontCoord &operator*() const;
        const ContourFontCoord *operator->() const;

        /// Comparison for it == end.
        bool operator==(const FontLetterIterator &rhs) const;
        bool operator!=(const FontLetterIterator &rhs) const;
    private:
        i16 mNumContours;
        const u16 *mContourIterator;
        const FontCoord *mCoordIterator;

        u16 mNumCoords;
        i16 mRemainingContours;
        u16 mRemainingCoords;
        ContourFontCoord mCoord;
    protected:
    };

    class FontLetter
    {
    public:
        FontLetter();
        FontLetter(const FontHolder &holder, const FontMapping &mapping);
        FontLetterIterator begin() const;
        FontLetterIterator end() const;

        /// Returns true, if the letter is mapped.
        bool valid() const;

        FWord xMin() const;
        FWord yMin() const;
        FWord xMax() const;
        FWord yMax() const;
    private:
        i16 mNumContours{0u};
        const u16 *mContourBegin{nullptr};
        const FontCoord *mCoordBegin{nullptr};

        FWord mXMin{0};
        FWord mYMin{0};
        FWord mXMax{0};
        FWord mYMax{0};
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
