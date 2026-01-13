#pragma once
#include "cli.hpp"
#include "anvil/script_compiler.hpp"
#include "process.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace anvil {
    class BuildCommand : public Command {
    public:
        std::string getName() const override {
            return "build";
        }

        std::string getDescription() const override {
            return "Compiles and runs the build script";
        }

        int execute(const std::vector<std::string> &args) override {
            fs::path rootDir = fs::current_path();
            fs::path userScript = rootDir / "build.cpp";
            fs::path srcDir = rootDir / "src";

            if (!fs::exists(userScript)) {
                std::cerr << "Error: build.cpp not found." << std::endl;
                return 1;
            }

            try {
                std::cout << "[Anvil] Compiling Build Script..." << std::endl;

                anvil::ScriptCompiler compiler(srcDir, rootDir / ".anvil");
                fs::path runner = compiler.compile(userScript);

                std::cout << "[Anvil] Loading..." << std::endl;
                anvil::exec(runner.string());
            } catch (const std::exception &e) {
                std::cerr << "[Anvil Error] " << e.what() << std::endl;
                return 1;
            }

            return 0;
        }
    };
}
