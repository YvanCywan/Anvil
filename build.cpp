#include "anvil/api.hpp"

void configure(anvil::Project& project) {
    project.name = "Anvil";

    project.add_executable("anvil", [](anvil::CppApplication& app) {
        app.standard = anvil::CppStandard::CPP_20;

        app.add_source("src/main/main.cpp");

        app.add_include("src");

#ifdef _WIN32
        app.add_link_flag("-static");
#endif
    });
    project.add_test("anvil_tests", [](anvil::CppApplication& app) {
        app.standard = anvil::CppStandard::CPP_20;
        app.add_source("src/test/test_main.cpp");
        app.add_include("src");
    });
}
