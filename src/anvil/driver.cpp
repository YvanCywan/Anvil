#include "api.hpp"
#include "ninja.hpp"
#include "dependency_manager.hpp"
#include <iostream>
#include <filesystem>

extern "C" void configure(anvil::Project& project);

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    anvil::Project project;
    
    configure(project);

    std::cout << "[Anvil] Graph Loaded: " << project.name << std::endl;
    
    fs::path rootDir = fs::current_path();
    std::cout << "[Anvil] Working Directory: " << rootDir << std::endl;

    anvil::DependencyManager deps(rootDir / ".anvil" / "tools");

    try {
        fs::path ninjaExe = deps.get_ninja();

        {
            anvil::NinjaWriter writer("build.ninja");
            writer.generate(project);
        }

        std::cout << "[Anvil] Executing Ninja..." << std::endl;
        std::string cmd = ninjaExe.string();
        return std::system(cmd.c_str());

    } catch (const std::exception& e) {
        std::cerr << "[Anvil Error] " << e.what() << std::endl;
        return 1;
    }
}