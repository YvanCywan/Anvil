#include "anvil/api.hpp"

#ifndef ANVIL_API_V2
// Legacy support for bootstrapping with Anvil 0.1.x
void anvil::Project::add_executable(const std::string &name, std::function<void(CppApplication &)> config) {
    application.name = name;
    config(application);
}
#endif

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
}
