#include "api.hpp"
#include "ninja.hpp"
#include <iostream>

extern "C" void configure(anvil::Project& project);

int main(int argc, char* argv[]) {
    anvil::Project project;
    
    configure(project);

    std::cout << "[Anvil] Graph Loaded: " << project.name << std::endl;
    
    anvil::NinjaWriter writer("build.ninja");
    writer.generate(project);

    std::cout << "[Anvil] Executing Ninja..." << std::endl;
    return std::system("ninja");
}