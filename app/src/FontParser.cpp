/**
 * @file FontParser.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "FontParser.h"

namespace ttf
{
    FontParser::FontParser(ttf::FontFile &ff) :
        mFF{ff}
    { }

    Font FontParser::parse()
    {
        mTOffsets.readFrom(mFF);

        Font result;

        for (std::size_t iii = 0; iii < mTOffsets.numTables; ++iii)
        {
            TableDirectory td;
            td.readFrom(mFF);
            mTables.emplace(td.tag, td);
        }

        if (!hasTable("head"))
        {
            throw std::runtime_error("No \"head\" table found!");
        }

        if (!hasTable("cmap"))
        {
            throw std::runtime_error("No \"cmap\" table found!");
        }

        if (!hasTable("loca"))
        {
            throw std::runtime_error("No \"loca\" table found!");
        }

        if (!hasTable("glyf"))
        {
            throw std::runtime_error("No \"glyf\" table found!");
        }

        TableDirectory &headDirectory{mTables["head"]};
        mTHead.readFrom(mFF, headDirectory.offset);

        TableDirectory &cmapDirectory{mTables["cmap"]};
        TableCmap cmap(mFF, cmapDirectory.offset, cmapDirectory.offset);

        TableDirectory &locaDirectory{mTables["loca"]};
        TableLoca loca(mFF, locaDirectory.offset, locaDirectory.length, mTHead.indexToLocFormat);

        TableDirectory &glyfDirectory{mTables["glyf"]};
        TableGlyf glyf(mFF, glyfDirectory.offset, glyfDirectory.offset, cmap, loca, result.holder());

        return result;
    }

    bool FontParser::hasTable(const char *name)
    {
        return mTables.find(name) != mTables.end();
    }
}
