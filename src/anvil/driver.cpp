#include "api.hpp"
#include "ninja.hpp"
#include "dependency_manager.hpp"
#include "pkg.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fmt/core.h>

extern "C" void configure(anvil::Project& project);

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    bool runAfterBuild = false;
    bool runTests = false;
    std::vector<std::string> runArgs;

    // Simple argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--run") {
            runAfterBuild = true;
        } else if (arg == "--test") {
            runTests = true;
        } else if (runAfterBuild) {
            // Collect arguments for the target application
            runArgs.push_back(arg);
        }
    }

    anvil::Project project;
    
    configure(project);

    fmt::print("[Anvil] Graph Loaded: {}\n", project.name);
    
    fs::path rootDir = fs::current_path();
    fmt::print("[Anvil] Working Directory: {}\n", rootDir.string());

    // Handle legacy mode where targets might be empty but application is set
    if (project.targets.empty() && !project.application.name.empty()) {
        project.targets.push_back(project.application);
    }

    // Verify sources exist for all targets
    bool missingSources = false;
    for (const auto& target : project.targets) {
        for (const auto& src : target.sources) {
            fs::path srcPath = rootDir / src;
            if (!fs::exists(srcPath)) {
                fmt::print(stderr, "[Anvil Error] Source file not found: {}\n", srcPath.string());

                fs::path parent = srcPath.parent_path();
                if (fs::exists(parent)) {
                    fmt::print(stderr, "Contents of {}:\n", parent.string());
                    for (const auto& entry : fs::directory_iterator(parent)) {
                        fmt::print(stderr, "  - {}\n", entry.path().filename().string());
                    }
                } else {
                    fmt::print(stderr, "Parent directory {} does not exist.\n", parent.string());

                    // Check if src directory exists at all
                    fs::path srcDir = rootDir / "src";
                    if (fs::exists(srcDir)) {
                         fmt::print(stderr, "Contents of {}:\n", srcDir.string());
                         for (const auto& entry : fs::directory_iterator(srcDir)) {
                            fmt::print(stderr, "  - {}\n", entry.path().filename().string());
                         }
                    }
                }
                missingSources = true;
            }
        }
    }

    if (missingSources) {
        return 1;
    }

    // --- NEW: Resolve Dependencies ---
    try {
        anvil::PackageManager pkgMgr(rootDir / ".anvil" / "libraries");
        pkgMgr.resolve(project);
    } catch (const std::exception& e) {
        return 1;
    }
    // ---------------------------------

    anvil::DependencyManager deps(rootDir / ".anvil" / "tools");

    try {
        fs::path ninjaExe = deps.get_ninja();

        {
            anvil::NinjaWriter writer("build.ninja");
            writer.generate(project);
        }

        fmt::print("[Anvil] Executing Ninja...\n");
        std::string cmd = ninjaExe.string();
        int buildResult = std::system(cmd.c_str());

        if (buildResult != 0) {
            return buildResult;
        }

        if (runTests) {
             bool allTestsPassed = true;
             bool testsFound = false;
             for (const auto& target : project.targets) {
                if (target.type == anvil::AppType::Test) {
                    testsFound = true;
                    fs::path binPath = rootDir / "bin" / target.name;
#ifdef _WIN32
                    binPath += ".exe";
#endif
                    if (fs::exists(binPath)) {
                        fmt::print("[Anvil] Running Test: {}...\n", target.name);
                        int result = std::system(binPath.string().c_str());
                        if (result != 0) {
                            fmt::print(stderr, "[Anvil] Test {} failed.\n", target.name);
                            allTestsPassed = false;
                        } else {
                            fmt::print("[Anvil] Test {} passed.\n", target.name);
                        }
                    } else {
                        fmt::print(stderr, "[Anvil Error] Test executable not found: {}\n", binPath.string());
                        allTestsPassed = false;
                    }
                }
             }

             if (!testsFound) {
                 fmt::print("[Anvil] No tests found.\n");
             }

             if (!allTestsPassed) return 1;
        }

        if (runAfterBuild) {
            // Find the first executable target to run
            const anvil::CppApplication* targetToRun = nullptr;
            for (const auto& target : project.targets) {
                if (target.type == anvil::AppType::Executable) {
                    targetToRun = &target;
                    break;
                }
            }

            if (targetToRun) {
                fs::path binPath = rootDir / "bin" / targetToRun->name;
#ifdef _WIN32
                binPath += ".exe";
#endif
                if (fs::exists(binPath)) {
                    fmt::print("[Anvil] Running {}...\n", targetToRun->name);
                    std::string runCmd = binPath.string();
                    for (const auto& arg : runArgs) {
                        runCmd += " " + arg;
                    }
                    return std::system(runCmd.c_str());
                } else {
                    fmt::print(stderr, "[Anvil Error] Executable not found: {}\n", binPath.string());
                    return 1;
                }
            } else {
                fmt::print("[Anvil] No executable target found to run.\n");
            }
        }

        return 0;

    } catch (const std::exception& e) {
        fmt::print(stderr, "[Anvil Error] {}\n", e.what());
        return 1;
    }
}