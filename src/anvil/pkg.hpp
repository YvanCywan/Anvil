#pragma once
#include "api.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include <algorithm>

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
            std::set<std::string> all_deps_set;
            for (const auto& target : project.targets) {
                for (const auto& dep : target.dependencies) {
                    all_deps_set.insert(dep);
                }
            }

            if (all_deps_set.empty()) {
                return;
            }

            std::cout << "[Anvil] Resolving project dependencies..." << std::endl;

            for (const auto& dep : all_deps_set) {
                install_dependency(dep);
            }

            std::vector<std::string> include_paths;
            std::vector<std::string> link_flags;
            if (fs::exists(libDir / "full_deploy")) {
                 for (const auto& entry : fs::recursive_directory_iterator(libDir / "full_deploy")) {
                    if (entry.is_directory()) {
                        if (entry.path().filename() == "include") {
                            include_paths.push_back(entry.path().string());
                        } else if (entry.path().filename() == "lib") {
                            for (const auto& lib : fs::directory_iterator(entry.path())) {
                                std::string ext = lib.path().extension().string();
                                if (ext == ".a" || ext == ".lib") {
                                    link_flags.push_back(lib.path().string());
                                }
                            }
                        }
                    }
                }
            }

            if (!include_paths.empty() || !link_flags.empty()) {
                std::cout << "[Anvil] Linking dependencies to all targets." << std::endl;
                for (auto& target : project.targets) {
                    for(const auto& p : include_paths) {
                        target.add_include(p);
                    }
                    for(const auto& f : link_flags) {
                        target.add_link_flag(f);
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

        std::string make_env_command(const std::string& cmd) {
#ifdef _WIN32
            return "set PYTHONPATH=" + conanEnvDir.string() + " && " + cmd;
#else
            return "PYTHONPATH=\"" + conanEnvDir.string() + "\" " + cmd;
#endif
        }

        void ensure_conan_installed() {
            pythonCmd = get_python_command();
            if (pythonCmd.empty()) {
                std::cerr << "[Anvil Error] Python not found." << std::endl;
                throw std::runtime_error("Python not found");
            }

            // Try 'python -m conan' (Conan 2.0 standard)
            conanCmd = pythonCmd + " -m conan";
            std::string checkCmd = make_env_command(conanCmd + " --version > /dev/null 2>&1");

            if (std::system(checkCmd.c_str()) == 0) {
                return;
            }

            // Try 'python -m conans.conan' (Legacy/Alternative)
            std::string legacyCmd = pythonCmd + " -m conans.conan";
            std::string checkLegacy = make_env_command(legacyCmd + " --version > /dev/null 2>&1");
            if (std::system(checkLegacy.c_str()) == 0) {
                conanCmd = legacyCmd;
                return;
            }

            std::cout << "[Anvil] Installing Conan locally to " << conanEnvDir.string() << "..." << std::endl;

            if (!fs::exists(conanEnvDir)) {
                fs::create_directories(conanEnvDir);
            }

            std::string installCmd = pythonCmd + " -m pip install conan --target \"" + conanEnvDir.string() + "\"";

            int result = std::system(installCmd.c_str());
            if (result != 0) {
                std::cerr << "[Anvil Error] Failed to install Conan locally." << std::endl;
                throw std::runtime_error("Conan installation failed");
            }

            // Verify again
            if (std::system(checkCmd.c_str()) == 0) {
                // conanCmd is already set to 'python -m conan'
            } else if (std::system(checkLegacy.c_str()) == 0) {
                conanCmd = legacyCmd;
            } else {
                std::cerr << "[Anvil Error] Conan installed but failed to run from local environment." << std::endl;
                // Debug output
                std::string debugCmd = make_env_command(conanCmd + " --version");
                std::system(debugCmd.c_str());
                throw std::runtime_error("Conan not working after local installation");
            }

            std::string profileCmd = make_env_command(conanCmd + " profile detect --force > /dev/null 2>&1");
            std::system(profileCmd.c_str());

            std::cout << "[Anvil] Conan installed successfully." << std::endl;
        }

        void install_dependency(const std::string& dep) {
            std::string cmd = make_env_command(
                conanCmd + " install --requires=" + dep +
                " --deployer=full_deploy" +
                " --output-folder=\"" + libDir.string() + "\"" +
                " --build=missing -v quiet"
            );

            std::cout << "  >> Installing " << dep << "..." << std::endl;
            int result = std::system(cmd.c_str());
            if (result != 0) {
                std::cerr << "[Anvil Error] Failed to install dependency: " << dep << std::endl;
                throw std::runtime_error("Dependency resolution failed");
            }
        }
    };
}
