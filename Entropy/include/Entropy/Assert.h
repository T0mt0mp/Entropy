/**
 * @file Entropy/Assert.h
 * @author Tomas Polasek
 * @brief Assert system used in Entropy ECS.
 */

#ifndef ECS_FIT_ASSERT_H
#define ECS_FIT_ASSERT_H

#include <cstdio>
#include <cstdlib>
#include <csignal>

#define ENT_UNUSED(VAR) (static_cast<void>(VAR))

#define ENT_ASSERT_FATAL(COND_EXP) \
    if ((COND_EXP)) \
    {} \
    else \
    { \
        fprintf(stderr, "Fatal error \"" #COND_EXP "\"" \
                        "%s: %d ( %s ) failed.\n", \
                        __FILE__, __LINE__, __func__); \
        fflush(stderr); \
        std::raise(SIGABRT); \
        exit(-1); \
    }

#ifndef NDEBUG
#   define ENT_ASSERT_BASE(COND_EXP, LINE, FILE, FUNC) \
    if ((COND_EXP)) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion \"" #COND_EXP "\"" \
                        "%s:%d ( %s ) failed.\n", \
                        FILE, LINE, FUNC); \
        fflush(stderr); \
        std::raise(SIGABRT); \
    }
#else
#   define ENT_ASSERT_BASE(COND_EXP, LINE, FILE, FUNC)
#endif

#ifndef NDEBUG
/// Equals true, if NDEBUG is NOT set.
#   define ENT_IS_DEBUG_BOOL true
/// Choose first, if debug is on, else choose second.
#   define ENT_CHOOSE_DEBUG(first, second) first
/// If NDEBUG is not defined, the command in brackets will be present.
#   define ENT_DO_IF_DEBUG(cmd) cmd
/// Flag for Entropy debugging.
#   define ENT_DEBUG
#else
/// Equals true, if NDEBUG is NOT set.
#   define ENT_IS_DEBUG_BOOL false
/// Choose first, if debug is on, else choose second.
#   define ENT_CHOOSE_DEBUG(first, second) second
/// If NDEBUG is not defined, the command in brackets will be present.
#   define ENT_DO_IF_DEBUG(_)
#endif

// Assert used in places, where the performance hit is not too big.
#ifndef NDEBUG_FAST
#   define ENT_ASSERT_FAST(COND_EXP) ENT_ASSERT_BASE((COND_EXP), \
                                 __LINE__, __FILE__, __func__)
#else
#   define ENT_ASSERT_FAST(_)
#endif

// Assert used in places, where the performance hit is quite notable.
#ifndef NDEBUG_SLOW
#   define ENT_ASSERT_SLOW(COND_EXP) ENT_ASSERT_BASE((COND_EXP), \
                                 __LINE__, __FILE__, __func__)
#else
#   define ENT_ASSERT_SLOW(_)
#endif

// Print warning message.
#ifndef NDEBUG
#   define ENT_WARNING(MSG) \
    do{ \
        fprintf(stderr, "Warning: \"" MSG "\"" \
                        "%s:%d ( %s ).\n", \
                        __FILE__, __LINE__, __func__); \
        fflush(stderr); \
    } while(false)
#else
#   define ENT_WARNING(_)
#endif

#ifdef NO_ENT_WARNING
#   undef ENT_WARNING
#   define ENT_WARNING(_)
#endif

#endif //ECS_FIT_ASSERT_H
