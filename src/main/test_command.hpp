#pragma once
#include "cli.hpp"
#include "build_command.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace anvil {
    class TestCommand : public Command {
    public:
        [[nodiscard]] std::string getName() const override {
            return "test";
        }

        [[nodiscard]] std::string getDescription() const override {
            return "Builds and runs tests";
        }

        int execute(const std::vector<std::string> &args, const std::string &exePath) override {
            fs::path rootDir = fs::current_path();
            fs::path userScript = rootDir / "build.cpp";

            fs::path exeDir = fs::absolute(exePath).parent_path();
            fs::path includeDir = exeDir.parent_path() / "include";

            if (!fs::exists(includeDir / "anvil" / "driver.cpp")) {
                includeDir = rootDir / "src";
            }

            if (!fs::exists(userScript)) {
                std::cerr << "Error: build.cpp not found." << std::endl;
                return 1;
            }

            try {
                std::cout << "[Anvil] Compiling Build Script..." << std::endl;

                std::unique_ptr<Toolchain> toolchain;
                const char* env_compiler = std::getenv("ANVIL_SCRIPT_COMPILER");
                if (env_compiler && std::string(env_compiler) == "gcc") {
                     toolchain = std::make_unique<GCCToolchain>();
                } else {
                     toolchain = std::make_unique<ClangToolchain>();
                }

                ScriptCompiler compiler(includeDir, rootDir / ".anvil", std::move(toolchain));
                fs::path runner = compiler.compile(userScript);

                std::cout << "[Anvil] Loading..." << std::endl;

                std::string cmd = runner.string() + " --test";
                for (const auto& arg : args) {
                    cmd += " " + arg;
                }

                return std::system(cmd.c_str());
            } catch (const std::exception &e) {
                std::cerr << "[Anvil Error] " << e.what() << std::endl;
                return 1;
            }
        }
    };
}