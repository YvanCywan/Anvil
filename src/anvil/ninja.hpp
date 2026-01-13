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
            const auto& app = project.application;

            std::string compiler = "clang++";
            if (app.compilerId == CompilerId::GCC) {
                compiler = "g++";
            }

            std::cout << "[Anvil] Configured Toolchain: " << compiler << std::endl;

            out << "rule cxx\n";
            out << "  command = " << compiler << " $FLAGS $INCLUDES -c $in -o $out\n";
            out << "  description = CXX $out\n\n";

            out << "rule link\n";
            out << "  command = " << compiler << " $FLAGS $in -o $out\n";
            out << "  description = LINK $out\n\n";

            std::vector<std::string> object_files;

            std::string flags = "-MD -MF $out.d";
            if (app.standard == CppStandard::CPP_20) flags += " -std=c++20";
            if (app.standard == CppStandard::CPP_17) flags += " -std=c++17";
            
            std::string includes;
            for (const auto& inc : app.include_dirs) includes += " -I" + inc;

            for (const auto& src : app.sources) {
                std::string obj = "$builddir/" + src + ".o";
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

            out << "build " << binary << ": link";
            for (const auto& obj : object_files) out << " " << obj;
            out << "\n";
            
            out << "default " << binary << "\n";
        }
    };
}