#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace anvil {

    enum class CppStandard { CPP_11, CPP_14, CPP_17, CPP_20, CPP_23 };
    enum class Linkage { Static, Dynamic };
    enum class Optimization { Debug, Release };
    enum class CompilerId { Clang, GCC, MSVC };
    enum class AppType { Executable, Test };

    struct CppApplication {
        std::string name;
        AppType type = AppType::Executable;
        CppStandard standard = CppStandard::CPP_20;
        Linkage linkage = Linkage::Static;
        CompilerId compilerId = CompilerId::Clang;
        std::vector<std::string> sources;
        std::vector<std::string> include_dirs;
        std::vector<std::string> defines;
        std::vector<std::string> link_flags;

        void add_source(const std::string& src) { sources.push_back(src); }
        void add_include(const std::string& dir) { include_dirs.push_back(dir); }
        void add_define(const std::string& def) { defines.push_back(def); }
        void add_link_flag(const std::string& flag) { link_flags.push_back(flag); }
        void set_compiler(CompilerId id) { compilerId = id; }
    };

    class Project {
    public:
        std::string name;
        std::string version;
        std::vector<CppApplication> targets;

        // Legacy support for older versions of Anvil that might expect this member
        CppApplication application;

        void add_anvil_include(CppApplication& app) {
            if (std::filesystem::exists("src/anvil/test.hpp")) {
                // Self-hosting: use local src directory
                app.add_include("src");
            } else {
                // Consumer project: find headers in .anvil/wrapper
                std::string wrapperDir = ".anvil/wrapper";
                bool found = false;
                if (std::filesystem::exists(wrapperDir)) {
                    for (const auto& entry : std::filesystem::directory_iterator(wrapperDir)) {
                        if (entry.is_directory()) {
                            std::string includePath = (entry.path() / "include").string();
                            if (std::filesystem::exists(includePath + "/anvil/test.hpp")) {
                                app.add_include(includePath);
                                found = true;
                                break;
                            }
                        }
                    }
                }
                if (!found) {
                    // Fallback or warning? For now, just try adding src in case user has custom setup
                    // But don't add it blindly if it doesn't exist?
                    // The original code added "src" blindly.
                    // Let's add "src" if it exists, otherwise we might fail.
                    if (std::filesystem::exists("src")) {
                        app.add_include("src");
                    }
                }
            }
        }

        void add_executable(const std::string& name, const std::function<void(CppApplication&)> &config) {
            CppApplication app;
            app.name = name;
            app.type = AppType::Executable;
            app.standard = CppStandard::CPP_20;

            add_anvil_include(app);

            if (std::filesystem::exists("src/main/main.cpp")) {
                app.add_source("src/main/main.cpp");
            }

#ifdef _WIN32
            app.add_link_flag("-static");
#endif

            config(app);
            targets.push_back(app);

            // Keep legacy member in sync for now if needed, though targets is preferred
            if (targets.size() == 1) {
                application = app;
            }
        }

        void add_test(const std::string& name, std::function<void(CppApplication&)> config) {
            CppApplication app;
            app.name = name;
            app.type = AppType::Test;
            app.standard = CppStandard::CPP_20;

            add_anvil_include(app);

            // Check for user-provided runner (must be non-empty)
            if (std::filesystem::exists("src/test/test_runner.cpp") && std::filesystem::file_size("src/test/test_runner.cpp") > 0) {
                app.add_source("src/test/test_runner.cpp");
            } else {
                // Generate default runner
                std::string generatedDir = ".anvil/generated";
                if (!std::filesystem::exists(generatedDir)) {
                    std::filesystem::create_directories(generatedDir);
                }

                std::string generatedRunner = generatedDir + "/" + name + "_runner.cpp";
                std::ofstream runnerFile(generatedRunner);
                if (runnerFile.is_open()) {
                    runnerFile << "#define ANVIL_TEST_MAIN\n";
                    runnerFile << "#include \"anvil/test.hpp\"\n";
                    runnerFile.close();
                    app.add_source(generatedRunner);
                } else {
                    std::cerr << "Error: Could not create generated test runner at " << generatedRunner << std::endl;
                }
            }

            if (std::filesystem::exists("src/test")) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator("src/test")) {
                    if (entry.path().extension() == ".cpp") {
                        std::string path = entry.path().string();
                        if (path.find("test_runner.cpp") == std::string::npos) {
                             app.add_source(path);
                        }
                    }
                }
            }

            config(app);
            targets.push_back(app);
        }
    };

    class BuildScript {
    public:
        virtual ~BuildScript() = default;
        virtual void configure(Project& project) = 0;
    };
}

extern "C" void configure(anvil::Project& project);