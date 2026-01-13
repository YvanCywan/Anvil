#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace anvil {
    namespace fs = std::filesystem;

    class Toolchain {
    public:
        virtual ~Toolchain() = default;
        virtual std::string getCompiler() const = 0;
        virtual std::string getLinker() const = 0;
        virtual std::string getCompileCommand(const fs::path& source, const fs::path& output, const std::vector<std::string>& flags) const = 0;
    };

    class ClangToolchain : public Toolchain {
    public:
        std::string getCompiler() const override {
            return "clang++";
        }

        std::string getLinker() const override {
            return "clang++";
        }

        std::string getCompileCommand(const fs::path& source, const fs::path& output, const std::vector<std::string>& flags) const override {
            std::string cmd = getCompiler();
            for (const auto& flag : flags) {
                cmd += " " + flag;
            }
            cmd += " " + source.string();
            cmd += " -o " + output.string();
            return cmd;
        }
    };
}