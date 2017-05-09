/**
 * @file FontParser.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef TTF_TEST_FONTPARSER_H
#define TTF_TEST_FONTPARSER_H

#include "Types.h"
#include "FontFile.h"
#include "Tables.h"
#include "Font.h"

namespace ttf
{
    /// Used for parsing font files.
    class FontParser
    {
    public:
        /// Initialize the parser
        FontParser(FontFile &ff);

        /// Parse the file.
        Font parse();
    private:
        /// Is there such a table?
        bool hasTable(const char *name);

        /// File of the parsed font.
        FontFile &mFF;
        /// Offset table.
        TableOffset mTOffsets;
        /// Head table.
        TableHead mTHead;
        /// Tables found.
        std::map<std::string, TableDirectory> mTables;
    protected:
    }; // class FontParser

}

#endif //TTF_TEST_FONTPARSER_H
