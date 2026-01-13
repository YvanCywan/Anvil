#pragma once
#include "cli.hpp"
#include "build_command.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace anvil {
    class RunCommand : public Command {
    public:
        [[nodiscard]] std::string getName() const override {
            return "run";
        }

        [[nodiscard]] std::string getDescription() const override {
            return "Builds and runs the application";
        }

        int execute(const std::vector<std::string> &args, const std::string &exePath) override {
            // Reuse the build command logic but pass a flag to the driver
            // Since we can't easily modify the BuildCommand to accept flags without changing its signature,
            // we will need to modify how arguments are passed to the script compiler or driver.

            // However, the driver is what executes ninja and then could execute the binary.
            // The driver is compiled from src/anvil/driver.cpp.
            // We need to pass a flag to the driver to tell it to run the output.

            // Let's instantiate a BuildCommand and run it, but we need to signal it to run.
            // The current BuildCommand implementation doesn't support passing args to the driver easily.
            // We should probably refactor BuildCommand or subclass it.

            // For now, let's copy the logic from BuildCommand but add the --run flag to the execution of the runner.

            fs::path rootDir = fs::current_path();
            fs::path userScript = rootDir / "build.cpp";

            // Derive include path from executable location
            fs::path exeDir = fs::absolute(exePath).parent_path();
            fs::path includeDir = exeDir.parent_path() / "include";

            // Fallback for development environment
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

                std::string cmd = runner.string() + " --run";
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