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
        fs::path conanEnvDir;
        std::string pythonCmd;
        std::string conanCmd;

    public:
        explicit PackageManager(fs::path root) : libDir(std::move(root)) {
            conanEnvDir = libDir.parent_path() / "tools" / "conan_env";
            ensure_conan_installed();
        }

        void resolve(Project& project) {
            for (auto& target : project.targets) {
                if (target.dependencies.empty()) continue;

                fmt::print("[Anvil] Resolving dependencies for {}...\n", target.name);

                for (const auto& dep : target.dependencies) {
                    install_dependency(dep);
                }

                if (fs::exists(libDir / "full_deploy")) {
                     for (const auto& entry : fs::recursive_directory_iterator(libDir / "full_deploy")) {
                        if (entry.is_directory()) {
                            if (entry.path().filename() == "include") {
                                target.add_include(entry.path().string());
                            } else if (entry.path().filename() == "lib") {
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
        std::string get_python_command() {
            if (std::system("python3 --version > /dev/null 2>&1") == 0) return "python3";
            if (std::system("python --version > /dev/null 2>&1") == 0) return "python";
            return "";
        }

        void ensure_conan_installed() {
            pythonCmd = get_python_command();
            if (pythonCmd.empty()) {
                fmt::print(stderr, "[Anvil Error] Python not found.\n");
                throw std::runtime_error("Python not found");
            }

#ifdef _WIN32
            fs::path conanExePath = conanEnvDir / "Scripts" / "conan.exe";
#else
            fs::path conanExePath = conanEnvDir / "bin" / "conan";
#endif

            if (fs::exists(conanExePath)) {
                conanCmd = "\"" + conanExePath.string() + "\"";
                return;
            }

            fmt::print("[Anvil] Installing Conan locally to {}...\n", conanEnvDir.string());

            std::string installCmd = pythonCmd + " -m pip install conan --target \"" + conanEnvDir.string() + "\"";

            int result = std::system(installCmd.c_str());
            if (result != 0) {
                fmt::print(stderr, "[Anvil Error] Failed to install Conan locally.\n");
                throw std::runtime_error("Conan installation failed");
            }

            if (!fs::exists(conanExePath)) {
                fmt::print(stderr, "[Anvil Error] Conan installed but executable not found at {}\n", conanExePath.string());
                throw std::runtime_error("Conan executable not found after installation");
            }

            conanCmd = "\"" + conanExePath.string() + "\"";

            std::string profileCmd = conanCmd + " profile detect --force > /dev/null 2>&1";
            std::system(profileCmd.c_str());

            fmt::print("[Anvil] Conan installed successfully.\n");
        }

        void install_dependency(const std::string& dep) {
            std::string cmd = conanCmd + " install --requires=" + dep +
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
