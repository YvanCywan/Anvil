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

                std::cout << "[Anvil] Resolving dependencies for " << target.name << "..." << std::endl;

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
                std::cerr << "[Anvil Error] Python not found." << std::endl;
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

            std::cout << "[Anvil] Installing Conan locally to " << conanEnvDir << "..." << std::endl;

            std::string installCmd = pythonCmd + " -m pip install conan --target \"" + conanEnvDir.string() + "\"";

            int result = std::system(installCmd.c_str());
            if (result != 0) {
                std::cerr << "[Anvil Error] Failed to install Conan locally." << std::endl;
                throw std::runtime_error("Conan installation failed");
            }

            if (!fs::exists(conanExePath)) {
                std::cerr << "[Anvil Error] Conan installed but executable not found at " << conanExePath << std::endl;
                throw std::runtime_error("Conan executable not found after installation");
            }

            conanCmd = "\"" + conanExePath.string() + "\"";

            std::string profileCmd = conanCmd + " profile detect --force > /dev/null 2>&1";
            std::system(profileCmd.c_str());

            std::cout << "[Anvil] Conan installed successfully." << std::endl;
        }

        void install_dependency(const std::string& dep) {
            std::string cmd = conanCmd + " install --requires=" + dep +
                              " --deployer=full_deploy" +
                              " --output-folder=\"" + libDir.string() + "\"" +
                              " --build=missing -v quiet";

            std::cout << "  >> Installing " << dep << "..." << std::endl;
            int result = std::system(cmd.c_str());
            if (result != 0) {
                std::cerr << "[Anvil Error] Failed to install dependency: " << dep << std::endl;
                throw std::runtime_error("Dependency resolution failed");
            }
        }
    };
}
