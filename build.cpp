#include "anvil/api.hpp"

void anvil::Project::add_executable(const std::string &name, std::function<void(CppApplication &)> config) {
    application.name = name;
    config(application);
}

void configure(anvil::Project& project) {
    project.name = "Anvil";

    project.add_executable("anvil", [](anvil::CppApplication& app) {
        app.standard = anvil::CppStandard::CPP_20;

        app.add_source("src/main/main.cpp");

        app.add_include("src");
    });
}
