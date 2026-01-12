#include <iostream>
#include <string>
#include <filesystem>
#ifdef _WIN32
    #include <process.h> // Windows equivalent of unistd.h
#else
    #include <unistd.h>  // Linux/Mac standard
#endif
#include "process.hpp"
#include "anvil/script_compiler.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) != "build") {
        std::cout << "Usage: anvil build" << std::endl;
        return 0;
    }

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

    } catch (const std::exception& e) {
        std::cerr << "[Anvil Error] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}