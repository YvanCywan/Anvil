#include <iostream>
#include <string>
#include <vector>
#include "cli.hpp"
#include "build_command.hpp"
#include "clean_command.hpp"
#include "run_command.hpp"
#include "test_command.hpp"

int main(int argc, char *argv[]) {
    std::cout << "[Anvil] Starting..." << std::endl;
    anvil::CommandRegistry registry;
    registry.registerCommand(std::make_unique<anvil::BuildCommand>());
    registry.registerCommand(std::make_unique<anvil::CleanCommand>());
    registry.registerCommand(std::make_unique<anvil::RunCommand>());
    registry.registerCommand(std::make_unique<anvil::TestCommand>());


    if (argc < 2) {
        registry.printHelp();
        return 0;
    }

    const std::string commandName = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return registry.execute(commandName, args, argv[0]);
}
