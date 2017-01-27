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
        CRIT_FAILED,
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
         * @param result Result to convert.
         * @return Text representation of the result.
         */
        static const char *resultToStr(TestResult result);

        /**
         * Convert TestResult to C-string. Short version.
         * @param result Result to convert.
         * @return Text representation of the result.
         */
        static const char *resultToStrShort(TestResult result);

        /**
         * Should this record be printed?
         * @return Returns true, if this record should be printed.
         */
        bool shouldBePrinted() const
        { return mResult != TestResult::PASSED; }

        /**
         * Should this record count as exception escaped?
         */
        bool exceptionEscaped() const
        { return mResult == TestResult::EXCEPTION; }

        /**
         * Should this record count to the passed tests?
         */
        bool passed() const
        { return mResult == TestResult::PASSED; }

        /**
         * Should this record count to the failed tests?
         */
        bool failed() const
        { return mResult != TestResult::PASSED; }

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
     * Wrapper object around results from a single test case.
     */
    class TestCase
    {
    public:
        /**
         * @param name
         * @param description
         */
        TestCase(const char *name, const char *description);

        /**
         * Record a new result.
         * @param record Record entry.
         */
        void record(TestRecord &&record);

        /**
         * Print information about this test case.
         * @param out Output stream.
         */
        void printInfo(std::ostream &out) const;
    private:
        /// How many exceptions escaped from testing function.
        u32 mExceptionsEscaped{0};
        /// Number of test assertions.
        u32 mNumTests{0};
        /// Number of successful test assertions.
        u32 mNumPassed{0};
        /// Number of failed test assertions.
        u32 mNumFailed{0};
        /// Name of this test case.
        const char *mName;
        /// Description of this test case.
        const char *mDescription;
        /// List of recorded entries for this test case.
        std::vector<TestRecord> mRecords;
    protected:
    };

    /**
     * Contains statistics about a single TestUnit run.
     */
    class TestRun
    {
    public:
        /**
         * Enter a new test case.
         * @param testCase The test case object.
         */
        void enterTestCase(TestCase &&testCase);

        /**
         * Record a new result.
         * Test case has to be entered first!
         * @param record Record entry.
         */
        void record(TestRecord &&record);

        /**
         * Print information about this test run.
         * @param out Output stream.
         */
        void printInfo(std::ostream &out) const;

        /**
         * Did all the tests succeed?
         * @return Returns true, if all tests succeeded.
         */
        bool passed() const
        { return mNumPassed >= mNumTests; }

        u32 getExceptionsEscaped() const
        { return mExceptionsEscaped; }

        u32 getNumTests() const
        { return mNumTests; }

        u32 getNumPassed() const
        { return mNumPassed; }

        u32 getNumFailed() const
        { return mNumFailed; }
    private:
        /// How many exceptions escaped from testing functions.
        u32 mExceptionsEscaped{0};
        /// Number of test assertions.
        u32 mNumTests{0};
        /// Number of successful test assertions.
        u32 mNumPassed{0};
        /// Number of failed test assertions.
        u32 mNumFailed{0};
        /// List of test cases.
        std::vector<TestCase> mTestCases;
    protected:
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
         * Are all test runs of this Unit finished?
         * @return Returns true, if all runs were finished.
         */
        bool allFinished() const
        { return mNumRan >= mNumRuns; }

        /**
         * Did any of the tests failed?
         * @return Returns true, if any of the tests failed.
         */
        bool anyFailed() const
        {
            for (const TestRun &tr : mRuns)
            {
                if (!tr.passed())
                    return true;
            }
            return false;
        }
    private:
        friend class TestBed;
        friend class TestHolder;

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
         * Attempt to run this test.
         * @return Returns false, if everything went ok.
         */
        bool run();

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

        /// Test statistics.
        std::vector<TestRun> mRuns;
    protected:
        /**
         * Setup function, ran only once - at the start.
         * Implemented by the test writer, should set all
         * required "global" variables.
         * May also contain testing macros.
         */
        virtual void setUp() {};

        /**
         * Teardown function, ran only once - at the end.
         * Implemented by the test writer, should clean up
         * everything done by the test.
         * May also contain testing macros.
         */
        virtual void tearDown() {};

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
        void _record(const char *code,
                     TestResult result,
                     const char *file,
                     u64 line,
                     const char *function,
                     std::string text);

        /**
         * Add prepared record.
         * @param record Adding this record.
         */
        void _record(TestRecord &record);

        /**
         * Enter a new test case.
         * @param name Name of the test case.
         * @param description Description string.
         */
        void _enterTestCase(const char *name,
                            const char *description);
    };
}

#endif //TESTING_TESTUNIT_H
