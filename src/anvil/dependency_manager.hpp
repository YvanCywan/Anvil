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

            std::string url;
#if defined(_WIN32)
            url = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip";
#elif defined(__APPLE__)
            url = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-mac.zip";
#elif defined(__linux__)
            url = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip";
#else
            throw std::runtime_error("Unsupported OS for automatic Ninja download");
#endif

            fs::path zipPath = toolsDir / "ninja.zip";

            std::string cmd = "curl -L -o " + zipPath.string() + " " + url;
            if (!exec(cmd)) {
                throw std::runtime_error("Failed to download Ninja");
            }

#ifdef _WIN32
            cmd = "tar -xf " + zipPath.string() + " -C " + toolsDir.string();
#else
            cmd = "unzip -o " + zipPath.string() + " -d " + toolsDir.string();
#endif

            if (!exec(cmd)) {
                throw std::runtime_error("Failed to unzip Ninja");
            }

            fs::remove(zipPath);

#ifndef _WIN32
            fs::permissions(ninjaPath, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add);
#endif

            return ninjaPath;
        }
    };
}