/**
*    Copyright (C) 2008 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    As a special exception, the copyright holders give permission to link the
*    code of portions of this program with the OpenSSL library under certain
*    conditions as described in each individual source file and distribute
*    linked combinations including the program with the OpenSSL library. You
*    must comply with the GNU Affero General Public License in all respects
*    for all of the code used other than as permitted herein. If you modify
*    file(s) with this exception, you may extend this exception to your
*    version of the file(s), but you are not obligated to do so. If you do not
*    wish to do so, delete this exception statement from your version. If you
*    delete this exception statement from all source files in the program,
*    then also delete it in the license file.
*/

/*
 * A C++ unit testing framework, taken from mongo/src/unittest/
 *
 */

#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#define STATUS_OK 1

/**
 * Fail unconditionally, reporting the given message.
 */
#define FAIL(MESSAGE) ::unittest::TestAssertion( __FILE__ , __LINE__ ).fail( (MESSAGE) )

/**
 * Fails unless "EXPRESSION" is true.
 */
#define ASSERT_TRUE(EXPRESSION) ::unittest::TestAssertion( __FILE__, __LINE__ ).failIf( \
            !(EXPRESSION), "Expected: " #EXPRESSION )
#define ASSERT(EXPRESSION) ASSERT_TRUE(EXPRESSION)

/**
 * Assert that a Status code is OK.
 */
#define ASSERT_OK(EXPRESSION) ASSERT_EQUALS(STATUS_OK, (EXPRESSION))

/**
 * Assert that a status code is anything but OK.
 */
#define ASSERT_NOT_OK(EXPRESSION) ASSERT_NOT_EQUALS(STATUS_OK, (EXPRESSION))

/**
 * Fails if "EXPRESSION" is true.
 */
#define ASSERT_FALSE(EXPRESSION) ::unittest::TestAssertion( __FILE__, __LINE__ ).failIf( \
            (EXPRESSION), "Expected: !(" #EXPRESSION ")" )

/*
 * Binary comparison assertions.
 */
#define ASSERT_EQUALS(a,b) _ASSERT_COMPARISON(Equal, a, b)
#define ASSERT_NOT_EQUALS(a,b) _ASSERT_COMPARISON(NotEqual, a, b)
#define ASSERT_LESS_THAN(a, b) _ASSERT_COMPARISON(LessThan, a, b)
#define ASSERT_NOT_LESS_THAN(a, b) _ASSERT_COMPARISON(NotLessThan, a, b)
#define ASSERT_GREATER_THAN(a, b) _ASSERT_COMPARISON(GreaterThan, a, b)
#define ASSERT_NOT_GREATER_THAN(a, b) _ASSERT_COMPARISON(NotGreaterThan, a, b)
#define ASSERT_LESS_THAN_OR_EQUALS(a, b) ASSERT_NOT_GREATER_THAN(a, b)
#define ASSERT_GREATER_THAN_OR_EQUALS(a, b) ASSERT_NOT_LESS_THAN(a, b)

/**
 * Binary comparison utility macro.  Do not use directly.
 */
#define _ASSERT_COMPARISON(COMPARISON, a, b) ::unittest::ComparisonAssertion( \
            #a, #b , __FILE__ , __LINE__ ).assert##COMPARISON( (a), (b) )

/**
 * Approximate equality assertion. Useful for comparisons on limited precision floating point
 * values.
 */
#define ASSERT_APPROX_EQUAL(a,b,ABSOLUTE_ERR) ::unittest::assertApproxEqual( \
            #a, #b, a, b, ABSOLUTE_ERR, __FILE__, __LINE__)

/**
 * Verify that the evaluation of "EXPRESSION" throws an exception of type EXCEPTION_TYPE.
 *
 * If "EXPRESSION" throws no exception, or one that is neither of type "EXCEPTION_TYPE" nor
 * of a subtype of "EXCEPTION_TYPE", the test is considered a failure and further evaluation
 * halts.
 */
#define ASSERT_THROWS(EXPRESSION, EXCEPTION_TYPE)                       \
    do {                                                                \
        bool threw = false;                                             \
        ::unittest::TestAssertion _testAssertion( __FILE__, __LINE__ ); \
        try {                                                            \
            EXPRESSION;                                               \
        } catch ( const EXCEPTION_TYPE& ) { threw = true; }            \
        if (!threw)                                                     \
            _testAssertion.fail("Expected expression " #EXPRESSION      \
                                " to throw " #EXCEPTION_TYPE " but it threw nothing."); \
    } while( false )


/**
 * Construct a single test, named "TEST_NAME" within the test case "CASE_NAME".
 *
 * Usage:
 *
 * TEST(MyModuleTests, TestThatFooFailsOnErrors) {
 *     ASSERT_EQUALS(error_success, foo(invalidValue));
 * }
 */
#define TEST(CASE_NAME, TEST_NAME) \
    class _TEST_TYPE_NAME(CASE_NAME, TEST_NAME) : public ::unittest::Test { \
    private:                                                            \
        virtual void _doTest();                                         \
                                                                        \
        static const RegistrationAgent<_TEST_TYPE_NAME(CASE_NAME, TEST_NAME) > _agent; \
    };                                                                  \
    const ::unittest::Test::RegistrationAgent<_TEST_TYPE_NAME(CASE_NAME, TEST_NAME) > \
            _TEST_TYPE_NAME(CASE_NAME, TEST_NAME)::_agent(#CASE_NAME, #TEST_NAME); \
    void _TEST_TYPE_NAME(CASE_NAME, TEST_NAME)::_doTest()

/**
 * Construct a single test named TEST_NAME that has access to a common class (a "fixture")
 * named "FIXTURE_NAME".
 *
 * Usage:
 *
 * class FixtureClass : public unittest::Test {
 * protected:
 *   int myVar;
 *   void setUp() { myVar = 10; }
 * };
 *
 * TEST(FixtureClass, TestThatUsesFixture) {
 *     ASSERT_EQUALS(10, myVar);
 * }
 */
#define TEST_F(FIXTURE_NAME, TEST_NAME) \
    class _TEST_TYPE_NAME(FIXTURE_NAME, TEST_NAME) : public FIXTURE_NAME { \
    private:                                                            \
        virtual void _doTest();                                         \
                                                                        \
        static const RegistrationAgent<_TEST_TYPE_NAME(FIXTURE_NAME, TEST_NAME) > _agent; \
    };                                                                  \
    const ::unittest::Test::RegistrationAgent<_TEST_TYPE_NAME(FIXTURE_NAME, TEST_NAME) > \
            _TEST_TYPE_NAME(FIXTURE_NAME, TEST_NAME)::_agent(#FIXTURE_NAME, #TEST_NAME); \
    void _TEST_TYPE_NAME(FIXTURE_NAME, TEST_NAME)::_doTest()

/**
 * Macro to construct a type name for a test, from its "CASE_NAME" and "TEST_NAME".
 * Do not use directly in test code.
 */
#define _TEST_TYPE_NAME(CASE_NAME, TEST_NAME)   \
    UnitTest__##CASE_NAME##__##TEST_NAME

namespace unittest {

    class Result;

    /**
     * Type representing the function composing a test.
     */
    typedef void(*TestFunction)(void);

    /**
     * Container holding a test function and its name.  Suites
     * contain lists of these.
     */
    class TestHolder {
    public:
        TestHolder(const std::string& name, const TestFunction& fn)
            : _name(name), _fn(fn) {}

        ~TestHolder() {}
        void run() const { _fn(); }
        std::string getName() const { return _name; }

    private:
        std::string _name;
        TestFunction _fn;
    };

    /**
     * Base type for unit test fixtures.  Also, the default fixture type used
     * by the TEST() macro.
     */
    class Test {
    public:
        Test();
        virtual ~Test();

        void run();

    protected:
        /**
         * Registration agent for adding tests to suites, used by TEST macro.
         */
        template <typename T>
        class RegistrationAgent {
        public:
            RegistrationAgent(const std::string& suiteName, const std::string& testName);
        };

        /**
         * This exception class is used to exercise the testing framework itself. If a test
         * case throws it, the framework would not consider it an error.
         */
        class FixtureExceptionForTesting : public std::exception {
        };

    private:
        /**
         * Called on the test object before running the test.
         */
        virtual void setUp();

        /**
         * Called on the test object after running the test.
         */
        virtual void tearDown();

        /**
         * The test itself.
         */
        virtual void _doTest() = 0;
    };

    /**
     * Representation of a collection of tests.
     *
     * One suite is constructed for each "CASE_NAME" when using the TEST macro.
     * Additionally, tests that are part of dbtests are manually assigned to suites
     * by the programmer by overriding setupTests() in a subclass of Suite.  This
     * approach is deprecated.
     */
    class Suite {
    public:
        Suite( const std::string& name );
        virtual ~Suite();

        template<class T>
        void add() { add<T>(typeid(T)); }

        template<class T , typename A >
        void add( const A& a ) {
            add(typeid(T).name(), a);
        }

        template<class T>
        void add(const std::string& name) {
            add(name, &Suite::runTestObject<T>);
        }

        void add(const std::string& name, const TestFunction& testFn);

        Result * run( const std::string& filter , int runsPerTest );

        static int run( const std::vector<std::string>& suites , const std::string& filter , int runsPerTest );

        /**
         * Get a suite with the given name, creating it if necessary.
         *
         * The implementation of this function must be safe to call during the global static
         * initialization block before main() executes.
         */
        static Suite *getSuite(const std::string& name);

    protected:
        virtual void setupTests();

    private:
        typedef std::vector<TestHolder *> TestHolderList;

        template <typename T>
        static void runTestObject() {
            T testObj;
            testObj.run();
        }

        template <typename T, typename A>
        static void runTestObjectWithArg(const A& a) {
            T testObj(a);
            testObj.run();
        }

        std::string _name;
        TestHolderList _tests;

        void registerSuite( const std::string& name , Suite* s );
    };

    /**
     * Collection of information about failed tests.  Used in reporting
     * failures.
     */
    class TestAssertionFailureDetails {
    public:
        TestAssertionFailureDetails( const std::string& theFile,
                                     unsigned theLine,
                                     const std::string& theMessage );

        const std::string file;
        const unsigned line;
        const std::string message;
    };

    /**
     * Exception thrown when a test assertion fails.
     *
     * Typically thrown by helpers in the TestAssertion class and its ilk, below.
     *
     * NOTE(schwerin): This intentionally does _not_ extend std::exception, so that code under
     * test that (foolishly?) catches std::exception won't swallow test failures.  Doesn't
     * protect you from code that foolishly catches ..., but you do what you can.
     */
    class TestAssertionFailureException {
    public:
        TestAssertionFailureException( const std::string& theFile,
                                       unsigned theLine,
                                       const std::string& theMessage );

        const std::string& getFile() const { return _details.file; }
        unsigned getLine() const { return _details.line; }
        const std::string& getMessage() const { return _details.message; }

        std::string toString() const;

    private:
        TestAssertionFailureDetails _details;
    };

    /**
     * Object representing an assertion about some condition.
     */
    class TestAssertion {

    public:
        /**
         * file string must stay in scope and remain unchanged for the lifetime
         * of the TestAssertion object.
         */
        TestAssertion( const char* file, unsigned line );
        ~TestAssertion();

        void fail( const std::string& message) const;
        void failIf( bool flag, const std::string &message ) const {
            if ( flag ) fail( message );
        }

    private:
        const char* _file;
        const unsigned _line;
    };

    /**
     * Specialization of TestAssertion for binary comparisons.
     */
    class ComparisonAssertion : private TestAssertion {
    public:
        /**
         * All char* arguments must stay in scope and remain unchanged for the lifetime
         * of the ComparisonAssertion object.
         */
        ComparisonAssertion( const char* aexp , const char* bexp ,
                             const char* file , unsigned line );

        template<typename A,typename B>
        void assertEqual( const A& a , const B& b ) {
            if ( a == b )
                return;
            fail(getComparisonFailureMessage("==", a, b));
        }

        template<typename A,typename B>
        void assertNotEqual( const A& a , const B& b ) {
            if ( a != b )
                return;
            fail(getComparisonFailureMessage("!=", a, b));
        }

        template<typename A,typename B>
        void assertLessThan( const A& a , const B& b ) {
            if ( a < b )
                return;
            fail(getComparisonFailureMessage("<", a, b));
        }

        template<typename A,typename B>
        void assertNotLessThan( const A& a , const B& b ) {
            if ( a >= b )
                return;
            fail(getComparisonFailureMessage(">=", a, b));
        }

        template<typename A,typename B>
        void assertGreaterThan( const A& a , const B& b ) {
            if ( a > b )
                return;
            fail(getComparisonFailureMessage(">", a, b));
        }

        template<typename A,typename B>
        void assertNotGreaterThan( const A& a , const B& b ) {
            if ( a <= b )
                return;
            fail(getComparisonFailureMessage("<=", a, b));
        }

    private:
        template< typename A, typename B>
        std::string getComparisonFailureMessage(const std::string &theOperator,
                                                const A& a, const B& b);

        const char* _aexp;
        const char* _bexp;
    };

    /**
     * Helper for ASSERT_APPROX_EQUAL to ensure that the arguments are evaluated only once.
     */
    template < typename A, typename B, typename ABSOLUTE_ERR >
    inline void assertApproxEqual(const char* aexp, const char* bexp,
                                  const A& a, const B& b, const ABSOLUTE_ERR& absoluteErr,
                                  const char* file , unsigned line) {
        if (std::abs(a - b) <= absoluteErr)
            return;
        std::stringstream ss;
        ss << "Expected " << aexp << " and " << bexp << " to be within " << absoluteErr
           << " of each other ((" << a << ") - (" << b << ") = " << (a - b) << ")";
        TestAssertion(file, line).fail(ss.str());
    }


    template <typename T>
    Test::RegistrationAgent<T>::RegistrationAgent(const std::string& suiteName,
                                                  const std::string& testName) {
        Suite::getSuite(suiteName)->add<T>(testName);
    }

    template<typename A, typename B>
    std::string ComparisonAssertion::getComparisonFailureMessage(const std::string &theOperator,
                                                                 const A &a, const B &b) {
        std::ostringstream os;
        os << "Expected " << _aexp << " " << theOperator << " " << _bexp
           << " (" << a << " " << theOperator << " " << b << ")";
        return os.str();
    }

    /**
     * Return a list of suite names.
     */
    std::vector<std::string> getAllSuiteNames();

}  // namespace unittest

