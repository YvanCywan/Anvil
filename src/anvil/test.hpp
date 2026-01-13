#pragma once
#include <iostream>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>

namespace anvil {

    class TestSuite {
    public:
        virtual ~TestSuite() = default;
        virtual void setup() {}
        virtual void tearDown() {}
    };

    using TestMethod = std::function<void(TestSuite*)>;

    struct TestInfo {
        std::string name;
        TestMethod method;
    };

    class TestRegistry {
    public:
        static TestRegistry& instance() {
            static TestRegistry inst;
            return inst;
        }

        void registerTest(const std::string& suiteName, const std::string& testName, std::function<TestSuite*()> factory, TestMethod method) {
            suites[suiteName].factory = std::move(factory);
            suites[suiteName].tests.push_back({testName, std::move(method)});
        }

        int runAll() {
            int passed = 0;
            int failed = 0;

            for (const auto& [suiteName, suiteInfo] : suites) {
                std::cout << "[Suite] " << suiteName << std::endl;

                for (const auto& test : suiteInfo.tests) {
                    std::cout << "  [Test] " << test.name << "... \n";

                    std::unique_ptr<TestSuite> instance(suiteInfo.factory());
                    try {
                        instance->setup();
                        test.method(instance.get());
                        instance->tearDown();
                        std::cout << "PASSED" << std::endl;
                        passed++;
                    } catch (const std::exception& e) {
                        std::cout << "FAILED (" << e.what() << ")" << std::endl;
                        failed++;
                    } catch (...) {
                        std::cout << "FAILED (Unknown error)" << std::endl;
                        failed++;
                    }
                }
            }

            std::cout << "\nResults: " << passed << " passed, " << failed << " failed." << std::endl;
            return failed > 0 ? 1 : 0;
        }

    private:
        struct SuiteInfo {
            std::function<TestSuite*()> factory;
            std::vector<TestInfo> tests;
        };
        std::map<std::string, SuiteInfo> suites;
    };

    class TestException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    #define ANVIL_ASSERT(condition) \
        if (!(condition)) { \
            throw anvil::TestException("Assertion failed: " #condition); \
        }

    #define ANVIL_ASSERT_EQUALS(expected, actual) \
        if ((expected) != (actual)) { \
            throw anvil::TestException("Assertion failed: " #expected " != " #actual); \
        }

    template<typename T>
    struct TestRegistrar {
        TestRegistrar(const std::string& suiteName, const std::string& testName, void (T::*method)()) {
            TestRegistry::instance().registerTest(suiteName, testName, []() { return new T(); }, [method](TestSuite* suite) {
                (static_cast<T*>(suite)->*method)();
            });
        }
    };

    #define ANVIL_TEST(Suite, Method) \
        static anvil::TestRegistrar<Suite> registrar_##Suite##_##Method(#Suite, #Method, &Suite::Method);

}

#ifdef ANVIL_TEST_MAIN
int main() {
    return anvil::TestRegistry::instance().runAll();
}
#endif
