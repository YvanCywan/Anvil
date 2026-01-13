#pragma once
#include "cli.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace anvil {
    class CleanCommand : public Command {
    public:
        [[nodiscard]] std::string getName() const override {
            return "clean";
        }

        [[nodiscard]] std::string getDescription() const override {
            return "Removes build artifacts";
        }

        int execute(const std::vector<std::string> &args, const std::string &exePath) override {
            const fs::path rootDir = fs::current_path();
            const fs::path buildDir = rootDir / ".anvil_build";
            const fs::path anvilDir = rootDir / ".anvil";

            if (fs::exists(buildDir)) {
                fs::remove_all(buildDir);
                std::cout << "[Anvil] Cleaned build artifacts." << std::endl;
            }

            if (fs::exists(anvilDir)) {
                fs::remove_all(anvilDir);
                std::cout << "[Anvil] Cleaned configuration." << std::endl;
            }

            return 0;
        }
    };
}
