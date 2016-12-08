/**
 * @file math/Math.h
 * @author Tomas Polasek
 * @brief Main math file.
 */

#ifndef MATH_MATH_H
#define MATH_MATH_H

#include "util/Types.h"
#include "util/Assert.h"

namespace math
{
    /**
     * Calculate average.
     * @param value Sum of the samples.
     * @param count Number of samples.
     * @return Average value of a sample.
     */
    template <typename ValType, typename CountType>
    static inline f64 avg(ValType value, CountType count)
    {
        return count ? ((f64) value) / ((f64) count) : 0.0;
    }

    /**
     * Return max from 2 values.
     * @param first The first value.
     * @param second The second value.
     * @return Returns the bigger of the 2 values. if
     *   they are equal, returns the first one.
     */
    template <typename T>
    inline T &max(T &first, T &second)
    {
        return first >= second ? first : second;
    }

    template <typename T>
    inline constexpr const T &max(const T &first, const T &second)
    {
        return first >= second ? first : second;
    }

    /**
     * Return min from 2 values.
     * @param first The first value.
     * @param second The second value.
     * @return Returns the smaller of the 2 values. if
     *   they are equal, returns the first one.
     */
    template <typename T>
    inline T &min(T &first, T &second)
    {
        return first <= second ? first : second;
    }

    template <typename T>
    inline constexpr T &min(const T &first, const T &second)
    {
        return first <= second ? first : second;
    }

    /**
     * Compare 2 values using given comparator.
     * @param first The first value.
     * @param second The second value.
     * @return The result of the comparation.
     */
    template <typename FT,
        typename ST,
        typename CT>
    inline bool compare(const FT &first, const ST &second, CT cmp = CT())
    {
        return cmp(first, second);
    }

    /**
     * Calculate smallest power of 2 number, which
     * is larger than specified value.
     * @param value Non-zero value, <= to the returned number.
     * @return Power of 2 number, >= to the value.
     */
    static inline u32 pow2RoundUp(u32 value)
    {
        ASSERT_SLOW(value != 0);

        --value;
        value |= value >> 0b1;
        value |= value >> 0b10;
        value |= value >> 0b100;
        value |= value >> 0b1000;
        value |= value >> 0b10000;

        return value + 1;
    }

    static inline u64 pow2RoundUp(u64 value)
    {
        ASSERT_SLOW(value != 0);

        --value;
        value |= value >> 0b1;
        value |= value >> 0b10;
        value |= value >> 0b100;
        value |= value >> 0b1000;
        value |= value >> 0b10000;
        value |= value >> 0b100000;

        return value + 1;
    }
}

#endif //MATH_MATH_H
