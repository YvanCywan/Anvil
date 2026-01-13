#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "api.hpp"

namespace anvil {

    class NinjaWriter {
        std::ofstream out;
    public:
        explicit NinjaWriter(const std::string& path) : out(path) {
            out << "ninja_required_version = 1.3\n";
            out << "builddir = .anvil_build\n\n";
        }

        void generate(const Project& project) {
            // Handle legacy single-application projects
            if (project.targets.empty() && !project.application.name.empty()) {
                // If targets is empty but application is set (legacy mode), wrap it
                std::vector<CppApplication> legacyTargets = { project.application };
                generateTargets(legacyTargets);
            } else {
                generateTargets(project.targets);
            }
        }

    private:
        void generateTargets(const std::vector<CppApplication>& targets) {
            if (targets.empty()) return;

            // Use the first target to determine the compiler for the rules
            const auto& firstApp = targets[0];
            std::string compiler = "clang++";
            if (firstApp.compilerId == CompilerId::GCC) {
                compiler = "g++";
            }

            std::cout << "[Anvil] Configured Toolchain: " << compiler << std::endl;

            out << "rule cxx\n";
            out << "  command = " << compiler << " $FLAGS $INCLUDES -c $in -o $out\n";
            out << "  description = CXX $out\n\n";

            out << "rule link\n";
            out << "  command = " << compiler << " $FLAGS $in -o $out $LINK_FLAGS\n";
            out << "  description = LINK $out\n\n";

            std::vector<std::string> all_binaries;

            for (const auto& app : targets) {
                std::vector<std::string> object_files;

                std::string flags = "-MD -MF $out.d";
                if (app.standard == CppStandard::CPP_20) flags += " -std=c++20";
                if (app.standard == CppStandard::CPP_17) flags += " -std=c++17";

                std::string includes;
                for (const auto& inc : app.include_dirs) includes += " -I" + inc;

                for (const auto& src : app.sources) {
                    // Unique object file path per target to avoid collisions if same source is used
                    std::string obj = "$builddir/" + app.name + "/" + src + ".o";
                    object_files.push_back(obj);

                    out << "build " << obj << ": cxx " << src << "\n";
                    out << "  FLAGS = " << flags << "\n";
                    out << "  INCLUDES = " << includes << "\n";
                    out << "  depfile = " << obj << ".d\n\n";
                }

                std::string binary = "bin/" + app.name;
                #ifdef _WIN32
                    binary += ".exe";
                #endif

                std::string link_flags;
                for (const auto& flag : app.link_flags) link_flags += " " + flag;

                out << "build " << binary << ": link";
                for (const auto& obj : object_files) out << " " << obj;
                out << "\n";
                out << "  LINK_FLAGS = " << link_flags << "\n";

                all_binaries.push_back(binary);
            }

            if (!all_binaries.empty()) {
                out << "default";
                for (const auto& bin : all_binaries) {
                    out << " " << bin;
                }
                out << "\n";
            }
        }
    };
}