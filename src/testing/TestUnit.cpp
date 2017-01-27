/**
 * @file testing/TestUnit.h
 * @author Tomas Polasek
 * @brief Unit testing suit.
 */

#include "testing/TestUnit.h"

namespace tb
{
    std::ostream &operator<<(std::ostream &out,
                             const TestRecord &tr)
    {
        out << "[" << TestRecord::resultToStrShort(tr.mResult)
            << "] {" << tr.mCode << "} : "
            << tr.mText << " < "
            << tr.mFile << ":" << tr.mLine << " > ";
            //<< "(" << tr.mFunction << ") "
        return out;
    }

    TestUnit::TestUnit(const char *name, u32 numRuns) :
        mName{name},
        mNumRuns{numRuns ? numRuns : 1}
    {
    }

    void TestUnit::unitRunTest()
    {
        testMain();

        mNumRan++;

        if (mNumRan < mNumRuns)
        {
            newStatistics();
        }
    }

    void TestUnit::reset()
    {
        mNumRan = 0;

        newStatistics();
    }

    void TestUnit::printInfo(std::ostream &out)
    {
        if (mNumRan < mNumRuns)
        {
            out << "====TestUnit : " << mName << " Failed to execute.====" << std::endl;
        }

        out << "====TestUnit : " << mName
            << " [" << mNumRan << " / "  << mNumRuns << "]====" << std::endl;

        if (mRuns.size() != mNumRan)
        {
            out << "Warning : Not printing info about non-complete tests!"
                << std::endl;
        }

        for (u32 run = 0; run < mNumRan; ++run)
        {
            TestRun &tr = mRuns[run];

            out << "===#" << run + 1
                          << " [ " << tr.getNumPassed() << "P / "
                          << tr.getNumFailed() << "F / "
                          << tr.getNumTests() << "T ] "
                          << "{ " << tr.getExceptionsEscaped() << "E }===" << std::endl;

            tr.printInfo(out);

            out << "===$" << run + 1 << "===" << std::endl;
        }

        out << "====$TestUnit : " << mName << "====\n" << std::endl;
    }

    TestRun &TestUnit::currentStatistics()
    {
        ASSERT_FAST(!mRuns.empty());
        return mRuns.back();
    }

    bool TestUnit::run()
    {
        newStatistics();

        do
        {
            unitRunTest();
        } while (mNumRan < mNumRuns);

        return false;
    }

    void TestUnit::_record(const char *code,
                          TestResult result,
                          const char *file,
                          u64 line,
                          const char *function,
                          std::string text)
    {
        currentStatistics().record(TestRecord(code, result, file, line, function, text));
    }

    void TestUnit::newStatistics()
    {
        mRuns.emplace_back(TestRun());
    }

    void TestUnit::_enterTestCase(const char *name, const char *description)
    {
        currentStatistics().enterTestCase(TestCase(name, description));
    }

    void TestUnit::_record(TestRecord &record)
    {
        currentStatistics().record(TestRecord(record));
    }

    const char *TestRecord::resultToStr(TestResult result)
    {
        switch (result)
        {
            case TestResult::PASSED:
                return "Passed";
            case TestResult::FAILED:
                return "Failed";
            case TestResult::CRIT_FAILED:
                return "Critically failed";
            case TestResult::EXCEPTION:
                return "Except";
            default:
                return "Unknown";
        }
    }

    const char *TestRecord::resultToStrShort(TestResult result)
    {
        switch (result)
        {
            case TestResult::PASSED:
                return "P";
            case TestResult::FAILED:
                return "F";
            case TestResult::CRIT_FAILED:
                return "C";
            case TestResult::EXCEPTION:
                return "E";
            default:
                return "U";
        }
    }

    TestRecord::TestRecord(const char *code,
                           TestResult result,
                           const char *file,
                           u64 line,
                           const char *function,
                           std::string text) :
        mCode{code},
        mResult{result},
        mFile{file},
        mLine{line},
        mFunction{function},
        mText{text}
    {
    }

    void TestRun::enterTestCase(TestCase &&testCase)
    {
        mTestCases.emplace_back(testCase);
    }

    void TestRun::record(TestRecord &&record)
    {
        ASSERT_FAST(!mTestCases.empty());
        mTestCases.back().record(std::forward<TestRecord&&>(record));

        mNumTests++;
        if (record.passed())
            mNumPassed++;
        if (record.exceptionEscaped())
            mExceptionsEscaped++;
        if (record.failed())
            mNumFailed++;
    }

    void TestRun::printInfo(std::ostream &out) const
    {
        for (const TestCase &testCase : mTestCases)
        {
            testCase.printInfo(out);
        }

    }

    TestCase::TestCase(const char *name, const char *description) :
        mName{name},
        mDescription{description}
    {
    }

    void TestCase::record(TestRecord &&record)
    {
        mRecords.emplace_back(record);

        mNumTests++;
        if (record.passed())
            mNumPassed++;
        if (record.exceptionEscaped())
            mExceptionsEscaped++;
        if (record.failed())
            mNumFailed++;
    }

    void TestCase::printInfo(std::ostream &out) const
    {
        out << "==Case : " << mName
            << " [ " << mNumPassed << "P / "
            << mNumFailed << "F / "
            << mNumTests << "T ] "
            << "{ " << mExceptionsEscaped << "E }==\n"
            << mDescription <<  std::endl;
        for (const TestRecord &tr : mRecords)
        {
            if (tr.shouldBePrinted())
            {
                out << tr << std::endl;
            }
        }
    }
}


