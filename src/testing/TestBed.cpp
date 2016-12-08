/**
 * @file testing/TestBed.h
 * @author Tomas Polasek
 * @brief Unit testing suit.
 */

#include "testing/TestBed.h"

namespace tb
{
    TestBed &TestBed::instance()
    {
        static TestBed tb;

        return tb;
    }

    TestBed::TestBed()
    {

    }

    TestBed::~TestBed()
    {

    }

    void TestBed::registerTest(UnitFactoryFun *factory)
    {
        mTests.push_back(TestHolder(factory));
    }

    void TestBed::runTests()
    {
        for (TestHolder &th : mTests)
        {
            th.instance()->run();
        }
    }

    void TestBed::report()
    {
        for (TestHolder &th: mTests)
        {
            th.instance()->printInfo(std::cout);
        }
    }

    void TestBed::resetTests()
    {
        for (TestHolder &th : mTests)
        {
            th.resetInstance();
        }
    }

    TestHolder::TestHolder(UnitFactoryFun *factory) :
        mFactory{factory}
    {
    }

    TestUnit *TestHolder::instance()
    {
        if (!mInstanceValid)
        {
            fillInstance();
        }

        return mInstance;
    }

    void TestHolder::fillInstance()
    {
        mInstance = mFactory();
        mInstanceValid = true;
    }

    void TestHolder::resetInstance()
    {
        if (mInstanceValid)
        {
            mInstanceValid = false;
            delete mInstance;
        }

        mInstance = nullptr;
    }

    TestHolder::~TestHolder()
    {
        if (mInstanceValid)
        {
            delete mInstance;
        }
    }

    TestHolder::TestHolder(TestHolder &&other)
    {
        if (this == &other)
        {
            return;
        }

        std::swap(mInstance, other.mInstance);
        std::swap(mInstanceValid, other.mInstanceValid);
        std::swap(mFactory, other.mFactory);
    }
}
