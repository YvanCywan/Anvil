#include "api.hpp"
#include "ninja.hpp"
#include "dependency_manager.hpp"
#include "pkg.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <map>

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

extern "C" void configure(anvil::Project& project);

namespace fs = std::filesystem;

// Helper to read file content
std::string read_file_content(const fs::path& path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return "";
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Helper to escape string for C++ source
std::string escape_string(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 1.1);
    for (char c : input) {
        if (c == '\\') output += "\\\\";
        else if (c == '"') output += "\\\"";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else output += c;
    }
    return output;
}

void generate_embedded_resources(const fs::path& libDir) {
    fs::path outputHeader = fs::current_path() / "src" / "anvil" / "embedded_resources.hpp";
    std::cerr << "[Anvil] Generating embedded resources to " << outputHeader << "..." << std::endl;

    std::ofstream out(outputHeader);
    out << "#pragma once\n";
    out << "#include <string_view>\n";
    out << "#include <map>\n";
    out << "#include <string>\n\n";
    out << "namespace anvil {\n";

    std::map<std::string, fs::path> files_to_embed;

    // Find json.hpp in libDir (Conan output)
    bool jsonFound = false;
    if (fs::exists(libDir / "full_deploy")) {
        std::vector<fs::path> candidates;
        for (const auto& entry : fs::recursive_directory_iterator(libDir / "full_deploy")) {
            if (entry.is_regular_file() && entry.path().filename() == "json.hpp") {
                candidates.push_back(entry.path());
            }
        }

        fs::path bestPath;
        for (const auto& p : candidates) {
            // Prefer path containing "include"
            if (p.string().find("include") != std::string::npos) {
                bestPath = p;
                break;
            }
        }
        if (bestPath.empty() && !candidates.empty()) {
            bestPath = candidates[0];
        }

        if (!bestPath.empty()) {
            fs::path parent = bestPath.parent_path();
            if (parent.filename() == "nlohmann") {
                // Found inside nlohmann directory - assume multi-header or structured single-header
                // Embed the entire nlohmann directory
                for (const auto& subEntry : fs::recursive_directory_iterator(parent)) {
                    if (subEntry.is_regular_file()) {
                        fs::path relPath = fs::relative(subEntry.path(), parent);
                        files_to_embed["nlohmann/" + relPath.generic_string()] = subEntry.path();
                    }
                }
            } else {
                // Found standalone json.hpp
                files_to_embed["nlohmann/json.hpp"] = bestPath;
            }
            jsonFound = true;
        }
    }

    if (!jsonFound) {
        std::cerr << "[Warning] json.hpp not found in " << libDir << std::endl;
    }

    // Embed source files
    fs::path srcDir = fs::current_path() / "src" / "anvil";
    if (fs::exists(srcDir)) {
        for (const auto& entry : fs::directory_iterator(srcDir)) {
            if (entry.path().extension() == ".hpp" || entry.path().extension() == ".cpp") {
                if (entry.path().filename() != "embedded_resources.hpp") {
                    files_to_embed["anvil/" + entry.path().filename().string()] = entry.path();
                }
            }
        }
    }

    // Write file contents as variables
    int counter = 0;
    for (const auto& [name, path] : files_to_embed) {
        std::string content = read_file_content(path);
        out << "inline const char file_" << counter << "[] = \"" << escape_string(content) << "\";\n";
        counter++;
    }

    out << "\ninline std::map<std::string, std::string_view> get_embedded_files() {\n";
    out << "    return {\n";

    counter = 0;
    for (const auto& [name, path] : files_to_embed) {
        out << "        {\"" << name << "\", file_" << counter << "},\n";
        counter++;
    }

    out << "    };\n";
    out << "}\n";
    out << "}\n";
}

// BSP Loop
int run_bsp_loop(const anvil::Project& project) {
#if __has_include(<nlohmann/json.hpp>)
    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) continue;

        if (line.starts_with("Content-Length: ")) {
            int length = std::stoi(line.substr(16));
            std::getline(std::cin, line); // Skip empty line

            std::vector<char> buffer(length);
            std::cin.read(buffer.data(), length);
            std::string content(buffer.begin(), buffer.end());

            try {
                json request = json::parse(content);
                std::string method = request["method"];
                json response;
                response["jsonrpc"] = "2.0";
                response["id"] = request["id"];

                if (method == "build/initialize") {
                    response["result"] = {
                        {"displayName", "Anvil"},
                        {"version", "0.1.0"},
                        {"bspVersion", "2.0.0"},
                        {"capabilities", {
                            {"compileProvider", {
                                {"languageIds", {"cpp"}}
                            }}
                        }}
                    };
                } else if (method == "workspace/buildTargets") {
                    json targets = json::array();
                    for (const auto& target : project.targets) {
                        targets.push_back({
                            {"id", {{"uri", "target:" + target.name}}},
                            {"displayName", target.name},
                            {"baseDirectory", fs::current_path().string()},
                            {"tags", {}},
                            {"languageIds", {"cpp"}},
                            {"dependencies", {}},
                            {"capabilities", {
                                {"canCompile", true},
                                {"canTest", target.type == anvil::AppType::Test},
                                {"canRun", target.type == anvil::AppType::Executable}
                            }}
                        });
                    }
                    response["result"] = {{"targets", targets}};
                } else if (method == "buildTarget/sources") {
                    json items = json::array();
                    for (const auto& target : project.targets) {
                        json sources = json::array();
                        for (const auto& src : target.sources) {
                            sources.push_back({
                                {"uri", "file://" + (fs::current_path() / src).string()},
                                {"kind", 1},
                                {"generated", false}
                            });
                        }
                        items.push_back({
                            {"target", {{"uri", "target:" + target.name}}},
                            {"sources", sources}
                        });
                    }
                    response["result"] = {{"items", items}};
                } else if (method == "buildTarget/cppOptions") {
                    json items = json::array();
                    for (const auto& target : project.targets) {
                        std::vector<std::string> copts = {"-std=c++20"};
                        for (const auto& inc : target.include_dirs) {
                            copts.push_back("-I" + (fs::current_path() / inc).string());
                        }
                        // Add defines if any (currently not exposed in Project/Target but structure is there)

                        items.push_back({
                            {"target", {{"uri", "target:" + target.name}}},
                            {"copts", copts},
                            {"defines", json::array()},
                            {"linkopts", target.link_flags}
                        });
                    }
                    response["result"] = {{"items", items}};
                } else if (method == "build/shutdown") {
                    response["result"] = nullptr;
                } else if (method == "build/exit") {
                    return 0;
                } else {
                    // Ignore unknown methods for now
                    continue;
                }

                std::string responseStr = response.dump();
                std::cout << "Content-Length: " << responseStr.length() << "\r\n\r\n" << responseStr << std::flush;

            } catch (const std::exception& e) {
                std::cerr << "[BSP Error] " << e.what() << std::endl;
            }
        }
    }
    return 0;
#else
    std::cerr << "[Anvil Error] BSP mode requires nlohmann/json.hpp (not found during compilation)." << std::endl;
    return 1;
#endif
}

int main(int argc, char* argv[]) {
    bool runAfterBuild = false;
    bool runTests = false;
    bool runBsp = false;
    std::vector<std::string> runArgs;

    // Simple argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--run") {
            runAfterBuild = true;
        } else if (arg == "--test") {
            runTests = true;
        } else if (arg == "--bsp") {
            runBsp = true;
        } else if (runAfterBuild) {
            // Collect arguments for the target application
            runArgs.push_back(arg);
        }
    }

    anvil::Project project;
    
    configure(project);

    // Handle legacy mode where targets might be empty but application is set
    if (project.targets.empty() && !project.application.name.empty()) {
        project.targets.push_back(project.application);
    }

    fs::path rootDir = fs::current_path();
    std::cerr << "[Anvil] Graph Loaded: " << project.name << std::endl;
    std::cerr << "[Anvil] Working Directory: " << rootDir << std::endl;

    // --- NEW: Resolve Dependencies ---
    try {
        anvil::PackageManager pkgMgr(rootDir / ".anvil" / "libraries");
        pkgMgr.resolve(project);

        // --- NEW: Generate Embedded Resources (Bootstrap) ---
        generate_embedded_resources(rootDir / ".anvil" / "libraries");
        // ----------------------------------------------------

    } catch (const std::exception& e) {
        return 1;
    }
    // ---------------------------------

    if (runBsp) {
        return run_bsp_loop(project);
    }

    // Verify sources exist for all targets
    bool missingSources = false;
    for (const auto& target : project.targets) {
        for (const auto& src : target.sources) {
            fs::path srcPath = rootDir / src;
            if (!fs::exists(srcPath)) {
                std::cerr << "[Anvil Error] Source file not found: " << srcPath << std::endl;

                fs::path parent = srcPath.parent_path();
                if (fs::exists(parent)) {
                    std::cerr << "Contents of " << parent << ":" << std::endl;
                    for (const auto& entry : fs::directory_iterator(parent)) {
                        std::cerr << "  - " << entry.path().filename() << std::endl;
                    }
                } else {
                    std::cerr << "Parent directory " << parent << " does not exist." << std::endl;

                    // Check if src directory exists at all
                    fs::path srcDir = rootDir / "src";
                    if (fs::exists(srcDir)) {
                         std::cerr << "Contents of " << srcDir << ":" << std::endl;
                         for (const auto& entry : fs::directory_iterator(srcDir)) {
                            std::cerr << "  - " << entry.path().filename() << std::endl;
                         }
                    }
                }
                missingSources = true;
            }
        }
    }

    if (missingSources) {
        return 1;
    }

    anvil::DependencyManager deps(rootDir / ".anvil" / "tools");

    try {
        fs::path ninjaExe = deps.get_ninja();

        {
            anvil::NinjaWriter writer("build.ninja");
            writer.generate(project);
        }

        std::cerr << "[Anvil] Executing Ninja..." << std::endl;
        std::string cmd = ninjaExe.string();
        int buildResult = std::system(cmd.c_str());

        if (buildResult != 0) {
            return buildResult;
        }

        if (runTests) {
             bool allTestsPassed = true;
             bool testsFound = false;
             for (const auto& target : project.targets) {
                if (target.type == anvil::AppType::Test) {
                    testsFound = true;
                    fs::path binPath = rootDir / "bin" / target.name;
#ifdef _WIN32
                    binPath += ".exe";
#endif
                    if (fs::exists(binPath)) {
                        std::cerr << "[Anvil] Running Test: " << target.name << "..." << std::endl;
                        int result = std::system(binPath.string().c_str());
                        if (result != 0) {
                            std::cerr << "[Anvil] Test " << target.name << " failed." << std::endl;
                            allTestsPassed = false;
                        } else {
                            std::cerr << "[Anvil] Test " << target.name << " passed." << std::endl;
                        }
                    } else {
                        std::cerr << "[Anvil Error] Test executable not found: " << binPath << std::endl;
                        allTestsPassed = false;
                    }
                }
             }

             if (!testsFound) {
                 std::cerr << "[Anvil] No tests found." << std::endl;
             }

             if (!allTestsPassed) return 1;
        }

        if (runAfterBuild) {
            // Find the first executable target to run
            const anvil::CppApplication* targetToRun = nullptr;
            for (const auto& target : project.targets) {
                if (target.type == anvil::AppType::Executable) {
                    targetToRun = &target;
                    break;
                }
            }

            if (targetToRun) {
                fs::path binPath = rootDir / "bin" / targetToRun->name;
#ifdef _WIN32
                binPath += ".exe";
#endif
                if (fs::exists(binPath)) {
                    std::cerr << "[Anvil] Running " << targetToRun->name << "..." << std::endl;
                    std::string runCmd = binPath.string();
                    for (const auto& arg : runArgs) {
                        runCmd += " " + arg;
                    }
                    return std::system(runCmd.c_str());
                } else {
                    std::cerr << "[Anvil Error] Executable not found: " << binPath << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "[Anvil] No executable target found to run." << std::endl;
            }
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[Anvil Error] " << e.what() << std::endl;
        return 1;
    }
}