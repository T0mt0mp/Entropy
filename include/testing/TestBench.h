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
#define BTC_Record(CODE_STR, RESULT, COMMENT) \
            _record(CODE_STR, RESULT, __FILE__, __LINE__, __func__, \
                static_cast<std::ostringstream&>(std::ostringstream().flush() << COMMENT).str())

#define BTC_ThrowRecord(CODE_STR, RESULT, COMMENT) \
            throw tb::TestRecord(CODE_STR, RESULT, __FILE__, __LINE__, __func__, \
                static_cast<std::ostringstream&>(std::ostringstream().flush() << COMMENT).str())

#define TU_Begin(NAME) \
            class TEST_CONCAT(_Test_, NAME) : private tb::TestUnit \
            { \
            public: \
                using ThisType = TEST_CONCAT(_Test_, NAME); \
            private: \
                struct TestCaseHolder { \
                    std::vector<std::tuple<void(ThisType::*)(), const char*, const char*>> mTestCases; \
                }; \
            public: \
                TEST_CONCAT(_Test_, NAME)() : \
                    TestUnit(#NAME) \
                {} \
                static TestCaseHolder &holderInstance() \
                { \
                    static TestCaseHolder holder; \
                    return holder; \
                } \
                static ThisType &instance() \
                { \
                    static ThisType inst; \
                    return inst; \
                } \
                static tb::TestUnit * factory() \
                { \
                    return &instance(); \
                } \
                void testMain() override final \
                { \
                    _enterTestCase("Setup", "Setup phase"); \
                    try { \
                        setUp(); \
                    } catch (tb::TestRecord &r) { \
                        _record(r); \
                        return; \
                    } catch (...) { \
                        BTC_Record("Exception!", tb::TestResult::EXCEPTION, "Exception escaped from setup!");\
                        return;\
                    }\
                    for (auto testCase : holderInstance().mTestCases) \
                    { \
                        _enterTestCase(std::get<1>(testCase), std::get<2>(testCase)); \
                        try { \
                            (this->*(std::get<0>(testCase)))(); \
                        } catch (tb::TestRecord &r) { \
                            _record(r); \
                        } catch (...) { \
                            BTC_Record("Exception!", tb::TestResult::EXCEPTION, "Exception escaped from case!");\
                            return;\
                        }\
                    } \
                    _enterTestCase("Teardown", "Teardown phase"); \
                    try { \
                        tearDown(); \
                    } catch (tb::TestRecord &r) { \
                        _record(r); \
                    } catch (...) { \
                        BTC_Record("Exception!", tb::TestResult::EXCEPTION, "Exception escaped from teardown!");\
                        return;\
                    }\
                }

#define TU_End(NAME) \
            }; \
            struct TEST_CONCAT(_Test_Registrator_, NAME) \
            { \
                TEST_CONCAT(_Test_Registrator_, NAME)() \
                { \
                    tb::TestBed::instance().registerTest( \
                        &TEST_CONCAT(_Test_, NAME)::factory); \
                } \
            } TEST_EXP_CONCAT(_Registrator_, NAME);

#define TU_Setup \
            void setUp() override final

#define TU_Teardown \
            void tearDown() override final

#define TU_Case(NAME, DESCRIPTION) \
            struct TEST_CONCAT(_Case_Registrator_, NAME) \
            { \
                TEST_CONCAT(_Case_Registrator_, NAME)() \
                { \
                    ThisType::holderInstance().mTestCases.push_back( \
                        std::make_tuple(&ThisType::TEST_CONCAT(Case_, NAME), #NAME, DESCRIPTION)); \
                } \
            } TEST_EXP_CONCAT(_Case_Registrator_INST_, __LINE__); \
            void TEST_CONCAT(Case_, NAME)()

//void TEST_CONCAT(CaseCaller_, NAME)() { TEST_CONCAT(Case_, NAME)(); };

#define TCC_Run() \
    tb::TestBed::instance().runTests()

#define TCC_Reset() \
    tb::TestBed::instance().resetTests()

#define TCC_Report() \
    tb::TestBed::instance().report()

#define TCC_Return \
    tb::TestBed::instance().reportSuccess()

#define TCC_ReturnCode \
    static_cast<u32>(TCC_Return)

// Testing commands

#define TC_Check(EXP) \
            do { \
                decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, "\"" << #EXP << "\" is true"); } \
                else \
                { BTC_Record(#EXP, tb::TestResult::FAILED, "\"" << #EXP << "\" is not true"); } \
            } while (false)

#define TC_CheckMessage(EXP, MSG) \
            do { \
                decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, MSG); } \
                else \
                { BTC_Record(#EXP, tb::TestResult::FAILED, MSG); } \
            } while (false)

#define TC_CheckEqual(EXP1, EXP2) \
            do { \
                decltype(EXP1) temp1{EXP1}; \
                decltype(EXP2) temp2{EXP2}; \
                if (temp1 == temp2) \
                { BTC_Record(#EXP1 " == " #EXP2, tb::TestResult::PASSED, temp1 << " == " << temp2); } \
                else \
                { BTC_Record(#EXP1 " != " #EXP2, tb::TestResult::FAILED, temp1 << " != " << temp2); } \
            } while (false)

#define TC_CheckException(EXP1, EXCEPTION_TYPE) \
            do { \
                bool caughtCorrect{false}; \
                bool caughtWrong{false}; \
                try { \
                    {(EXP1);}; \
                } catch (EXCEPTION_TYPE e) { \
                    caughtCorrect = true; \
                } catch (...) { \
                    caughtWrong = true; \
                } \
                if (caughtCorrect) \
                { BTC_Record(#EXP1, tb::TestResult::PASSED, "Successfully produced exception : " << #EXCEPTION_TYPE); } \
                else if (caughtWrong)\
                { BTC_Record(#EXP1, tb::TestResult::FAILED, "Code produced wrong exception"); } \
                else \
                { BTC_Record(#EXP1, tb::TestResult::FAILED, "Code failed to produce any exception"); } \
            } while (false)

#define TC_CheckNoException(EXP1) \
            do { \
                bool caughtWrong{false}; \
                try { \
                    {(EXP1);}; \
                } catch (...) { \
                    caughtWrong = true; \
                } \
                if (caughtWrong)\
                { BTC_Record(#EXP1, tb::TestResult::FAILED, "Code produced exception"); } \
                else \
                { BTC_Record(#EXP1, tb::TestResult::PASSED, "Code did not produce any exceptions"); } \
            } while (false)

#define TC_CheckConstexprEqual(EXP1, EXP2) \
            do { \
                constexpr decltype(EXP1) temp1{EXP1}; \
                constexpr decltype(EXP2) temp2{EXP2}; \
                if (temp1 == temp2) \
                { BTC_Record(#EXP1 " == " #EXP2, tb::TestResult::PASSED, temp1 << " == " << temp2); } \
                else \
                { BTC_Record(#EXP1 " != " #EXP2, tb::TestResult::FAILED, temp1 << " != " << temp2); } \
            } while (false)

#define TC_CheckConstexpr(EXP) \
            do { \
                constexpr decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, "\"" << #EXP << "\" is true"); } \
                else \
                { BTC_Record(#EXP, tb::TestResult::FAILED, "\"" << #EXP << "\" is not true"); } \
            } while (false)

#define TC_Require(EXP) \
            do { \
                const decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, "\"" << #EXP << "\" is true"); } \
                else \
                { BTC_ThrowRecord(#EXP, tb::TestResult::CRIT_FAILED, "\"" << #EXP << "\" is not true, exiting case"); } \
            } while (false)

#define TC_RequireMessage(EXP, MSG) \
            do { \
                decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, MSG); } \
                else \
                { BTC_ThrowRecord(#EXP, tb::TestResult::CRIT_FAILED, MSG); } \
            } while (false)

#define TC_RequireEqual(EXP1, EXP2) \
            do { \
                decltype(EXP1) temp1{EXP1}; \
                decltype(EXP2) temp2{EXP2}; \
                if (temp1 == temp2) \
                { BTC_Record(#EXP1 " == " #EXP2, tb::TestResult::PASSED, temp1 << " == " << temp2); } \
                else \
                { BTC_ThrowRecord(#EXP1 " != " #EXP2, tb::TestResult::CRIT_FAILED, temp1 << " != " << temp2); } \
            } while (false)

#define TC_RequireConstexprEqual(EXP1, EXP2) \
            do { \
                constexpr decltype(EXP1) temp1{EXP1}; \
                constexpr decltype(EXP2) temp2{EXP2}; \
                if (temp1 == temp2) \
                { BTC_Record(#EXP1 " == " #EXP2, tb::TestResult::PASSED, temp1 << " == " << temp2); } \
                else \
                { BTC_ThrowRecord(#EXP1 " != " #EXP2, tb::TestResult::CRIT_FAILED, temp1 << " != " << temp2); } \
            } while (false)

#define TC_RequireConstexpr(EXP) \
            do { \
                constexpr decltype(EXP) temp{EXP}; \
                if (temp) \
                { BTC_Record(#EXP, tb::TestResult::PASSED, "\"" << #EXP << "\" is true"); } \
                else \
                { BTC_ThrowRecord(#EXP, tb::TestResult::CRIT_FAILED, "\"" << #EXP << "\" is not true, exiting case"); } \
            } while (false)

#define TC_RequireException(EXP1, EXCEPTION_TYPE) \
            do { \
                bool caughtCorrect{false}; \
                bool caughtWrong{false}; \
                try { \
                    {(EXP1);}; \
                } catch (EXCEPTION_TYPE e) { \
                    caughtCorrect = true; \
                } catch (...) { \
                    caughtWrong = true; \
                } \
                if (caughtCorrect) \
                { BTC_Record(#EXP1, tb::TestResult::PASSED, "Successfully produced exception : " << #EXCEPTION_TYPE); } \
                else if (caughtWrong)\
                { BTC_ThrowRecord(#EXP1, tb::TestResult::CRIT_FAILED, "Code produced wrong exception"); } \
                else \
                { BTC_ThrowRecord(#EXP1, tb::TestResult::CRIT_FAILED, "Code failed to produce any exception"); } \
            } while (false)

#define TC_RequireNoException(EXP1) \
            do { \
                bool caughtWrong{false}; \
                try { \
                    {(EXP1);}; \
                } catch (...) { \
                    caughtWrong = true; \
                } \
                if (caughtWrong)\
                { BTC_ThrowRecord(#EXP1, tb::TestResult::CRIT_FAILED, "Code produced exception"); } \
                else \
                { BTC_Record(#EXP1, tb::TestResult::PASSED, "Code did not produce any exceptions"); } \
            } while (false)

#define TC_Error(MSG) \
            BTC_Record("Reached error statement", tb::TestResult::FAILED, MSG)

#define TC_Fail(MSG) \
            BTC_ThrowRecord("Reached fail statement", tb::TestResult::CRIT_FAILED, MSG)

namespace tb
{
}

#endif //TESTING_TESTBENCH_H
