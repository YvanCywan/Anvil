#pragma once
#include "cli.hpp"
#include "anvil/script_compiler.hpp"
#include "anvil/toolchain.hpp"
#include "process.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

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

            // Resolve include directory with fallback for development environment
            fs::path includeDir = absExePath.parent_path().parent_path() / "include";
            if (!fs::exists(includeDir / "anvil" / "driver.cpp")) {
                includeDir = projectRoot / "src";
            }

            // 3. Implement Auto-Repair
            if (!fs::exists(userScript)) {
                std::cout << "[Anvil] Repairing: 'build.cpp' not found..." << std::endl;
                std::ofstream ofs(userScript);
                if (ofs) {
                    ofs << "#include \"src/anvil/api.hpp\"\n\n";
                    ofs << "void configure(anvil::Project& project) {\n";
                    ofs << "    project.name = \"New Project\";\n";
                    ofs << "    project.add_executable(\"app\", [](anvil::CppApplication& app) {\n";
                    ofs << "        app.add_source(\"src/main.cpp\");\n";
                    ofs << "    });\n";
                    ofs << "}\n";
                    ofs.close();
                    std::cout << "[Anvil] Created default build.cpp at " << userScript << std::endl;
                } else {
                    std::cerr << "Error: Failed to create build.cpp at " << userScript << std::endl;
                    return 1;
                }
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