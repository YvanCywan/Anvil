#include "src/anvil/api.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

#ifndef ANVIL_API_V2
// Legacy support for bootstrapping with Anvil 0.1.x
void anvil::Project::add_executable(const std::string &name, const std::function<void(CppApplication &)> &config) {
    application.name = name;
    config(application);
}
#endif

void configure(anvil::Project& project) {
    project.name = "Anvil";

    project.add_executable("anvil", [](anvil::CppApplication& app) {
        app.add_include("src");
    });

    project.add_test("anvil_tests", [](anvil::CppApplication& app) {
    });
}
