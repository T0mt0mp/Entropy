/**
 * @file FontFile.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef TTF_TEST_FONTFILE_H
#define TTF_TEST_FONTFILE_H

#include "Types.h"

namespace ttf
{
    /**
     * Abstraction of a font file.
     */
    class FontFile
    {
    public:
        /**
         * Create abstraction on given file.
         * @param filename Font filename.
         */
        explicit FontFile(const char *filename);

        /// Free all and any allocated resources.
        ~FontFile();

        FontFile(const FontFile &rhs);
        FontFile(FontFile &&rhs);

        FontFile &operator=(const FontFile &rhs);
        FontFile &operator=(FontFile &&rhs);

        void copy(const FontFile &rhs);
        void swap(FontFile &&rhs);

        /// Size of the file in bytes.
        inline std::size_t size() const;

        /// Remember the current cursor position.
        inline void pushCursor();
        /// Use the remembered cursor position.
        inline void popCursor();

        /// Move cursor to given position.
        inline void moveTo(std::size_t newPosition);

        /// Move cursor by given amount of bytes.
        inline void moveBy(std::size_t addPosition);

        /// Current cursor position.
        inline std::size_t cursorPos() const;

        /// Read byte to given location.
        inline void readTo(u8 &to);
        inline void readTo(u8 *to);

        /// Read byte from index to given location.
        inline void readFTo(std::size_t from, u8 &to);
        inline void readFTo(std::size_t from, u8 *to);

        /// Read number of bytes to given location, without byte reorganization.
        template <typename T>
        inline void readToWR(T &to, std::size_t size = sizeof(T));

        /// Read number of bytes to given location, with byte reorganization.
        template <typename T>
        inline void readTo(T &to, std::size_t size = sizeof(T));

        /// Read number of bytes from index to given location, without byte reorganization.
        template <typename T>
        inline void readFToWR(std::size_t from, T &to, std::size_t size = sizeof(T));

        /// Read number of bytes from index to given location, with byte reorganization.
        template <typename T>
        inline void readFTo(std::size_t from, T &to, std::size_t size = sizeof(T));

        /// Read float as given type.
        template<typename T>
        inline void readFloat(float &to, std::size_t bias);
        template<typename T>
        inline void readFloatF(std::size_t from, float &to, std::size_t bias);

        /// Read a shortFrac.
        inline void readShortFracTo(ShortFrac &to);
        inline void readShortFracFTo(std::size_t from,ShortFrac &to);

        /// Read a Fixed.
        inline void readFixedTo(Fixed &to);
        inline void readFixedFTo(std::size_t from, Fixed &to);

        /// Read a F2Dot14.
        inline void readF2Dot14To(F2Dot14 &to);
        inline void readF2Dot14FTo(std::size_t from, F2Dot14 &to);

        /// Read required size for given type.
        template <typename T>
        inline T read();
        template <typename T>
        inline T read(std::size_t from);
        /// Read one byte.
        inline u8 read8u();
        inline u8 read8u(std::size_t from);
        inline i8 read8i();
        inline i8 read8i(std::size_t from);
        /// Read two bytes.
        inline u16 read16u();
        inline u16 read16u(std::size_t from);
        inline i16 read16i();
        inline i16 read16i(std::size_t from);
        /// Read four bytes.
        inline u32 read32u();
        inline u32 read32u(std::size_t from);
        inline i32 read32i();
        inline i32 read32i(std::size_t from);
    private:
        /// Index in the data buffer.
        std::size_t mCursor{0u};
        /// Size of the data buffer.
        std::size_t mDataSize{0u};
        /// Remembered position.
        std::size_t mCursorMemory{0u};
        /// Data itself.
        u8 *mData{nullptr};
    protected:
    }; // class FontFile
}

#include "FontFile.inl"

#endif //TTF_TEST_FONTFILE_H
