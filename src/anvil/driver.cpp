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

    // Verify sources exist
    bool missingSources = false;
    for (const auto& src : project.application.sources) {
        fs::path srcPath = rootDir / src;
        if (!fs::exists(srcPath)) {
            std::cerr << "[Anvil Error] Source file not found: " << srcPath << std::endl;

            fs::path parent = srcPath.parent_path();
            if (fs::exists(parent)) {
                std::cerr << "Contents of " << parent << ":" << std::endl;
                for (const auto& entry : fs::directory_iterator(parent)) {
                    std::cerr << "  - " << entry.path().filename() << std::endl;
                }
            } else {
                std::cerr << "Parent directory " << parent << " does not exist." << std::endl;

                // Check if src directory exists at all
                fs::path srcDir = rootDir / "src";
                if (fs::exists(srcDir)) {
                     std::cerr << "Contents of " << srcDir << ":" << std::endl;
                     for (const auto& entry : fs::directory_iterator(srcDir)) {
                        std::cerr << "  - " << entry.path().filename() << std::endl;
                     }
                }
            }
            missingSources = true;
        }
    }

    if (missingSources) {
        return 1;
    }

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