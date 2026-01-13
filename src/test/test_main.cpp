#include "anvil/test.hpp"
#include "main/app.hpp"

class AppTests : public anvil::TestSuite {
public:
    void testNoArgs() {
        char* argv[] = { (char*)"anvil" };
        int result = anvil::App::run(1, argv);
        ANVIL_ASSERT_EQUALS(0, result);
    }

    void testUnknownCommand() {
        char* argv[] = { (char*)"anvil", (char*)"unknown" };
        int result = anvil::App::run(2, argv);
        ANVIL_ASSERT_EQUALS(1, result);
    }
};

ANVIL_TEST(AppTests, testNoArgs)
ANVIL_TEST(AppTests, testUnknownCommand)
