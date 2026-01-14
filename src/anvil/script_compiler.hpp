#pragma once
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include "main/process.hpp"
#include "toolchain.hpp"
#include <iostream>
#include <fstream>

#if __has_include("embedded_resources.hpp")
#include "embedded_resources.hpp"
#endif

namespace anvil {
    namespace fs = std::filesystem;

    class ScriptCompiler {
        fs::path sourceDir;
        fs::path buildDir;
        std::unique_ptr<Toolchain> toolchain;

    public:
        ScriptCompiler(fs::path src, fs::path build, std::unique_ptr<Toolchain> tc = nullptr)
            : sourceDir(std::move(src)), buildDir(std::move(build)) {
            if (tc) {
                toolchain = std::move(tc);
            } else {
                toolchain = std::make_unique<ClangToolchain>();
            }
        }

        [[nodiscard]] fs::path compile(const fs::path& userScript) const {
            fs::create_directories(buildDir);

            // Hydrate embedded files if available
            fs::path embeddedDir = buildDir / "embedded_src";
            hydrate_embedded_files(embeddedDir);

            // Ensure bootstrap JSON is available
            fs::path bootstrapDir = buildDir / "bootstrap";
            ensure_bootstrap_json(bootstrapDir);

            fs::path runnerExe = buildDir / "runner";
#ifdef _WIN32
            runnerExe += ".exe";
#endif

            // Check if we can skip compilation
            if (fs::exists(runnerExe) && fs::exists(userScript)) {
                auto runnerTime = fs::last_write_time(runnerExe);
                auto scriptTime = fs::last_write_time(userScript);

                if (runnerTime > scriptTime) {
                    // Runner is newer than the script, so we can reuse it
                    return runnerExe;
                }
            }

            // Use embedded driver if available, otherwise fallback to sourceDir
            fs::path driverSrc;
            if (fs::exists(embeddedDir / "anvil" / "driver.cpp")) {
                driverSrc = embeddedDir / "anvil" / "driver.cpp";
            } else {
                driverSrc = sourceDir / "anvil" / "driver.cpp";
            }

            std::vector<std::string> flags = {
                "-std=c++20",
                "-DANVIL_API_V2",
                driverSrc.string()
            };

            // Add include paths
            if (fs::exists(embeddedDir)) {
                flags.push_back("-I " + embeddedDir.string());
            }

            // Add bootstrap directory for json.hpp
            if (fs::exists(bootstrapDir)) {
                flags.push_back("-I " + bootstrapDir.string());
            }

            // Fallback include
            flags.push_back("-I " + sourceDir.string());

            // Header Discovery: Scan .anvil/libraries/full_deploy for include directories
            // This is still needed for other dependencies the user might add
            fs::path libDir = buildDir / "libraries" / "full_deploy";
            if (fs::exists(libDir)) {
                for (const auto& entry : fs::recursive_directory_iterator(libDir)) {
                    if (entry.is_directory() && entry.path().filename() == "include") {
                        flags.push_back("-I " + entry.path().string());
                    }
                }
            }

#ifdef _WIN32
            flags.push_back("-static");
#endif

            std::string cmd = toolchain->getCompileCommand(userScript, runnerExe, flags);

            std::cout << "[Anvil] Compiling build script with: " << toolchain->getCompiler() << std::endl;

            if (!exec(cmd)) {
                throw std::runtime_error("Failed to compile build script");
            }

            return runnerExe;
        }

    private:
        void hydrate_embedded_files(const fs::path& targetDir) const {
#if __has_include("embedded_resources.hpp")
            if (!fs::exists(targetDir)) {
                fs::create_directories(targetDir);
            }

            auto files = get_embedded_files();
            for (const auto& [name, content] : files) {
                fs::path filePath = targetDir / name;
                fs::create_directories(filePath.parent_path());

                std::ofstream out(filePath, std::ios::binary);
                out.write(content.data(), content.size());
            }
#endif
        }

        void ensure_bootstrap_json(const fs::path& targetDir) const {
            fs::path jsonPath = targetDir / "nlohmann" / "json.hpp";
            if (fs::exists(jsonPath)) {
                return;
            }

            std::cout << "[Anvil] Bootstrapping: Downloading json.hpp..." << std::endl;
            fs::create_directories(jsonPath.parent_path());

            std::string url = "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp";

#ifdef _WIN32
            // Use a temporary powershell script to avoid quoting issues
            fs::path scriptPath = targetDir / "download_json.ps1";
            {
                std::ofstream scriptFile(scriptPath);
                scriptFile << "$ProgressPreference = 'SilentlyContinue'\n";
                scriptFile << "Invoke-WebRequest -Uri '" << url << "' -OutFile '" << jsonPath.string() << "'\n";
                scriptFile << "if (!$?) { exit 1 }\n";
            }

            std::string cmd = "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() + "\"";

            if (!exec(cmd)) {
                fs::remove(scriptPath);
                throw std::runtime_error("Failed to download json.hpp via PowerShell");
            }
            fs::remove(scriptPath);
#else
            std::string cmd = "curl -L -o " + jsonPath.string() + " " + url;
            if (!exec(cmd)) {
                throw std::runtime_error("Failed to download json.hpp");
            }
#endif
        }
    };
}