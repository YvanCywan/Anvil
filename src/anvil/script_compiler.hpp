#pragma once
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include "main/process.hpp"
#include "toolchain.hpp"
#include <iostream>

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

            const fs::path driverSrc = sourceDir / "anvil" / "driver.cpp";

            std::vector<std::string> flags = {
                "-std=c++20",
                "-I " + sourceDir.string(),
                "-DANVIL_API_V2",
                driverSrc.string()
            };

            // Header Discovery: Scan .anvil/libraries/full_deploy for include directories
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
    };
}