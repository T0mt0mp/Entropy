/**
 * @file util/Threding.h
 * @author Tomas Polasek
 * @brief Thread utilities.
 */

#ifndef UTIL_THREADING_H
#define UTIL_THREADING_H

#include <atomic>
#include <thread>

#include "util/Types.h"

namespace thr
{
    /**
     * Spin lock implementation, using std::atomic_flag.
     */
    class SpinLock
    {
    public:
        /**
         * Lock this lock.
         * Yields this thread, if the lock is locked.
         */
        void lock()
        {
            while (mFlag.test_and_set(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }
        }

        i8 tryLock()
        {
            return mFlag.test_and_set(std::memory_order_acquire);
        }

        /**
         * Unlock this lock.
         */
        void unlock()
        {
            mFlag.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
    protected:
    };

}

#endif //UTIL_THREADING_H
