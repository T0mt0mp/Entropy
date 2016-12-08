/**
 * @file testing/TestUnit.h
 * @author Tomas Polasek
 * @brief Unit testing suit.
 */

#ifndef TESTING_TESTUNIT_H
#define TESTING_TESTUNIT_H

#include <vector>
#include <string>
#include <sstream>

#include "util/Types.h"
#include "util/Assert.h"

namespace tb
{
    class TestBed;
    class TestUnit;

    void _registerTest(TestUnit *testInstance);

    /**
     * Possible test results.
     */
    enum class TestResult
    {
        PASSED,
        FAILED,
        EXCEPTION,
        UNK
    };

    /**
     * Contains information about single test assertion.
     */
    class TestRecord
    {
    public:
        /**
         * @param code Code inside the assertion.
         * @param result Result of the test.
         * @param file Name of the file, where the record was taken.
         * @param line Line of code, where the record was taken.
         * @param function Name of the function of the record.
         * @param text Optional text.
         */
        TestRecord(const char *code,
                   TestResult result,
                   const char *file,
                   u64 line,
                   const char *function,
                   std::string text);

        /**
         * Convert TestResult to C-string.
         * @return Text representation of the result.
         */
        static const char *resultToStr(TestResult result);

        /**
         * Should this record be printed?
         * @return Returns true, if this record should be printed.
         */
        bool shouldBePrinted();

        friend std::ostream &operator<<(std::ostream &out,
                                        const TestRecord &tr);
    private:
    protected:
        /// Code inside the assertion.
        const char* mCode{nullptr};
        /// Result of the test.
        TestResult mResult{TestResult::UNK};
        /// Name of the file.
        const char* mFile{nullptr};
        /// Line of the test.
        u64 mLine{0};
        /// Name of the function of the test.
        const char* mFunction{nullptr};
        /// Optional text.
        std::string mText;
    };

    /**
     * Contains statistics about a single TestUnit run.
     */
    class TestRun
    {
    public:
        /// How many exceptions escaped from testing functions.
        u32 mExceptionsEscaped{0};
        /// Number of test assertions.
        u32 mNumTests{0};
        /// Number of successful test assertions.
        u32 mNumPassed{0};
        /// Number of failed test assertions.
        u32 mNumFailed{0};
        /// List of recorded test assertions.
        std::vector<TestRecord> mRecords;
    private:
    protected:
    };

    enum class TestStatus
    {
        BEFORE_SETUP,
        FAILED_SETUP,
        RUNNABLE,
        BEFORE_TEARDOWN,
        FAILED_TEARDOWN,
        UNK
    };

    /**
     * Testing unit.
     */
    class TestUnit
    {
    public:
        /**
         * @param name Name of this TestUnit.
         * @param numRuns How many times should the testMain
         * be ran.
         */
        TestUnit(const char *name, u32 numRuns = 1);

        virtual ~TestUnit()
        { }

        /**
         * Setup function, ran only once - at the start.
         * Implemented by the test writer, should set all
         * required "global" variables.
         * May also contain testing macros.
         */
        virtual void setUp() = 0;

        /**
         * Teardown function, ran only once - at the end.
         * Implemented by the test writer, should clean up
         * everything done by the test.
         * May also contain testing macros.
         */
        virtual void tearDown() = 0;

        /**
         * Main test function, may be ran multiple times.
         * Contains testing macros.
         */
        virtual void testMain() = 0;

        /**
         * Record a test result.
         * @param code Code executed in the test.
         * @param result Result of the test.
         * @param file Name of the file, where the record was taken.
         * @param line Line of code, where the record was taken.
         * @param function Name of the function of the record.
         * @param text Optional text.
         */
        void record(const char *code,
                    TestResult result,
                    const char *file,
                    u64 line,
                    const char *function,
                    std::string text);
    private:
        friend class TestBed;
        /**
         * Wrapper method for the setUp method.
         * Called by the TestBed before running this test.
         */
        void unitSetUp();

        /**
         * Wrapper method for the TearDown method.
         * Called by the TestBed after running the test.
         */
        void unitTearDown();

        /**
         * Wrapper method for the testMain method.
         * Called by the TestBed 1 and more times.
         */
        void unitRunTest();

        /**
         * Get the current statistics object.
         * @return Test statistics object.
         */
        TestRun &currentStatistics();

        /**
         * Reset statistics.
         */
        void reset();

        /**
         * Get status of this TestUnit.
         * @return Status of this TestUnit.
         */
        TestStatus getStatus();

        /**
         * Attempt to run this test.
         * @return Returns false, if everything went ok.
         */
        i8 run();

        /**
         * Create new statistics object on the stack.
         */
        void newStatistics();

        /**
         * Print information about this TestUnit to
         * given ostream.
         * @param out Output stream.
         */
        void printInfo(std::ostream &out);

        /// Name of this TestUnit.
        const char *mName{nullptr};
        /// How many times should this test be ran.
        u32 mNumRuns{0};
        /// How many times has this test been run.
        u32 mNumRan{0};
        /// State of this test.
        TestStatus mStatus{TestStatus::UNK};

        /// Test statistics.
        std::vector<TestRun> mRuns;
    protected:
    };
}

#endif //TESTING_TESTUNIT_H
