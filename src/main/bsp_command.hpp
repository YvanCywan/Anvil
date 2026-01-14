#pragma once
#include "cli.hpp"
#include "anvil/script_compiler.hpp"
#include "anvil/toolchain.hpp"
#include "process.hpp"
#include <filesystem>
#include <iostream>

namespace anvil {
    namespace fs = std::filesystem;

    class BspCommand : public Command {
    public:
        std::string getName() const override {
            return "bsp";
        }

        std::string getDescription() const override {
            return "Starts the Build Server Protocol server";
        }

        int execute(const std::vector<std::string> &args, const std::string &exePath) override {
            fs::path exeDir = fs::path(exePath).parent_path();
            fs::path anvilHome = exeDir.parent_path().parent_path(); // .anvil/wrapper/version -> .anvil
            fs::path projectRoot = anvilHome.parent_path();
            fs::path buildDir = projectRoot / ".anvil";
            fs::path userScript = projectRoot / "build.cpp";

            // Ensure we have the include directory
            fs::path includeDir = exeDir.parent_path() / "include";

            if (!fs::exists(userScript)) {
                std::cerr << "Error: build.cpp not found in " << projectRoot << std::endl;
                return 1;
            }

            try {
                std::cout << "[Anvil] Starting BSP Mode..." << std::endl;
                ScriptCompiler compiler(includeDir, buildDir);
                fs::path runnerExe = compiler.compile(userScript);

                std::string cmd = runnerExe.string() + " --bsp";

                // On Windows, we might need to quote the path if it contains spaces
                // But for now, let's assume simple paths or that exec handles it

                // We use system() here to hand over control, but exec() from process.hpp is also fine
                // Since we want to run the runner in BSP mode, we just execute it.
                return std::system(cmd.c_str());

            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
            }
        }
    };
}