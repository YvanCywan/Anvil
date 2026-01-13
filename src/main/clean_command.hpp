#pragma once
#include "cli.hpp"
#include "anvil/script_compiler.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace anvil {
    class CleanCommand : public Command {
    public:
        [[nodiscard]] std::string getName() const override {
            return "clean";
        }

        [[nodiscard]] std::string getDescription() const override {
            return "Cleans the build artefacts of the project";
        }

        int execute(const std::vector<std::string> &args) override {
            const fs::path rootDir = fs::current_path();

            if (fs::path userScript = rootDir / "build.cpp"; !fs::exists(userScript)) {
                std::cerr << "Error: build.cpp not found." << std::endl;
                return 1;
            }

            try {
                std::cout << "[Anvil] Cleaning Build Artefacts..." << std::endl;
                fs::remove_all(rootDir / ".anvil_build");
                fs::remove_all(rootDir / "bin");
                fs::remove(rootDir / ".d");
                fs::remove(rootDir / "build.ninja");

            } catch (const std::exception &e) {
                std::cerr << "[Anvil Error] " << e.what() << std::endl;
                return 1;
            }

            return 0;
        }
    };
}
