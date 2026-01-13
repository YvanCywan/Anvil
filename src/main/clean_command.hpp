#pragma once
#include "cli.hpp"
#include <filesystem>
#include <iostream>
#include <fmt/core.h>

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
                fmt::print("[Anvil] Cleaned build artifacts.\n");
            }

            if (fs::exists(anvilDir)) {
                fs::remove_all(anvilDir);
                fmt::print("[Anvil] Cleaned configuration.\n");
            }

            return 0;
        }
    };
}
