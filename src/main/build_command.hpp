#pragma once
#include "cli.hpp"
#include "anvil/script_compiler.hpp"
#include "process.hpp"
#include <filesystem>
#include <iostream>
#include <fmt/core.h>

namespace fs = std::filesystem;

namespace anvil {
    class BuildCommand : public Command {
    public:
        [[nodiscard]] std::string getName() const override {
            return "build";
        }

        [[nodiscard]] std::string getDescription() const override {
            return "Compiles and runs the build script";
        }

        int execute(const std::vector<std::string> &args, const std::string &exePath) override {
            fs::path rootDir = fs::current_path();
            fs::path userScript = rootDir / "build.cpp";

            // Derive include path from executable location
            fs::path exeDir = fs::absolute(exePath).parent_path();
            fs::path includeDir = exeDir.parent_path() / "include";

            // Fallback for development environment (when running from bin/anvil inside the project)
            if (!fs::exists(includeDir / "anvil" / "driver.cpp")) {
                includeDir = rootDir / "src";
            }

            if (!fs::exists(userScript)) {
                fmt::print(stderr, "Error: build.cpp not found.\n");
                return 1;
            }

            try {
                fmt::print("[Anvil] Compiling Build Script...\n");

                // Check for compiler override via environment variable
                std::unique_ptr<Toolchain> toolchain;
                const char* env_compiler = std::getenv("ANVIL_SCRIPT_COMPILER");
                if (env_compiler && std::string(env_compiler) == "gcc") {
                     toolchain = std::make_unique<GCCToolchain>();
                } else {
                     toolchain = std::make_unique<ClangToolchain>();
                }

                ScriptCompiler compiler(includeDir, rootDir / ".anvil", std::move(toolchain));
                fs::path runner = compiler.compile(userScript);

                fmt::print("[Anvil] Loading...\n");
                exec(runner.string());
            } catch (const std::exception &e) {
                fmt::print(stderr, "[Anvil Error] {}\n", e.what());
                return 1;
            }

            return 0;
        }
    };
}
