/**
 * @file Tables.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "Tables.h"

namespace ttf
{
    void TableOffset::readFrom(FontFile &f)
    {
        f.readTo(scalerType);
        f.readTo(numTables);
        f.readTo(searchRange);
        f.readTo(entrySelector);
        f.readTo(rangeShift);
    }

    std::ostream &operator<<(std::ostream &out, const TableOffset &table)
    {
        out << "Offset table: \n"
            << "  ScalerType: " << table.scalerType << "\n"
            << "  NumTables: " << table.numTables << "\n"
            << "  SearchRange: " << table.searchRange << "\n"
            << "  EntrySelector: " << table.entrySelector << "\n"
            << "  RangeShift: " << table.rangeShift;
        return out;
    }

    void TableDirectory::readFrom(FontFile &f)
    {
        for (u16 iii = 0; iii < TAG_LENGTH; ++iii)
        {
            f.readTo(tag[iii]);
        }
        tag[TAG_LENGTH] = '\0';
        f.readTo(checkSum);
        f.readTo(offset);
        f.readTo(length);
    }

    bool TableDirectory::checksumValid(FontFile &f) const
    {
        if (offset + length > f.size())
        {
            return false;
        }

        u32 sum{0u};
        std::size_t size{length + 3 / 4};
        for (std::size_t iii = offset; iii < offset + size; iii += 4)
        {
            sum += f.read32u(iii);
        }

        return sum == checkSum;
    }

    std::ostream &operator<<(std::ostream &out, const TableDirectory &table)
    {
        out << "Tag: " << table.tag
            << ";\tOffset: " << table.offset
            << ";\tLength: " << table.length;
        return out;
    }

    void TableHead::readFrom(FontFile &f, u32 offset)
    {
        f.pushCursor();
        f.moveTo(offset);

        f.readFixedTo(version);
        f.readFixedTo(fontRevision);
        f.readTo(checkSumAdjustment);
        f.readTo(magicNumber);
        f.readTo(flags);
        f.readTo(unitsPerEm);
        f.readTo(created);
        f.readTo(modified);
        f.readTo(xMin);
        f.readTo(yMin);
        f.readTo(xMax);
        f.readTo(yMax);
        f.readTo(macStyle);
        f.readTo(lowestRecPPEM);
        f.readTo(fontDirectionHint);
        f.readTo(indexToLocFormat);
        f.readTo(glyphDataFormat);

        f.popCursor();

        if (magicNumber != MAGIC_NUMBER_VAL)
        {
            throw std::runtime_error("Magic number does not equal the correct value!");
        }
    }

    std::ostream &operator<<(std::ostream &out, const TableHead &table)
    {
        out << "Head table: \n"
            << "  Version: " << table.version << "\n"
            << "  FontRevision: " << table.fontRevision << "\n"
            << "  UnitsPerEm: " << table.unitsPerEm << "\n"
            << "  xMin: " << table.xMin << "\n"
            << "  yMin: " << table.yMin << "\n"
            << "  xMax: " << table.xMax << "\n"
            << "  yMax: " << table.yMax;
        return out;
    }

    TableLoca::TableLoca(FontFile &f, u32 offset, u32 length, i16 indexToLocFormat)
    {
        readFrom(f, offset, length, indexToLocFormat);
    }

    TableLoca::~TableLoca()
    {
        if (mLocations)
        {
            delete[] mLocations;
        }
    }

    void TableLoca::readFrom(FontFile &f, u32 offset, u32 length, i16 indexToLocFormat)
    {
        f.pushCursor();
        f.moveTo(offset);

        if (indexToLocFormat == 1)
        { // u32 offsets.
            mUsedLong = true;
            // Last record does not represent a glyph.
            mNumGlyphs = (length / sizeof(u32)) - 1u;

            mLocations = new GlyphLocation[mNumGlyphs + 1u];
            if (!mLocations)
            {
                throw std::runtime_error("Unable to allocate glyph location array!");
            }

            mLocations[0].offset = f.read32u();
            for (u32 iii = 1; iii <= mNumGlyphs; ++iii)
            {
                mLocations[iii].offset = f.read32u();
                mLocations[iii - 1].length = mLocations[iii].offset - mLocations[iii - 1u].offset;
            }
        }
        else
        { // u16 offsets.
            // Last record does not represent a glyph.
            mNumGlyphs = (length / sizeof(u16)) - 1u;

            mLocations = new GlyphLocation[mNumGlyphs + 1u];
            if (!mLocations)
            {
                throw std::runtime_error("Unable to allocate glyph location array!");
            }

            mLocations[0].offset = f.read16u() * 2u;
            for (u32 iii = 1; iii <= mNumGlyphs; ++iii)
            {
                mLocations[iii].offset = f.read32u() * 2u;
                mLocations[iii - 1].length = mLocations[iii].offset - mLocations[iii - 1u].offset;
            }
        }

        f.popCursor();
    }

    u32 TableLoca::numGlyphs() const
    { return mNumGlyphs; }

    const GlyphLocation &TableLoca::forGlyph(u32 index) const
    { return mLocations[index]; }

    std::ostream &operator<<(std::ostream &out, const TableLoca &table)
    {
        out << "Loca table:\n"
            << "  Number of glyphs: " << table.numGlyphs() << "\n"
            << "  Used long: " << (table.mUsedLong ? "yes" : "no") << "\n"
            << "  Glyphs: \n";

        u64 toPrint{table.numGlyphs() < 10u ? table.numGlyphs() : 10u};
        for (u64 iii = 0; iii < toPrint; ++iii)
        {
            const GlyphLocation &loc{table.forGlyph(iii)};
            out << "    Offset: " << loc.offset << ";\tLength: " << loc.length << "\n";
        }

        out << "  End of glyph examples.";

        return out;
    }

    TableGlyf::TableGlyf(FontFile &f, u32 offset, u32 length, const TableCmap &cmap,
                         const TableLoca &loca, FontHolder &fh)
    {
        readFrom(f, offset, length, cmap, loca, fh);
    }

    TableGlyf::~TableGlyf()
    {

    }

    void TableGlyf::readFrom(FontFile &f, u32 offset, u32 length, const TableCmap &cmap,
                             const TableLoca &loca, FontHolder &fh)
    {
        f.pushCursor();

        for (std::size_t index = 0; index < CHAR_MAPPING; ++index)
        {
            u32 glyphIndex{cmap.mapping()[index]};
            if (glyphIndex == 0)
            {
                mFailedGlyphs++;
                continue;
            }

            const GlyphLocation &loc{loca.forGlyph(glyphIndex)};
            GlyphHeader header;

            u32 glyphOffset{loc.offset + offset};
            if (glyphOffset >= offset + length)
            {
                mFailedGlyphs++;
                continue;
            }

            f.moveTo(glyphOffset);
            f.readTo(header.numberOfContours);
            f.readTo(header.xMin);
            f.readTo(header.yMin);
            f.readTo(header.xMax);
            f.readTo(header.yMax);

            mNumGlyphs++;

            if (header.numberOfContours >= 0)
            {
                readSimpleGlyph(f, header, fh, index);
                mSimpleGlyphs++;
            }
            else
            {
                mCompoundGlyphs++;
            }
        }
        f.popCursor();
    }

    std::ostream &operator<<(std::ostream &out, const TableGlyf &table)
    {
        out << "Table glyf: \n"
            << "  Number of glyphs: " << table.mNumGlyphs << "\n"
            << "  Number of simple glyphs: " << table.mSimpleGlyphs << "\n"
            << "  Number of compound glyphs: " << table.mCompoundGlyphs << "\n"
            << "  Number of failed glyphs: " << table.mFailedGlyphs;
        return out;
    }

    void TableGlyf::readSimpleGlyph(FontFile &f, const GlyphHeader &header, FontHolder &fh, std::size_t index)
    {
        FontMapping &mapping = fh.mapping[index];
        mapping.contourIndex = fh.contours.size();
        mapping.coordinateIndex = fh.coordinates.size();
        mapping.numContours = header.numberOfContours;
        mapping.xMin = header.xMin;
        mapping.yMin = header.yMin;
        mapping.xMax = header.xMax;
        mapping.yMax = header.yMax;
        mapping.isMapped = true;

        u16 numPoints{0u};
        for (u16 iii = 0; iii < header.numberOfContours; ++iii)
        {
            u16 contour = f.read16u();
            fh.contours.push_back(contour);
            if (contour > numPoints)
            {
                numPoints = contour;
            }
        }

        numPoints++;

        u16 instructionLength = f.read16u();
        f.moveBy(instructionLength);

        std::vector<u8> flags;
        for (u16 iii = 0; iii < numPoints; ++iii)
        {
            u8 flag = f.read8u();
            flags.push_back(flag);

            if (flag & FLAG_REPEAT)
            {
                u8 numRepeats = f.read8u();
                iii += numRepeats;
                for (u8 rep = 0; rep < numRepeats; ++rep)
                {
                    flags.push_back(flag);
                }
            }
        }

        i16 currentPos{0u};

        std::vector<i16> xCoords;

        for (u16 iii = 0; iii < numPoints; ++iii)
        {
            u8 flag = flags[iii];
            if (flag & FLAG_X_SHORT)
            { // u8
                if (flag & FLAG_X_SAME)
                { // Delta positive
                    currentPos += f.read8u();
                }
                else
                { // Delta negative
                    currentPos -= f.read8u();
                }
            }
            else
            { // i16
                if (flag & FLAG_X_SAME)
                { // Same as previous
                }
                else
                { // Delta negative
                    currentPos += f.read16i();
                }
            }

            xCoords.push_back(currentPos);
        }

        currentPos = 0u;

        std::vector<i16> yCoords;

        for (u16 iii = 0; iii < numPoints; ++iii)
        {
            u8 flag = flags[iii];
            if (flag & FLAG_Y_SHORT)
            { // u8
                if (flag & FLAG_Y_SAME)
                { // Delta positive
                    currentPos += f.read8u();
                }
                else
                { // Delta negative
                    currentPos -= f.read8u();
                }
            }
            else
            { // i16
                if (flag & FLAG_Y_SAME)
                { // Same as previous
                }
                else
                { // Delta negative
                    currentPos += f.read16i();
                }
            }

            yCoords.push_back(currentPos);
        }

        for (u16 index = 0; index < numPoints; ++index)
        {
            fh.coordinates.emplace_back(xCoords[index], yCoords[index], flags[index] & FLAG_ON_CURVE);
        }
    }

    TableCmap::TableCmap(FontFile &f, u32 offset, u32 length)
    {
        readFrom(f, offset, length);
    }

    TableCmap::~TableCmap()
    {

    }

    void TableCmap::readFrom(FontFile &f, u32 offset, u32 length)
    {
        clearMapping();

        f.pushCursor();
        f.moveTo(offset);

        u16 version = f.read16u();
        u16 numTables = f.read16u();

        bool found{false};
        for (u16 iii = 0; iii < numTables && !found; ++iii)
        {
            u16 platformId = f.read16u();
            u16 encodingId = f.read16u();
            u32 encOffset = f.read32u();

            if (encodingId == 10u)
            {
                found = parseEncoding(f, offset + encOffset);
            }
        }

        f.popCursor();

        if (!found)
        {
            throw std::runtime_error("Supported encoding table not found!");
        }
    }

    auto TableCmap::mapping() const -> const u32 (&)[CHAR_MAPPING]
    { return mMapping; }

    std::ostream &operator<<(std::ostream &out, const TableCmap &table)
    {
        out << "Table cmap: \n";

        for (std::size_t iii = 0; iii < CHAR_MAPPING; ++iii)
        {
            out << static_cast<char>(iii) << ":" << table.mMapping[iii] << "\t";
            if (iii % 10 == 0)
            {
                out << "\n";
            }
        }
        out << "\n";

        return out;
    }

    void TableCmap::clearMapping()
    {
        memset(mMapping, 0, sizeof(mMapping));
    }

    bool TableCmap::parseEncoding(FontFile &f, u32 offset)
    {
        u16 format = f.read16u(offset);
        if (format != 12)
        {
            return false;
        }

        std::size_t pos{f.cursorPos()};
        f.moveTo(offset + 2u);

        u16 reserved = f.read16u();
        u32 length = f.read32u();
        u32 language = f.read32u();
        u32 numGroups = f.read32u();

        for (u32 grpIndex = 0; grpIndex < numGroups; ++grpIndex)
        {
            u32 startCharCode = f.read32u();
            u32 endCharCode = f.read32u();
            u32 startGlyphId = f.read32u();

            if (endCharCode <= CHAR_MAPPING)
            {
                std::size_t end{CHAR_MAPPING < endCharCode ? CHAR_MAPPING : endCharCode};
                for (std::size_t iii = startCharCode; iii < end; ++iii)
                {
                    mMapping[iii] = static_cast<u32>(startGlyphId + iii - startCharCode);
                }
            }
        }

        f.moveTo(pos);
        return true;
    }
}
