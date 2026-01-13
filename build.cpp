#include "anvil/api.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void configure(anvil::Project& project) {
    project.name = "Anvil";

    project.add_executable("anvil", [](anvil::CppApplication& app) {
        // Anvil needs access to "src" because it includes "anvil/api.hpp" which is in "src/anvil"
        app.add_include("src");
    });

    project.add_test("anvil_tests", [](anvil::CppApplication& app) {
        // Configuration moved to anvil::Project::add_test
    });
}
