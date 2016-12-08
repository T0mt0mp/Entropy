/**
 * @file util/Assert.h
 * @author Tomas Polasek
 * @brief Assert macros.
 */

#ifndef UTIL_ASSERT_H_H
#define UTIL_ASSERT_H_H

#include <cstdio>
#include <cstdlib>
#include <csignal>

#define UNUSED(VAR) ((void)VAR)

#define ASSERT_FATAL(COND_EXP) \
    if ((COND_EXP)) \
    {} \
    else \
    { \
        fprintf(stderr, "Fatal error \"" #COND_EXP "\"" \
                        "%s : %d ( %s ) failed.\n", \
                        __FILE__, __LINE__, __func__); \
        exit(-1); \
    }

#ifndef NDEBUG
#   define _ASSERT_BASE(COND_EXP, LINE, FILE, FUNC) \
    if ((COND_EXP)) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion \"" #COND_EXP "\"" \
                        "%s : %d ( %s ) failed.\n", \
                        FILE, LINE, FUNC); \
        std::raise(SIGABRT); \
    }
#else
#   define _ASSERT_BASE(COND_EXP, LINE, FILE, FUNC)
#endif

#ifndef NDEBUG_FAST
#   define ASSERT_FAST(COND_EXP) _ASSERT_BASE((COND_EXP), \
                                 __LINE__, __FILE__, __func__)
#else
#   define ASSERT_FAST(_)
#endif

#ifndef NDEBUG_SLOW
#   define ASSERT_SLOW(COND_EXP) _ASSERT_BASE((COND_EXP), \
                                 __LINE__, __FILE__, __func__)
#else
#   define ASSERT_SLOW(_)
#endif

#endif //UTIL_ASSERT_H_H
