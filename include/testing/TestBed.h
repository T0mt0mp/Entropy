/**
 * @file testing/TestBed.h
 * @author Tomas Polasek
 * @brief Unit testing suit.
 */

#ifndef TESTING_TESTBED_H
#define TESTING_TESTBED_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <csignal>

#include "testing/TestUnit.h"

namespace tb
{
    /// Function type for creating test instances.
    /// Returns pointer to the TestUnit allocated using "new"!
    using UnitFactoryFun = TestUnit*();

    class TestHolder
    {
    public:
        TestHolder(const TestHolder &other) = delete;
        TestHolder(TestHolder &&other);
        TestHolder &operator=(const TestHolder &other) = delete;
        TestHolder &operator=(TestHolder &&other) = delete;

        /**
         * Create TestUnit holder.
         * @param factory Factory function for creating the TestUnit.
         */
        TestHolder(UnitFactoryFun *factory);
        ~TestHolder();

        /**
         * Get instance of the TestUnit.
         * @return Instance of the TestUnit created by the factory function.
         */
        TestUnit *instance();

        /**
         * Reset instance of the TestUnit.
         * Lazy method, does not actually create new one,
         * until instance() is called.
         */
        void resetInstance();
    private:
        /**
         * Fill instance member with a valid ptr and set the flag.
         */
        void fillInstance();

        /// Factory function for creating the TestUnit objects.
        UnitFactoryFun *mFactory{nullptr};
        /// Holds the instance of the TestUnit.
        TestUnit *mInstance{nullptr};
        /// Is the instance filled with valid ptr?
        i8 mInstanceValid{false};
    protected:
    };

    /**
     * Test manager. Singleton.
     * Contains list of tests.
     */
    class TestBed
    {
    public:
        /// Status enum for the testing.
        enum class TestStatus
        {
            /// All tests were successful.
            SUCCESS = 0,
            /// At least one test failed.
            FAILED,
            /// Tests did not finish.
            DNF
        };

        /**
         * Register TestUnit instance to this TestBed.
         * @param test TestUnit to add.
         */
        void registerTest(UnitFactoryFun *factory);

        /**
         * Run all registered tests.
         */
        void runTests();

        /**
         * Reset internal structure.
         */
        void resetTests();

        /**
         * Report information about tests.
         */
        void report();

        /**
         * Return status code for the finished tests.
         * @return
         */
        TestStatus reportSuccess();

        /**
         * Get singleton instance of the TestBed.
         * @return Singleton instance of the TestBed.
         */
        static TestBed &instance();
    private:
        TestBed();
        ~TestBed();
    protected:
        /// List of tests.
        std::vector<TestHolder> mTests;
    };
}

#endif //TESTING_TESTBED_H
