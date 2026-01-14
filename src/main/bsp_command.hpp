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
            // 1. Resolve absolute path to the executable
            std::error_code ec;
            fs::path absExePath = fs::canonical(exePath, ec);
            if (ec) {
                // Fallback if canonical fails (e.g. file doesn't exist, though it should since it's running)
                absExePath = fs::absolute(exePath);
            }

            fs::path currentDir = absExePath.parent_path();
            fs::path projectRoot;
            bool foundAnvil = false;

            // 2. Walk up the tree to find the .anvil directory
            while (true) {
                if (fs::exists(currentDir / ".anvil") && fs::is_directory(currentDir / ".anvil")) {
                    projectRoot = currentDir;
                    foundAnvil = true;
                    break;
                }

                if (currentDir == currentDir.parent_path()) {
                    // Reached root
                    break;
                }
                currentDir = currentDir.parent_path();
            }

            if (!foundAnvil) {
                std::cerr << "Error: Could not find .anvil directory in any parent of " << absExePath << std::endl;
                return 1;
            }

            fs::path buildDir = projectRoot / ".anvil";
            fs::path userScript = projectRoot / "build.cpp";

            // Assuming 'include' is relative to the executable location as per original logic,
            // but we should probably make this robust too.
            // Original: exeDir.parent_path() / "include"
            // If the structure is fixed inside the wrapper (bin/anvil, include/...), then:
            fs::path includeDir = absExePath.parent_path().parent_path() / "include";

            if (!fs::exists(userScript)) {
                std::cerr << "Error: build.cpp not found in " << projectRoot << std::endl;
                return 1;
            }

            try {
                std::cout << "[Anvil] Starting BSP Mode..." << std::endl;
                ScriptCompiler compiler(includeDir, buildDir);
                fs::path runnerExe = compiler.compile(userScript);

                std::string cmd = runnerExe.string() + " --bsp";

                return std::system(cmd.c_str());

            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
            }
        }
    };
}