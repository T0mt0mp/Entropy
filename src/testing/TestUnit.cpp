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
        out << TestRecord::resultToStr(tr.mResult) <<
            " {" << tr.mCode << "} " <<
            tr.mFile << ":" << tr.mLine <<
            "(" << tr.mFunction << ") " <<
            tr.mText;
        return out;
    }

    TestUnit::TestUnit(const char *name, u32 numRuns) :
        mName{name},
        mNumRuns{numRuns},
        mStatus{TestStatus::BEFORE_SETUP}
    {
    }

    void TestUnit::unitSetUp()
    {
        reset();

        try
        {
            setUp();
        } catch(...)
        {
            currentStatistics().mExceptionsEscaped++;
        }

        if ( !(currentStatistics().mNumFailed ||
               currentStatistics().mExceptionsEscaped)
           )
        {
            mStatus = TestStatus::RUNNABLE;
        }
        else
        {
            mStatus = TestStatus::FAILED_SETUP;
        }
    }

    void TestUnit::unitTearDown()
    {
        mStatus = TestStatus::BEFORE_SETUP;

        try
        {
            tearDown();
        } catch(...)
        {
            currentStatistics().mExceptionsEscaped++;
            mStatus = TestStatus ::FAILED_TEARDOWN;
        }
    }

    void TestUnit::unitRunTest()
    {
        try
        {
            testMain();
        } catch(...)
        {
            currentStatistics().mExceptionsEscaped++;
        }

        mNumRan++;

        if (mNumRan < mNumRuns)
        {
            mStatus = TestStatus::RUNNABLE;
            newStatistics();
        }
        else
        {
            mStatus = TestStatus::BEFORE_TEARDOWN;
        }
    }

    void TestUnit::reset()
    {
        mNumRan = 0;

        newStatistics();

        mStatus = TestStatus::BEFORE_SETUP;
    }

    TestStatus TestUnit::getStatus()
    {
        return mStatus;
    }

    void TestUnit::printInfo(std::ostream &out)
    {
        if (mStatus != TestStatus::BEFORE_SETUP)
        {
            out << "Test : " << mName << " Failed to execute." << std::endl;
        }

        out << "Test : " << mName
            << " [" << mNumRan << " / "  << mNumRuns << "]" << std::endl;

        if (mRuns.size() != mNumRan)
        {
            out << "Warning : Not printing info about non-complete tests!"
                << std::endl;
        }

        for (u32 run = 0; run < mNumRan; ++run)
        {
            TestRun &tr = mRuns[run];

            out << "Run #" << run << std::endl;
            out << "[ " << tr.mNumPassed << " / "
                << tr.mNumFailed << " / "
                << tr.mNumTests << " ] "
                << "{! " << tr.mExceptionsEscaped << "}" << std::endl;

            for (TestRecord &record : tr.mRecords)
            {
                if (record.shouldBePrinted())
                {
                    out << record << std::endl;
                }
            }

            out << "End of run #" << run << "\n" << std::endl;
        }
    }

    TestRun &TestUnit::currentStatistics()
    {
        ASSERT_FAST(!mRuns.empty());
        return mRuns.back();
    }

    i8 TestUnit::run()
    {
        if (mStatus != TestStatus::BEFORE_SETUP)
        {
            return true;
        }

        unitSetUp();

        if (mStatus != TestStatus::RUNNABLE)
        {
            return true;
        }

        do
        {
            unitRunTest();
        } while (mStatus == TestStatus::RUNNABLE);

        if (mStatus != TestStatus::BEFORE_TEARDOWN)
        {
            return true;
        }

        unitTearDown();

        if (mStatus != TestStatus::BEFORE_SETUP)
        {
            return false;
        }

        return false;
    }

    void TestUnit::record(const char *code,
                          TestResult result,
                          const char *file,
                          u64 line,
                          const char *function,
                          std::string text)
    {
        TestRun &tr = currentStatistics();
        tr.mNumTests++;

        switch (result)
        {
            case TestResult::PASSED:
                tr.mNumPassed++;
                break;
            case TestResult::FAILED:
                tr.mNumFailed++;
                break;
            case TestResult::EXCEPTION:
                tr.mExceptionsEscaped++;
                break;
            default:
                break;
        }

        tr.mRecords.push_back(
            TestRecord(code, result, file, line, function, text));
    }

    void TestUnit::newStatistics()
    {
        mRuns.push_back(TestRun());
    }

    const char *TestRecord::resultToStr(TestResult result)
    {
        switch (result)
        {
            case TestResult::PASSED:
                return "Passed";
            case TestResult::FAILED:
                return "Failed";
            case TestResult::EXCEPTION:
                return "Except";
            default:
                return "Unknown";
        }
    }

    bool TestRecord::shouldBePrinted()
    {
        return mResult != TestResult::PASSED;
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
}


