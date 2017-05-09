/**
 * @file Tables.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef TTF_TEST_TABLES_H
#define TTF_TEST_TABLES_H

#include "Types.h"
#include "FontFile.h"
#include "Font.h"

namespace ttf
{
    struct TableOffset
    {
        u32 scalerType;
        u16 numTables;
        u16 searchRange;
        u16 entrySelector;
        u16 rangeShift;

        /// Read data into this table.
        void readFrom(FontFile &f);
        /// Print to out.
        friend std::ostream &operator<<(std::ostream &out, const TableOffset &table);
    }; // struct TableOffset

    struct TableDirectory
    {
        static constexpr std::size_t TAG_LENGTH{4};
        char tag[TAG_LENGTH + 1u];
        u32 checkSum;
        u32 offset;
        u32 length;

        /// Read data into this table.
        void readFrom(FontFile &f);
        /// Check the checksum.
        bool checksumValid(FontFile &f) const;
        /// Print to out.
        friend std::ostream &operator<<(std::ostream &out, const TableDirectory &table);
    }; // struct TableDirectory

    struct TableHead
    {
        static constexpr u32 MAGIC_NUMBER_VAL{0x5F0F3CF5};
        Fixed version;
        Fixed fontRevision;
        u32 checkSumAdjustment;
        u32 magicNumber;
        u16 flags;
        u16 unitsPerEm;
        i64 created;
        i64 modified;
        FWord xMin;
        FWord yMin;
        FWord xMax;
        FWord yMax;
        u16 macStyle;
        u16 lowestRecPPEM;
        i16 fontDirectionHint;
        i16 indexToLocFormat;
        i16 glyphDataFormat;

        /// Read data into this table.
        void readFrom(FontFile &f, u32 offset);
        /// Print to out.
        friend std::ostream &operator<<(std::ostream &out, const TableHead &table);
    }; // struct TableHead

    struct GlyphLocation
    {
        u32 offset;
        u32 length;
    }; // struct GlyphLocation

    class TableLoca
    {
    public:
        /// Read data into this table.
        TableLoca(FontFile &f, u32 offset, u32 length, i16 indexToLocFormat);

        ~TableLoca();

        /// Read data into this table.
        void readFrom(FontFile &f, u32 offset, u32 length, i16 indexToLocFormat);

        /// Get number of glyphs
        u32 numGlyphs() const;

        /// Get glyph location and length for given glyph.
        const GlyphLocation &forGlyph(u32 index) const;

        /// Print to out.
        friend std::ostream &operator<<(std::ostream &out, const TableLoca &table);
    private:
        GlyphLocation *mLocations{nullptr};
        u32 mNumGlyphs{0u};
        bool mUsedLong{false};
    protected:
    }; // class TableLoca

    class TableCmap
    {
    public:
        TableCmap(FontFile &f, u32 offset, u32 length);

        ~TableCmap();

        /// Read data into this table.
        void readFrom(FontFile &f, u32 offset, u32 length);

        /// Get the mapping array.
        auto mapping() const -> const u32 (&)[CHAR_MAPPING];

        friend std::ostream &operator<<(std::ostream &out, const TableCmap &table);
    private:
        /// Clear the mapping array.
        void clearMapping();

        /// Parse encoding on given offset.
        bool parseEncoding(FontFile &f, u32 offset);

        /// Mapping from ascii to glyph indexes
        u32 mMapping[CHAR_MAPPING];
    protected:
    }; // class TableCmap

    struct GlyphHeader
    {
        i16 numberOfContours;
        FWord xMin;
        FWord yMin;
        FWord xMax;
        FWord yMax;
    };

    class TableGlyf
    {
    public:
        static constexpr u8 FLAG_ON_CURVE{1u};
        static constexpr u8 FLAG_X_SHORT{2u};
        static constexpr u8 FLAG_Y_SHORT{4u};
        static constexpr u8 FLAG_REPEAT{8u};
        static constexpr u8 FLAG_X_SAME{16u};
        static constexpr u8 FLAG_Y_SAME{32u};

        /// Read data into this table.
        TableGlyf(FontFile &f, u32 offset, u32 length, const TableCmap &cmap,
                  const TableLoca &loca, FontHolder &fh);

        ~TableGlyf();

        /// Read data into this table.
        void readFrom(FontFile &f, u32 offset, u32 length, const TableCmap &cmap,
                      const TableLoca &loca, FontHolder &fh);

        /// Print to out.
        friend std::ostream &operator<<(std::ostream &out, const TableGlyf &table);
    private:
        /// Read a simple glyph.
        void readSimpleGlyph(FontFile &f, const GlyphHeader &header, FontHolder &fh, std::size_t index);

        u32 mNumGlyphs{0u};
        u32 mFailedGlyphs{0u};
        u32 mSimpleGlyphs{0u};
        u32 mCompoundGlyphs{0u};
    protected:
    }; // class TableGlyf
}

#endif //TTF_TEST_TABLES_H
