#include "src/anvil/api.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void configure(anvil::Project& project) {
    project.name = "Anvil";

    project.add_executable("anvil", [](anvil::CppApplication& app) {
    });

    project.add_test("anvil_tests", [](anvil::CppApplication& app) {
    });
}
