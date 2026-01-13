#pragma once
#include "api.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

namespace anvil {
    namespace fs = std::filesystem;

    class PackageManager {
        fs::path libDir;

    public:
        explicit PackageManager(fs::path root) : libDir(std::move(root)) {}

        void resolve(Project& project) {
            for (auto& target : project.targets) {
                if (target.dependencies.empty()) continue;

                std::cout << "[Anvil] Resolving dependencies for " << target.name << "..." << std::endl;

                // 1. Install dependencies via Conan
                for (const auto& dep : target.dependencies) {
                    install_dependency(dep);
                }

                // 2. Scan the deployed artifacts and link them
                // The full_deploy structure is usually: .anvil/libraries/full_deploy/host/<name>/<version>/...
                // We scan recursively for "include" and "lib" directories.
                if (fs::exists(libDir / "full_deploy")) {
                     for (const auto& entry : fs::recursive_directory_iterator(libDir / "full_deploy")) {
                        if (entry.is_directory()) {
                            if (entry.path().filename() == "include") {
                                target.add_include(entry.path().string());
                            } else if (entry.path().filename() == "lib") {
                                // Add all static libs found in lib folders
                                for (const auto& lib : fs::directory_iterator(entry.path())) {
                                    std::string ext = lib.path().extension().string();
                                    if (ext == ".a" || ext == ".lib") {
                                        target.add_link_flag(lib.path().string());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    private:
        void install_dependency(const std::string& dep) {
            // We use --deployer=full_deploy to get raw artifacts
            // We use --build=missing to ensure binaries exist
            std::string cmd = "conan install --requires=" + dep +
                              " --deployer=full_deploy" +
                              " --output-folder=" + libDir.string() +
                              " --build=missing -v quiet";

            std::cout << "  >> Installing " << dep << "..." << std::endl;
            int result = std::system(cmd.c_str());
            if (result != 0) {
                std::cerr << "[Anvil Error] Failed to install dependency: " << dep << std::endl;
                std::cerr << "Ensure 'conan' is in your PATH." << std::endl;
                throw std::runtime_error("Dependency resolution failed");
            }
        }
    };
}