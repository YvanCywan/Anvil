#include "api.hpp"
#include "ninja.hpp"
#include "dependency_manager.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

extern "C" void configure(anvil::Project& project);

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    bool runAfterBuild = false;
    std::vector<std::string> runArgs;

    // Simple argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--run") {
            runAfterBuild = true;
        } else if (runAfterBuild) {
            // Collect arguments for the target application
            runArgs.push_back(arg);
        }
    }

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
        int buildResult = std::system(cmd.c_str());

        if (buildResult != 0) {
            return buildResult;
        }

        if (runAfterBuild) {
            fs::path binPath = rootDir / "bin" / project.application.name;
#ifdef _WIN32
            binPath += ".exe";
#endif
            if (fs::exists(binPath)) {
                std::cout << "[Anvil] Running " << project.application.name << "..." << std::endl;
                std::string runCmd = binPath.string();
                for (const auto& arg : runArgs) {
                    runCmd += " " + arg;
                }
                return std::system(runCmd.c_str());
            } else {
                std::cerr << "[Anvil Error] Executable not found: " << binPath << std::endl;
                return 1;
            }
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[Anvil Error] " << e.what() << std::endl;
        return 1;
    }
}