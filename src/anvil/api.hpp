#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace anvil {

    enum class CppStandard { CPP_11, CPP_14, CPP_17, CPP_20, CPP_23 };
    enum class Linkage { Static, Dynamic };
    enum class Optimization { Debug, Release };
    enum class CompilerId { Clang, GCC, MSVC };

    struct CppApplication {
        std::string name;
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
        CppApplication application;

        void add_executable(const std::string& name, std::function<void(CppApplication&)> config);
    };

    class BuildScript {
    public:
        virtual ~BuildScript() = default;
        virtual void configure(Project& project) = 0;
    };
}

extern "C" void configure(anvil::Project& project);