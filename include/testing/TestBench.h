/**
 * @file testing/TestBench.h
 * @author Tomas Polasek
 * @brief Unit testing suit.
 */

#ifndef TESTING_TESTBENCH_H
#define TESTING_TESTBENCH_H

#include "TestBed.h"

// Unit testing macros.

// Utility macros
#define TEST_CONCAT(FIRST, SECOND) FIRST ## SECOND
#define TEST_EXP_CONCAT(FIRST, SECOND) TEST_CONCAT(FIRST, SECOND)

// Structural macros
#define TEST_UNIT_BEG(NAME, RUNS) \
            class TEST_CONCAT(_Test_, NAME) : private tb::TestUnit \
            { \
            public: \
                TEST_CONCAT(_Test_, NAME)() : \
                    TestUnit(#NAME, RUNS) \
                {} \
                static tb::TestUnit * factory() \
                { \
                    return new TEST_CONCAT(_Test_, NAME)(); \
                } \

#define TEST_UNIT_END(NAME) \
            }; \
            struct TEST_CONCAT(_Test_Registrator_, NAME) \
            { \
                TEST_CONCAT(_Test_Registrator_, NAME)() \
                { \
                    tb::TestBed::instance().registerTest( \
                        &TEST_CONCAT(_Test_, NAME)::factory); \
                } \
            } TEST_EXP_CONCAT(_Registrator_, __LINE__);

#define TEST_UNIT_SETUP_BEG \
            void setUp() override final \
            {

#define TEST_UNIT_SETUP_END \
            }

#define TEST_UNIT_TEARDOWN_BEG \
            void tearDown() override final \
            {

#define TEST_UNIT_TEARDOWN_END \
            }

#define TEST_UNIT_MAIN_BEG \
            void testMain() override final \
            {

#define TEST_UNIT_MAIN_END \
            }

#define TC_Run() \
    tb::TestBed::instance().runTests()

#define TC_Reset() \
    tb::TestBed::instance().resetTests()

#define TC_Report() \
    tb::TestBed::instance().report()

// Testing commands

#define TC_Record(CODE_STR, RESULT, COMMENT) \
            record(CODE_STR, RESULT, __FILE__, __LINE__, __func__, COMMENT)

#define TC_RecordCond(COND, CODE_STR, R_T, C_T, R_F, C_F) \
            if ( (COND) ) \
            { TC_Record(CODE_STR, R_T, C_T); } \
            else \
            { TC_Record(CODE_STR, R_F, C_F); }

#define TC_AssertTrue(CODE) \
            TC_RecordCond( (CODE) , #CODE, \
                        tb::TestResult::PASSED, "Is true", \
                        tb::TestResult::FAILED, "Is NOT true")

#define TC_Expect(CODE, EXP_VAL) \
            do { \
                auto temp{CODE}; \
                if (temp == (EXP_VAL)) \
                { TC_Record(#CODE, tb::TestResult::PASSED, "Got the expected value."); } \
                else \
                { TC_Record(#CODE, tb::TestResult::FAILED, \
                            std::string("Expected ") + std::to_string(EXP_VAL) + " got " + std::to_string(temp) \
                );} \
            } while (false);

namespace tb
{
}

#endif //TESTING_TESTBENCH_H
