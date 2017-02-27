/**
 * @file app/Timer.h
 * @author Tomas Polasek
 * @brief Simple timer.
 */

#ifndef SIMPLE_GAME_TIMER_H
#define SIMPLE_GAME_TIMER_H

#include "Types.h"

/// Simple timer.
class Timer
{
private:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
public:
    using seconds = std::chrono::seconds;
    using milliseconds = std::chrono::milliseconds;
    using microseconds = std::chrono::microseconds;
    using nanoseconds = std::chrono::nanoseconds;
    /// Initialize the timer and start it.
    Timer()
    { reset(); }

    /// Get the current time.
    time_point now() const
    { return clock::now(); }

    /// Start the timer.
    void reset()
    { mStart = now(); }

    /**
     * Get the elapsed time and reset the timer.
     * @tparam UnitT Unit, in which the time will be returned (e.g. Timer::seconds).
     * @return Elapsed time from the last reset.
     */
    template <typename UnitT>
    u64 elapsedReset()
    {
        u64 elapsedTime{elapsed<UnitT>()};
        reset();
        return elapsedTime;
    }

    /**
     * Get how many time elapsed from the start time.
     * @tparam UnitT Unit, in which the time will be returned (eg. Timer::seconds).
     * @return Returns elapsed time in requested units.
     */
    template <typename UnitT>
    u64 elapsed() const
    { return std::chrono::duration_cast<UnitT>(now() - mStart).count(); }
private:
    /// Time, when the timer started.
    time_point mStart;
protected:
}; // class Timer


#endif //SIMPLE_GAME_TIMER_H
