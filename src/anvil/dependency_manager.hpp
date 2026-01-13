#pragma once
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "main/process.hpp"

namespace anvil {
    namespace fs = std::filesystem;

    class DependencyManager {
        fs::path toolsDir;

    public:
        explicit DependencyManager(fs::path root) : toolsDir(std::move(root)) {
            fs::create_directories(toolsDir);
        }

        fs::path get_ninja() {
            fs::path ninjaPath = toolsDir / "ninja";
#ifdef _WIN32
            ninjaPath += ".exe";
#endif
            if (fs::exists(ninjaPath)) {
                return ninjaPath;
            }

            std::cout << "[Anvil] Downloading Ninja..." << std::endl;
            // Simplified download logic - in a real scenario, this would detect OS/Arch
            // For now, assuming macOS/Linux and using a direct URL or a package manager wrapper
            // This is a placeholder for the actual download logic

            // Example: Download a static ninja binary
            // In a real implementation, we would use curl/wget to download from github releases

            // For this prototype, we will assume the user has it or we fail.
            // But the request is to download it.

            // Let's try to download a known version for macOS (since the user is on macOS)
            std::string url = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-mac.zip";
            fs::path zipPath = toolsDir / "ninja.zip";

            std::string cmd = "curl -L -o " + zipPath.string() + " " + url;
            if (!exec(cmd)) {
                throw std::runtime_error("Failed to download Ninja");
            }

            cmd = "unzip -o " + zipPath.string() + " -d " + toolsDir.string();
            if (!exec(cmd)) {
                throw std::runtime_error("Failed to unzip Ninja");
            }

            fs::remove(zipPath);
            fs::permissions(ninjaPath, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add);

            return ninjaPath;
        }
    };
}