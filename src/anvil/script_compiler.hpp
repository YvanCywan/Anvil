#pragma once
#include <string>
#include <utility>
#include <filesystem>
#include "main/process.hpp"

namespace anvil {
    namespace fs = std::filesystem;

    class ScriptCompiler {
        fs::path sourceDir;
        fs::path buildDir;

    public:
        ScriptCompiler(fs::path  src, fs::path  build)
            : sourceDir(std::move(src)), buildDir(std::move(build)) {}

        [[nodiscard]] fs::path compile(const fs::path& userScript) const {
            fs::create_directories(buildDir);

            fs::path runnerExe = buildDir / "runner";
#ifdef _WIN32
            runnerExe += ".exe";
#endif
            const fs::path driverSrc = sourceDir / "anvil" / "driver.cpp";
            std::string cmd = "clang++ -std=c++20";
            cmd += " -I " + sourceDir.string();
            cmd += " " + driverSrc.string();
            cmd += " " + userScript.string();
            cmd += " -o " + runnerExe.string();

            if (!exec(cmd)) {
                throw std::runtime_error("Failed to compile build script");
            }

            return runnerExe;
        }
    };
}