#pragma once
#include "api.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fmt/core.h>

namespace anvil {
    namespace fs = std::filesystem;

    class PackageManager {
        fs::path libDir;

    public:
        explicit PackageManager(fs::path root) : libDir(std::move(root)) {
            ensure_conan_installed();
        }

        void resolve(Project& project) {
            for (auto& target : project.targets) {
                if (target.dependencies.empty()) continue;

                fmt::print("[Anvil] Resolving dependencies for {}...\n", target.name);

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
        void ensure_conan_installed() {
            if (std::system("conan --version > /dev/null 2>&1") == 0) {
                return; // Conan is already installed
            }

            fmt::print("[Anvil] Conan not found. Attempting to install via pip...\n");

            // Try pip3 first, then pip
            if (std::system("pip3 install conan") != 0) {
                if (std::system("pip install conan") != 0) {
                    fmt::print(stderr, "[Anvil Error] Failed to install Conan automatically.\n");
                    fmt::print(stderr, "Please install Python and run 'pip install conan' manually.\n");
                    throw std::runtime_error("Conan installation failed");
                }
            }

            // Verify installation
            if (std::system("conan --version > /dev/null 2>&1") != 0) {
                fmt::print(stderr, "[Anvil Error] Conan installed but not found in PATH.\n");
                fmt::print(stderr, "You may need to restart your terminal or add Python scripts to PATH.\n");
                throw std::runtime_error("Conan not found after installation");
            }

            fmt::print("[Anvil] Conan installed successfully.\n");
        }

        void install_dependency(const std::string& dep) {
            // We use --deployer=full_deploy to get raw artifacts
            // We use --build=missing to ensure binaries exist
            // Quote paths to handle spaces
            std::string cmd = "conan install --requires=" + dep +
                              " --deployer=full_deploy" +
                              " --output-folder=\"" + libDir.string() + "\"" +
                              " --build=missing -v quiet";

            fmt::print("  >> Installing {}...\n", dep);
            int result = std::system(cmd.c_str());
            if (result != 0) {
                fmt::print(stderr, "[Anvil Error] Failed to install dependency: {}\n", dep);
                throw std::runtime_error("Dependency resolution failed");
            }
        }
    };
}
