#pragma once
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include "main/process.hpp"
#include "toolchain.hpp"

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
            const fs::path driverSrc = sourceDir / "anvil" / "driver.cpp";

            std::vector<std::string> flags = {
                "-std=c++20",
                "-I " + sourceDir.string(),
                "-DANVIL_API_V2",
                driverSrc.string()
            };

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